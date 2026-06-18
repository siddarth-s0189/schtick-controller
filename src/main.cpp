/*
    Active controller:
    PID / LQR / Limit Cycle / MPC 
    OR
    System identification (for LQR and MPC)
*/

#include "pid/pid_main.h"
// #include "executable/lqr_main.h"
// #include "executable/limit_cycle_main.h"
// #include "executable/sysid_main.h"

void setup()
{
    pidSetup();
}

void loop()
{
    pidLoop();
}