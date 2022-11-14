#include "PPintrin.h"

// implementation of absSerial(), but it is vectorized using PP intrinsics
void absVector(float *values, float *output, int N)
{
  __pp_vec_float x;
  __pp_vec_float result;
  __pp_vec_float zero = _pp_vset_float(0.f);
  __pp_mask maskAll, maskIsNegative, maskIsNotNegative;

  //  Note: Take a careful look at this loop indexing.  This example
  //  code is not guaranteed to work when (N % VECTOR_WIDTH) != 0.
  //  Why is that the case?
  for (int i = 0; i < N; i += VECTOR_WIDTH)
  {

    // All ones
    maskAll = _pp_init_ones();

    // All zeros
    maskIsNegative = _pp_init_ones(0);

    // Load vector of values from contiguous memory addresses
    _pp_vload_float(x, values + i, maskAll); // x = values[i];

    // Set mask according to predicate
    _pp_vlt_float(maskIsNegative, x, zero, maskAll); // if (x < 0) {

    // Execute instruction using mask ("if" clause)
    _pp_vsub_float(result, zero, x, maskIsNegative); //   output[i] = -x;

    // Inverse maskIsNegative to generate "else" mask
    maskIsNotNegative = _pp_mask_not(maskIsNegative); // } else {

    // Execute instruction ("else" clause)
    _pp_vload_float(result, values + i, maskIsNotNegative); //   output[i] = x; }

    // Write results back to memory
    _pp_vstore_float(output + i, result, maskAll);
  }
}

void clampedExpVector(float *values, int *exponents, float *output, int N)
{
  __pp_vec_float x;
  __pp_vec_int exp;
  __pp_vec_float clampedMax = _pp_vset_float(9.999999f);
  __pp_vec_float result;
  __pp_vec_int one = _pp_vset_int(1);
  __pp_vec_int zero = _pp_vset_int(0);
  __pp_mask maskAll, maskExpZero, maskExpNotZero, maskClamped;

  // Handle remaining lanes when N % VECTOR_WIDTH != 0
  for (int i = N;i < N + VECTOR_WIDTH;i++) {
    values[i] = 0.0f;
    exponents[i] = 1;
  }
  
  //
  // PP STUDENTS TODO: Implement your vectorized version of
  // clampedExpSerial() here.
  //
  // Your solution should work for any value of
  // N and VECTOR_WIDTH, not just when VECTOR_WIDTH divides N
  //
  for (int i = 0; i < N; i += VECTOR_WIDTH)
  {
    
    maskAll = _pp_init_ones();                        // All ones

    maskExpZero = _pp_init_ones(0);                   // a mask to indicate lanes whose exponent == 0
    
    maskExpNotZero = _pp_init_ones(0);                // a mask to indicate lanes whose exponent != 0

    maskClamped = _pp_init_ones(0);                   // a mask to indicate lanes whose result > 9.99999f

    _pp_vload_float(x, values + i, maskAll);          // x = values[i : (i + VECTOR_WIDTH - 1)]

    _pp_vload_int(exp, exponents + i, maskAll);       // exp = exponents[i : (i + VECTOR_WIDTH - 1)]

    _pp_veq_int(maskExpZero, exp, zero, maskAll);     // find lanes whose exp == 0

    maskExpNotZero = _pp_mask_not(maskExpZero);       // find lanes whose exp != 0

    _pp_vset_float(result, 1.0f, maskExpZero);        // if the jth-lane is activated in maskExpZero, result[j] = 1.0f

    _pp_vmove_float(result, x, maskExpNotZero);       // else, result[j] = value[j]

    _pp_vsub_int(exp, exp, one, maskExpNotZero);      // exp = exp - 1 for lanes whose exp != 0 

    _pp_vgt_int(maskExpNotZero, exp, zero, maskAll);  // find lanes whose exp != 0

    while (_pp_cntbits(maskExpNotZero)) {
      _pp_vmult_float(result, result, x, maskExpNotZero);    // result = result * x  
      
      _pp_vsub_int(exp, exp, one, maskExpNotZero);           // exp = exp - 1 

      _pp_vgt_int(maskExpNotZero, exp, zero, maskAll);       // find lanes whose exp != 0
    }
    
    _pp_vgt_float(maskClamped, result, clampedMax, maskAll);  // result = result > clampedMaxThres

    _pp_vset_float(result, 9.999999f, maskClamped);           // result[j] = 9.999999f

    _pp_vstore_float(output + i, result, maskAll);            // Write results back to memory
  }
}

// returns the sum of all elements in values
// You can assume N is a multiple of VECTOR_WIDTH
// You can assume VECTOR_WIDTH is a power of 2
float arraySumVector(float *values, int N)
{

  //
  // PP STUDENTS TODO: Implement your vectorized version of arraySumSerial here
  //

  float sum = 0.f;
  __pp_vec_float x, haddResult, interResult;
  __pp_mask maskAll;

  for (int i = 0; i < N; i += VECTOR_WIDTH)
  {
      maskAll = _pp_init_ones();

      _pp_vload_float(x, values + i, maskAll);  // x = value[i : (i + VECTOR_WIDTH - 1)]

      _pp_hadd_float(haddResult, x);  // Add up addjecnt pairs
      
      _pp_interleave_float(interResult, haddResult);  // Move sum of addject pairs to front-half of array

      for (int j = 0; j < VECTOR_WIDTH / 2; j++){    // Sum front-half of array
	      sum += interResult.value[j];
      }
  }

  return sum;
}