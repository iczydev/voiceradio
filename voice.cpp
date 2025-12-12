#include "pxt.h"
#include "MicroBit.h"

using namespace pxt;

#define SAMPLE_RATE 4000
#define RECORD_MS   1000
#define NUM_SAMPLES (SAMPLE_RATE * RECORD_MS / 1000)

// μ-law converter
static inline uint8_t linearToMuLaw(int16_t sample) {
    const int BIAS = 0x84;
    int sign = (sample < 0) ? 0x80 : 0;
    if (sign) sample = -sample;
    sample += BIAS;

    int exponent = 7;
    for (int expMask = 0x4000; (sample & expMask) == 0 && exponent > 0; expMask >>= 1)
        exponent--;

    int mantissa = (sample >> (exponent + 3)) & 0x0F;
    return ~(sign | (exponent << 4) | mantissa);
}

// μ-law → 16-bit PCM
static inline int16_t muLawToLinear(uint8_t ulaw) {
    ulaw = ~ulaw;
    int sign = (ulaw & 0x80);
    int exponent = (ulaw >> 4) & 0x07;
    int mantissa = ulaw & 0x0F;
    int sample = ((mantissa << 3) + 0x84) << exponent;
    return sign ? -(sample) : sample;
}

namespace voiceRadio {

Buffer record() {
    auto buf = mkBuffer(NUM_SAMPLES);
    auto b = getBuffer(buf);

    for (int i = 0; i < NUM_SAMPLES; i++) {
        int16_t sample = uBit.io.microphone.getAnalogValue() - 512;
        b->data[i] = linearToMuLaw(sample);

        uBit.sleep(1000 / SAMPLE_RATE);
    }
    return buf;
}

void play(Buffer b) {
    auto dat = getBuffer(b);

    for (int i = 0; i < dat->length; i++) {
        int16_t sample = muLawToLinear(dat->data[i]);
        int pwm = sample + 2048;
        uBit.io.speaker.setAnalogValue(pwm);

        uBit.sleep(1000 / SAMPLE_RATE);
    }

    uBit.io.speaker.setAnalogValue(0);
}

}
