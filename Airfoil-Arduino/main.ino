//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                                 Main function                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
    
void setup() {
  previousTime = micros();
  SerialOpen(SERIAL_PORT, SERIAL_BAUDRATE);
  pinMode(LED_BUILTIN, OUTPUT);

  /* Initialise the sensor */
  if (!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    digitalWrite(LED_BUILTIN, LOW);
    while (1);
  }

  // If the LED does not light up, there is an error initializing the IMU
  digitalWrite(LED_BUILTIN, HIGH);

  initPWM();
  PWMwrite(PWM_MIN, PWM_MIN);
}

void loop() {  
  sensors_event_t orientationData , angVelocityData;
  bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
  bno.getEvent(&angVelocityData, Adafruit_BNO055::VECTOR_GYROSCOPE);
  
  if(currentTime > taskTime){ //50 Hz task
    taskTime = currentTime + 20000;
    encode_protocol(orientationData.orientation.y, angVelocityData.gyro.y);
  }

  decode_protocol(&pwm1, &pwm2);

  if(pwm1 == 1500 && pwm2 == 1500){
    if(cnt < 10) cnt++;
    else{
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      cnt = 0;
    }
  }
  PWMwrite(pwm1, pwm2);
  
  currentTime = micros();
  cycleTime = currentTime - previousTime;
  previousTime = currentTime;
}
