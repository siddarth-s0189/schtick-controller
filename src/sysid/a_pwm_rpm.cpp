#include <Arduino.h>

// =====================================================
// MOTOR PINS
// =====================================================

#define AIN1 25
#define AIN2 26
#define PWMA 27

// =====================================================
// ENCODER PINS
// =====================================================

#define ENCODER_A 34
#define ENCODER_B 35

// =====================================================
// ENCODER PARAMETERS
// =====================================================

static volatile long encoderCount = 0;

static const float countsPerRev = 203.96f;

// =====================================================
// PWM PARAMETERS
// =====================================================

static const int pwmChannel = 0;
static const int pwmFreq = 20000;
static const int pwmResolution = 8;

// =====================================================
// TEST PARAMETERS
// =====================================================

static const int pwmSteps[] =
{
    0,
    5,
    10,
    15,
    20,
    25,
    30,
    35,
    40,
    45,
    50,
    55,
    60,
    65,
    70,
    75,
    80,
    85,
    90,
    95,
    100,
    105,
    110,
    115,
    120,
    125,
    130,
    135,
    140,
    145,
    150,
    155,
    160,
    165,
    170,
    175,
    180,
    185,
    190,
    195,
    200,
    205,
    210,
    215,
    220,
    225,
    230,
    235,
    240,
    245,
    250,
    255
};

static const int numSteps =
    sizeof(pwmSteps) /
    sizeof(pwmSteps[0]);

static int currentStep = 0;

static unsigned long stepStartTime = 0;

static bool resultPrinted = false;

// =====================================================
// ENCODER ISR
// =====================================================

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

// =====================================================
// MOTOR FUNCTION
// =====================================================

static void setMotor(int pwm)
{
    if (pwm > 0)
    {
        digitalWrite(AIN1, HIGH);
        digitalWrite(AIN2, LOW);
    }
    else if (pwm < 0)
    {
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, HIGH);

        pwm = -pwm;
    }
    else
    {
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, LOW);
    }

    ledcWrite(pwmChannel, pwm);
}

// =====================================================
// SETUP
// =====================================================

void pwmSetup()
{
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);

    ledcSetup(
        pwmChannel,
        pwmFreq,
        pwmResolution
    );

    ledcAttachPin(
        PWMA,
        pwmChannel
    );

    pinMode(
        ENCODER_A,
        INPUT_PULLUP
    );

    pinMode(
        ENCODER_B,
        INPUT_PULLUP
    );

    attachInterrupt(
        digitalPinToInterrupt(ENCODER_A),
        encoderISR,
        RISING
    );

    Serial.println();
    Serial.println("PWM,RPM");

    stepStartTime = millis();

    setMotor(
        pwmSteps[currentStep]
    );
}

// =====================================================
// LOOP
// =====================================================

void pwmLoop()
{
    unsigned long now = millis();

    // ----------------------------------
    // After 2 sec:
    // measure RPM
    // ----------------------------------

    if (
        !resultPrinted &&
        (now - stepStartTime >= 2000)
    )
    {
        noInterrupts();
        long counts = encoderCount;
        encoderCount = 0;
        interrupts();

        float revs =
            (float)counts /
            countsPerRev;

        float rpm =
            (revs / 2.0f) *
            60.0f;

        Serial.print(
            pwmSteps[currentStep]
        );

        Serial.print(",");

        Serial.println(rpm);

        resultPrinted = true;
    }

    // ----------------------------------
    // After 3 sec:
    // move to next PWM
    // ----------------------------------

    if (
        now - stepStartTime >= 3000
    )
    {
        currentStep++;

        if (
            currentStep >= numSteps
        )
        {
            setMotor(0);

            Serial.println();
            Serial.println(
                "TEST COMPLETE"
            );

            while (true)
            {
                delay(1000);
            }
        }

        noInterrupts();
        encoderCount = 0;
        interrupts();

        setMotor(
            pwmSteps[currentStep]
        );

        stepStartTime = now;

        resultPrinted = false;
    }
}
