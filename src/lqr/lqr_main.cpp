#include <Arduino.h>

#include "control/state_estimator.h"
#include "lqr/lqr_balance.h"
#include "control/motor_driver.h"
#include "control/sensors.h"

float applyDeadzoneCompensation(float u)
{
    const float DEADZONE = 50.0f;
    const float EPSILON  = 5.0f;

    if (fabs(u) < EPSILON)
    {
        return 0.0f;
    }

    if (u > 0.0f)
    {
        return u + DEADZONE;
    }
    else
    {
        return u - DEADZONE;
    }
}

void lqrSetup()
{
    Serial.begin(115200);

    sensorsSetup();

    motorDriverSetup();

    Serial.println(
        "theta,thetaDot,wheelRPM,rawPWM,clippedPWM");
}

void lqrLoop()
{
    sensorsUpdate();

    StateVector x =
        estimateState();

    float rawPWM =
        computeLQR(
            x.theta,
            x.thetaDot,
            x.wheelSpeed);

    float pwm =
        applyDeadzoneCompensation(rawPWM);

    setMotorPWM(pwm);

    Serial.print(rawPWM);
    Serial.print(",");

    Serial.println(pwm);

    delay(5);
}