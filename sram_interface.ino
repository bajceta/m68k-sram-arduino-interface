/* - PA - 22-29 - data DQ1-DQ8                                                                                            */
/* - PC - 37-30 - address A0 - A7 */
/* - PL - 42-49 - address A8 - A15  */
/* - PK - data high byt byte */
/* - PG0 - 41  - A16  */
/* - PB3 - 50 - !WU */
/* - PB2 - 51 - !WL */
/* - PB1 - 52 - !S1  */
/* - PB0 - 53 - !OE  */

/* PORTC = lowByte(address); */
/* PORTL = highByte(address); */
/* PORTA = lowByte(value); */
/* PORTK = highByte(value); */

/*
   Start procedure:
   h - halt cpu
   p - program sram
   c3 - clock @ 2MHz
   or
   c1 - clock @ 4MHz
   s - float bus pins and remove halt and reset

   Stop procedure:
   h - halt cpu
   m - read address and data lines ( a few times, till it settles at ffff )
   b - read memory block ( confirm execution results )

 */
#define CHECK_BIT(var, pos) ((var) & (1<<pos))
#define INPUT_SIZE 30
char input[INPUT_SIZE + 1];
byte input_index=0;
bool stringComplete = false;  // whether the string is complete


byte OE = 1<<0;
byte S1 = 1<<1;
byte WL = 1<<2;
byte WU = 1<<3;

void setup()
{
    // put your setup code here, to run once:
    /* DDRA=B11111111; */
    /* DDRC=B11111111; */
    /* DDRL=B11111111; */
    DDRG = DDRG | B00000001;
    DDRB = DDRB | B00001111;


    //enable chip
    /* PORTB &= ~S1; */
    /* PORTB |= S2 | W | OE; */

    Serial.begin(115200);
    /* Serial.begin(9600); */
    Serial.println("Hello Computer");
    status();
}


void setupTimer(int ocr2)
{
    //
    //
    // Use of timer2 to generate a signal for a particular frequency on pin 11
    //
    //

    /* const int freqOutputPin = 11;   // OC2A output pin for ATmega328 boards */
    const int freqOutputPin = 10; // OC2A output for Mega boards

    // Constants are computed at compile time

    // If you change the prescale value, it affects CS22, CS21, and CS20
    // For a given prescale value, the eight-bit number that you
    // load into OCR2A determines the frequency according to the
    // following formulas:
    //
    // With no prescaling, an ocr2val of 3 causes the output pin to
    // toggle the value every four CPU clock cycles. That is, the
    // period is equal to eight slock cycles.
    //
    // With F_CPU = 16 MHz, the result is 2 MHz.
    //
    // Note that the prescale value is just for printing; changing it here
    // does not change the clock division ratio for the timer!  To change
    // the timer prescale division, use different bits for CS22:0 below
    const int prescale  = 1;
    const int ocr2aval  = ocr2;
    // The following are scaled for convenient printing
    //

    // Period in microseconds
    const float period    = 5.0 * prescale * (ocr2aval+1) / (F_CPU/1.0e6);

    // Frequency in Hz
    const float freq      = 1.0e6 / period;

    if (ocr2 == 0 ) {
        Serial.println("stop timer");
        pinMode(freqOutputPin, INPUT);
    } else {
        pinMode(freqOutputPin, OUTPUT);
    }
    // Set Timer 2 CTC mode with no prescaling.  OC2A toggles on compare match
    //
    // WGM22:0 = 010: CTC Mode, toggle OC
    // WGM2 bits 1 and 0 are in TCCR2A,
    // WGM2 bit 2 is in TCCR2B
    // COM2A0 sets OC2A (arduino pin 11 on Uno or Duemilanove) to toggle on compare match
    //
    TCCR2A = ((1 << WGM21) | (1 << COM2A0));

    // Set Timer 2  No prescaling  (i.e. prescale division = 1)
    //
    // CS22:0 = 001: Use CPU clock with no prescaling
    // CS2 bits 2:0 are all in TCCR2B
    TCCR2B = (1 << CS20);

    // Make sure Compare-match register A interrupt for timer2 is disabled
    TIMSK2 = 0;
    // This value determines the output frequency
    OCR2A = ocr2aval;

    Serial.print("Period    = ");
    Serial.print(period);
    Serial.println(" microseconds");
    Serial.print("Frequency = ");
    Serial.print(freq);
    Serial.println(" Hz");
}

void write(unsigned int address, unsigned int value)
{

    DDRG |= B00000001;
    DDRB |= B00001111;
    DDRA=0xFF;
    DDRK=0xFF;
    DDRC = 0xFF;
    DDRL = 0xFF;
    DDRG |= 0x01;
    PORTG &= ~0x01;
    PORTC = lowByte(address);
    PORTL = highByte(address);
    // swap bytes to write word as low endian
    PORTA = lowByte(value);
    PORTK = highByte(value);
    PORTB &= ~(WL | WU);
    PORTB &= ~(WL | WU);
    /* delay(1); */
    /* status(); */
    /* PORTB |= W; */
    PORTB |= WL | WU;
    DDRA = 0;
    DDRK = 0;
}

uint16_t read(unsigned int address)
{
    DDRA = 0x00;
    DDRK = 0x00;
    PORTA = 0xFF;
    PORTK = 0xFF;
    DDRC = 0xFF;
    DDRL = 0xFF;
    DDRG |= 0x01;
    PORTC = lowByte(address);
    PORTL = highByte(address);
    PORTG &= ~0x01;
    PORTB &= ~OE;
    /* PORTB &= ~OE; */
    PORTB &= ~OE;
    /* delay(1); */
    // swap again
    uint16_t val = PINA | PINK<<8;
    /* status(); */
    PORTB |= OE;
    return val;
}

uint8_t readByte(unsigned int address)
{
    int movedAddress = address>>1;
    boolean low = CHECK_BIT(address,0);
    DDRA = 0x00;
    DDRK = 0x00;
    PORTA = 0x00;
    PORTK = 0x00;
    DDRC = 0xFF;
    DDRL = 0xFF;
    DDRG |= 0x01;
    PORTC = lowByte(movedAddress);
    PORTL = highByte(movedAddress);
    PORTG &= ~0x01;
    DDRB |= OE;
    PORTB &= ~OE;
    PORTB &= ~OE;
    /* delay(1); */
    // swap again
    /* uint16_t val = PINA | PINK<<8; */
    uint8_t val = low ? PINA : PINK;
    /* status(); */
    PORTB |= OE;
    return val;
}

void sramOutputEnable()
{
    DDRB |= OE;
    PORTB &= ~OE;
}

void program()
{
    uint16_t data[] = {0x0000, 0x0000, 0x0000, 0x0008,
        0x103C, 0x0043, 0x227C, 0x0000, 0x0006,0x1280, 0x60F2};
    int len = sizeof(data)/sizeof(data[0]);
    for(int i=0;i<len;i++){
        write(i, data[i]);
    }
    for(int i=0;i<len;i++){
        int val = read(i);
        Serial.print(val, HEX);
    }
}

void dataZero()
{
    // data zero
    DDRA = 0xFF;
    DDRK = 0xFF;
    PORTA = 0x00;
    PORTK = 0x00;
    // address float
    DDRC = 0x00;
    DDRL = 0x00;
    DDRG &= ~0x01;

    PORTC = 0x00;
    PORTL = 0x00;
    PORTG &= ~0x01;
}

void floatPins()
{
    DDRA = 0x00;
    DDRC = 0x00;
    DDRL = 0x00;
    DDRK = 0x00;
    DDRB &= ~0x0F;
    DDRG &= ~0x01;
    PORTA = 0x00;
    PORTK = 0x00;
    PORTC = 0x00;
    PORTL = 0x00;
}

void monitor()
{
    /* halt(); */
    /* program(); */
    /* floatPins(); */
    /* setupTimer(1); */
    /* sramOutputEnable(); */
    /* resetCPU(); */

    /* program(); */
    floatPins();
    /* setupTimer(200); */
    /* resetCPU(); */

    boolean first = true;
    uint8_t pins[4];
    uint8_t pins_new[4];
    /* while(true) { */
    pins_new[0] = PINL;
    pins_new[1] = PINC;
    pins_new[2] = PINA;
    pins_new[3] = PINK;

    if (first || pins_new[0]!=pins[0]|| pins_new[1]!=pins[1]|| pins_new[2]!=pins[2]|| pins_new[3]!=pins[3]){
        PrintHex83(&pins_new[0], 4);
        first = false;
        for(int i=0;i<4;i++){
            pins[i] = pins_new[i];
        }
    }
    /* } */
}

void readBlock()
{
    for (int i=0;i<40;i++) {
        Serial.println(readByte(i),HEX);
    }
}

void halt()
{
    pinMode(11, OUTPUT);
    pinMode(12, OUTPUT);
    digitalWrite(11, LOW);       // sets the digital pin 13 on
    digitalWrite(12, LOW);       // sets the digital pin 13 on
}

void resetCPU()
{
    floatPins();
    pinMode(11, OUTPUT);
    pinMode(12, OUTPUT);
    digitalWrite(11, LOW);       // sets the digital pin 13 on
    digitalWrite(12, LOW);       // sets the digital pin 13 on
    delay(300);                  // waits for a second

    pinMode(11, INPUT_PULLUP);
    pinMode(12, INPUT_PULLUP);
}

void status()
{

    Serial.println("");
    Serial.println("Regs:");
    Serial.print(" PORTA: ");Serial.print(PORTA,BIN);
    Serial.print(" PORTB: ");Serial.print(PORTB,BIN);
    Serial.print(" PORTC: ");Serial.print(PORTC,BIN);
    Serial.print(" PORTG: ");Serial.print(PORTG,BIN);
    Serial.print(" PORTL: ");Serial.println(PORTL,BIN);
}


void simple_test()
{
    // put your main code here, to run repeatedly:
    Serial.println("");
    Serial.println("gonna 22");
    write(10, 0x1FAD);
    write(15, 0x239D);
    write(20, 0x888D);
    write(12331, 0x9076);
    write(22, 0xFFFF);
    write(24, 0x0000);

    uint16_t ret = read(10);
    Serial.println(ret, HEX);
    ret = read(15);
    Serial.println(ret, HEX);
    ret = read(20);
    Serial.println(ret, HEX);
    ret = read(12331);
    Serial.println(ret, HEX);

    ret = read(22);
    Serial.println(ret, HEX);
    ret = read(24);
    Serial.println(ret, HEX);
    write(0,0x0000);
    write(1,0xFFFF);
    write(2,0x0000);
    write(3,0xFFFF);
 
    write(4,0x0000);
    write(5,0xFFFF);
    write(6,0x0000);
    write(7,0xFFFF);
    write(8,0x0000);
    write(9,0xFFFF);

    status();
    delay(2000);
}


void loop()
{
    if (stringComplete) {
        Serial.println(input);
        handleCommand(input);
        stringComplete = false;
    }
}

void handleCommand(char* input)
{
    performAction(input, input+1);
}

byte readPortValue(char port)
{
    volatile uint8_t *targetPin;
    switch (port) {
        case 'a':
            targetPin = &PINA;
            break;
        case 'l':
            targetPin = &PINL;
            break;
        case 'c':
            targetPin = &PINC;
            break;
        case 'k':
            targetPin = &PINK;
            break;
        default:
            Serial.println("Don't know about this port");
            break;
    }

    return *targetPin;
}

void setPortValue(char port, char* value)
{
    volatile uint8_t *targetPort, *targetDir, *targetPin;
    switch (port) {
        case 'a':
            targetPort = &PORTA;
            targetDir = &DDRA;
            break;
        case 'l':
            targetPort = &PORTL;
            targetDir = &DDRL;
            break;
        case 'c':
            targetPort = &PORTC;
            targetDir = &DDRC;
            break;
        case 'k':
            targetPort = &PORTK;
            targetDir = &DDRK;
            break;
        default:
            Serial.println("Don't know about this port");
            break;
    }

    char *endptr;
    long val = strtol(value, &endptr, 16);
    if (val == 0xAA ) {
        // toggle port to input, float
        *targetDir = 0x00;
        *targetPort = 0x00;
    }
    else if (val == 0xAB){
        // toggle port to input, pull pins high
        *targetDir = 0x00;
        *targetPort = 0xFF;
    } else {
        // set the value as speficied
        *targetDir = 0xFF;
        *targetPort = lowByte(val);
    }
}

void PrintHex83(uint8_t *data, uint8_t length) // prints 8-bit data in hex
{
    char tmp[length*2+1];
    byte first ;
    int j=0;
    for (uint8_t i=0; i<length; i++)
    {
        first = (data[i] >> 4) | 48;
        if (first > 57) tmp[j] = first + (byte)39;
        else tmp[j] = first ;
        j++;

        first = (data[i] & 0x0F) | 48;
        if (first > 57) tmp[j] = first + (byte)39;
        else tmp[j] = first;
        j++;
    }
    tmp[length*2] = 0;
    Serial.println(tmp);
}

void performAction(char* command, char* arg)
{
    Serial.println("evaluate command");
    char c = command[0];
    if (c=='c') {
        Serial.println("setup timer");
        byte ocr = atoi(arg);
        setupTimer(ocr);
    } else if (c=='f') {
        floatPins();
    } else if (c=='h') {
        Serial.println("halt cpu");
        halt();
    } else if (c=='t') {
        simple_test();
    } else if (c=='m') {
        monitor();
    } else if (c=='w') {
        setPortValue(arg[0], arg+1);
    } else if (c=='r') {
        byte val = readPortValue(arg[0]);
        Serial.println(val);
    } else if (c=='z') {
        dataZero();
        Serial.println("data zero && address float");
    } else if (c=='p') {
        program();
        Serial.println("program data");
    } else if (c=='b') {
        Serial.println("read block");
        readBlock();
    } else if (c=='s') {
        resetCPU();
        Serial.println("reset done");
    } else {
        Serial.println("unknown command");
    }
}

void serialEvent()
{
    while (Serial.available()) {
        // get the new byte:
        char inChar = (char)Serial.read();
        // add it to the inputString:
        input[input_index]=inChar;
        input_index++;
        // if the incoming character is a newline, set a flag so the main loop can
        // do something about it:
        if (inChar == '\n') {
            input[input_index]=0;
            stringComplete = true;
            input_index = 0;
        }
    }
}

