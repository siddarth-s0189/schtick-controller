#include "c_free_response.h"

#include <Arduino.h>
#include <Wire.h>
#include <math.h>

static const unsigned long TEST_DURATION_MS = 5000;
static unsigned long testStartTime = 0;
static bool testFinished          = false;

#define MPU_ADDR 0x68

static int16_t axRaw, ayRaw, azRaw;
static int16_t gxRaw, gyRaw, gzRaw;

static float gxBias = 0.0f;
static float gyBias = 0.0f;
static float gzBias = 0.0f;

static float angleDeg          = 0.0f;
static float mountingOffsetDeg = 0.0f;
static const float ALPHA       = 0.98f;

static unsigned long lastSampleTime = 0;

static void readMPU()
{
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 14, true);

    axRaw = (Wire.read() << 8) | Wire.read();
    ayRaw = (Wire.read() << 8) | Wire.read();
    azRaw = (Wire.read() << 8) | Wire.read();
    Wire.read(); Wire.read();
    gxRaw = (Wire.read() << 8) | Wire.read();
    gyRaw = (Wire.read() << 8) | Wire.read();
    gzRaw = (Wire.read() << 8) | Wire.read();
}

void pendulumFreeResponseSetup()
{
    Wire.begin();

    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B); Wire.write(0x00);
    Wire.endTransmission(true);

    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1B); Wire.write(0x18);
    Wire.endTransmission(true);

    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1C); Wire.write(0x10);
    Wire.endTransmission(true);

    delay(100);

    // =====================================
    // GYRO BIAS CALIBRATION
    // Hold craft at true vertical — still.
    // =====================================

    Serial.println("CALIBRATING — hold craft vertical and still...");

    const int CAL_SAMPLES = 200;
    float sumGx = 0.0f, sumGy = 0.0f, sumGz = 0.0f;

    for (int i = 0; i < CAL_SAMPLES; i++)
    {
        readMPU();
        sumGx += (float)gxRaw;
        sumGy += (float)gyRaw;
        sumGz += (float)gzRaw;
        delay(10);
    }

    gxBias = sumGx / (float)CAL_SAMPLES;
    gyBias = sumGy / (float)CAL_SAMPLES;
    gzBias = sumGz / (float)CAL_SAMPLES;

    Serial.print("BIAS: gx=");
    Serial.print(gxBias / 16.4f, 3);
    Serial.print(" gy=");
    Serial.print(gyBias / 16.4f, 3);
    Serial.print(" gz=");
    Serial.print(gzBias / 16.4f, 3);
    Serial.println(" deg/s");

    // =====================================
    // MOUNTING OFFSET CALIBRATION
    //
    // Craft is still at true vertical here.
    // Whatever the accel reads right now IS
    // the IMU tilt error — store and subtract
    // every sample so true vertical = 0 deg.
    // =====================================

    readMPU();

    mountingOffsetDeg =
        atan2f((float)ayRaw, (float)azRaw)
        * 180.0f / PI;

    Serial.print("MOUNTING OFFSET: ");
    Serial.print(mountingOffsetDeg, 3);
    Serial.println(" deg");

    // Seed filter at zero (offset already removed)
    angleDeg = 0.0f;

    Serial.println("READY — release when set");
    Serial.println("time_ms,angle_deg,omega_deg_s,ax,ay,az,gx,gy,gz");

    lastSampleTime = millis();
    testStartTime  = millis();
}

void pendulumFreeResponseLoop()
{
    if (testFinished) return;

    if (millis() - testStartTime >= TEST_DURATION_MS)
    {
        Serial.println("TEST COMPLETE");
        testFinished = true;
        return;
    }

    unsigned long now = millis();

    if (now - lastSampleTime >= 10)
    {
        float dt = (now - lastSampleTime) / 1000.0f;
        lastSampleTime = now;

        readMPU();

        float gxDegPerSec = (gxRaw - gxBias) / 16.4f;
        float gyDegPerSec = (gyRaw - gyBias) / 16.4f;
        float gzDegPerSec = (gzRaw - gzBias) / 16.4f;

        float gyroIntegrated = angleDeg + gxDegPerSec * dt;

        // Subtract mounting offset so true vertical = 0
        float accelAngleDeg =
            atan2f((float)ayRaw, (float)azRaw)
            * 180.0f / PI
            - mountingOffsetDeg;

        angleDeg =
            ALPHA * gyroIntegrated +
            (1.0f - ALPHA) * accelAngleDeg;

        Serial.print(now);            Serial.print(",");
        Serial.print(angleDeg, 4);    Serial.print(",");
        Serial.print(gxDegPerSec, 4); Serial.print(",");
        Serial.print(axRaw);          Serial.print(",");
        Serial.print(ayRaw);          Serial.print(",");
        Serial.print(azRaw);          Serial.print(",");
        Serial.print(gxRaw);          Serial.print(",");
        Serial.print(gyRaw);          Serial.print(",");
        Serial.println(gzRaw);
    }
}