// Signal Resampler
// Dan Jackson

// To calculate coefficients:
//   gcc -DRESAMPLER_CALCULATE_COEFFICIENTS -DRESAMPLER_TEST butter.c wav.c resampler.c -lm -o resampler && ./resampler chirp.wav
// ...otherwise:
//   gcc -DRESAMPLER_TEST wav.c resampler.c -o resampler && ./resampler chirp.wav

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "resampler.h"


// Zero data for upsample
static const resampler_data_t zeroData[RESAMPLER_MAX_AXES] = {0};  // ideally FILTER_FROM_INPUT(0), but still zero.

// Greatest common divisor
static int gcd(int u, int v) {
	int shift = 0;
	if (u == 0 && v == 0) return 0;
	if (u == 0) return v;
	if (v == 0) return u;
	while (!((u&1) || (v&1))) { shift++; u >>= 1; v >>= 1; }
	while (!(u&1)) u >>= 1;
	do {
		while (!(v&1)) v >>= 1;
		if (u > v) { int t = u; u = v; v = t; }
	} while ((v -= u) != 0);
	return u << shift;
}


// Apply the filter, specified by the coefficients b & a, to count elements of data X, returning in data Y (can be same as X), where z[] tracks the final/initial conditions.
static void resampler_filter(int numCoefficients, const filter_data_t *b, const filter_data_t *a, const filter_data_t *X, filter_data_t *Y, int count, filter_data_t *z) {
	if (numCoefficients > 0) {
		int m, i;
		z[numCoefficients - 1] = 0;
		for (m = 0; m < count; m++) {
			filter_data_t oldXm = X[m];
			filter_data_t newXm = FILTER_MULTIPLY(b[0], oldXm) + z[0];
			for (i = 1; i < numCoefficients; i++)
			{
				z[i - 1] = FILTER_MULTIPLY(b[i], oldXm) + z[i] - FILTER_MULTIPLY(a[i], newXm);
			}
			Y[m] = newXm;
		}
	}
	else if (X != Y) 
	{
		// Pass-through, and destination is not the source
		memmove(Y, X, sizeof(filter_data_t) * count);
	}
	return;
}


// Initialize the resampler state
bool resampler_init(resampler_t *resampler, int inFrequency, int outFrequency, int axes) {
	memset(resampler, 0, sizeof(*resampler));
	
	// Rational resampler: outFrequency = inFrequency * (P/Q)
	resampler->inFrequency = inFrequency;	// Q
	resampler->outFrequency = outFrequency;	// P
	resampler->axes = axes;

	// Simplify rational fraction (if possible)
	int divisor = gcd(resampler->inFrequency, resampler->outFrequency);
	resampler->upSample = resampler->outFrequency / divisor;		// Upsample: 1:P
	resampler->lowPass = ((resampler->outFrequency < resampler->inFrequency) ? resampler->outFrequency : resampler->inFrequency) / 2;		// Low-pass filter
	resampler->downSample = resampler->inFrequency / divisor;		// Downsample: Q:1

	resampler->intermediateFrequency = resampler->inFrequency * resampler->upSample;

	resampler->numCoefficients = 0;
	
	// Start at end of upsample/downsample cycles
	resampler->upPos = resampler->upSample;
	resampler->downPos = resampler->downSample;

//	printf("DEBUG: %d -> %d Hz (upsample 1:%d; low-pass %d Hz; downsample %d:1)\n", resampler->inFrequency, resampler->outFrequency, resampler->upSample, resampler->lowPass, resampler->downSample);

	// Filter coefficients
#ifdef RESAMPLER_CALCULATE_COEFFICIENTS
	{
		// Filter parameters
		const int order = 4;
		double Fc1 = -1;								// Low-pass
		double Fc2 = resampler->lowPass;				// Cut-off frequency is half the lowest sample rate
		double Fs = resampler->intermediateFrequency;	// Intermediate signal frequency

		// Calculate normalized cut-offs
		double W1 = Fc1 > 0 ? (Fc1 / (Fs / 2)) : Fc1;
		double W2 = Fc2 > 0 ? (Fc2 / (Fs / 2)) : Fc2;

		// Calculate coefficients
		double B[BUTTERWORTH_MAX_COEFFICIENTS(BUTTERWORTH_MAX_ORDER)];
		double A[BUTTERWORTH_MAX_COEFFICIENTS(BUTTERWORTH_MAX_ORDER)];
		resampler->numCoefficients = CoefficientsButterworth(order, W1, W2, B, A);
		for (int j = 0; j < resampler->numCoefficients; j++) {
			resampler->A[j] = FILTER_FROM_FLOAT(A[j]);
			resampler->B[j] = FILTER_FROM_FLOAT(B[j]);

			#if defined(FILTER_SIMULATE_FIXED) && defined(RESAMPLER_FILTER_DOUBLE)
				// Simulate a fixed-point precision in coefficients
				const double scale = (1 << FILTER_SIMULATE_FIXED);
				resampler->B[j] = (int32_t)(B[j] * scale + (B[j] < 0 ? -0.5 : 0.5)) / scale;
				resampler->A[j] = (int32_t)(A[j] * scale + (A[j] < 0 ? -0.5 : 0.5)) / scale;
				printf("WARNING: Simulating fixed point precision in coefficients: B[%d] = %.15f;  // Float: %.15f\n", j, resampler->B[j], B[j]);
				printf("WARNING: Simulating fixed point precision in coefficients: A[%d] = %.15f;  // Float: %.15f\n", j, resampler->A[j], A[j]);
			#endif
		}

#if 1	// Output for adding a an in-built conversion
		printf("\t// %d -> %d Hz (upsample 1:%d; low-pass %d Hz; downsample %d:1)\n", resampler->inFrequency, resampler->outFrequency, resampler->upSample, resampler->lowPass, resampler->downSample);
		printf("\tif (resampler->intermediateFrequency == %d * %d /*=%d*/ && resampler->lowPass == %d / 2 /*=%d*/) {\n", resampler->upSample, resampler->inFrequency, resampler->intermediateFrequency, ((resampler->outFrequency < resampler->inFrequency) ? resampler->outFrequency : resampler->inFrequency), resampler->lowPass);
		printf("\t\tresampler->numCoefficients = %d;\n", resampler->numCoefficients);
		for (int j = 0; j < resampler->numCoefficients; j++) {
			printf("\t\tresampler->B[%d] = FILTER_FROM_FLOAT(%+.15f);", j, B[j]);
			#ifndef RESAMPLER_FILTER_DOUBLE
				printf(" // Fixed: %+.15f", FILTER_TO_FLOAT(resampler->B[j]));
			#endif
			printf("\n");
		}
		for (int j = 0; j < resampler->numCoefficients; j++) {
			printf("\t\tresampler->A[%d] = FILTER_FROM_FLOAT(%+.15f);", j, A[j]);
			#ifndef RESAMPLER_FILTER_DOUBLE
				printf(" // Fixed: %+.15f", FILTER_TO_FLOAT(resampler->A[j]));
			#endif
			printf("\n");
		}
		printf("\t}\n");
#endif

	}
#else

	// 50 -> 30 Hz (upsample 1:3; low-pass 15 Hz; downsample 5:1)
	if (resampler->intermediateFrequency == 3 * 50 /*=150*/ && resampler->lowPass == 30 / 2 /*=15*/) {
		resampler->numCoefficients = 5;
		resampler->B[0] = FILTER_FROM_FLOAT(+0.004824343357716); // Fixed: +0.004882812500000
		resampler->B[1] = FILTER_FROM_FLOAT(+0.019297373430865); // Fixed: +0.019287109375000
		resampler->B[2] = FILTER_FROM_FLOAT(+0.028946060146297); // Fixed: +0.028930664062500
		resampler->B[3] = FILTER_FROM_FLOAT(+0.019297373430865); // Fixed: +0.019287109375000
		resampler->B[4] = FILTER_FROM_FLOAT(+0.004824343357716); // Fixed: +0.004882812500000
		resampler->A[0] = FILTER_FROM_FLOAT(+1.000000000000000); // Fixed: +1.000000000000000
		resampler->A[1] = FILTER_FROM_FLOAT(-2.369513007182038); // Fixed: -2.369506835937500
		resampler->A[2] = FILTER_FROM_FLOAT(+2.313988414415880); // Fixed: +2.313964843750000
		resampler->A[3] = FILTER_FROM_FLOAT(-1.054665405878568); // Fixed: -1.054687500000000
		resampler->A[4] = FILTER_FROM_FLOAT(+0.187379492368185); // Fixed: +0.187377929687500
	}

	// 100 -> 30 Hz (upsample 1:3; low-pass 15 Hz; downsample 10:1)
	if (resampler->intermediateFrequency == 3 * 100 /*=300*/ && resampler->lowPass == 30 / 2 /*=15*/) {
		resampler->numCoefficients = 5;
		resampler->B[0] = FILTER_FROM_FLOAT(+0.000416599204407); // Fixed: +0.000366210937500
		resampler->B[1] = FILTER_FROM_FLOAT(+0.001666396817626); // Fixed: +0.001708984375000
		resampler->B[2] = FILTER_FROM_FLOAT(+0.002499595226440); // Fixed: +0.002441406250000
		resampler->B[3] = FILTER_FROM_FLOAT(+0.001666396817626); // Fixed: +0.001708984375000
		resampler->B[4] = FILTER_FROM_FLOAT(+0.000416599204407); // Fixed: +0.000366210937500
		resampler->A[0] = FILTER_FROM_FLOAT(+1.000000000000000); // Fixed: +1.000000000000000
		resampler->A[1] = FILTER_FROM_FLOAT(-3.180638548874719); // Fixed: -3.180664062500000
		resampler->A[2] = FILTER_FROM_FLOAT(+3.861194348994213); // Fixed: +3.861206054687500
		resampler->A[3] = FILTER_FROM_FLOAT(-2.112155355110968); // Fixed: -2.112182617187500
		resampler->A[4] = FILTER_FROM_FLOAT(+0.438265142261980); // Fixed: +0.438232421875000
	}

	// 50 -> 40 Hz (upsample 1:4; low-pass 20 Hz; downsample 5:1)
	if (resampler->intermediateFrequency == 4 * 50 /*=200*/ && resampler->lowPass == 40 / 2 /*=20*/) {
		resampler->numCoefficients = 5;
		resampler->B[0] = FILTER_FROM_FLOAT(+0.004824343357716); // Fixed: +0.004882812500000
		resampler->B[1] = FILTER_FROM_FLOAT(+0.019297373430865); // Fixed: +0.019287109375000
		resampler->B[2] = FILTER_FROM_FLOAT(+0.028946060146297); // Fixed: +0.028930664062500
		resampler->B[3] = FILTER_FROM_FLOAT(+0.019297373430865); // Fixed: +0.019287109375000
		resampler->B[4] = FILTER_FROM_FLOAT(+0.004824343357716); // Fixed: +0.004882812500000
		resampler->A[0] = FILTER_FROM_FLOAT(+1.000000000000000); // Fixed: +1.000000000000000
		resampler->A[1] = FILTER_FROM_FLOAT(-2.369513007182038); // Fixed: -2.369506835937500
		resampler->A[2] = FILTER_FROM_FLOAT(+2.313988414415880); // Fixed: +2.313964843750000
		resampler->A[3] = FILTER_FROM_FLOAT(-1.054665405878568); // Fixed: -1.054687500000000
		resampler->A[4] = FILTER_FROM_FLOAT(+0.187379492368185); // Fixed: +0.187377929687500
	}
	
	// 100 -> 40 Hz (upsample 1:2; low-pass 20 Hz; downsample 5:1)
	if (resampler->intermediateFrequency == 2 * 100 /*=200*/ && resampler->lowPass == 40 / 2 /*=20*/) {
		resampler->numCoefficients = 5;
		resampler->B[0] = FILTER_FROM_FLOAT(+0.004824343357716); // Fixed: +0.004882812500000      
		resampler->B[1] = FILTER_FROM_FLOAT(+0.019297373430865); // Fixed: +0.019287109375000
		resampler->B[2] = FILTER_FROM_FLOAT(+0.028946060146297); // Fixed: +0.028930664062500
		resampler->B[3] = FILTER_FROM_FLOAT(+0.019297373430865); // Fixed: +0.019287109375000
		resampler->B[4] = FILTER_FROM_FLOAT(+0.004824343357716); // Fixed: +0.004882812500000
		resampler->A[0] = FILTER_FROM_FLOAT(+1.000000000000000); // Fixed: +1.000000000000000
		resampler->A[1] = FILTER_FROM_FLOAT(-2.369513007182038); // Fixed: -2.369506835937500
		resampler->A[2] = FILTER_FROM_FLOAT(+2.313988414415880); // Fixed: +2.313964843750000
		resampler->A[3] = FILTER_FROM_FLOAT(-1.054665405878568); // Fixed: -1.054687500000000
		resampler->A[4] = FILTER_FROM_FLOAT(+0.187379492368185); // Fixed: +0.187377929687500
	}

#endif

	// Initialize filter state
	for (int j = 0; j < resampler->axes; j++) {
		for (int k = 0; k < resampler->numCoefficients; k++) {
			resampler->z[j][k] = FILTER_FROM_INPUT(0);
		}
	}

	return resampler->numCoefficients > 0;
}


// Call to present input samples -- the supplied data must remain valid while resample_output() is polled
void resampler_input(resampler_t *resampler, const resampler_data_t *input, size_t count) {
	resampler->input = input;
	resampler->inputSamplesRemaining = count;
	// Configure at end of current upsample position (next read will be the new data)
	resampler->upPos = resampler->upSample;
}


// Call to receive output samples -- returns >0 when output samples exist, returns 0 when the input has been fully consumed and no further output samples are available
size_t resampler_output(resampler_t *resampler, resampler_data_t *output, size_t count) {
	size_t outputCount = 0;

	// Loop to process resampling
	for(;;) {

		// Try to advance the upsampler
		if (resampler->upPos + 1 >= resampler->upSample) {
			// End of upsampler cycle, try to get the next sample to start a new cycle
			if (resampler->inputSamplesRemaining > 0) {
				// Start of upsampling cycle
				resampler->upPos = 0;

				// Use the current data
				resampler->currentData = resampler->input;

				// Advance the input and remaining count
				resampler->input += resampler->axes;
				resampler->inputSamplesRemaining--;
			} else {
				// End of input -- return >0 if output data exists, otherwise 0 to indicate new data needed
				return outputCount;
			}
		} else {
			// Upsampler not at end of cycle
			resampler->upPos++;
		}

		// Use the actual data at the start of the upsample cycle, otherwise zero (the filter interpolates the data)
		const resampler_data_t *inData = (resampler->upPos == 0) ? resampler->currentData : zeroData;

		// Apply upsampled data through per-channel filters
		for (int j = 0; j < resampler->axes; j++) {
			filter_data_t v = FILTER_FROM_INPUT(inData[j]);
			v *= resampler->upSample;	// Apply gain: must scale values to keep constant average energy after upsample interpolation filter
			resampler_filter(resampler->numCoefficients, resampler->B, resampler->A, &v, &v, 1, resampler->z[j]);
			resampler->filtered[j] = FILTER_TO_OUTPUT(v);
		}

		// Take output at start of next downsample cycle
		if (resampler->downPos + 1 > resampler->downSample) {

			// If no further output capacity...
			if (outputCount >= count) {
				// Output full -- return (>0) to indicate output data (and that no new data should be provided yet)
				return outputCount;
			}

			// Otherwise, start new downsample cycle and output sample
			resampler->downPos = 0;

			// Copy output data
			resampler_data_t *outData = output + outputCount * resampler->axes;
			for (int j = 0; j < resampler->axes; j++) {
				outData[j] = resampler->filtered[j];
			}

			outputCount++;
		}

		// Advance downsample cycle
		resampler->downPos++;
	}

}


#ifdef RESAMPLER_TEST

#include <stdio.h>
#include <stdint.h>

#include "wav.h"

int resampler_test(const char *inFilename, const char *outFilename, int outFrequency) {
	
	if (inFilename == NULL || inFilename[0] == '\0') { fprintf(stderr, "ERROR: Input filename not specified.\n"); return 1; }
	if (outFilename == NULL || outFilename[0] == '\0') { fprintf(stderr, "ERROR: Output filename not specified.\n"); return 1; }
	
	FILE *fp = fopen(inFilename, "rb");
	if (fp == NULL) { fprintf(stderr, "ERROR: Problem opening input file: %s\n", inFilename); return 1; }
	
	WavInfo wavInfo = {0};
	if (!WavRead(&wavInfo, fp)) { fprintf(stderr, "ERROR: Problem understanding .WAV file: %s\n", inFilename); fclose(fp); return 1; }
	
	if (wavInfo.bytesPerChannel != sizeof(int16_t)) { fprintf(stderr, "ERROR: Unhandled format .WAV file - not 16-bit samples: %s\n", inFilename); fclose(fp); return 1; }
	
	size_t rawBufferSize = sizeof(int16_t) * wavInfo.chans * wavInfo.numSamples;
	int16_t *rawBuffer = (int16_t *)malloc(rawBufferSize);
	if (rawBuffer == NULL) { fprintf(stderr, "ERROR: Problem allocating memory for raw buffer: %llu\n", (unsigned long long)rawBufferSize); fclose(fp); return 1; }

	if (fread(rawBuffer, 1, rawBufferSize, fp) != rawBufferSize) { fprintf(stderr, "ERROR: Problem reading data from .WAV file: %s\n", inFilename); fclose(fp); return 1; }
	fclose(fp);
	
	// wavInfo.freq
	fprintf(stderr, "DEBUG: Read %lu samples (%d-channel, @%d Hz)\n", wavInfo.numSamples, wavInfo.chans, wavInfo.freq);
	
	// Resampler
	resampler_t resampler;
	int chans = wavInfo.chans;
	if (chans == 4) chans = 3; // Special-case x/y/z/aux -> x/y/z
	if (chans > RESAMPLER_MAX_AXES) chans = RESAMPLER_MAX_AXES;

	// Compact/reformat data
	size_t bufferSize = sizeof(resampler_data_t) * chans * wavInfo.numSamples;
	resampler_data_t *buffer = (resampler_data_t *)malloc(bufferSize);
	if (buffer == NULL) { fprintf(stderr, "ERROR: Problem allocating memory for buffer: %llu\n", (unsigned long long)bufferSize); return 1; }
	for (int i = 0; i < wavInfo.numSamples; i++) {
		for (int j = 0; j < chans; j++) {
			buffer[i * chans + j] = rawBuffer[i * wavInfo.chans + j];
		}
	}

	// Initialize the resampler
	if (!resampler_init(&resampler, wavInfo.freq, outFrequency, chans)) {
		fprintf(stderr, "ERROR: Support for specified frequencies not implemented (and RESAMPLER_CALCULATE_COEFFICIENTS not defined).\n");
		return 1;
	}

	// Output size
	const size_t outputSamples = wavInfo.numSamples * outFrequency / wavInfo.freq;
	const size_t outputSize = outputSamples * chans * sizeof(int16_t);
	int16_t *outputBuffer = (int16_t *)malloc(outputSize);
	if (outputBuffer == NULL) { fprintf(stderr, "ERROR: Problem allocating memory for output buffer: %llu\n", (unsigned long long)outputSize); return 1; }
	memset(outputBuffer, 0, outputSize);

	// Processing chunk sizes
	#define OUTPUT_MAX_SAMPLES 8
	#define INPUT_MAX_SAMPLES 8

	// Process
	resampler_data_t *sample = buffer;
	resampler_data_t output[OUTPUT_MAX_SAMPLES * RESAMPLER_MAX_AXES];
	int totalOut = 0;
	for (int i = 0; i < wavInfo.numSamples;) {
		
		size_t inputSampleCount = INPUT_MAX_SAMPLES;
		if (wavInfo.numSamples - i < inputSampleCount) inputSampleCount = wavInfo.numSamples - i;

#if 0
		for (int j = 0; j < inputSampleCount; j++) {
			resampler_data_t *inSample = input + (j * chans);
			printf("%.2f,%d,%d,%d\n", (float)(i + j) / wavInfo.freq, inSample[0], inSample[1], inSample[2]);
		}
#endif

//printf(">>> @ I=%d (+%d) O=%d\n", i, (int)inputSampleCount, totalOut);

		resampler_input(&resampler, sample, inputSampleCount);

		size_t outputCount;
		while ((outputCount = resampler_output(&resampler, output, OUTPUT_MAX_SAMPLES)) != 0) {
//printf("<<< O+=%d\n", (int)outputCount);
			for (int i = 0; i < outputCount; i++) {
				size_t op = totalOut + i;
				int16_t *out = outputBuffer + (op * chans);
				resampler_data_t *outSample = output + (i * chans);
//				printf("%.2f,%d,%d,%d,%s\n", (float)op / outFrequency, outSample[0], outSample[1], outSample[2], (op < outputSamples) ? "" : "1");
				if (op < outputSamples) {
					for (int j = 0; j < chans; j++) {
						out[j] = outSample[j];
					}
				}
			}
			totalOut += outputCount;
		}
//printf("<<< -\n");

		
		sample += inputSampleCount * chans;
		i += inputSampleCount;
	}
	
	// Output file
	FILE *ofp = fopen(outFilename, "wb");
	if (ofp == NULL) { fprintf(stderr, "ERROR: Problem opening output file: %s\n", outFilename); return 1; }
	WavInfo outWavInfo = {0};
	outWavInfo.bytesPerChannel = 2;
    outWavInfo.chans = chans;
    outWavInfo.freq = outFrequency;
    outWavInfo.numSamples = outputSamples;
	WavWrite(&outWavInfo, ofp);
	size_t written = fwrite(outputBuffer, 1, outputSize, ofp);
	fclose(ofp);
	if (written != outputSize) { fprintf(stderr, "ERROR: Problem writing to output file.\n"); return 1; }
	fprintf(stderr, "DEBUG: Written %lu samples (%d-channel, @%d Hz)\n", outputSamples, chans, outFrequency);

	fprintf(stderr, "DEBUG: Computed %d output samples, expected %d, %.3f %%, saved rate = %.2f Hz, generated rate = %.2f Hz, \n", totalOut, (int)outputSamples, 100.0 * totalOut / outputSamples, outputSamples / ((double)wavInfo.numSamples / wavInfo.freq),  totalOut / ((double)wavInfo.numSamples / wavInfo.freq)); 

	return 0;
}

int main(int argc, char *argv[]) {
	int outFrequency = 30;

#if 0
	for (int y = 0; y <= 12; y ++) {
		for (int x = 0; x <= 12; x++) {
			if (x == 0 && y == 0) continue;
			int d = gcd(x, y);
			printf("[%d, %d] / %d = [%d, %d]\n", x, y, d, x / d, y / d);
		}
	}
	return 0;
#endif
	
	const char *inFilename = NULL;
	const char *outFilename = NULL;
	
	if (argc == 2) inFilename = argv[1];
	if (argc == 3) outFilename = argv[2];
	if (argc == 4) outFrequency = atoi(argv[3]);

	if (inFilename == NULL || inFilename[0] == '\0') {
		fprintf(stderr, "ERROR: Input filename not specified.\n");
		return 1;
	}

	// Create output filename if not specified
	char *newFilename = NULL;
	if (outFilename == NULL || outFilename[0] == '\0') {
		const char *newExt = ".out.wav";
		newFilename = (char *)malloc(strlen(inFilename) + strlen(newExt) + 1);
		if (newFilename == NULL) { fprintf(stderr, "ERROR: Problem creating output file name\n"); return 1; }
		strcpy(newFilename, inFilename);
		for (char *p = newFilename + strlen(newFilename); p >= newFilename && *p != '\\' && *p != '/'; p--) {
			if (*p == '.') { *p = '\0'; break; }
		}
		strcat(newFilename, newExt);
		outFilename = newFilename;
	}

	int returnValue = resampler_test(inFilename, outFilename, outFrequency);

	if (newFilename != NULL) free(newFilename);
	return returnValue;
}

#endif
