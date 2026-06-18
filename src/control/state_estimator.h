#pragma once

struct StateVector
{
    float theta;
    float thetaDot;
    float wheelSpeed;
};

StateVector estimateState();