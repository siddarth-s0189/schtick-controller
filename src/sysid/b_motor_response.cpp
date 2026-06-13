// ============================================
// TEST 2 : MOTOR STEP RESPONSE
//
// Measures:
// - startup characterization
// - direction symmetry
// - reversal response
// - spin-down response
//
// CSV Output:
// time_ms,pwm,rpm
// ============================================

#include <Arduino.h>

// --------------------------------------------
// MOTOR PINS
// --------------------------------------------

#define AIN1 25
#define AIN2 26
#define PWMA 27

// --------------------------------------------
// ENCODER PINS
// --------------------------------------------

#define ENCODER_A 34
#define ENCODER_B 35

// --------------------------------------------
// PWM CONFIG
// --------------------------------------------

static const int pwmChannel = 0;
static const int pwmFreq = 20000;
static const int pwmResolution = 8;

// --------------------------------------------
// ENCODER VARIABLES
// --------------------------------------------

static volatile long encoderCount = 0;

static const float countsPerRev = 203.96f;

static long lastCount = 0;
static unsigned long lastRPMTime = 0;

static float rpm = 0.0f;

// --------------------------------------------
// TEST TIMING
// --------------------------------------------

enum TestState
{
    WAIT_AT_ZERO,
    FORWARD,
    REVERSE,
    FORWARD_AGAIN,
    COAST_DOWN,
    FINISHED
};

static TestState state = WAIT_AT_ZERO;

static unsigned long stateStartTime = 0;

// --------------------------------------------
// ENCODER ISR
// --------------------------------------------

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

// --------------------------------------------
// MOTOR CONTROL
// --------------------------------------------

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

// --------------------------------------------
// SETUP
// --------------------------------------------

void motorResponseSetup()
{
    // serial

    Serial.begin(115200);

    // motor

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

    // encoder

    pinMode(ENCODER_A, INPUT_PULLUP);
    pinMode(ENCODER_B, INPUT_PULLUP);

    attachInterrupt(
        digitalPinToInterrupt(ENCODER_A),
        encoderISR,
        RISING
    );

    stateStartTime = millis();

    Serial.println();
    Serial.println("time_ms,pwm,rpm");
}

// --------------------------------------------
// RPM UPDATE
// --------------------------------------------

static void updateRPM()
{
    unsigned long now = millis();

    if (now - lastRPMTime >= 20)
    {
        noInterrupts();
        long currentCount = encoderCount;
        interrupts();

        long deltaCount =
            currentCount - lastCount;

        float dt =
            (now - lastRPMTime) / 1000.0f;

        float revs =
            (float)deltaCount /
            countsPerRev;

        rpm =
            (revs / dt) * 60.0f;

        lastCount = currentCount;
        lastRPMTime = now;
    }
}

// --------------------------------------------
// TEST SEQUENCE
// --------------------------------------------

static int commandedPWM = 0;

static void updateStateMachine()
{
    unsigned long elapsed =
        millis() - stateStartTime;

    switch (state)
    {
        case WAIT_AT_ZERO:

            commandedPWM = 0;

            if (elapsed >= 1000)
            {
                state = FORWARD;
                stateStartTime = millis();
            }

            break;

        case FORWARD:

            commandedPWM = 255;

            if (elapsed >= 2000)
            {
                state = REVERSE;
                stateStartTime = millis();
            }

            break;

        case REVERSE:

            commandedPWM = -255;

            if (elapsed >= 2000)
            {
                state = FORWARD_AGAIN;
                stateStartTime = millis();
            }

            break;

        case FORWARD_AGAIN:

            commandedPWM = 255;

            if (elapsed >= 2000)
            {
                state = COAST_DOWN;
                stateStartTime = millis();
            }

            break;

        case COAST_DOWN:

            commandedPWM = 0;

            if (elapsed >= 2000)
            {
                state = FINISHED;
                stateStartTime = millis();

                Serial.println("TEST COMPLETE");
            }

            break;

        case FINISHED:

            commandedPWM = 0;

            break;
    }

    setMotor(commandedPWM);
}

// --------------------------------------------
// LOOP
// --------------------------------------------

void motorResponseLoop()
{
    updateRPM();

    updateStateMachine();

    static unsigned long lastLog = 0;

    unsigned long now = millis();

    if (now - lastLog >= 20)
    {
        Serial.print(now);
        Serial.print(",");

        Serial.print(commandedPWM);
        Serial.print(",");

        Serial.println(rpm);

        lastLog = now;
    }
}

// // ============================================
// // TEST 2 : MOTOR STEP RESPONSE
// //
// // Measures:
// // - startup characterization
// // - direction symmetry
// // - reversal response
// // - spin-down response
// //
// // CSV Output:
// // time_ms,pwm,rpm
// // ============================================

// #include <Arduino.h>

// // --------------------------------------------
// // MOTOR PINS
// // --------------------------------------------

// #define AIN1 25
// #define AIN2 26
// #define PWMA 27

// // --------------------------------------------
// // ENCODER PINS
// // --------------------------------------------

// #define ENCODER_A 34
// #define ENCODER_B 35

// // --------------------------------------------
// // PWM CONFIG
// // --------------------------------------------

// const int pwmChannel = 0;
// const int pwmFreq = 20000;
// const int pwmResolution = 8;

// // --------------------------------------------
// // ENCODER VARIABLES
// // --------------------------------------------

// volatile long encoderCount = 0;

// const float countsPerRev = 203.96f;

// long lastCount = 0;
// unsigned long lastRPMTime = 0;

// float rpm = 0.0f;

// // --------------------------------------------
// // TEST TIMING
// // --------------------------------------------

// enum TestState
// {
//     WAIT_AT_ZERO,
//     FORWARD,
//     REVERSE,
//     FORWARD_AGAIN,
//     COAST_DOWN,
//     FINISHED
// };

// TestState state = WAIT_AT_ZERO;

// unsigned long stateStartTime = 0;

// // --------------------------------------------
// // ENCODER ISR
// // --------------------------------------------

// void IRAM_ATTR encoderISR()
// {
//     int b = digitalRead(ENCODER_B);

//     if (b == HIGH)
//     {
//         encoderCount++;
//     }
//     else
//     {
//         encoderCount--;
//     }
// }

// // --------------------------------------------
// // MOTOR CONTROL
// // --------------------------------------------

// void setMotor(int pwm)
// {
//     if (pwm > 0)
//     {
//         digitalWrite(AIN1, HIGH);
//         digitalWrite(AIN2, LOW);
//     }
//     else if (pwm < 0)
//     {
//         digitalWrite(AIN1, LOW);
//         digitalWrite(AIN2, HIGH);
//         pwm = -pwm;
//     }
//     else
//     {
//         digitalWrite(AIN1, LOW);
//         digitalWrite(AIN2, LOW);
//     }

//     ledcWrite(pwmChannel, pwm);
// }

// // --------------------------------------------
// // SETUP
// // --------------------------------------------

// void motorResponseSetup()
// {
//     // serial

//     Serial.begin(115200);

//     // motor

//     pinMode(AIN1, OUTPUT);
//     pinMode(AIN2, OUTPUT);

//     ledcSetup(
//         pwmChannel,
//         pwmFreq,
//         pwmResolution
//     );

//     ledcAttachPin(
//         PWMA,
//         pwmChannel
//     );

//     // encoder

//     pinMode(ENCODER_A, INPUT_PULLUP);
//     pinMode(ENCODER_B, INPUT_PULLUP);

//     attachInterrupt(
//         digitalPinToInterrupt(ENCODER_A),
//         encoderISR,
//         RISING
//     );

//     stateStartTime = millis();

//     Serial.println();
//     Serial.println("time_ms,pwm,rpm");
// }

// // --------------------------------------------
// // RPM UPDATE
// // --------------------------------------------

// void updateRPM()
// {
//     unsigned long now = millis();

//     if (now - lastRPMTime >= 20)
//     {
//         noInterrupts();
//         long currentCount = encoderCount;
//         interrupts();

//         long deltaCount =
//             currentCount - lastCount;

//         float dt =
//             (now - lastRPMTime) / 1000.0f;

//         float revs =
//             (float)deltaCount /
//             countsPerRev;

//         rpm =
//             (revs / dt) * 60.0f;

//         lastCount = currentCount;
//         lastRPMTime = now;
//     }
// }

// // --------------------------------------------
// // TEST SEQUENCE
// // --------------------------------------------

// int commandedPWM = 0;

// void updateStateMachine()
// {
//     unsigned long elapsed =
//         millis() - stateStartTime;

//     switch (state)
//     {
//         case WAIT_AT_ZERO:

//             commandedPWM = 0;

//             if (elapsed >= 1000)
//             {
//                 state = FORWARD;
//                 stateStartTime = millis();
//             }

//             break;

//         case FORWARD:

//             commandedPWM = 255;

//             if (elapsed >= 2000)
//             {
//                 state = REVERSE;
//                 stateStartTime = millis();
//             }

//             break;

//         case REVERSE:

//             commandedPWM = -255;

//             if (elapsed >= 2000)
//             {
//                 state = FORWARD_AGAIN;
//                 stateStartTime = millis();
//             }

//             break;

//         case FORWARD_AGAIN:

//             commandedPWM = 255;

//             if (elapsed >= 2000)
//             {
//                 state = COAST_DOWN;
//                 stateStartTime = millis();
//             }

//             break;

//         case COAST_DOWN:

//             commandedPWM = 0;

//             if (elapsed >= 2000)
//             {
//                 state = FINISHED;
//                 stateStartTime = millis();

//                 Serial.println("TEST COMPLETE");
//             }

//             break;

//         case FINISHED:

//             commandedPWM = 0;

//             break;
//     }

//     setMotor(commandedPWM);
// }

// // --------------------------------------------
// // LOOP
// // --------------------------------------------

// void motorResponseLoop()
// {
//     updateRPM();

//     updateStateMachine();

//     static unsigned long lastLog = 0;

//     unsigned long now = millis();

//     if (now - lastLog >= 20)
//     {
//         Serial.print(now);
//         Serial.print(",");

//         Serial.print(commandedPWM);
//         Serial.print(",");

//         Serial.println(rpm);

//         lastLog = now;
//     }
// }