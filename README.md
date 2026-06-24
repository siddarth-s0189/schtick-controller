# SCHTICK

A reaction wheel inverted pendulum built using an ESP32, MPU6050, and custom reaction wheel assembly.

![SCHTICK Demo](pid_balance_demo.gif)

---

## Overview

SCHTICK began as an attempt to stabilize a reaction wheel inverted pendulum using modern control techniques.

The original workflow was:

1. System Identification
2. State-Space Modeling
3. LQR Control
4. PID Control
5. Limit Cycle Exploration

While simulation results were promising, real-world hardware constraints revealed several practical limitations involving actuator authority, parameter estimation uncertainty, and model mismatch.

This repository documents both the successes and failures encountered during development.

---

## Hardware

* ESP32
* MPU6050 IMU
* DC Motor + Reaction Wheel
* H-Bridge Motor Driver
* Custom 3D Printed Structure

Additional details can be found in:

* `parts/`
* `assembly/`

---

## Repository Structure

```text
docs/                   Project documentation

matlab/
├── sysid/
├── modeling/
├── lqr/
├── pid/

src/
├── sysid/
├── control/
├── lqr/
├── pid/
├── limit_cycle/
```

---

## Results

### LQR

* Successful in simulation
* Failed to consistently stabilize hardware

### PID

* Achieved short-duration balancing (~3 seconds)
* Revealed actuator authority limitations

### Limit Cycles

* Currently being explored as an alternative control strategy

---

## Documentation

Detailed project notes are available in:

* `docs/control_artifacts.md`
* `docs/system_identification.md`
* `docs/modeling.md`
* `docs/lqr_controller.md`
* `docs/pid_controller.md`
* `docs/limit_cycling.md`
* `docs/lessons_learned.md`
* `docs/future_directions.md`

---

## Key Takeaway

The hardest part of this project was not designing the controller.

It was understanding why mathematically correct controllers behaved differently once real hardware constraints entered the picture.

---

## License

MIT License
