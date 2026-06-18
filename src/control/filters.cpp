#include "filters.h"

float lowPass(
    float previous,
    float measurement,
    float alpha)
{
    return
        alpha * previous +
        (1.0f - alpha) * measurement;
}

ComplementaryFilter::ComplementaryFilter(float alpha)
{
    _alpha = alpha;
    _angleDeg = 0.0f;
}

void ComplementaryFilter::reset(float angleDeg)
{
    _angleDeg = angleDeg;
}

float ComplementaryFilter::update(
    float accelAngleDeg,
    float gyroRateDegS,
    float dt)
{
    float gyroPrediction =
        _angleDeg +
        gyroRateDegS * dt;

    _angleDeg =
        _alpha * gyroPrediction +
        (1.0f - _alpha) * accelAngleDeg;

    return _angleDeg;
}