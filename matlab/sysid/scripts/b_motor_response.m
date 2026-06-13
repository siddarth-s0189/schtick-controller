clc;
clear;
close all;

%% =========================================
% LOAD DATA
% ==========================================

data = readtable("motor_response.csv");

t   = data.time_ms;
pwm = data.pwm;
rpm = data.rpm;

%% =========================================
% FULL EXPERIMENT
% ==========================================

figure;
plot(t,rpm,'LineWidth',1.5);
grid on;

xlabel('Time (ms)');
ylabel('RPM');

title('Motor Response Test');

%% =========================================
% FIND TEST REGIONS
% ==========================================

idx_startup = find(pwm == 255,1,'first');

idx_rev1 = find( ...
    pwm(2:end)==-255 & ...
    pwm(1:end-1)==255 , ...
    1,'first') + 1;

idx_rev2 = find( ...
    pwm(2:end)==255 & ...
    pwm(1:end-1)==-255 , ...
    1,'first') + 1;

idx_stop = find( ...
    pwm(2:end)==0 & ...
    pwm(1:end-1)==255 , ...
    1,'last') + 1;

%% =========================================
% STARTUP CHARACTERIZATION
% ==========================================

startup_t = t(idx_startup:idx_rev1-1);
startup_rpm = rpm(idx_startup:idx_rev1-1);

figure;
plot(startup_t,startup_rpm,'LineWidth',1.5);
grid on;

xlabel('Time (ms)');
ylabel('RPM');

title('Startup Response (0 -> 255)');

%% =========================================
% STEADY STATE RPM
% ==========================================

steadyRPM = mean(startup_rpm(end-20:end));

fprintf('\n');
fprintf('Steady-state RPM = %.2f\n',steadyRPM);

%% =========================================
% RISE TIME
% ==========================================

target63 = 0.632 * abs(steadyRPM);

idx63 = find(abs(startup_rpm) >= target63,1,'first');

riseTime = startup_t(idx63) - startup_t(1);

fprintf('63%% Rise Time = %.1f ms\n',riseTime);

%% =========================================
% REVERSAL #1
% ==========================================

rev1_t = t(idx_rev1:idx_rev2-1);

rev1_rpm = rpm(idx_rev1:idx_rev2-1);

figure;
plot(rev1_t,rev1_rpm,'LineWidth',1.5);
grid on;

xlabel('Time (ms)');
ylabel('RPM');

title('Full Reversal (+255 -> -255)');

%% =========================================
% ZERO CROSSING TIME
% ==========================================

crossIdx1 = find(rev1_rpm > 0,1,'first');

crossTime1 = rev1_t(crossIdx1) - rev1_t(1);

fprintf('\n');
fprintf('Reversal Crossing Time = %.1f ms\n',crossTime1);

%% =========================================
% REVERSAL #2
% ==========================================

rev2_t = t(idx_rev2:idx_stop-1);

rev2_rpm = rpm(idx_rev2:idx_stop-1);

figure;
plot(rev2_t,rev2_rpm,'LineWidth',1.5);
grid on;

xlabel('Time (ms)');
ylabel('RPM');

title('Reverse Reversal (-255 -> +255)');

crossIdx2 = find(rev2_rpm < 0,1,'first');

crossTime2 = rev2_t(crossIdx2) - rev2_t(1);

fprintf('Reverse Crossing Time = %.1f ms\n',crossTime2);

%% =========================================
% SUMMARY
% ==========================================

fprintf('\n');
fprintf('=========================\n');
fprintf('SUMMARY\n');
fprintf('=========================\n');

fprintf('Steady RPM      : %.2f\n',steadyRPM);
fprintf('Rise Time       : %.1f ms\n',riseTime);
fprintf('Cross Time (+-) : %.1f ms\n',crossTime1);
fprintf('Cross Time (-+) : %.1f ms\n',crossTime2);