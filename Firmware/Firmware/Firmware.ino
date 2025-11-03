#include <Arduino.h>
#include <RotaryEncoder.h>
#include <LedControl.h>

#define CTRL_PIN 6 //D6
#define TAP_TEMPO_PIN 7 //D7
#define TAP_TEMPO_LED_PIN 12 //D12

#define SW1A_PIN 5 //D5
#define SW1B_PIN 4 //D4
#define SW2A_PIN 3 //D3
#define SW2B_PIN 2 //D2
#define SW3A_PIN 0 //D0
#define SW3B_PIN 1 //D1

#define CS_PIN 10 //D10
#define MOSI_PIN 11 //D11
#define CLK_PIN 13 //D13

// PARAMS
#define MIN_STEPS 1
#define MAX_STEPS 32


RotaryEncoder encoder_1(SW1A_PIN, SW1B_PIN, RotaryEncoder::LatchMode::TWO03);
RotaryEncoder encoder_2(SW2A_PIN, SW2B_PIN, RotaryEncoder::LatchMode::TWO03);
RotaryEncoder encoder_3(SW3A_PIN, SW3B_PIN, RotaryEncoder::LatchMode::TWO03);
int pos_1 = 0;
int pos_2 = 0;
int pos_3 = 0;

// Milliseconds
int interval = 1000;

int sequence[MAX_STEPS];

int clamp(int v, int lo, int hi) {
  const int t = v < lo ? lo : v;
  return t > hi ? hi : t;
}

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // Update encoders
  encoder_1.tick();
  int newPos = encoder_1.getPosition();
  if (pos_1 != newPos) pos_1 = newPos;
  pos_1 = clamp(pos_1, MIN_STEPS, MAX_STEPS);

  encoder_2.tick();
  newPos = encoder_2.getPosition();
  if (pos_2 != newPos) pos_2 = newPos;
  // Can't have more pulses than num steps
  pos_2 = clamp(pos_2, 0, MAX_STEPS);

  encoder_3.tick();
  newPos = encoder_3.getPosition();
  if (pos_3 != newPos) pos_3 = newPos;
  // No clamp for infinite rotation;
  // pos_3 = clamp(pos_3, 0, 127);

  calculate_euclidean_sequence(pos_1, pos_2, pos_3, sequence);

  static int idx = 0;
  idx = tick_euclidean(idx);
}

int increment_idx(int idx) {
  int newIndex = idx + 1;
  if (newIndex > MAX_STEPS) {
    newIndex = 0;
  }
  return newIndex;
}

int tick_euclidean(int idx) {
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    bool currentState = sequence[idx];
    if (currentState) {
      digitalWrite(CTRL_PIN, HIGH);
    } else {
      digitalWrite(CTRL_PIN, LOW);
    }

    return increment_idx(idx);
  }
}

void rotate_seq(int *seq, int steps, int rotate) {
  int output[steps];
  int val = steps - rotate;

  for (int i = 0; i < steps; i++) {
    output[i] = seq[abs((i + val) % steps)];
  }

  for (int i = 0; i < steps; i++) {
      seq[i] = output[i];
  }
}

void calculate_euclidean_sequence(int steps, int pulses, int rotate, int* seq) {
  rotate += 1;
  int finalRhythm[steps];
  int bucket = 0;

  for (int i = 0; i < steps; i++) {
    bucket += pulses;
    if (bucket >= steps) {
      bucket -= steps;
      finalRhythm[i] = 1;
    } else {
      finalRhythm[i] = 0;
    }
  }

  if (rotate != 0) {
    rotate_seq(finalRhythm, steps, rotate);
  }

  for (int i = 0; i < steps; i++) {
      seq[i] = finalRhythm[i];
  }
}
