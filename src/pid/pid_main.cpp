#include <Arduino.h>

#include "control/state_estimator.h"
#include "pid/pid_balance.h"
#include "control/motor_driver.h"
#include "control/sensors.h"

void pidSetup()
{
    Serial.begin(115200);

    sensorsSetup();

    motorDriverSetup();

    Serial.println("PID controller started");
}

void pidLoop()
{
    sensorsUpdate();

    StateVector x =
        estimateState();

    float pwm =
        computePID(
            x.theta,
            x.thetaDot);

    setMotorPWM(pwm);

    delay(5);
}