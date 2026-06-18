#include "lqr_balance.h"

static constexpr float K_THETA =
    -4255.2229f;

static constexpr float K_THETA_DOT =
    -554.6996f;

static constexpr float K_WHEEL =
    -16.5485f;

float computeLQR(
    float thetaRad,
    float thetaDotRadS,
    float wheelSpeedRadS)
{
    return -(
        K_THETA     * thetaRad +
        K_THETA_DOT * thetaDotRadS +
        K_WHEEL     * wheelSpeedRadS
    );
}
