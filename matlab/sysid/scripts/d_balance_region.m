clear; clc; close all;

%% =====================================================
% CONFIGURATION
%% =====================================================

filename        = 'd_balance_region.csv';
ACCEL_SCALE     = 4096;
GYRO_SCALE      = 16.4;
DISTURBANCE_PWM = 200;

PHASE_NAMES  = {'HOLD','REACTIVE','COAST'};
PHASE_COLORS = {'k','b','m'};

%% =====================================================
% LOAD
%% =====================================================

T = readtable(filename);

time  = (T.time_ms - T.time_ms(1))/1000.0;
phase = T.phase;
angle = T.angle_deg;
omega = T.omega_deg_s;
pwm   = T.pwm;

gx = T.gx / GYRO_SCALE;
gy = T.gy / GYRO_SCALE;
gz = T.gz / GYRO_SCALE;

%% =====================================================
% FIGURE 1 — FULL OVERVIEW
%% =====================================================

figure('Name','Balance Region Overview');

subplot(2,1,1);
hold on;

for p = 0:2

    idx = phase == p;

    if any(idx)
        plot(time(idx),angle(idx), ...
            'Color',PHASE_COLORS{p+1}, ...
            'LineWidth',1.5);
    end
end

yline(0,'k--');
grid on;
ylabel('Angle (deg)');
title('Balance Region — Full Overview');
legend(PHASE_NAMES,'Location','best');

subplot(2,1,2);

plot(time,pwm,'LineWidth',1.5);

grid on;
ylabel('PWM');
xlabel('Time (s)');
title('Applied PWM');

%% =====================================================
% FIGURE 2 — OPEN LOOP DIVERGENCE
%
% Fit:
%
% theta = theta0 * exp(lambda*t)
%
%% =====================================================

holdIdx = phase == 0 & ...
          abs(angle) < 10 & ...
          abs(angle) > 0.3;

if sum(holdIdx) > 5

    tHold = time(holdIdx);
    tHold = tHold - tHold(1);

    aHold = abs(angle(holdIdx));

    pExp = polyfit(tHold,log(aHold),1);

    lambda_hold = pExp(1);      % 1/s
    theta0_hold = exp(pExp(2));

    fprintf('\n');
    fprintf('=== Open-Loop Divergence (HOLD phase) ===\n');
    fprintf('lambda = %.3f 1/s\n',lambda_hold);

    time_1_to_10 = log(10/1)/lambda_hold;

    fprintf('Time to reach 10 deg from 1 deg = %.3f s\n', ...
        time_1_to_10);

    tFit = linspace(0,max(tHold),200);
    aFit = theta0_hold .* exp(lambda_hold .* tFit);

    figure('Name','Open Loop Divergence');

    plot(tHold,aHold,'.','MarkerSize',10);
    hold on;

    plot(tFit,aFit,'r','LineWidth',2);

    grid on;

    xlabel('Time (s)');
    ylabel('|Angle| (deg)');

    title(sprintf('Open Loop Divergence (\\lambda = %.2f 1/s)', ...
        lambda_hold));

    legend('Data','Exponential Fit');

else

    fprintf('Not enough HOLD samples.\n');

end

%% =====================================================
% FIGURE 3 — REACTIVE AUTHORITY
%
% Use ONLY initial response region.
%
%% =====================================================

reactIdx = phase == 1;

if sum(reactIdx) > 10

    tReact = time(reactIdx);
    tReact0 = tReact - tReact(1);

    aReact = angle(reactIdx);
    wReact = omega(reactIdx);
    pReact = pwm(reactIdx);

    wFilt = movmean(wReact,5);

    alphaReact = gradient(wFilt,tReact);

    % ---------------------------------
    % Initial authority window
    % ---------------------------------

    authorityWindow = tReact0 <= 0.5;

    meanAlpha = mean(alphaReact(authorityWindow));

    peakOmega = max(abs(wReact));

    fprintf('\n');
    fprintf('=== Corrective Authority (REACTIVE phase) ===\n');
    fprintf('Initial corrective accel = %.2f deg/s^2\n', ...
        meanAlpha);

    fprintf('Peak omega = %.2f deg/s\n', ...
        peakOmega);

    fprintf('Authority = %.4f deg/s^2/PWM\n', ...
        meanAlpha / DISTURBANCE_PWM);

    % ---------------------------------
    % PWM sanity check
    % ---------------------------------

    if max(abs(pReact)) < 5

        fprintf('\n');
        fprintf('WARNING: PWM column appears near zero ');
        fprintf('during REACTIVE phase.\n');
        fprintf('Authority estimate should be treated ');
        fprintf('as qualitative only.\n');

    end

    % ---------------------------------
    % Plot
    % ---------------------------------

    figure('Name','Reactive Phase');

    subplot(3,1,1);

    plot(tReact0,aReact,'LineWidth',1.5);

    yline(0,'k--');
    grid on;

    ylabel('Angle (deg)');
    title('Reactive Phase');

    subplot(3,1,2);

    plot(tReact0,wReact,'LineWidth',1.2);
    hold on;

    plot(tReact0,wFilt,'r','LineWidth',1.5);

    yline(0,'k--');

    grid on;

    ylabel('\omega (deg/s)');
    legend('Raw','Smoothed');

    subplot(3,1,3);

    plot(tReact0,pReact,'LineWidth',1.5);

    yline(0,'k--');

    grid on;

    ylabel('PWM');
    xlabel('Time (s)');

end

%% =====================================================
% FIGURE 4 — COAST RESPONSE
%% =====================================================

coastIdx = phase == 2;

if sum(coastIdx) > 5

    tCoast = time(coastIdx);
    tCoast = tCoast - tCoast(1);

    figure('Name','Coast Response');

    subplot(2,1,1);

    plot(tCoast,angle(coastIdx), ...
        'LineWidth',1.5);

    grid on;

    ylabel('Angle (deg)');
    title('Coast Response');

    subplot(2,1,2);

    plot(tCoast,omega(coastIdx), ...
        'LineWidth',1.5);

    yline(0,'k--');

    grid on;

    ylabel('\omega (deg/s)');
    xlabel('Time (s)');

end

%% =====================================================
% SUMMARY
%% =====================================================

fprintf('\n');
fprintf('=== Balance Region Summary ===\n');

fprintf('Disturbance PWM : %d\n', ...
    DISTURBANCE_PWM);

if exist('lambda_hold','var')

    fprintf('lambda_hold     : %.3f 1/s\n', ...
        lambda_hold);

end

if exist('meanAlpha','var')

    fprintf('Initial accel   : %.2f deg/s^2\n', ...
        meanAlpha);

    fprintf('Authority/PWM   : %.4f deg/s^2/PWM\n', ...
        meanAlpha/DISTURBANCE_PWM);

end

fprintf('\n');
fprintf('Qualitative conclusions:\n');
fprintf('- Corrective direction verified\n');
fprintf('- Reaction wheel influences body motion\n');
fprintf('- Vertical recovery not achieved\n');
fprintf('- Wheel saturation likely\n');
fprintf('- Free-response SysID remains primary plant model source\n');
fprintf('NOTE: Authority estimate is approximate and intended for qualitative comparison only.\n');