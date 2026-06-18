clear; clc; close all;

%% Build model

p = params();

[A,B,C,D] = build_state_space(p);

%% Eigenvalues

disp('State-space eigenvalues:')
disp(eig(A))

%% Validation

fprintf('\n=== Model Validation ===\n');

lambda_model = max(real(eig(A)));
lambda_exp   = 8.115;

err = abs(lambda_model-lambda_exp)/lambda_exp*100;

fprintf('Model lambda        = %.3f rad/s\n', lambda_model);
fprintf('Experimental lambda = %.3f rad/s\n', lambda_exp);
fprintf('Error               = %.2f %%\n', err);

if err < 10
    fprintf('Validation PASSED (<10%% error)\n');
else
    fprintf('Validation FAILED (>10%% error)\n');
end

%% Controllability

Co = [B A*B A^2*B];

r = rank(Co);

fprintf('\nControllability matrix rank = %d\n', r);

if r == 3
    fprintf('System is fully controllable.\n');
else
    fprintf('System is NOT fully controllable.\n');
end

