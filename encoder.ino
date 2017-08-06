#define PINA 5
#define PINB 6

#define PULSE_PER_REV 2400

uint8_t enc_state = 0;
int32_t enc_position = 0;

static void enc_update_isr() {
    uint8_t p1val = digitalRead(PINA);
    uint8_t p2val = digitalRead(PINB);
    uint8_t state = enc_state & 3;
    if (p1val) state |= 4;
    if (p2val) state |= 8;
    enc_state = (state >> 2);
    switch (state) {
      case 1: case 7: case 8: case 14:
        enc_position++;
        break;
      case 2: case 4: case 11: case 13:
        enc_position--;
        break;
      case 3: case 12:
        enc_position += 2;
        break;
      case 6: case 9:
        enc_position -= 2;
        break;
    }
    // wrap 0..2400
    while(enc_position >= PULSE_PER_REV) enc_position -= PULSE_PER_REV;
    while(enc_position <  0)    enc_position += PULSE_PER_REV;
}

// returns result in the range -180..180
float enc_read() {
    noInterrupts();
    float result = enc_position;
    interrupts();
    return (result/float(PULSE_PER_REV)) * 360.0 - 180.0;
}

void enc_setup()
{
  noInterrupts();

  // setup ports
  pinMode(PINA, INPUT);
  digitalWrite(PINA, HIGH);  // internal pullup
  
  pinMode(PINB, INPUT);
  digitalWrite(PINB, HIGH);

  // initialize
  enc_update_isr();
  enc_position = PULSE_PER_REV/2;  // midpoint

  // set interrupts
  attachInterrupt(digitalPinToInterrupt(PINA), enc_update_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PINB), enc_update_isr, CHANGE);
  interrupts();
}

