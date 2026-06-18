#include "state_estimator.h"
#include "sensors.h"

#include <Arduino.h>
#include <math.h>

StateVector estimateState()
{
    StateVector x;

    x.theta =
        getAngleDeg() * PI / 180.0f;

    x.thetaDot =
        getGyroXDegS() * PI / 180.0f;

    x.wheelSpeed =
        getWheelRPM() * 2.0f * PI / 60.0f;

    return x;
}