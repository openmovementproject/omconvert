// Signal Resampler

#ifndef RESAMPLER_H
#define RESAMPLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>



#define RESAMPLER_FILTER_DOUBLE

#ifdef RESAMPLER_FILTER_DOUBLE
#include "butter.h"
#endif


#define RESAMPLER_MAX_AXES 3		// Triaxial

typedef int16_t resampler_data_t;

typedef struct {
	int axes;

	int inFrequency;				// Input frequency (NOTE: integer at present)
	int outFrequency;				// Output frequency (NOTE: integer at present)

	int lowPass;					// Low-pass filter frequency (NOTE: integer at present)

	int upSample;					// Rational upsample rate 1:N
	int downSample;					// Rational downsample rate N:1

	// Set by resampler_input()
	const resampler_data_t *input;
	size_t inputSamplesRemaining;
	const resampler_data_t *currentData;

	// State for upsample/downsample
	int upPos;
	int downPos;

	// Filter coefficients
#ifdef RESAMPLER_FILTER_DOUBLE
	double B[BUTTERWORTH_MAX_COEFFICIENTS(BUTTERWORTH_MAX_ORDER)];
	double A[BUTTERWORTH_MAX_COEFFICIENTS(BUTTERWORTH_MAX_ORDER)];
#else
	#error "Unimplemented"
#endif
	int numCoefficients;

	// State for (per axis) filter - final/initial condition tracking
	double z[RESAMPLER_MAX_AXES][BUTTERWORTH_MAX_COEFFICIENTS(BUTTERWORTH_MAX_ORDER)];

	// Filter output value
	resampler_data_t filtered[RESAMPLER_MAX_AXES];

} resampler_t;

void resampler_init(resampler_t *resampler, int inFrequency, int outFrequency, int axes);
void resampler_input(resampler_t *resampler, const resampler_data_t *input, size_t countSamples);
size_t resampler_output(resampler_t *resampler, resampler_data_t *output, size_t countSamples);

#ifdef __cplusplus
}
#endif

#endif
