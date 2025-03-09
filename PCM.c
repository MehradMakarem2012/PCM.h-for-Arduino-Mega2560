#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

/*
  The PCM library by DavidMellis _ ReWrited for Mega 2560 by Mehrad Makarem 2025
  See the pcm library page in MOS website _ Mehrad Makarem: https://mos.ct.ws/MEGA_PCM
*/

/*
  You can change the sample rate to your audio rate.
  Note: The higher the sample rate you use, the higher the quality, but the shorter the audio duration. (I recommend using 8000)
*/
#define SAMPLE_RATE 8000

//____________________________

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// Set the default variables:
unsigned char const *sounddata_data = 0;
int sounddata_length = 0;
volatile uint16_t sample;

// Speaker pin for Mega on Digital 9 "Do Not Change!"
int speakerPin = 9;

// Timer 1 pause function for playing samples
ISR(TIMER1_COMPA_vect) {
  if (sample >= sounddata_length) {
    if (sample == sounddata_length + 128) {  // Ramp-down مقدار تغییر یافته
      stopPlayback();
    } else {
      OCR2B = 255 - (sample - sounddata_length);
    }
  } else {
    OCR2B = pgm_read_byte(&sounddata_data[sample]);
  }
  ++sample;
}

// startPlayback function:
void startPlayback(const unsigned char *data, int length) {
  sounddata_data = data;
  sounddata_length = length;

  pinMode(speakerPin, OUTPUT);

  // Set the timer 2 for PWM:
  TCCR2A = _BV(WGM21) | _BV(WGM20) | _BV(COM2B1);
  TCCR2B = _BV(CS20);
  OCR2B = 128;

  // Setting Timer 1 for sampling intervals
  cli();
  TCCR1A = 0;
  TCCR1B = _BV(WGM12) | _BV(CS10);
  OCR1A = F_CPU / SAMPLE_RATE - 1;  // محاسبه بر اساس فرکانس
  TIMSK1 = _BV(OCIE1A);
  sei();

  sample = 0;
}

// stopPlayback function:
void stopPlayback() {
  TIMSK1 &= ~_BV(OCIE1A);
  TCCR1B &= ~_BV(CS10);
  TCCR2B &= ~_BV(CS20);
  digitalWrite(speakerPin, LOW);
}
//End the library