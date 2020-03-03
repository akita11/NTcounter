#define LOAD 2

void setup() {
  pinMode(LOAD, OUTPUT);
}

void wait()
{
  for (uint8_t i = 0; i < 13; i++)
    asm("nop");
}

void loop() {
  int i, j;
  // for NTcounterRX15
//  for (j = 0; j < 5; j++) {
    // 1.2ms * 5 = 6ms
    // "1" : 26us * 23 = 600us
    // 1/38kHz = 26.3us
    for (i = 0; i < 23; i++) {
      digitalWrite(LOAD, HIGH); wait();
      digitalWrite(LOAD, LOW); wait();
    }
    for (i = 0; i < 23; i++) {
      digitalWrite(LOAD, LOW); wait();
      digitalWrite(LOAD, LOW); wait();
    }
//  }
//  delay(194); // Cycle=200ms > 25ms
  delay(44); // Cycle=50ms > 25ms
}
