%% =====================================================
% LQR CONTROLLER
%% =====================================================

Q = diag([1000 100 1]);
R = 1;

K = lqr(A,B,Q,R);

disp('LQR Gain K:')
disp(K)

Acl = A - B*K;

disp('Closed-loop poles:')
disp(eig(Acl))

%% =====================================================
% SIMULATION
%% =====================================================

dt = 0.001;
tEnd = 3.0;

t = 0:dt:tEnd;

x0 = [
    deg2rad(5)
    0
    0
];

x = zeros(3,length(t));
x(:,1) = x0;

for k = 1:length(t)-1

    xdot = Acl*x(:,k);

    x(:,k+1) = x(:,k) + xdot*dt;

end

%% =====================================================
% PLOTS
%% =====================================================

thetaDeg = rad2deg(x(1,:));

thetaDotDeg = rad2deg(x(2,:));

wheelRPM = x(3,:)*60/(2*pi);

figure
plot(t,thetaDeg,'LineWidth',2)
grid on
xlabel('Time (s)')
ylabel('Angle (deg)')
title('LQR Angle Response')

figure

subplot(3,1,1)
plot(t,thetaDeg,'LineWidth',1.5)
grid on
ylabel('\theta (deg)')

subplot(3,1,2)
plot(t,thetaDotDeg,'LineWidth',1.5)
grid on
ylabel('\omega (deg/s)')

subplot(3,1,3)
plot(t,wheelRPM,'LineWidth',1.5)
grid on
ylabel('Wheel RPM')
xlabel('Time (s)')

%% =====================================================
% SUMMARY
%% =====================================================

fprintf('\nPeak angle = %.2f deg\n',max(abs(thetaDeg)));
fprintf('Final angle = %.4f deg\n',thetaDeg(end));