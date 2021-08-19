// Signal Resampler

#ifndef RESAMPLER_H
#define RESAMPLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>


// Config
//#define RESAMPLER_CALCULATE_COEFFICIENTS	// Support generating aribtrary filters (not just a fixed set)
#define RESAMPLER_FILTER_DOUBLE				// Use floating-point calculations (otherwise, fixed-point for embedded)
#define RESAMPLER_MAX_AXES 3				// Triaxial
typedef int16_t resampler_data_t;

#ifdef RESAMPLER_CALCULATE_COEFFICIENTS
	#include "butter.h"
	#define RESAMPLER_MAX_COEFFICIENTS BUTTERWORTH_MAX_COEFFICIENTS(BUTTERWORTH_MAX_ORDER)
#else
	#define RESAMPLER_MAX_COEFFICIENTS 10		// e.g. 10: 9th-order LP or 7th-order BP
#endif

#ifdef RESAMPLER_FILTER_DOUBLE
	typedef double filter_data_t;
	#define FILTER_FROM_INPUT(_x) ((filter_data_t)(_x))
	#define FILTER_TO_OUTPUT(_x) ((resampler_data_t)(_x))
	#define FILTER_FROM_FLOAT(_x) (_x)
	#define FILTER_TO_FLOAT(_x) (_x)
#else
	#error "Fixed-point unimplemented"
	//typedef int32_t filter_data_t;
	#define FILTER_FRACTIONAL 8						// Number of fixed-point fractional bits
	#define FILTER_SCALING (1<<FILTER_FRACTIONAL)	// Scaling value for fixed-point

	#define FILTER_FROM_INPUT(_x) ((filter_data_t)(_x) << FILTER_FRACTIONAL)
	#define FILTER_TO_OUTPUT(_x) ((resampler_data_t)(_x) >> FILTER_FRACTIONAL)	// Consider: rounding nearest rather than rounding down
	#define FILTER_FROM_FLOAT(_x) ((filter_data_t)((_x) * FILTER_SCALING))
	#define FILTER_TO_FLOAT(_x) ((float)(_x) / FILTER_SCALING)
#endif

typedef struct {
	int axes;

	int inFrequency;				// Input frequency (NOTE: integer at present)
	int outFrequency;				// Output frequency (NOTE: integer at present)

	int lowPass;					// Low-pass filter frequency (NOTE: integer at present)
	int intermediateFrequency;		// Intermediate (upsampled) frequency (NOTE: integer at present)

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
	filter_data_t B[RESAMPLER_MAX_COEFFICIENTS];
	filter_data_t A[RESAMPLER_MAX_COEFFICIENTS];
	int numCoefficients;

	// State for (per axis) filter - final/initial condition tracking
	filter_data_t z[RESAMPLER_MAX_AXES][RESAMPLER_MAX_COEFFICIENTS];

	// Filter output value
	resampler_data_t filtered[RESAMPLER_MAX_AXES];

} resampler_t;

bool resampler_init(resampler_t *resampler, int inFrequency, int outFrequency, int axes);
void resampler_input(resampler_t *resampler, const resampler_data_t *input, size_t countSamples);
size_t resampler_output(resampler_t *resampler, resampler_data_t *output, size_t countSamples);

#ifdef __cplusplus
}
#endif

#endif
