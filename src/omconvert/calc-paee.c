/*
* Copyright (c) 2014, Newcastle University, UK.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

// Open Movement SVM Processor
// Dan Jackson, 2014


/*
Cut-point calculation
"Validation of the GENEA Accelerometer" by Esliger et al.

  SVMgs = SUM(abs(sqrt(x^2 + y^2 + z^2) – 1))

Where sum is for 60 seconds at 80Hz, so divide all cut-points below by 4800 to work on mean SVM-1

									GENEA	R/Dom	L/N-D	Waist	
Sedentary	 < 1.5 METS										
Light   	>= 1.5 METS	< 4 METS	 386	 386	 217	  77	
Moderate	>= 4.0 METS	< 7 METS	 542	 440	 645	 220	
Vigorous	>= 7.0 METS	        	1811	2099	1811	2057	

// Original SVMgs = SUM(abs(sqrt(x ^ 2 + y ^ 2 + z ^ 2) – 1)) at 80Hz (from "Validation of the GENEA Accelerometer" by Esliger et al.) so these cut-points * 1/(60*80) for average SVM
// Sedentary, Light (>= 1.5 METS, < 4 METS), Moderate (>= 4 METS, < 7 METS), Vigorous (>= 7 METS)

'wrist':                386/80/60 542/80/60 1811/80/60
Esliger(40-63)-wristR:  386/80/60 440/80/60 2099/80/60
Esliger(40-63)-wristL:  217/80/60 645/80/60 1811/80/60
Esliger(40-63)-waist:    77/80/60 220/80/60 2057/80/60
*/

/*
From Schaefer et al ("Establishing and Evaluating Wrist Cutpoints for the GENEActiv Accelerometer in Youth")
(6-11 years)
-- 75 Hz data, band-pass filtered 0.2-15Hz, average SVM-1 values, already normalized:
0.190
0.314
0.998
Sedentary, Light (>= 1.5 METS, < 3 METS), Moderate (>= 3 METS, < 6 METS), Vigorous (>= 6 METS)

Schaefer(6-11)-wristND: 0.190 0.314 0.998
*/

/*
From Phillips et al ("Calibration of the GENEA accelerometer for assessment of physical activity intensity in children")
(8-14 years old)
Author, Site, Rate, Sedentary/Light/Moderate/Vigorous (g/s) - normalized by *(1/80)
Phillips et al, Right Wrist, 80Hz, @1s, <6, <22, <56 (>=56)
Phillips et al, Left Wrist, 80Hz, @1s,  <7, <20, <60 (>=60)
Phillips et al, Hip, 80Hz, @1s,         <3, <17, <51 (>=51)
Sedentary, Light (>= 1.5 METS, < 3 METS), Moderate (>= 3 METS, < 6 METS), Vigorous (>= 6 METS)

Phillips(8-14)-wristR:   6/80 22/80 56/80
Phillips(8-14)-wristL:   7/80 20/80 60/80
Phillips(8-14)-hip:      3/80 17/80 51/80
*/

/*
From Roscoe et al ("Calibration of GENEActiv accelerometer wrist cut-points for the assessment of physical activity intensity of preschool aged children.")
(4-5 years old)
Roscoe et al, Non-dominant Wrist, 87.5Hz, @1s, <5.3, <8.6 (>=8.6)
Roscoe et al, Dominant Wrist, 87.5Hz, @1s,     <8.1, <9.3 (>=9.3)
Sedentary, Light (>= 2 METS , < 3 METS) , Moderate and above (>= 3 METS, < 6 METS), NO Vigorous

Roscoe(4-5)-wristND:     5.3/87.5 8.6/87.5
Roscoe(4-5)-wristD:      8.1/87.5 9.3/87.5
*/

/*
From Dillon et al ("Number of Days Required to Estimate Habitual Activity Using Wrist-Worn GENEActiv Accelerometer: A Cross-Sectional Study", 2016)
...cut points appear to be re-targeted and the same as from "Criterion validity and calibration of the GENEActiv accelerometer in adults" (2015).

Dillon et al, Dominant wrist,     100Hz, @15s, <191.8, <281.6, <595 (>=595)
Dillon et al, Non-dominant wrist, 100Hz, @15s, <158.5, <261.9, <495 (>=495)
Sedentary, Light (>= 1.5 METS, < 3 METS), Moderate (>= 3 METS, < 6 METS), Vigorous (>= 6 METS)

Dillon-wristD:    191.8/100/15 281.6/100/15 595/100/15
Dillon-wristND:   158.5/100/15 261.9/100/15 495/100/15
...Dillon-wristD  (2016) same as Dillon-wristD  (2015): 230/30/60 338/30/60 714/30/60
...Dillon-wristND (2016) same as Dillon-wristND (2015): 190/30/60 314/30/60 594/30/60
*/

/*
From Powell et al ("Simultaneous validation of five activity monitors for use in adult populations")
(mean age 39.9 +/-11.5 years)

Powell et al, Dominant wrist, 30Hz, @15s,     <51, <68, <142 (>=142)
Powell et al, Non-dominant wrist, 30Hz, @15s, <47, <64, <157 (>=157)
Sedentary, Light (>= 1.5 METS, < 3 METS), Moderate (>= 3 METS, < 6 METS), Vigorous (>= 6 METS)

Powell-wristD:           51/30/15 68/30/15 142/30/15
Powell-wristND:          47/30/15 64/30/15 157/30/15
*/

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "calc-paee.h"


#define PAEE_NORMALIZE_80Hz (1.0 / (60.0 * 80.0))		// Normalize as figures in paper were direct sum of abs(SVM-1) for 60 seconds at 80Hz
const double paeeCutPointWrist[]  = { 386.0 * PAEE_NORMALIZE_80Hz, 542.0 * PAEE_NORMALIZE_80Hz, 1811.0 * PAEE_NORMALIZE_80Hz, 0 };	// In GENEA analysis
const double paeeCutPointWristR[] = { 386.0 * PAEE_NORMALIZE_80Hz, 440.0 * PAEE_NORMALIZE_80Hz, 2099.0 * PAEE_NORMALIZE_80Hz, 0 };
const double paeeCutPointWristL[] = { 217.0 * PAEE_NORMALIZE_80Hz, 645.0 * PAEE_NORMALIZE_80Hz, 1811.0 * PAEE_NORMALIZE_80Hz, 0 };
const double paeeCutPointWaist[]  = {  77.0 * PAEE_NORMALIZE_80Hz, 220.0 * PAEE_NORMALIZE_80Hz, 2057.0 * PAEE_NORMALIZE_80Hz, 0 };

#define AXES 3


// Load data
char PaeeInit(paee_status_t *status, paee_configuration_t *configuration)
{
	memset(status, 0, sizeof(paee_status_t));
	status->configuration = configuration;

	if (configuration->minuteEpochs <= 0) { configuration->minuteEpochs = 1; }

	if (status->configuration->sampleRate <= 0.0)
	{
		fprintf(stderr, "ERROR: PAEE sample rate not specified.\n");
		return 0;
	}

	if (status->configuration->cutPoints == NULL)
	{
		fprintf(stderr, "ERROR: PAEE cut point values not specified.\n");
		return 0;
	}

	// Calculate number of cut-point levels
	status->numCutPoints = 0;
	for (int i = 0; i < PAEE_MAX_CUT_POINTS; i++)
	{
		//fprintf(stderr, "DEBUG: custom cut point %d = %f\n", status->numCutPoints, status->configuration->cutPoints[i]);
		// Sentinal last value
		if (status->configuration->cutPoints[i] <= 0.0)
		{
			break;
		}
		status->numCutPoints++;
	}

	status->file = NULL;
	if (configuration->filename != NULL && strlen(configuration->filename) > 0)
	{
		status->file = fopen(configuration->filename, "wt");
		if (status->file == NULL)
		{
			fprintf(stderr, "ERROR: PAEE file not opened.\n");
		}
	}

	// .CSV header
	if (status->file && configuration->headerCsv)
	{
		fprintf(status->file, "Time,Sedentary (mins),Light (mins),Moderate (mins),Vigorous (mins)\n");
	}

	// Filter parameters
	int order = 4;
	double Fc1 = 0.5;
	double Fc2 = 20;			// 15
	double Fs = status->configuration->sampleRate;
	if (Fc2 >= Fs / 2) { Fc2 = -1.0; }				// High-pass filter instead (upper band cannot exceed Nyquist limit)

	// Calculate normalized cut-offs
	double W1 = Fc1 / (Fs / 2);
	double W2 = Fc2 / (Fs / 2);

	// Calculate coefficients
	status->numCoefficients = CoefficientsButterworth(order, W1, W2, status->B, status->A);

	/*
	// Display coefficients
	int i;
	printf("B: ");
	for (i = 0; i < status->numCoefficients; i++) { printf("%f ", status->B[i]); }
	printf("\n");
	printf("A: ");
	for (i = 0; i < status->numCoefficients; i++) { printf("%f ", status->A[i]); }
	printf("\n");
	*/

	// Reset
	status->epochStartTime = 0;
	status->sample = 0;
status->sumSvm = 0;

	return (status->file != NULL) ? 1 : 0;
}


void PaeePrint(paee_status_t *status)
{
	int c;
	if (status->file != NULL)
	{
		char timestring[24];	// 2000-01-01 12:00:00.000\0

		time_t tn = (time_t)status->epochStartTime;
		struct tm *tmn = gmtime(&tn);
		float sec = tmn->tm_sec + (float)(status->epochStartTime - (time_t)status->epochStartTime);
		sprintf(timestring, "%04d-%02d-%02d %02d:%02d:%02d", 1900 + tmn->tm_year, tmn->tm_mon + 1, tmn->tm_mday, tmn->tm_hour, tmn->tm_min, (int)sec);	// (int)((sec - (int)sec) * 1000)

		fprintf(status->file, "%s", timestring);
		for (c = 0; c < status->numCutPoints + 1; c++)
		{
			fprintf(status->file, ",%u", (int)(status->minutesAtLevel[c] + 0.5));
		}
		fprintf(status->file, "\n");

	}
}


// Processes the specified value
bool PaeeAddValue(paee_status_t *status, double* accel, double temp, bool valid)
{
	int c;

	if (status->epochStartTime == 0)
	{
		status->epochStartTime = status->configuration->startTime + (status->sample / status->configuration->sampleRate);
	}

	// SVM
	double sumSquared = 0;
	for (c = 0; c < AXES; c++)
	{
		double v = accel[c];
		sumSquared += v * v;
	}
	double svm = sqrt(sumSquared);

#if 0
	if (!(status->configuration->mode < 4))	// Mode 0/1 are SVM-1, modes 2-4 are SVM
#endif
	{
		svm -= 1;
	}

	if (status->configuration->filter)
	{
		filter(status->numCoefficients, status->B, status->A, &svm, &svm, 1, status->z);
	}

#if 0
	// SVM mode (must be after filtering)
	switch (status->configuration->mode & 3)
	{
		case 0: 
#endif
			svm = fabs(svm); 
#if 0
			break;					// Standard abs(v) mode
		case 1: if (svm < 0) { svm = 0.0; } break;		// Clamp max(0,v) mode
		case 2: break;									// Pass-through mode (sum will include values <0 !)
		case 3:	break;									// (reserved)
	}
#endif

	if (valid)
	{
		status->sumSvm += svm;
		status->intervalSample++;
	}

	status->sample++;

	// Report PAEE epoch
	int interval = ((int)(status->configuration->sampleRate * 60 + 0.5));
	if (status->sample > 0 && status->sample % interval == 0)
	{
		double meanSvm = 0;
		if (status->intervalSample > 0) { meanSvm = status->sumSvm / status->intervalSample; }

		// Add to the correct cut points
		for (c = status->numCutPoints; c >= 0; c--)
		{
			if (c == 0 || meanSvm >= status->configuration->cutPoints[c - 1])
			{
				status->minutesAtLevel[c]++;
				break;
			}
		}

		status->minute++;

		if (status->minute >= status->configuration->minuteEpochs)
		{
			PaeePrint(status);

			status->minute = 0;
			for (c = 0; c < status->numCutPoints + 1; c++) { status->minutesAtLevel[c] = 0; }
			status->epochStartTime = 0;
		}

		status->sumSvm = 0;
		status->intervalSample = 0;
	}

	return true;
}

// Free data resources
int PaeeClose(paee_status_t *status)
{
	if (status->minute > 0)
	{
		PaeePrint(status);	// Print PAEE for last block if it has at least one minute in
	}
	if (status->file != NULL)
	{ 
		fclose(status->file);
	}
	return 0;
}
