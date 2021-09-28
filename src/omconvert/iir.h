// Fixed Point IIR filters
// IMPORTANT: Currently uses a static buffer, so single stream only

#include <stdint.h>

typedef int32_t iir_value_t;
typedef int32_t iir_fixed_t;
typedef int64_t iir_multiply_t;				// Temporary holder type for multiplication (before shift down)

#define IIR_FRACTIONAL 13					// Number of fixed-point fractional bits
#define IIR_SCALING (1<<IIR_FRACTIONAL)		// Scaling value for fixed-point
#define IIR_BIAS (1<<(IIR_FRACTIONAL - 1))	// Half unit bias

#define IIR_FROM_INPUT(_x) ((iir_fixed_t)(_x) << (IIR_FRACTIONAL))
#define IIR_TO_OUTPUT(_x) ((iir_value_t)(((_x)) >> (IIR_FRACTIONAL)))	// Does rounding make much difference here? (+ IIR_BIAS)
#define IIR_FROM_FLOAT(_x) ((iir_fixed_t)((_x) * IIR_SCALING + ((_x) < 0 ? -0.5 : 0.5)))		// 
//#define IIR_TO_FLOAT(_x) ((double)(_x) / IIR_SCALING)					// Only used for display
#define IIR_MULTIPLY(_x, _y) ((iir_fixed_t)(((iir_multiply_t)(_x) * (_y) + IIR_BIAS) >> IIR_FRACTIONAL))

// IIR Butterworth order 2, 30 Hz, highpass 0.5 Hz.
// Adapted from code generated using mkfilter by A.J. Fisher
// http://www.massmind.org/cgi-bin/mkfscript.asp?type=Butterworth&pass=High&o=2&sr=30&c1=0.5
inline static iir_value_t iir_order2_rate30_highpass0_5(iir_value_t input) {
    const iir_fixed_t GAIN = IIR_FROM_FLOAT(1.0/ 1.076862368e+000);
    static iir_fixed_t xv[2+1] = {0}, yv[2+1] = {0};
    xv[0] = xv[1]; xv[1] = xv[2]; 
    xv[2] = input * GAIN;
    yv[0] = yv[1]; yv[1] = yv[2]; 
    yv[2] = (xv[0] + xv[2]) - 2 * xv[1]
            + IIR_MULTIPLY(IIR_FROM_FLOAT(-0.8623486260), yv[0])
            + IIR_MULTIPLY(IIR_FROM_FLOAT(1.8521464854), yv[1]);
    iir_value_t output = IIR_TO_OUTPUT(yv[2]);
    return output;
}

// IIR Butterworth order 2, 40 Hz, highpass 0.5 Hz.
// Adapted from code generated using mkfilter by A.J. Fisher
// http://www.massmind.org/cgi-bin/mkfscript.asp?type=Butterworth&pass=High&o=2&sr=40&c1=0.5
inline static iir_value_t iir_order2_rate40_highpass0_5(iir_value_t input) {
    const iir_fixed_t GAIN = IIR_FROM_FLOAT(1.0/ 1.057108315e+000);
    static iir_fixed_t xv[2+1] = {0}, yv[2+1] = {0};
    xv[0] = xv[1]; xv[1] = xv[2]; 
    xv[2] = input * GAIN;
    yv[0] = yv[1]; yv[1] = yv[2]; 
    yv[2] = (xv[0] + xv[2]) - 2 * xv[1]
            + IIR_MULTIPLY(IIR_FROM_FLOAT(-0.8948743446), yv[0])
            + IIR_MULTIPLY(IIR_FROM_FLOAT(1.8890330794), yv[1]);
    iir_value_t output = IIR_TO_OUTPUT(yv[2]);
    return output;
}

// IIR Butterworth order 2, 50 Hz, highpass 0.5 Hz.
// Adapted from code generated using mkfilter by A.J. Fisher
// http://www.massmind.org/cgi-bin/mkfscript.asp?type=Butterworth&pass=High&o=2&sr=50&c1=0.5
inline static iir_value_t iir_order2_rate50_highpass0_5(iir_value_t input) {
    const iir_fixed_t GAIN = IIR_FROM_FLOAT(1.0/ 1.045431062e+000);
    static iir_fixed_t xv[2+1] = {0}, yv[2+1] = {0};
    xv[0] = xv[1]; xv[1] = xv[2]; 
    xv[2] = input * GAIN;
    yv[0] = yv[1]; yv[1] = yv[2]; 
    yv[2] = (xv[0] + xv[2]) - 2 * xv[1]
            + IIR_MULTIPLY(IIR_FROM_FLOAT(-0.9149758348), yv[0])
            + IIR_MULTIPLY(IIR_FROM_FLOAT(1.9111970674), yv[1]);
    iir_value_t output = IIR_TO_OUTPUT(yv[2]);
    return output;
}

// IIR Butterworth order 2, 100 Hz, highpass 0.5 Hz.
// Adapted from code generated using mkfilter by A.J. Fisher
// http://www.massmind.org/cgi-bin/mkfscript.asp?type=Butterworth&pass=High&o=2&sr=100&c1=0.5
inline static iir_value_t iir_order2_rate100_highpass0_5(iir_value_t input) {
    const iir_fixed_t GAIN = IIR_FROM_FLOAT(1.0/ 1.022463023e+000);
    static iir_fixed_t xv[2+1] = {0}, yv[2+1] = {0};
    xv[0] = xv[1]; xv[1] = xv[2]; 
    xv[2] = input * GAIN;
    yv[0] = yv[1]; yv[1] = yv[2]; 
    yv[2] = (xv[0] + xv[2]) - 2 * xv[1]
            + IIR_MULTIPLY(IIR_FROM_FLOAT(-0.9565436765), yv[0])
            + IIR_MULTIPLY(IIR_FROM_FLOAT(1.9555782403), yv[1]);
    iir_value_t output = IIR_TO_OUTPUT(yv[2]);
    return output;
}

#ifdef IIR_RATE
    #if (IIR_RATE == 100)
        #define iir_order2_highpass0_5 iir_order2_rate100_highpass0_5
    #elif (IIR_RATE == 50)
        #define iir_order2_highpass0_5 iir_order2_rate50_highpass0_5
    #elif (IIR_RATE == 40)
        #define iir_order2_highpass0_5 iir_order2_rate40_highpass0_5
    #elif (IIR_RATE == 30)
        #define iir_order2_highpass0_5 iir_order2_rate30_highpass0_5
    #else
        #error "Unhandled IIR_RATE"
    #endif
#endif
