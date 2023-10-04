#include "adc_isr.h"
#include "fourier.h"
#include <IntervalTimer.h>

#include <ADC.h>
#include <stdint.h>
#include <logger.h>

ADC *adc = new ADC();
float frequency_magnitudes[N_FREQUENCIES] = {0}; // magnitude of each frequency being measured

// when the measurement finishes, this will be called
void adc_isr() {
    const uint16_t reading = adc->adc0->readSingle();
    fourier_update(frequency_magnitudes, reading);

    // For the ARM M7 (Teensy 4.0), we need to ensure memory access inside ISR completes before returning
    // which we accomlish by inserting DSB (Data Sync Barrier) into the ASM.
    #if defined(__IMXRT1062__) // i.MX RT1062 => Teensy 4.0/4.1
    asm("DSB"); // insert DSB (Data Sync Barrier) in ASM.
    #endif
}

void adc_setup(){
    adc->adc0->setResolution(12); // our ADC happens to output a 12 bit integer
    adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED); // run as fast as possible hehe
    adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED); // run as fast as possible hehe
    adc->adc0->enableInterrupts(adc_isr);
    logger.info("ADC setup complete")
}

void adc_timer_callback(void) {
    adc->adc0->startSingleRead(MICROPHONE_IN_PIN);
}
