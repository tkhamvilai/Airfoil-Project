clear; clc; close all

%% Connect to Arduino
clear device
device = serialport("COM11",115200);
pause(3);

%% Parameter
max_itr = 1000;
theta = 0; % range -90 to 90 deg
theta_dot = 0; % rad/s
pwm1 = 1500; % range 1000 - 2000
pwm2 = 1500; % range 1000 - 2000

%% Read sensor data
disp('start')
tic
for itr = 1:max_itr
    raw_data = read(device,11,"uint8");
    data = decode_protocol(raw_data);
    if ~isempty(data.theta) && ~isempty(data.theta_dot)
        theta = data.theta
        theta_dot = data.theta_dot
    end
    encode_protocol(device, pwm1, pwm2);
%     pause(1);
end
toc

%% close serial port
clear device