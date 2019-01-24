
 /* - PA - 22-29 - data DQ1-DQ8                                                                                            */
 /* - PC - 37-30 - address A0 - A7 */
 /* - PL - 42-49 - address A8 - A15  */
 /* - PG0 - 41  - A16  */
 /* - PB3 - 50 - S2 */
 /* - PB2 - 51 - !W */
 /* - PB1 - 52 - !S1  */
 /* - PB0 - 53 - !OE  */


byte OE = 1<<0;
byte S1 = 1<<1;
byte W  = 1<<2;
byte S2 = 1<<3;

void setup() {
  // put your setup code here, to run once:
  DDRA=B11111111;
  DDRC=B11111111;
  DDRL=B11111111;
  DDRG = DDRG | B00000001;
  DDRB = DDRB | B00001111;


  //enable chip
  PORTB &= ~S1;
  PORTB |= S2 | W | OE;
  
  Serial.begin(9600);
  Serial.println("Hello Computer");
  status();
}

void write(unsigned int address, unsigned int value) {
  DDRA=0xFF;
  DDRK=0xFF;
  PORTG &= ~1;
  PORTC = lowByte(address);
  PORTL = highByte(address); 
  PORTA = lowByte(value);
  PORTK = highByte(value);
  PORTB &= ~W;
  PORTB &= ~W;
  /* delay(1); */
  /* status(); */
  /* PORTB |= W; */
  PORTB |= W;
  DDRA = 0;
  DDRK = 0;
}
int read(unsigned int address) {
  DDRA = 0x00;
  DDRK = 0x00;
  PORTA = 0xFF;
  PORTK = 0xFF;
  PORTC = lowByte(address);
  PORTL = highByte(address); 
  PORTB &= ~OE;
  /* PORTB &= ~OE; */
  PORTB &= ~OE;
  /* delay(1); */
  int val = PINA | PINK<<8;
  /* status(); */
  PORTB |= OE;
  return val;
}

void status() {

  Serial.println("");
  Serial.println("Regs:");
  Serial.print(" PORTA: ");Serial.print(PORTA,BIN);
  Serial.print(" PORTB: ");Serial.print(PORTB,BIN);
  Serial.print(" PORTC: ");Serial.print(PORTC,BIN);
  Serial.print(" PORTG: ");Serial.print(PORTG,BIN);
  Serial.print(" PORTL: ");Serial.println(PORTL,BIN);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("");
  Serial.println("gonna 22");
  write(10, 0x1FAD);
  write(15, 0x239D);
  write(20, 0x888D);
  write(12331, 0x9076);
  unsigned int ret = read(10);
  Serial.println(ret, HEX);
  ret = read(15);
  Serial.println(ret, HEX);
  ret = read(20);
  Serial.println(ret, HEX);
  ret = read(12331);
  Serial.println(ret, HEX);

  status();
  delay(2000);
}
