function data = decode_protocol(raw_data)
% LZ protocol Arduino->MATLAB
% 11 byte protocol: char, char, float, float, byte
% 'L', 'Z', pitch angle, pitch rate, crc
    if char(raw_data(1)) == 'L' && char(raw_data(2)) == 'Z'
        crc = 0;
        for i = 1:length(raw_data)-1
            crc = bitxor(crc, raw_data(i));
        end
        if crc == raw_data(end)
            data.theta = typecast(uint8(raw_data(3:6)), 'single');
            data.theta_dot = typecast(uint8(raw_data(7:10)), 'single');
        else
            data.theta = [];
            data.theta_dot = [];
        end
    else
        data.theta = [];
        data.theta_dot = [];
    end
end