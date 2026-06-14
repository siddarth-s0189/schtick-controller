#include "d_balance_region.h"

#include <Arduino.h>
#include <Wire.h>
#include <math.h>

// =====================================
// TEST CONFIG
// =====================================

#define PWM_PIN   25
#define DIR_PIN_A 26
#define DIR_PIN_B 27

static const int   DISTURBANCE_PWM   = 200;
static const float SAFETY_CUTOFF_DEG = 30.0f;

// =====================================
// PHASES
//
// OBSERVE: motor off, free divergence
// REACTIVE: motor opposes current lean
// COAST:   motor off, free decay
// =====================================

enum Phase {
    PHASE_OBSERVE  = 0,
    PHASE_REACTIVE = 1,
    PHASE_COAST    = 2,
    PHASE_DONE     = 3
};

static const unsigned long PHASE_DURATION_MS[] = {
    2000,   // OBSERVE
    4000,   // REACTIVE
    1000    // COAST
};

static Phase         currentPhase = PHASE_OBSERVE;
static unsigned long phaseStart   = 0;
static bool          testFinished = false;

// =====================================
// MPU6050
// =====================================

#define MPU_ADDR 0x68

static int16_t axRaw, ayRaw, azRaw;
static int16_t gxRaw, gyRaw, gzRaw;

// =====================================
// CALIBRATION
// =====================================

static float gxBias         = 0.0f;
static float gyBias         = 0.0f;
static float gzBias         = 0.0f;
static float mountingOffset = 0.0f;

// =====================================
// DEADBAND
// =====================================

static const float DEADBAND_DEG     = 1.0f;

// =====================================
// ANGLE ESTIMATION
// =====================================

static float       angleDeg = 0.0f;
static const float ALPHA    = 0.98f;

// =====================================
// MOTOR
// =====================================

static int currentPWM = 0;

// =====================================
// TIMING
// =====================================

static unsigned long lastSampleTime = 0;
static unsigned long testStartTime  = 0;

// =====================================
// HELPERS
// =====================================

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

static void setMotorPWM(int pwm)
{
    currentPWM = pwm;

    if (pwm > 0)
    {
        digitalWrite(DIR_PIN_A, HIGH);
        digitalWrite(DIR_PIN_B, LOW);
        analogWrite(PWM_PIN, pwm);
    }
    else if (pwm < 0)
    {
        digitalWrite(DIR_PIN_A, LOW);
        digitalWrite(DIR_PIN_B, HIGH);
        analogWrite(PWM_PIN, -pwm);
    }
    else
    {
        digitalWrite(DIR_PIN_A, LOW);
        digitalWrite(DIR_PIN_B, LOW);
        analogWrite(PWM_PIN, 0);
    }
}

// =====================================
// SETUP
// =====================================

void balanceRegionSetup()
{
    Wire.begin();

    pinMode(PWM_PIN,   OUTPUT);
    pinMode(DIR_PIN_A, OUTPUT);
    pinMode(DIR_PIN_B, OUTPUT);
    setMotorPWM(0);

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
    // =====================================

    Serial.println("CALIBRATING — hold vertical and still...");

    const int CAL_SAMPLES = 200;
    float sumGx = 0, sumGy = 0, sumGz = 0;

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
    // =====================================

    readMPU();

    mountingOffset =
        atan2f((float)ayRaw, (float)azRaw)
        * 180.0f / PI;

    Serial.print("MOUNTING OFFSET: ");
    Serial.print(mountingOffset, 3);
    Serial.println(" deg");

    angleDeg = 0.0f;

    Serial.println("READY");
    Serial.println("Phases: OBSERVE(2s) -> REACTIVE(4s) -> COAST(1s)");
    Serial.println("time_ms,phase,angle_deg,omega_deg_s,pwm,ax,ay,az,gx,gy,gz");

    lastSampleTime = millis();
    testStartTime  = millis();
    phaseStart     = millis();
    currentPhase   = PHASE_OBSERVE;
}

// =====================================
// LOOP
// =====================================

void balanceRegionLoop()
{
    if (testFinished) return;

    unsigned long now = millis();

    // =====================================
    // PHASE SEQUENCER
    // =====================================

    if (currentPhase < PHASE_DONE &&
        (now - phaseStart) >= PHASE_DURATION_MS[currentPhase])
    {
        currentPhase = (Phase)(currentPhase + 1);
        phaseStart   = now;

        if (currentPhase == PHASE_DONE)
        {
            setMotorPWM(0);
            testFinished = true;
            Serial.println("TEST COMPLETE");
            return;
        }
    }

    // =====================================
    // MOTOR COMMAND BY PHASE
    // =====================================

    switch (currentPhase)
    {
        case PHASE_OBSERVE:
            // Motor off — pure free divergence
            setMotorPWM(0);
            break;

        case PHASE_REACTIVE:
            if (fabsf(angleDeg) >= SAFETY_CUTOFF_DEG)
            {
                setMotorPWM(0);
            }
            else if (fabsf(angleDeg) < DEADBAND_DEG)
            {
                // Within deadband — motor off to prevent
                // chattering as angle crosses zero
                setMotorPWM(0);
            }
            else if (angleDeg > 0)
            {
                // Leaning CCW — push CW to correct
                setMotorPWM(-DISTURBANCE_PWM);
            }
            else
            {
                // Leaning CW — push CCW to correct
                setMotorPWM(DISTURBANCE_PWM);
            }
            break;

        case PHASE_COAST:
            setMotorPWM(0);
            break;

        default:
            setMotorPWM(0);
            break;
    }

    // =====================================
    // SAMPLE AT 100 Hz
    // =====================================

    if (now - lastSampleTime >= 10)
    {
        float dt = (now - lastSampleTime) / 1000.0f;
        lastSampleTime = now;

        readMPU();

        float gxDegPerSec = (gxRaw - gxBias) / 16.4f;
        float gyDegPerSec = (gyRaw - gyBias) / 16.4f;
        float gzDegPerSec = (gzRaw - gzBias) / 16.4f;

        float gyroIntegrated = angleDeg + gxDegPerSec * dt;

        float accelAngleDeg =
            atan2f((float)ayRaw, (float)azRaw)
            * 180.0f / PI
            - mountingOffset;

        angleDeg =
            ALPHA * gyroIntegrated +
            (1.0f - ALPHA) * accelAngleDeg;

        Serial.print(now);            Serial.print(",");
        Serial.print(currentPhase);   Serial.print(",");
        Serial.print(angleDeg, 4);    Serial.print(",");
        Serial.print(gxDegPerSec, 4); Serial.print(",");
        Serial.print(currentPWM);     Serial.print(",");
        Serial.print(axRaw);          Serial.print(",");
        Serial.print(ayRaw);          Serial.print(",");
        Serial.print(azRaw);          Serial.print(",");
        Serial.print(gxRaw);          Serial.print(",");
        Serial.print(gyRaw);          Serial.print(",");
        Serial.println(gzRaw);
    }
}