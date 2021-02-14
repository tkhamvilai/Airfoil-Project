//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                     Serial Receive & Transmit Function                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

uint32_t read32(uint8_t port) {
  uint32_t t = read16(port);
  t+= (uint32_t)read16(port)<<16;
  return t;
}
uint16_t read16(uint8_t port) {
  uint16_t t = read8(port);
  t+= (uint16_t)read8(port)<<8;
  return t;
}
uint8_t read8(uint8_t port)  {
  return inBuf[indRX[port]++][port]&0xff;
}

void serialize32(uint8_t port, uint32_t a) {
  serialize8(port, (a    ) & 0xFF);
  serialize8(port, (a>> 8) & 0xFF);
  serialize8(port, (a>>16) & 0xFF);
  serialize8(port, (a>>24) & 0xFF);
}

void serialize16(uint8_t port, int16_t a) {
  serialize8(port, (a   ) & 0xFF);
  serialize8(port, (a>>8) & 0xFF);
}

void serialize8(uint8_t port, uint8_t a) {
  uint8_t t = serialHeadTX[port];
  if (++t >= TX_BUFFER_SIZE) t = 0;
  serialBufferTX[t][port] = a;
  checksum[port] ^= a;
  serialHeadTX[port] = t;
}

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                            Serial General Function                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

void UartSendData(uint8_t port) {
  #if defined(PROMINI) || defined(UNO)
    UCSR0B |= (1<<UDRIE0);
  #endif
  #if defined(MEGA)
    switch (port) {
      case 0: UCSR0B |= (1<<UDRIE0); break;
      case 1: UCSR1B |= (1<<UDRIE1); break;
      case 2: UCSR2B |= (1<<UDRIE2); break;
      case 3: UCSR3B |= (1<<UDRIE3); break;
    }
  #endif
}

void SerialWrite(uint8_t port, uint8_t c){
  serialize8(port, c);
  UartSendData(port);
}

static void inline SerialOpen(uint8_t port, uint32_t baud) {
  uint8_t h = ((F_CPU  / 4 / baud -1) / 2) >> 8;
  uint8_t l = ((F_CPU  / 4 / baud -1) / 2);
  switch (port) {
    #if defined(PROMINI) || defined(UNO)
      case 0: UCSR0A  = (1<<U2X0); UBRR0H = h; UBRR0L = l; UCSR0B |= (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0); break;
    #endif
    #if defined(MEGA)
      case 0: UCSR0A  = (1<<U2X0); UBRR0H = h; UBRR0L = l; UCSR0B |= (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0); break;
      case 1: UCSR1A  = (1<<U2X1); UBRR1H = h; UBRR1L = l; UCSR1B |= (1<<RXEN1)|(1<<TXEN1)|(1<<RXCIE1); break;
      case 2: UCSR2A  = (1<<U2X2); UBRR2H = h; UBRR2L = l; UCSR2B |= (1<<RXEN2)|(1<<TXEN2)|(1<<RXCIE2); break;
      case 3: UCSR3A  = (1<<U2X3); UBRR3H = h; UBRR3L = l; UCSR3B |= (1<<RXEN3)|(1<<TXEN3)|(1<<RXCIE3); break;
    #endif
  }
}

static void inline SerialEnd(uint8_t port) {
  switch (port) {
    #if defined(PROMINI)
      case 0: UCSR0B &= ~((1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0)|(1<<UDRIE0)); break;
    #endif
    #if defined(MEGA)
      case 0: UCSR0B &= ~((1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0)|(1<<UDRIE0)); break;
      case 1: UCSR1B &= ~((1<<RXEN1)|(1<<TXEN1)|(1<<RXCIE1)|(1<<UDRIE1)); break;
      case 2: UCSR2B &= ~((1<<RXEN2)|(1<<TXEN2)|(1<<RXCIE2)|(1<<UDRIE2)); break;
      case 3: UCSR3B &= ~((1<<RXEN3)|(1<<TXEN3)|(1<<RXCIE3)|(1<<UDRIE3)); break;
    #endif
  }
}

uint8_t SerialAvailable(uint8_t port) {
  return ((uint8_t)(serialHeadRX[port] - serialTailRX[port]))%RX_BUFFER_SIZE;
}

uint8_t SerialRead(uint8_t port) {
  uint8_t t = serialTailRX[port];
  uint8_t c = serialBufferRX[t][port];
  if (serialHeadRX[port] != t) {
    if (++t >= RX_BUFFER_SIZE) t = 0;
    serialTailRX[port] = t;
  }
  return c;
}

static void inline store_uart_in_buf(uint8_t data, uint8_t portnum) {
  uint8_t h = serialHeadRX[portnum];
  if (++h >= RX_BUFFER_SIZE) h = 0;
  if (h == serialTailRX[portnum]) return; // we did not bite our own tail?
  serialBufferRX[serialHeadRX[portnum]][portnum] = data;  
  serialHeadRX[portnum] = h;
}

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                           Serial Interrupt Function                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#if defined(PROMINI) || defined(MEGA) || defined(UNO)
  #if defined(PROMINI) || defined(UNO)
  ISR(USART_UDRE_vect) {  // Serial 0 on a PROMINI
  #endif
  #if defined(MEGA)
  ISR(USART0_UDRE_vect) { // Serial 0 on a MEGA
  #endif
    uint8_t t = serialTailTX[0];
    if (serialHeadTX[0] != t) {
      if (++t >= TX_BUFFER_SIZE) t = 0;
      UDR0 = serialBufferTX[t][0];  // Transmit next byte in the ring
      serialTailTX[0] = t;
    }
    if (t == serialHeadTX[0]) UCSR0B &= ~(1<<UDRIE0); // Check if all data is transmitted . if yes disable transmitter UDRE interrupt
  }
#endif
#if defined(MEGA)
  ISR(USART1_UDRE_vect) { // Serial 1 on a MEGA or on a PROMICRO
    uint8_t t = serialTailTX[1];
    if (serialHeadTX[1] != t) {
      if (++t >= TX_BUFFER_SIZE) t = 0;
      UDR1 = serialBufferTX[t][1];  // Transmit next byte in the ring
      serialTailTX[1] = t;
    }
    if (t == serialHeadTX[1]) UCSR1B &= ~(1<<UDRIE1);
  }
  
  ISR(USART2_UDRE_vect) { // Serial 2 on a MEGA
    uint8_t t = serialTailTX[2];
    if (serialHeadTX[2] != t) {
      if (++t >= TX_BUFFER_SIZE) t = 0;
      UDR2 = serialBufferTX[t][2];
      serialTailTX[2] = t;
    }
    if (t == serialHeadTX[2]) UCSR2B &= ~(1<<UDRIE2);
  }
  
  ISR(USART3_UDRE_vect) { // Serial 3 on a MEGA
    uint8_t t = serialTailTX[3];
    if (serialHeadTX[3] != t) {
      if (++t >= TX_BUFFER_SIZE) t = 0;
      UDR3 = serialBufferTX[t][3];
      serialTailTX[3] = t;
    }
    if (t == serialHeadTX[3]) UCSR3B &= ~(1<<UDRIE3);
  }
#endif

#if defined(PROMINI)
  ISR(USART_RX_vect)  { store_uart_in_buf(UDR0, 0); }
#endif

#if defined(MEGA)
  ISR(USART0_RX_vect)  { store_uart_in_buf(UDR0, 0); }
  ISR(USART1_RX_vect)  { store_uart_in_buf(UDR1, 1); }
  ISR(USART2_RX_vect)  { store_uart_in_buf(UDR2, 2); }
  ISR(USART3_RX_vect)  { store_uart_in_buf(UDR3, 3); }
#endif

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                             Serial Send Function                         //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

void Send(uint8_t port, char a){
  SerialWrite(port, (uint8_t)a);
}

void Send(uint8_t port, uint8_t a){
  SerialWrite(port, a);
}

void Send(uint8_t port, int16_t a){
  SerialWrite(port, a >> 8);
  SerialWrite(port, a &= 0b11111111);
}

void Send(uint8_t port, uint16_t a){
  SerialWrite(port, a >> 8);
  SerialWrite(port, a &= 0b11111111);
}

/* float and double are the same on AVR */

void Send(uint8_t port, float a){
  uint8_t b[4];
  memcpy(&b, &a, 4);
  for(int8_t i = 0; i < 4; i++){
    SerialWrite(port, b[i]);
  }  
}

void Send(uint8_t port, double a){
  uint8_t b[4];
  memcpy(&b, &a, 4);
  for(int8_t i = 0; i < 4; i++){
    SerialWrite(port, b[i]);
  }
}

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                             Serial Print Function                        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

// These functions convert number to string

void Print(uint8_t port, uint8_t a){
  String c = (String)(a);
   for(uint8_t i=0; i<c.length(); i++){
     SerialWrite(port, c[i]);
   }  
}

void Print(uint8_t port, int16_t a){
  String c = (String)(a);
   for(uint8_t i=0; i<c.length(); i++){
     SerialWrite(port, c[i]);
   }  
}

void Print(uint8_t port, uint16_t a){
  String c = (String)(a);
   for(uint8_t i=0; i<c.length(); i++){
     SerialWrite(port, c[i]);
   }  
}

void Print(uint8_t port, int32_t a){
  String c = (String)(a);
   for(uint8_t i=0; i<c.length(); i++){
     SerialWrite(port, c[i]);
   }  
}

void Print(uint8_t port, uint32_t a){
  String c = (String)(a);
   for(uint8_t i=0; i<c.length(); i++){
     SerialWrite(port, c[i]);
   }  
}

void Print(uint8_t port, float a){
  if (a > 4294967040.0) Print(port, "ovf");
  else if (a <-4294967040.0) Print (port, "ovf");
  else{
    int32_t c;
    Print(port, (int32_t)a);
    Print(port, '.');
    for(uint8_t i=0; i<7; i++){
      a*=10;
      c = (int32_t)a;
      Print(port, abs(c%10));
    } 
  }
}

void Print(uint8_t port, double a){
  if (a > 4294967040.0) Print(port, "ovf");
  else if (a <-4294967040.0) Print (port, "ovf");
  else{
    int32_t c;
    Print(port, (int32_t)a);
    Print(port, '.');
    for(uint8_t i=0; i<7; i++){
      a*=10;
      c = (int32_t)a;
      Print(port, abs(c%10));
    } 
  }
}

void Print(uint8_t port, char c){
  SerialWrite(port, c);
}

void Print(uint8_t port, String c){
   for(uint8_t i=0; i<c.length(); i++){
     SerialWrite(port, c[i]);
   }  
}

///////////////////////////////////////////////////////////////////////

void Println(uint8_t port){
   SerialWrite(port, 10); 
}

void Println(uint8_t port, uint8_t c){
   Print(port, c);
   SerialWrite(port, 10); 
}

void Println(uint8_t port, int16_t c){
   Print(port, c);
   SerialWrite(port, 10); 
}

void Println(uint8_t port, uint16_t c){
   Print(port, c);
   SerialWrite(port, 10); 
}

void Println(uint8_t port, int32_t c){
   Print(port, c);
   SerialWrite(port, 10); 
}

void Println(uint8_t port, uint32_t c){
   Print(port, c);
   SerialWrite(port, 10); 
}

void Println(uint8_t port, float c){
   Print(port, c);
   SerialWrite(port, 10); 
}

void Println(uint8_t port, double c){
   Print(port, c);
   SerialWrite(port, 10); 
}

void Println(uint8_t port, char c){
   Print(port, c);
   SerialWrite(port, 10); 
}

void Println(uint8_t port, String c){
   Print(port, c);
   SerialWrite(port, 10); 
}

///////////////////////////////////////////////////////////////////////

void PrintTab(uint8_t port){
  SerialWrite(port, 9);  
}
