//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                    *** only for Arduino 168/328P ***                     //
//                                                                          //
//                            D11 D3            D9  D10                     //
//                            2A  2B            1A  1B                 `    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
void initPWM(){
  DDRB |= (_BV(DDB1) | _BV(DDB2)); //set PORTB (D9, D10) as an OUTPUT (Servo) 50 hz
  
  TCCR1A |= (_BV(COM1A1) | _BV(COM1B1)); //set CompareOutputMode to D9, D10
  TCCR1A |= _BV(WGM11); TCCR1A &= ~(_BV(WGM10)); TCCR1B |= _BV(WGM13); //use Phase Correct 16 bits
  TCCR1B &= ~(_BV(CS10)); //set Prescaler to 8 bits
  ICR1 = 0x4E20; //20000
  //for SERVO
}

void PWMwrite(uint16_t PWM_Command_1, uint16_t PWM_Command_2){
  PWM_Command_1 = constrain(PWM_Command_1, PWM_MIN, PWM_MAX);
  PWM_Command_2 = constrain(PWM_Command_2, PWM_MIN, PWM_MAX);
  
  //Servo
  OCR1A = PWM_Command_1;
  OCR1B = PWM_Command_2;
}
