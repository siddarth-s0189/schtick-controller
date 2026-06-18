#pragma once

float lowPass(
    float previous,
    float measurement,
    float alpha);


class ComplementaryFilter
{
public:

    ComplementaryFilter(float alpha = 0.98f);

    float update(
        float accelAngleDeg,
        float gyroRateDegS,
        float dt);

    void reset(float angleDeg);

private:

    float _alpha;
    float _angleDeg;
};