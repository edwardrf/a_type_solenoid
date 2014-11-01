/**
 * typeself program.
 * To operate the typewriter, you need to connect both power supply connector with 24v. Then open serial console to the arduino board with baudrate of 115200.
 * This program operates by taking command from the serial port, each individual commands are expained below in the switch statement of loop funciton.
 * Each command starts with one character, some of them has parameters, some does not, in the following explaination, X respresents a character, N represents a number, B represents either 0 or 1
 * tX : type a single character, and move to next space
 * pX : type a single character without moving the carrage
 * TN : type out "typeself" for N times then go to next line
 * rB N: roll paper in direction B for N steps, B: 0 rolls to next line, 1 rolls back. A line is about 1000 steps
 * mB N: move the carrage in the direction B for N steps, B: 0 carrage moves to left, 1 moves to right, a character is about 160steps wide
 * i, j, k, l: each of this moves the pager in the up, left, down, right direction for easier positioning.
 * R  : Return to start of the line
 * E  : Go to the end of the line
 * CX : Color X, X=='r' red, X=='b' black
 * SN : Shift N == 1 shift, N == 0 small case
 *
 *
 * TODO:
 *  0. Type a sentense,space, enter(cr, lf)
 *  1. Response to PC command when finished
 *  2. Ramp up and down for moving the carrage
 *  3. Color, and shift
 */
#include <Servo.h>
// NOTE: as < and > does not exist on the keyboard, and keyboard as 1/2, 1/4, c/ as extra keys, they are used as subsititutes
// 1/2 ==> '<'
// 1/4 ==> '>'
// c/  ==> '\'
// /   ==> '|' // Linux does not allow '/' as part of a file name
// Ascii :       !, ", #, $, %, &, ', (, ), *, +, ,, -, ., /, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, :, ;, <, =, >, ?, @, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z, [, \, ], ^, _, `, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z, {, |, }, ~
int krow[128] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 1, 4, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 2, 1, 2, 4, 3, 3, 4, 4, 3, 2, 3, 3, 3, 2, 3, 3, 3, 4, 4, 2, 2, 2, 2, 3, 2, 2, 4, 2, 4, 2, 4,-1, 3,-1,-1, 1,-1, 3, 4, 4, 3, 2, 3, 3, 3, 2, 3, 3, 3, 4, 4, 2, 2, 2, 2, 3, 2, 2, 4, 2, 4, 2, 4,-1, 4,-1,-1};
int kcol[128] = {1, 2, 3, 4, 5, 7, 8, 9,10,11,12, 8,11, 9,10,10, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,10,11,12,11,10,11, 1, 5, 3, 3, 3, 4, 5, 6, 8, 7, 8, 9, 7, 6, 9,10, 1, 4, 2, 5, 7, 4, 2, 2, 6, 1,-1,11,-1,-1, 6,-1, 1, 5, 3, 3, 3, 4, 5, 6, 8, 7, 8, 9, 7, 6, 9,10, 1, 4, 2, 5, 7, 4, 2, 2, 6, 1,-1,10,-1,-1};
int ksft[128] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 0,-1,-1, 1,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1, 0,-1,-1};

void setup();
void loop();
void typeself();
void type(char);
void key(char);
void roll(int dir, int steps);
void move(int dir, int steps);
void roll(int dir);
void moveWithSpeed(int dir, int speed);
void shift(int); // 0 small case, 1 captial / shift
void color(int); // 0 black, 1 red
void cr(); // carrage return
void ce(); // carrage end
void nl(); // Next line
void nc(); // Next char

Servo shiftServo, colorServo;
int shiftState;
char colorState;

void setup(){

  // Pin 22~33 to control the colum switches, Pin 34~37 Controls stepper motor step and dir pin
  for(int i = 22; i < 38; i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }

  // Pin 4~7 controls the row switches
  for(int i = 4; i < 8; i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }

  // pin 51, 53 controls the enabled pin of the stepper motor controller
  pinMode(51, OUTPUT);
  pinMode(53, OUTPUT);
  digitalWrite(51, HIGH);
  digitalWrite(53, HIGH);

  // pin 42, 43 end stops, 42: start, 43: stop
  pinMode(42, INPUT);
  pinMode(43, INPUT);
  digitalWrite(42, HIGH); // Use internal pull up
  digitalWrite(43, HIGH);

  // Attach servos
  shiftServo.attach(44); // 90 normal, 60 shift
  colorServo.attach(45); // 90 red, 110 black
  shiftServo.write(90);
  colorServo.write(90);

  Serial.begin(115200);
  Serial.println("Ready");
}

void loop(){
  char c = 0;
  // Wait for serial console command
  if(Serial.available()){
    c = Serial.read();
    Serial.println(c);
    int dir, stp, sft;
    char col;
    switch(c) {
      case 'r': // Command r, roll the paper up or down
        // E.g.  type the following command in the serial console to roll the paper down one row:
        // r0 1000
        // '0' indicates direction, 0: roll down, 1: roll back
        dir = Serial.parseInt();
        stp = Serial.parseInt();
        roll(dir, stp);
        Serial.print("Rolled : ");
        Serial.print(dir);
        Serial.print("\t");
        Serial.print(stp);
        Serial.println();
        break;
      case 'm': // Command to move the carrage left and right
      // m0, 160 moves right one character
        dir = Serial.parseInt();
        stp = Serial.parseInt();
        move(dir, stp);
        Serial.print("Rolled : ");
        Serial.print(dir);
        Serial.print("\t");
        Serial.print(stp);
        Serial.println();
        break;
      case 't': // Type one character and move to next position, e.g. command: tttytpte would type out "type" on paper
        while(!Serial.available());
        c = Serial.read();
        type(c);
        break;
      case 'p': // Type one character without moving the carrage, command: papt  would result in over lapping and t type on paper
        while(!Serial.available());
        c = Serial.read();
        key(c);
        break;
      case 'T': // Type out the "typeself" word n times and goes to new line. e.g. T3T4 would type:
        // typeself typeself typeself
        // typeself typeself typeself typeself
        stp = Serial.parseInt();
        for(int j = 0; j < stp; j++) {
          typeself();
          move(0, 160);
        }
        move(1, 1300 * stp);
        roll(0, 1500);
        break;
      case 'i': // Move the paper up a bit
        roll(0, 1000);
        break;
      case 'k': // Move the paper down a bit
        roll(1, 1000);
        break;
      case 'j': // Move the paper left a bit
        move(1, 100);
        break;
      case 'l': // Move the paper right a bit
        move(0, 100);
        break;
      case 'R': // carrage return
        cr();
        break;
      case 'E': // Move to the end of the line
        ce();
        break;
      case 'S': // Shift
        sft = Serial.parseInt();
        shift(sft);
        break;
      case 'C':
        while(!Serial.available());
        col = Serial.read();
        color(col);
        break;
    }
    Serial.print("OK");
    Serial.println(Serial.available());
  }
}

void typeself() {
  type('t');
  type('y');
  type('p');
  type('e');
  type('s');
  type('e');
  type('l');
  type('f');
  type(' ');
}

void type(char c) {
  key(c);
  nc();
  delay(50);
}

void key(char c) {
  int idx = c - 33;
  if(idx < 0) return;
  int row = krow[idx] - 1;
  int col = kcol[idx] - 1;

  if(ksft[idx] != shiftState) shift(ksft[idx]);

  Serial.print(row);
  Serial.print('\t');
  Serial.print(col);
  Serial.println();

  if(row >= 0) {
    digitalWrite(22 + col, HIGH);
    digitalWrite(4 + row, HIGH);
  }
  delay(15);

  // Gental release to prevent the core to jump out.
  for(int i = 255; i > 0; i--) {
    analogWrite(4 + row, i);
    delayMicroseconds(500);
  }

  digitalWrite(22 + col, LOW);
  digitalWrite(4 + row, LOW);

}

void roll(int dir, int steps) {
  digitalWrite(51, LOW);
  for(int i = 0; i < steps; i++) {
    roll(dir);
  }
  digitalWrite(51, HIGH);
}


void move(int dir, int steps) {
  digitalWrite(53, LOW);
  int speed = 150;
  for(int i = steps; i > 0; i--) {
    // if(speed > 100 && steps > 150) speed --;
    // if(speed < 150 && steps <= 150) speed ++;
    moveWithSpeed(dir, speed);
  }
  digitalWrite(53, HIGH);
}

void roll(int dir){
  digitalWrite(35, dir);
  digitalWrite(34, HIGH);
  delayMicroseconds(100);
  digitalWrite(34, LOW);
  delayMicroseconds(100);
}

void moveWithSpeed(int dir, int speed){
  if((dir && digitalRead(42) == LOW) || (!dir && digitalRead(43) == LOW)) return;
  digitalWrite(37, dir);
  digitalWrite(36, HIGH);
  delayMicroseconds(speed);
  digitalWrite(36, LOW);
  delayMicroseconds(speed);
}

void cr() {
  digitalWrite(53, LOW);
  int speed = 500;
  while(digitalRead(42) != LOW) {
    if(speed > 100) speed --;
    moveWithSpeed(1, speed);
  }
  digitalWrite(53, HIGH);
}

void ce() {
  digitalWrite(53, LOW);
  int speed = 500;
  while(digitalRead(43) != LOW) {
    if(speed > 100) speed --;
    moveWithSpeed(0, speed);
  }
  digitalWrite(53, HIGH);
  Serial.println("CE");
}

void nl(){
  roll(1, 1200);
}

void nc() {
  move(0, 200);
}

void shift(int s){
  shiftState = s;
  if(s)
    shiftServo.write(64);
  else
    shiftServo.write(90);
  delay(800);
}

void color(int c){
  colorState = c;
  if(c == 'r')
    colorServo.write(110);
  else
    colorServo.write(90);
  delay(500);
}
