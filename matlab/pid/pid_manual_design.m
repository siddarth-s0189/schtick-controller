clear;
clc;
close all;

%% =====================================================
% MANUAL PID TUNING RECORD
%% =====================================================

Kp = 0.0;
Ki = 0.0;
Kd = 0.0;

fprintf('\n');
fprintf('=====================================\n');
fprintf('CURRENT PID GAINS\n');
fprintf('=====================================\n');

fprintf('Kp = %.4f\n',Kp);
fprintf('Ki = %.4f\n',Ki);
fprintf('Kd = %.4f\n',Kd);

%% =====================================================
% TEST LOG
%% =====================================================

fprintf('\n');
fprintf('=====================================\n');
fprintf('TUNING PROCEDURE\n');
fprintf('=====================================\n');

fprintf('1. Set Ki = 0\n');
fprintf('2. Set Kd = 0\n');
fprintf('3. Increase Kp until oscillation begins\n');
fprintf('4. Reduce Kp by 10-20%%\n');
fprintf('5. Increase Kd until oscillation damps out\n');
fprintf('6. Add very small Ki if needed\n');

%% =====================================================
% OBSERVATION TEMPLATE
%% =====================================================

fprintf('\n');
fprintf('=====================================\n');
fprintf('OBSERVATIONS\n');
fprintf('=====================================\n');

fprintf('Kp too low  -> Falls immediately\n');
fprintf('Kp moderate -> Begins resisting fall\n');
fprintf('Kp too high -> Oscillates rapidly\n');

fprintf('\n');

fprintf('Kd too low  -> Overshoots repeatedly\n');
fprintf('Kd moderate -> Settles cleanly\n');
fprintf('Kd too high -> Sluggish response\n');

fprintf('\n');

fprintf('Ki too low  -> Small steady-state bias\n');
fprintf('Ki too high -> Slow oscillation / drift\n');

%% =====================================================
% SESSION NOTES
%% =====================================================

notes = {
    ''
    ''
    ''
    ''
};

disp(' ');
disp('Session Notes:');
disp(notes);