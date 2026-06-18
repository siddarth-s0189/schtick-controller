function dx = nonlinear_dynamics(~, x, u, p)

% ==========================================
% States
% ==========================================

theta     = x(1);     % rad
theta_dot = x(2);     % rad/s
omega_w   = x(3);     % rad/s

% ==========================================
% PWM Deadzone
% ==========================================

if abs(u) < p.deadzone_pwm
    u_eff = 0;
else
    u_eff = sign(u) * (abs(u) - p.deadzone_pwm);
end

% ==========================================
% Wheel Dynamics
% First-order motor model
% ==========================================

omega_w_dot = ...
    (-omega_w + p.motor_gain_rad*u_eff) ...
    / p.tau_motor;

% ==========================================
% Reaction Torque
% ==========================================

tau_reaction = -p.Jw * omega_w_dot;

% ==========================================
% Pendulum Dynamics
% ==========================================

theta_ddot = ...
    ( p.m_total * p.g * p.l_com * sin(theta) ...
      + tau_reaction ) ...
    / p.Jp;

% ==========================================
% State Derivative
% ==========================================

dx = [
    theta_dot;
    theta_ddot;
    omega_w_dot
];

end