function encode_protocol(device, pwm1, pwm2)
% LZ protocol MATALB->Arduino
% 7 byte protocol: char, char, uint16, uint16, byte
% 'L', 'Z', pwm1, pwm2, crc
    pwm1 = uint16(pwm1);
    pwm2 = uint16(pwm2);
    pwm1_b1 = uint8(bitshift(pwm1,-8));
    pwm1_b2 = uint8(bitand(pwm1,255));
    pwm2_b1 = uint8(bitshift(pwm2,-8));
    pwm2_b2 = uint8(bitand(pwm2,255));
    L = uint8('L');
    Z = uint8('Z');
    
    crc = uint8(0);
    crc = bitxor(crc, L);
    crc = bitxor(crc, Z);
    crc = bitxor(crc, pwm1_b1);
    crc = bitxor(crc, pwm1_b2);
    crc = bitxor(crc, pwm2_b1);
    crc = bitxor(crc, pwm2_b2);     
        
%     msg = [L Z pwm1_b1 pwm1_b2 pwm2_b1 pwm2_b2];
    msg = [L, Z, pwm1_b1, pwm1_b2, pwm2_b1, pwm2_b2, crc];
    write(device, msg, 'uint8');
    
%     write(device, L, 'uint8');
%     write(device, Z, 'uint8');
end