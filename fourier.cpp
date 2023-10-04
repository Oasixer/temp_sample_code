#include "fourier.h"

// Ring Buffer to hold the time domain
uint16_t ring_buffer[MAX_SZ_WINDOW] = {0};
uint16_t idx_in_ringbuf = 0; 
uint16_t fourier_window_size = MAX_SZ_WINDOW;
complex_t freq_domain[N_FREQUENCIES] = {{0}};  // list to represent the fourier domain

void fourier_initialize(uint16_t N){
  if (N > MAX_SZ_WINDOW){
    logger.error("ERROR: Received window size (%i) is larger than the max. ignoring it.\n", N);
    return;
  } else {
    logger.info("Setting window size to %i\n", N);
  }
  fourier_window_size = N;

  // initialize the ring buffer to zeros
  for (uint16_t i = 0; i < MAX_SZ_WINDOW; i++){
    ring_buffer[i] = 0;
  }
  idx_in_ringbuf = 0;

  // Calculating shift based on frequencies
  uint16_t F_NATURAL = F_SAMPLE / fourier_window_size;
  for (uint8_t i = 0; i < N_FREQUENCIES; ++i){
    uint16_t k = FREQUENCIES[i] / F_NATURAL;
    float theta = 2. * M_PI * k / fourier_window_size;
    shift_factor[i].real = cosf(theta);
    shift_factor[i].imaginary = sinf(theta);
  }

  // initialize the frequency domains to zeros
  for (uint8_t i = 0; i < N_FREQUENCIES; ++i){
    freq_domain[i].real = 0;
    freq_domain[i].imaginary = 0;
  }
}

void fourier_update(float *magnitudes, uint16_t new_sample){
  uint16_t old_sample = ring_buffer[idx_in_ringbuf];
  ring_buffer[idx_in_ringbuf] = new_sample; // overwrite old_sample
  
  // wrap back to 0 by modding against the window size
  idx_in_ringbuf = (idx_in_ringbuf+1) % fourier_window_size;
  
  float difference = (float)(new_sample - old_sample);

  // For each frequency we monitor, update the frequency domain using the new sample
  // then update the frequency magnitudes
  for (uint8_t i = 0; i < N_FREQUENCIES; ++i){
    // add difference
    freq_domain[i].real += difference;
    
    // multiply by shift factor
    freq_domain[i] = {
      (freq_domain[i].real * shift_factor[i].real) - (freq_domain[i].imaginary * shift_factor[i].imaginary),
      (freq_domain[i].real * shift_factor[i].imaginary) + (freq_domain[i].imaginary * shift_factor[i].real),
    };

    // calculate magnitude using freq domain and sqrt of the squares of real and imaginary parts
    magnitudes[i] = sqrtf(
      freq_domain[i].real * freq_domain[i].real + 
      freq_domain[i].imaginary * freq_domain[i].imaginary);
  }
}
