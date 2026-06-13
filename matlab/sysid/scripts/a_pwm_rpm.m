clear;
clc;
close all;

% Test 1 data
PWM = [
0
5
10
15
20
25
30
35
40
45
50
55
60
65
70
75
80
85
90
95
100
105
110
115
120
125
130
135
140
145
150
155
160
165
170
175
180
185
190
195
200
205
210
215
220
225
230
235
240
245
250
255
];

RPM = abs([
0.00
0.00
0.00
0.00
0.00
0.00
0.00
0.00
0.00
0.00
-28.98
-45.74
-51.19
-56.92
-62.51
-68.10
-73.40
-79.13
-84.87
-90.16
-95.75
-101.34
-106.79
-112.52
-117.67
-123.99
-129.29
-135.32
-141.35
-146.79
-152.68
-158.71
-163.86
-170.18
-175.48
-181.80
-188.13
-194.16
-200.04
-205.63
-211.22
-216.95
-222.69
-228.43
-234.02
-240.05
-246.08
-251.23
-257.26
-263.58
-269.32
-277.41
]);

%% Plot raw data

figure;
plot(PWM,RPM,'o-','LineWidth',1.5);
grid on;
xlabel('PWM');
ylabel('RPM');
title('Motor Static Map');

%% Dead zone estimate

idx = find(RPM > 1,1,'first');
deadZonePWM = PWM(idx);

fprintf('Dead zone ≈ PWM %d\n',deadZonePWM);

%% Linear region fit

linearIdx = PWM >= 50;

p = polyfit(PWM(linearIdx),RPM(linearIdx),1);

motorGain = p(1);
offset = p(2);

fprintf('Motor gain = %.4f RPM/PWM\n',motorGain);
fprintf('Offset = %.4f RPM\n',offset);

rpmFit = polyval(p,PWM);

figure;
plot(PWM,RPM,'o');
hold on;
plot(PWM,rpmFit,'LineWidth',2);
grid on;
xlabel('PWM');
ylabel('RPM');
title('Linear Fit');
legend('Measured','Linear Fit');

%% Saturation check

slope = diff(RPM)./diff(PWM);

figure;
plot(PWM(2:end),slope,'LineWidth',1.5);
grid on;
xlabel('PWM');
ylabel('dRPM/dPWM');
title('Local Slope');
