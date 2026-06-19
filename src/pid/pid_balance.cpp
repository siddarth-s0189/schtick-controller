#include "pid_balance.h"

#include <Arduino.h>

/*
 * MANUAL TUNING STARTING VALUES
 *
 * Ki = 0 initially.
 *
 * Tune Kp first.
 * Then Kd.
 * Then introduce a small Ki.
 */

static float Kp = -4200.0f;
static float Ki = 0.0f;
static float Kd = -300.0f;

float computePID(
    float thetaRad,
    float thetaDotRadS)
{
    static float integral = 0.0f;

    const float dt = 0.005f;   // 5 ms loop

    /*
     * P term
     */
    float P = Kp * thetaRad;

    /*
     * I term
     */
    integral += thetaRad * dt;

    float I = Ki * integral;

    /*
     * D term
     *
     * thetaDot already comes from the gyro/state estimator.
     */
    float D = Kd * thetaDotRadS;

    /*
     * Control output
     */
    float u = P + I + D;

    /*
     * Motor limits
     */
    u = constrain(u, -255.0f, 255.0f);

    return u;
}