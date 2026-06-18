clear; clc;

thisFile = mfilename('fullpath');
[thisDir,~,~] = fileparts(thisFile);

addpath(fullfile(thisDir,'..','modeling'));

p = params();

[A,B,C,D] = build_state_space(p);

Q = diag([10 100 1]);
R = 100;

K = lqr(A,B,Q,R);

fprintf('\n===== LQR EXPORT =====\n\n');

fprintf('const float K_THETA     = %.4ff;\n', K(1));
fprintf('const float K_THETA_DOT = %.4ff;\n', K(2));
fprintf('const float K_WHEEL     = %.4ff;\n', K(3));

fprintf('\nRaw gain matrix:\n');
disp(K);