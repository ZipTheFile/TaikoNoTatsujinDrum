#define CHANNELS 4
#define SAMPLE_CACHE_LENGTH 16
#define DEBUG 0

#include <Keyboard.h>
#include <limits.h>
#include "cache.h"

// Define a structure to hold channel-specific data
struct ChannelData {
    int inputPin;
    int outputPin;
    char outputKey;
    float sensitivity;
    unsigned long hitThreshold;
    unsigned long resetThreshold;
    Cache<int, SAMPLE_CACHE_LENGTH> inputWindow;
    unsigned long power;
    unsigned long lastPower;
    bool triggered;
    unsigned long triggeredTime;
};

// Initialize channel data
ChannelData channels[] = {
    {A0, 5, 'd', 1.0, 1, 2},
    {A1, 6, 'f', 1.0, 8, 1},
    {A2, 7, 'j', 1.0, 8, 1},
    {A3, 8, 'k', 1.0, 1, 2}
};

short maxIndex = -1;
float maxPower = 0;

void setup() {
    Serial.begin(115200);
    Keyboard.begin();
    analogReference(DEFAULT);

    for (byte i = 0; i < CHANNELS; i++) {
        channels[i].power = 0;
        channels[i].lastPower = 0;
        channels[i].triggered = false;
        pinMode(channels[i].outputPin, OUTPUT);
    }
}

void loop() {
    processChannels();
    checkTriggerConditions();
    debugOutput();
}

void processChannels() {
    for (byte i = 0; i < CHANNELS; i++) {
        int reading = analogRead(channels[i].inputPin);
        channels[i].inputWindow.put(reading);
        channels[i].power = channels[i].sensitivity *
                            (channels[i].power - channels[i].inputWindow.get(1) + channels[i].inputWindow.get());

        if (channels[i].lastPower > maxPower && channels[i].power < channels[i].lastPower) {
            maxPower = channels[i].lastPower;
            maxIndex = i;
        }
        channels[i].lastPower = channels[i].power;
    }
}

void checkTriggerConditions() {
    if (maxIndex != -1) {
        if (channels[maxIndex].lastPower < channels[maxIndex].resetThreshold) {
            channels[maxIndex].triggered = false;
            digitalWrite(channels[maxIndex].outputPin, LOW);
            maxIndex = -1;
            maxPower = 0;
        } else if (!channels[maxIndex].triggered && maxPower >= channels[maxIndex].hitThreshold) {
            channels[maxIndex].triggered = true;
            digitalWrite(channels[maxIndex].outputPin, HIGH);
#if !DEBUG
            Keyboard.print(channels[maxIndex].outputKey);
#endif
        }
    }
}

void debugOutput() {
#if DEBUG
    for (byte i = 0; i < CHANNELS; i++) {
        Serial.print(channels[i].power);
        Serial.print(" ");
    }
    Serial.print("\n");
#endif
}
