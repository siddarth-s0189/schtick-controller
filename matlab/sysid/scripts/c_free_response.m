clear;
clc;
close all;

%% =====================================================
% CONFIGURATION
%
% Set TRIAL to 2 or 3 (or 4 when you have it).
% Each trial uses different accel scaling:
%
%   Trial 2 — collected with OLD code (default
%   MPU6050 ±2g range). Scale = 16384 LSB/g.
%
%   Trial 3+ — collected with NEW code (±8g
%   range set explicitly). Scale = 4096 LSB/g.
%
% Gyro scale is 16.4 LSB/(deg/s) for ALL trials
% because ±2000 deg/s was set in both versions.
%% =====================================================

TRIAL = 5;   % <-- change this when you add a new CSV

switch TRIAL
    case 2
        filename    = 'c_free_response_5.csv';
        ACCEL_SCALE = 16384;   % ±2g, old code
    otherwise
        filename    = sprintf('c_free_response_%d.csv', TRIAL);
        ACCEL_SCALE = 4096;    % ±8g, new code
end

GYRO_SCALE = 16.4;   % LSB/(deg/s), same for all trials

%% =====================================================
% LOAD DATA
%% =====================================================

T = readtable(filename);

time     = T.time_ms;
time     = time - time(1);
time     = time / 1000;        % ms -> seconds

angleDeg = T.angle_deg;        % complementary filter output from C++
omegaDeg = T.omega_deg_s;      % bias-corrected gx rate from C++

ax = T.ax;
ay = T.ay;
az = T.az;

gx = T.gx;
gy = T.gy;
gz = T.gz;

%% =====================================================
% REMOVE BAD ROWS
%% =====================================================

valid = ...
    ~isnan(time) & ...
    ~isnan(angleDeg) & ...
    ~isnan(ax);

time     = time(valid);
angleDeg = angleDeg(valid);
omegaDeg = omegaDeg(valid);
ax = ax(valid);  ay = ay(valid);  az = az(valid);
gx = gx(valid);  gy = gy(valid);  gz = gz(valid);

%% =====================================================
% TIME STEP
%% =====================================================

dt = [0; diff(time)];

%% =====================================================
% CONVERT TO PHYSICAL UNITS
%% =====================================================

axG = ax / ACCEL_SCALE;
ayG = ay / ACCEL_SCALE;
azG = az / ACCEL_SCALE;

gxDeg = gx / GYRO_SCALE;
gyDeg = gy / GYRO_SCALE;
gzDeg = gz / GYRO_SCALE;

%% =====================================================
% SANITY CHECK: ACCELEROMETER MAGNITUDE
%
% When the pendulum is at rest |a| must be ~1g.
% If it reads 4g you have the wrong ACCEL_SCALE.
%% =====================================================

accelMag = sqrt(axG.^2 + ayG.^2 + azG.^2);

fprintf('=== Accel magnitude check ===\n');
fprintf('  mean |a| = %.3f g  (expect ~1.0)\n', mean(accelMag));
fprintf('  scale used: 1 g = %d LSB  (±%dg range)\n', ...
    ACCEL_SCALE, round(32768/ACCEL_SCALE));

%% =====================================================
% FIGURE 1 — SENSOR SATURATION CHECK
%% =====================================================

sat = ...
    abs(ax) >= 32767 | abs(ay) >= 32767 | abs(az) >= 32767 | ...
    abs(gx) >= 32767 | abs(gy) >= 32767 | abs(gz) >= 32767;

figure;
stem(time, sat);
grid on;
xlabel('Time (s)');
ylabel('Saturated?');
title('Sensor Saturation');

fprintf('\nSaturated samples: %d / %d\n', sum(sat), length(sat));

%% =====================================================
% FIGURE 2 — GYRO AXIS CONFIRMATION
%
% The pendulum rotates about the IMU X-axis (gx).
% Integrating gx must reconstruct the logged angle.
% gy and gz should stay near zero throughout.
%% =====================================================

gyroAngleX = cumsum(gxDeg .* dt);
gyroAngleY = cumsum(gyDeg .* dt);
gyroAngleZ = cumsum(gzDeg .* dt);

figure;
plot(time, angleDeg, 'k', 'LineWidth', 2);
hold on;
plot(time, gyroAngleX, 'LineWidth', 1.5);
plot(time, gyroAngleY, 'LineWidth', 1.2, 'LineStyle', '--');
plot(time, gyroAngleZ, 'LineWidth', 1.2, 'LineStyle', ':');
grid on;
legend('Logged angle (C++ CF)', 'Integrated gx', 'Integrated gy', 'Integrated gz');
xlabel('Time (s)');
ylabel('Angle (deg)');
title('Gyro Axis Confirmation — gx must track logged angle');

%% =====================================================
% FIGURE 3 — ACCELEROMETER ANGLE CANDIDATES
%
% For rotation about X, the correct formula is
% atan2(ay, az). Confirm it matches the logged
% angle in the static floor-rest region (post-impact).
%% =====================================================

accelAngleYZ = atan2d(ayG, azG);   % CORRECT for this mounting
accelAngleXZ = atan2d(axG, azG);
accelAngleXY = atan2d(axG, ayG);   % was used incorrectly in old code

figure;
plot(time, accelAngleYZ, 'LineWidth', 2);
hold on;
plot(time, accelAngleXZ, 'LineWidth', 1.2, 'LineStyle', '--');
plot(time, accelAngleXY, 'LineWidth', 1.2, 'LineStyle', ':');
plot(time, angleDeg, 'k', 'LineWidth', 1.5);
grid on;
legend( ...
    'atan2(ay,az)  ← CORRECT', ...
    'atan2(ax,az)', ...
    'atan2(ax,ay)  ← was wrong', ...
    'Logged angle');
xlabel('Time (s)');
ylabel('Angle (deg)');
title('Accelerometer Angle Candidates');

%% =====================================================
% OFFLINE COMPLEMENTARY FILTER
%
% Re-runs the fixed filter on raw sensor data.
% Seeded from accel at t=0, uses gx + atan2(ay,az).
% This is independent of whatever C++ logged.
%% =====================================================

alpha = 0.98;

cfAngle    = zeros(size(time));
cfAngle(1) = accelAngleYZ(1);

for k = 2:length(time)
    gyroPredict = cfAngle(k-1) + gxDeg(k) * dt(k);
    cfAngle(k)  = alpha * gyroPredict + (1 - alpha) * accelAngleYZ(k);
end

%% =====================================================
% FIGURE 4 — ANGLE COMPARISON
%% =====================================================

figure;
plot(time, angleDeg, 'k', 'LineWidth', 2);
hold on;
plot(time, cfAngle,    'LineWidth', 1.5);
plot(time, gyroAngleX, 'LineWidth', 1.2, 'LineStyle', '--');
grid on;
legend( ...
    'Logged angle (from C++)', ...
    'Offline CF (gx + atan2(ay,az))', ...
    'Pure gx integration');
xlabel('Time (s)');
ylabel('Angle (deg)');
title('Angle Comparison');

%% =====================================================
% FALL WINDOW DETECTION
%
% Automatically finds when the pendulum actually
% starts moving by looking for the first sample
% where |gxDeg| exceeds a threshold.
% Avoids fitting the flat holding region as part
% of the exponential, which would produce a
% falsely low lambda.
%
% fallEnd is set to just before floor impact,
% identified as when gx spikes then abruptly drops.
%% =====================================================

%% =====================================================
% FALL WINDOW DETECTION
%% =====================================================

fallThresh = 5.0;   % deg/s

fallStartIdx = find(abs(gxDeg) > fallThresh, 1, 'first');

% Use the peak of gx as fallEnd — this is the last moment
% of clean free fall before the catch transient corrupts the data
[~, peakIdx] = max(abs(gxDeg));
fallEndIdx   = peakIdx;

% Safety
fallStartIdx = max(fallStartIdx, 2);
fallEndIdx   = min(fallEndIdx, length(time) - 1);

fallStart = time(fallStartIdx);
fallEnd   = time(fallEndIdx);

fprintf('\nAuto-detected fall window: %.3f s to %.3f s\n', fallStart, fallEnd);

fallIdx = time >= fallStart & time <= fallEnd;

%% =====================================================
% FIGURE 5 — FALL EVENT ZOOM
%% =====================================================

figure;

subplot(2,1,1);
plot(time(fallIdx), angleDeg(fallIdx), 'k', 'LineWidth', 2);
hold on;
plot(time(fallIdx), cfAngle(fallIdx), 'LineWidth', 1.5);
grid on;
legend('Logged (C++)', 'Offline CF');
ylabel('Angle (deg)');
title('Fall Region');

subplot(2,1,2);
plot(time(fallIdx), gxDeg(fallIdx), 'LineWidth', 1.5);
hold on;
plot(time(fallIdx), gyDeg(fallIdx), 'LineWidth', 1.2, 'LineStyle', '--');
plot(time(fallIdx), gzDeg(fallIdx), 'LineWidth', 1.2, 'LineStyle', ':');
grid on;
legend('gx (rotation axis)', 'gy', 'gz');
xlabel('Time (s)');
ylabel('Rate (deg/s)');
title('Gyro During Fall');

%% =====================================================
% GYRO BIAS ESTIMATE FROM STATIC REGION
%
% After impact the pendulum sits still on the floor.
% True angular velocity = 0, so mean gx = residual bias.
% Noted here for reference; will matter for PID later.
%% =====================================================

staticThresh = 2.0;   % deg/s
staticIdx    = abs(gxDeg) < staticThresh & time > (fallEnd + 0.2);

if sum(staticIdx) > 10
    gxBiasEst = mean(gxDeg(staticIdx));
    fprintf('\n=== Gyro bias estimate (for future PID reference) ===\n');
    fprintf('  Residual gx bias = %.3f deg/s  (%d static samples)\n', ...
        gxBiasEst, sum(staticIdx));
    fprintf('  Phantom drift over 5s = %.2f deg\n', gxBiasEst * 5);
else
    gxBiasEst = 0;
    fprintf('\nNot enough static samples to estimate bias.\n');
end

%% =====================================================
% FIGURE 6 — EXPONENTIAL FIT
%
% Inverted pendulum near vertical diverges as:
%   theta(t) = theta0 * exp(lambda * t)
%
% lambda = sqrt(g / L_eff)
%
% Fit is done in log-space (linear regression on
% log(|angle|) vs time) using only the clean fall
% window detected above.
%% =====================================================

tFit  = time(fallIdx);
aFit  = abs(cfAngle(fallIdx));

tFit0 = tFit - tFit(1);

% Reject samples where angle is too small (noisy log)
validFit = aFit > 0.5;
tFit0    = tFit0(validFit);
aFit     = aFit(validFit);

if sum(validFit) > 5

    p      = polyfit(tFit0, log(aFit), 1);
    lambda = p(1);
    theta0 = exp(p(2));

    g_const = 9.81;
    L_eff   = g_const / lambda^2;

    fprintf('\n=== Pendulum natural dynamics ===\n');
    fprintf('  lambda (unstable pole)  = %.3f rad/s\n', lambda);
    fprintf('  theta0                  = %.3f deg\n',   theta0);
    fprintf('  Effective length L_eff  = %.3f m  (%.1f cm)\n', L_eff, L_eff*100);
    fprintf('  Approx divergence time  = 1/lambda = %.3f s\n', 1/lambda);

    tOverlay   = linspace(tFit0(1), tFit0(end), 200);
    fitOverlay = theta0 * exp(lambda * tOverlay);

    figure;
    plot(tFit0, aFit, 'w.', 'MarkerSize', 12);
    hold on;
    plot(tOverlay, fitOverlay, 'r', 'LineWidth', 2);
    grid on;
    xlabel('Time from fall start (s)');
    ylabel('|Angle| (deg)');
    title(sprintf('Exponential fit: \\lambda = %.2f rad/s, L_{eff} = %.1f cm', ...
        lambda, L_eff*100));
    legend('Data', sprintf('%.3f * exp(%.2f t)', theta0, lambda));

else
    fprintf('\nNot enough valid fall samples for exponential fit.\n');
end

%% =====================================================
% SYSID SUMMARY
%% =====================================================

fprintf('\n=== SysID Summary ===\n');
fprintf('  Trial          : %d\n',    TRIAL);
fprintf('  Accel scale    : %d LSB/g  (±%dg)\n', ACCEL_SCALE, round(32768/ACCEL_SCALE));
fprintf('  Gyro scale     : %.1f LSB/(deg/s)  (±2000 deg/s)\n', GYRO_SCALE);
fprintf('  Rotation axis  : GX\n');
fprintf('  Accel formula  : atan2(ay, az)\n');
fprintf('  CF alpha       : %.2f\n',  alpha);
fprintf('  Angle range    : %.2f to %.2f deg\n', min(cfAngle), max(cfAngle));
if exist('lambda', 'var')
    fprintf('  Unstable pole  : %.3f rad/s\n', lambda);
    fprintf('  L_eff          : %.3f m\n',     L_eff);
end