#include "motor_driver.h"

#include <Arduino.h>

static constexpr int AIN1 = 25;
static constexpr int AIN2 = 26;
static constexpr int PWMA = 27;

static constexpr int PWM_CHANNEL = 0;
static constexpr int PWM_FREQ = 20000;
static constexpr int PWM_RES = 8;

void motorDriverSetup()
{
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);

    ledcSetup(
        PWM_CHANNEL,
        PWM_FREQ,
        PWM_RES);

    ledcAttachPin(
        PWMA,
        PWM_CHANNEL);

    setMotorPWM(0);
}

void setMotorPWM(float pwm)
{
    int command = (int)pwm;

    command = constrain(command,-255,255);

    if(command > 0)
    {
        digitalWrite(AIN1,HIGH);
        digitalWrite(AIN2,LOW);

        ledcWrite(
            PWM_CHANNEL,
            command);
    }
    else if(command < 0)
    {
        digitalWrite(AIN1,LOW);
        digitalWrite(AIN2,HIGH);

        ledcWrite(
            PWM_CHANNEL,
            -command);
    }
    else
    {
        digitalWrite(AIN1,LOW);
        digitalWrite(AIN2,LOW);

        ledcWrite(
            PWM_CHANNEL,
            0);
    }
}