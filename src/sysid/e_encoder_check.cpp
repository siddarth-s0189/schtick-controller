#include <Arduino.h>

// ====================================
// CHANGE THESE TO YOUR REAL PINS
// ====================================

#define ENCODER_A 34
#define ENCODER_B 35

// ====================================
// ENCODER VARIABLES
// ====================================

static volatile long encoderCount = 0;

static const float countsPerRev = 203.96f; // averaged estimate from 27 revs

static unsigned long lastRPMTime = 0;
static long lastCount = 0;

// ====================================
// INTERRUPT
// ====================================

static void IRAM_ATTR encoderISR()
{
    int b = digitalRead(ENCODER_B);

    if (b == HIGH)
    {
        encoderCount++;
    }
    else
    {
        encoderCount--;
    }
}

// ====================================
// SETUP
// ====================================

void encoderCheckSetup()
{
    pinMode(ENCODER_A, INPUT_PULLUP);
    pinMode(ENCODER_B, INPUT_PULLUP);

    attachInterrupt(
        digitalPinToInterrupt(ENCODER_A),
        encoderISR,
        RISING
    );

    Serial.println();
    Serial.println("=== ENCODER TEST ===");
    Serial.println("Columns:");
    Serial.println("time_ms,count,rpm");
}

// ====================================
// LOOP
// ====================================

void encoderCheckLoop()
{
    unsigned long now = millis();

    if (now - lastRPMTime >= 100)
    {
        // long currentCount = encoderCount;
        noInterrupts();
        long currentCount = encoderCount;
        interrupts();

        long deltaCount = currentCount - lastCount;

        float dt = (now - lastRPMTime) / 1000.0f;

        float revs =
            (float)deltaCount /
            countsPerRev;

        float rpm =
            (revs / dt) * 60.0f;

        Serial.print(now);
        Serial.print(",");

        Serial.print(currentCount);
        Serial.print(",");

        Serial.println(rpm);

        lastCount = currentCount;
        lastRPMTime = now;
    }
}