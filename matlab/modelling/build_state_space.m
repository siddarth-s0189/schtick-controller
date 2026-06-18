function [A,B,C,D] = build_state_space(p)

% ==========================================
% State Matrix
% ==========================================

A = [
    0                                   1           0;
    p.m_total*p.g*p.l_com/p.Jp         0      p.Jw/(p.Jp*p.tau_motor);
    0                                   0     -1/p.tau_motor
];

% ==========================================
% Input Matrix
% ==========================================

B = [
    0;
   -p.Jw*p.motor_gain_rad/(p.Jp*p.tau_motor);
    p.motor_gain_rad/p.tau_motor
];

% ==========================================
% Output Matrix
% ==========================================

C = eye(3);

D = zeros(3,1);

end