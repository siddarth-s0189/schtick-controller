 ========================= SUMMARY ========================= 
## Test A PWM-RPM Relationship
 ### Goal: 
 1. Determine relationship between RPM at motor shaft and the PWM input supplied as current. Ideally, the relationship must be a linear fit: y = mx + b
 2. Determine motor dead zone (initial region where PWM increases but RPM is still at 0), motor gain (ie.'m' = rate at which RPM increases with PWM), saturation point (point at which motor gain starts to decrease)
 ### Procedure:
 Hold pendulum still as PWM is increased incrementally and encoder reads counts (previous experiments allowed us to determine estimate of counts/revolution).
 ### Results:
 Dead zone ≈ PWM 50 
 Motor gain = 1.1580 RPM/PWM 
 Offset = -20.3859 RPM
 Saturation: not found
 ### Figure Analysis:
 1. Linear Fit: Orange line is the best-fit straight line, Blue line is the plotted data. After initial dead zone, motor gain matches near-perfectly with the optimal linear fit. This validates the y = mx + b relationshiop very well. 
 2. Local Slope: Graph plots the derivative of RPM with respect to PWM against PWM. Let's us see the variation in motor gain zoomed in. After initial dead zone wake (huge spike), motor gain settles and does not drop at the end, which further confirms the absence of saturation within the PWM limits. 

## Test B Motor Response
 ### Goal:
 1. Startup characterization: allows us the understand how startup from 0 PWM influences motor response as opposed to when the motor/wheel already has angular momentum. 
 2. Directly estimate maximum available corrective torque by reversing from 255 to -255 PWM (the two extremes) and vice versa.
 3. Doing the full reversal in both opposite directions allows us to infer directional assymmetry if any. 
 ### Procedure:
 Hold pendulum still in fixed position, only wheel must move freely. Motor starts and rises PWM to -255, does full rise to +255 and reverses back to -255. Then, coasts down back to 0 PWM. PWM delta rate constant. Once wheel settles, experiment is over. Point is to observe how RPM changes with time when PWM change over time is kept at a constant rate, aka motor response. 
 ### Results:
 Steady-state RPM : -276.67 
 63% Rise Time : 300.0 ms 
 Cross Time (+-) : 140.0 ms 
 Cross Time (-+) : 160.0 ms
 Saturation (confirmed verdict): not present for practical purposes
 ### Figure Analysis:
 1. Full Response: provides a bird's eye view of the entire experiment. Startup from 0 to -255 PWM, then full reversal from -255 to 255 PWM and opposite reversal back to -255 PWM. Then, coast down back to 0 PWM. PWM is always incrementally (by 5) changed at a constant rate. Goal is to observe how RPM changes with time, as motor response is a factor. 
 2. Startup Characterization: ideally for a first-order motor, RPM at 63.2% power = 0.632 * Steady-state RPM. Plugging in -276.67, we should hit 174.8 RPM approx. at 63.2% power (1 - 1/e is where the value comes from). Inferred from the graph, -175 rpm is achieved 300ms after startup. Hence time constant τ = 0.3s. While slow and non-ideal for a self-stabilizing inverted pendulum, it is due to the heavy and high MOI wheel attached to the motor. 
 3. Direction asymmetry is present but minimal. Difference in cross time in full-reversals in opposite directions is 20ms. Relative error is approx. 13%.
 4. Graphs suggest that reversal is not linear; motor torque first halts existing momentum and then, accelerates wheel opposite direction.

## Test C Pendulum Free Response
 ### Goal:
 1. Estimate unstable pole λ, which quantifies how quickly the pendulum diverges from upright equilibrium.
 2. Estimate the effective pendulum length L_eff from experimental data. L_eff is not the geometric pivot-COM distance, it is instead the equivalent simple-pendulum length that reproduces the measured dynamics.
 3. Validate the IMU angle estimation pipeline (accelerometer scaling, gyro scaling, complementary filter, and calibration).
 ### Procedure:
 Hold pendulum still at a small angle (0-5 degrees) close to vertical as calibration occurs, then let it fall until it reaches around 25-35 degrees when you catch it with your hand without letting it impact on the ground. Ensure no obstruction while it is falling or any momentum introduced by hand as you release it. Goal is to observe its natural free fall. Once you catch it, keep the pendulum still until the timer runs out.
 ### Results:
 Mean accelerometer magnitude: 1.024 g
 Accelerometer saturation events: 0 / 499 samples
 Residual gyro bias: −1.287 deg/s
 Estimated drift over 5 s: −6.44 deg
 Auto-detected fall window: 0.850–1.270 s
 Initial angle (θ₀): 0.752°
 Unstable pole (λ): 8.115 rad/s
 Effective length (L_eff): 0.149 m
 Characteristic divergence time (1/λ): 0.123 s
 ### Figure Analysis:

## Test 4 Balance Region 
 ### Goal:
 1. Verify that reaction-wheel torque produces corrective motion in the expected direction.
 2. Estimate corrective authority near the upright operating region.
 3. Observe behaviour during sustained corrective action.
 4. Assess suitability of the actuator for closed-loop stabilization.
 ### Procedure:
 This experiment uses a HOLD-REACT-COAST serial phase approach. In the HOLD phase, you must keep the pendulum appromixately upright. Donot keep it rigid but hold as loose as you can while maintaining its vertical positioning. Just as it is about to transition into REACT phase, let the pendulum fall in whatever direction is it naturally leaning at, then stop it around 20-30 degrees by catching it with your hand. Let the rest of the expeirment complete. If motor authority is strong enough, pendulum might rise back to vertical, or it can visually appear not to fight the fall at all. If the latter occurs (as it did for me), it does not necessarily mean something is wrong. It's still good data.  
 ### Results: 
 Disturbance PWM: 200
 Open-loop divergence coefficient (hold phase): 1.497 s⁻¹
 Time to reach 10° from 1°: 1.538 s
 Initial corrective angular acceleration: −39.60 deg/s²
 Peak angular velocity during correction: 26.62 deg/s
 Corrective authority: −0.1980 deg/s²/PWM
 ### Figure Analysis: