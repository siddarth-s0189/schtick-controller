#include "sensors.h"
#include "filters.h"

#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#define MPU_ADDR 0x68

#define ENCODER_A 34
#define ENCODER_B 35

static const float COUNTS_PER_REV = 203.96f;

static volatile long encoderCount = 0;

static int16_t axRaw;
static int16_t ayRaw;
static int16_t azRaw;

static int16_t gxRaw;
static int16_t gyRaw;
static int16_t gzRaw;

static float gxBias = 0.0f;

static float mountingOffsetDeg = 0.0f;

static float gyroXDegS = 0.0f;

static float wheelRPM = 0.0f;

static unsigned long lastUpdateMs = 0;

static long lastEncoderCount = 0;

static ComplementaryFilter compFilter(0.98f);

static float angleDeg = 0.0f;

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

static void readMPU()
{
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);
    Wire.endTransmission(false);

    Wire.requestFrom(MPU_ADDR, 14, true);

    axRaw = (Wire.read() << 8) | Wire.read();
    ayRaw = (Wire.read() << 8) | Wire.read();
    azRaw = (Wire.read() << 8) | Wire.read();

    Wire.read();
    Wire.read();

    gxRaw = (Wire.read() << 8) | Wire.read();
    gyRaw = (Wire.read() << 8) | Wire.read();
    gzRaw = (Wire.read() << 8) | Wire.read();
}

void sensorsSetup()
{
    Wire.begin();

    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B);
    Wire.write(0x00);
    Wire.endTransmission(true);

    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1B);
    Wire.write(0x18);
    Wire.endTransmission(true);

    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1C);
    Wire.write(0x10);
    Wire.endTransmission(true);

    pinMode(ENCODER_A, INPUT_PULLUP);
    pinMode(ENCODER_B, INPUT_PULLUP);

    attachInterrupt(
        digitalPinToInterrupt(ENCODER_A),
        encoderISR,
        RISING);

    delay(100);

    float sum = 0.0f;

    for(int i = 0; i < 200; i++)
    {
        readMPU();

        sum += gxRaw;

        delay(5);
    }

    gxBias = sum / 200.0f;

    readMPU();

    mountingOffsetDeg =
        atan2f(
            (float)ayRaw,
            (float)azRaw)
        * 180.0f / PI;

    compFilter.reset(0.0f);

    lastUpdateMs = millis();
}

void sensorsUpdate()
{
    unsigned long now = millis();

    float dt =
        (now - lastUpdateMs) /
        1000.0f;

    if(dt <= 0.0f)
        return;

    lastUpdateMs = now;

    readMPU();

    gyroXDegS =
        (gxRaw - gxBias) /
        16.4f;

    float accelAngleDeg =
        atan2f(
            (float)ayRaw,
            (float)azRaw)
        * 180.0f / PI
        - mountingOffsetDeg;

    angleDeg =
        compFilter.update(
            accelAngleDeg,
            gyroXDegS,
            dt);

    noInterrupts();
    long currentCount = encoderCount;
    interrupts();

    long deltaCount =
        currentCount -
        lastEncoderCount;

    lastEncoderCount =
        currentCount;

    float revs =
        deltaCount /
        COUNTS_PER_REV;

    wheelRPM =
        (revs / dt) * 60.0f;
}

float getAngleDeg()
{
    return angleDeg;
}

float getGyroXDegS()
{
    return gyroXDegS;
}

float getWheelRPM()
{
    return wheelRPM;
}

// MPU6050 reading
// complementary filter
// encoder reading
// rpm calculation