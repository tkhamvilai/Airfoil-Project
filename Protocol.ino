//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                        LZ Protocol (Arduino->MATLAB)                     //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

// 11 byte protocol: char, char, float, float, byte
// 'L', 'Z', pitch angle, pitch rate, crc

void encode_protocol(double theta, double theta_dot){ // float is the same as double for AVR
  uint8_t crc = 0;
  crc ^= uint8_t('L');
  crc ^= uint8_t('Z');
  uint8_t b[sizeof(double)];
  memcpy(&b, &theta, sizeof(double));
  for(int8_t i = 0; i < sizeof(double); i++){
    crc ^= b[i];
  }
  memcpy(&b, &theta_dot, sizeof(double));
  for(int8_t i = 0; i < sizeof(double); i++){
    crc ^= b[i];
  }     
  Send(SERIAL_PORT, 'L'); 
  Send(SERIAL_PORT, 'Z'); 
  Send(SERIAL_PORT, theta);
  Send(SERIAL_PORT, theta_dot);
  Send(SERIAL_PORT, crc);
}

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                        LZ Protocol (MATLAB->Arduino)                     //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

// 7 byte protocol: char, char, uint16, uint16, byte
// 'L', 'Z', pwm1, pwm2, crc

void decode_protocol(uint16_t* data1, uint16_t* data2){ 
 while(SerialAvailable(SERIAL_PORT)){
    uint8_t c = SerialRead(SERIAL_PORT);    
    if(state == 0 && c == uint8_t('L')){
      state++;
      crc ^= c;
    }
    else if (state == 1 && c == uint8_t('Z')){
      state++;
      crc ^= c;
    }
    else if(state == 2){
      state++;
      crc ^= c;
      tmp1 |= (c << 8);
    }
    else if(state == 3){
      state++;
      crc ^= c;
      tmp1 |= c;
    }
    else if(state == 4){
      state++;
      crc ^= c;
      tmp2 |= (c << 8);
    }
    else if(state == 5){
      state++;
      crc ^= c;
      tmp2 |= c;
    }
    else if(state == 6 && crc == c){      
      state = 0;
      crc = 0;      
      *data1 = tmp1;
      *data2 = tmp2;    
      tmp1 = 0;
      tmp2 = 0;  
    }
    else{
      *data1 = 0;
      *data2 = 0;
      state = 0;
      crc = 0;
      tmp1 = 0;
      tmp2 = 0;      
    }
  } 
}
