I am not primarily an embedded engineer, but I thought this code was an interesting sample :).

This is a small part of a much larger codebase at https://github.com/kiranjray/fydp_software which is for my capstone project in university. In the project, we had underwater drones communicating using acoustic signals and resolving each other's 3D location by timing the difference in the pings. Each underwater drone has a unique frequency that it uses in its pings to identify it.

I contributed to many parts of the codebase but mainly the backend Rust server that orchestrated the nodes, and the firmware on the nodes. I've extracted a couple files in particular but this is from the `firmware/main` directory in the repository above, and feel free to read more. I also wrote the network integrations so that all of the audio data and other messages can be bidirectionally transmitted over TCP at ~20MB/s

In adc_isr.cpp, An ISR (Instruction Routine) function called `adc_isr` gets triggered repeatedly (specifically 500,000 times per second) in order to measure the frequencies present in a live audio feed. The analog microphone signal goes through an ADC (Analog Digital Converter) and the ADC triggers the ISR function, all running on an ARM M7 microprocessor.

To explain further what I mean by "measure the frequencies present", it means to determine the various audio pitches that makeup the sound measured by the microphone (ie. how loud is the bass, how loud is the treble, etc.) In particular, we have a certain set of frequencies that we want to listen for in the audio feed. The algorithm implemented in fourier.cpp is a specific variation of a Fast Fourier Transform (FFT) called a Sliding Window Fourier Transform.

- Inside `adc_isr.cpp` -> `adc_isr()` we read a new sample from the ADC, and pass it to `fourier.cpp` -> `fourier_update()`.
- Then in `fourier_update()`, that sample is appended to a ringbuffer so that the algorithm always has access to the last `WINDOW_SIZE` samples, where WINDOW_SIZE depends on how long you want the window to be in real time
  - That is, given the algorithm runs 500,000 times per second, if `WINDOW_SIZE` was 50,000 then we would be measuring the amplitudes of our target frequencies over the last 100 ms == 1/10th of a second). The lower the widow size, the quicker a new sound will be picked up, but the more succeptible to noise the algorithm will be.
- Each cycle, the frequency domain which contains the magnitude information is updated by adding the effect of `new_sample`, and subtracting the effect of `old_sample`, where `old_sample` is the sample previously occupying the position in the ringbuffer about to be overwritten.
- The magnitude of the frequency domain for each frequency gives the "loudness" of each frequency at any given time. Any program can grab `frequency_magnitudes` from `adc_isr.cpp` to stuff like triggering a function when a certain tone reaches a certain volume.
  - In the actual codebase, there is an algorithm in the firmware that looks for certain thresholds in the frequency magnitudes to determine when a ping from a node has been heard.
