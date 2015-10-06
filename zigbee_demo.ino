#include <LiquidCrystal.h>
#define B_NOBTTN -1
#define B_SELECT 0
#define B_LEFT 1
#define B_DOWN 2
#define B_UP 3
#define B_RIGHT 4

// Program Variables
int running = 1;
const int programDelay = 5; 

// LCD Variables
LiquidCrystal lcd(8, 13, 9, 4, 5, 6, 7);
const int button_vals[5] = {800, 600, 400, 200, 50};

// Motor Variables
int motorPins[] = {47,49,51,53};
int motorSeq[5][4] = { 
{HIGH, HIGH, LOW, LOW},
{LOW, HIGH, HIGH, LOW},
{LOW, LOW, HIGH, HIGH},
{HIGH, LOW, LOW, HIGH},
{LOW, LOW, LOW, LOW} };
int motorSeqRev[5][4] = { 
{HIGH, LOW, LOW, HIGH},
{LOW, LOW, HIGH, HIGH},
{LOW, HIGH, HIGH, LOW},
{HIGH, HIGH, LOW, LOW},
{LOW, LOW, LOW, LOW} };
int motorCurPos = 1; // This fixes a strange start up bug in actuation without really affecting things
float motorCurDeg = 0;
int motorCurDir = 0;
int motorCurPhase = 4;
int motorSpinToggle = 0;
int motorActToggle = 0;
int motorOscToggle = 0;

// Relay Variables
int relayPins[] = {22,24,26,28,30,32,34,36};
int relayTotal = sizeof(relayPins);
int relayStates[] = { LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW };
int relayShowToggle = 0;
int relayShowPhase = 0;
int relayShowPeriod = 4;
int relayShowSeq[][8] = {
  { HIGH, LOW, LOW, LOW, HIGH, LOW, LOW, LOW },
  { LOW, HIGH, LOW, LOW, LOW, HIGH, LOW, LOW },
  { LOW, LOW, HIGH, LOW, LOW, LOW, HIGH, LOW },
  { LOW, LOW, LOW, HIGH, LOW, LOW, LOW, HIGH }
};
long relayDelayPhase = 0;
long relayDelayPeriod = 15000;

// Utility Methods
int getKeypadButton() {
  int analogVal = analogRead(0);
  for( int buttonId = 5; buttonId >= 0; buttonId-- ) {
    if( analogVal < button_vals[buttonId] ) 
      return buttonId;
  }
  return -1;
};

void printRelays() {
  lcd.setCursor(7,1);
  for (int i = 0; i< 8; i++) {
    if( relayStates[i] == LOW ) {
      lcd.print(i+1); 
    } else {
      lcd.print("_"); 
    }
  } 
}

void relaySetPat(int a[]) {
  for ( int i = 0; i < 8; i++) {
   digitalWrite(relayPins[i], a[i]); 
  }
};

void motorSetPhase(int a[]) {
  for( int i = 0; i < 4; i++) {
   digitalWrite(motorPins[i], a[i]); 
  }
};

void ackCmd(char cmd) {
  Serial.write('#');
  Serial.write(cmd);
  Serial.write('\n');
};

void ackErr(char cmd) {
  Serial.write('!');
  Serial.write(cmd);
  Serial.write('\n'); 
}

void sendState() {
 Serial.write('$');
 
  // Relay Data
 for (int i = 0; i < 8; i++) {
   if ( relayStates[i] )
     Serial.write('1');
   else
     Serial.write('0'); 
  }
  
 Serial.write(' '); //seperator
 
 if ( relayShowToggle )
   Serial.write('1');
 else 
   Serial.write('0'); 
   
 Serial.write(' ');
 
 // Motor Data
 if ( motorSpinToggle ) 
   Serial.write('1');
 else
   Serial.write('0'); 
   
 Serial.write(' ');
  
 if ( motorOscToggle ) 
   Serial.write('1');
 else
   Serial.write('0'); 
   
 Serial.write(' ');
 
 if ( motorCurDir )
   Serial.write('1');
 else
   Serial.write('0'); 
   
 Serial.write(' ');
 
 char str[4];
 sprintf(str, "%d", motorCurPos);
 Serial.write(str);
  
 Serial.write('\n');
}

void reverseMotorSeq() {
  int temp[5][4];
  motorCurDir = motorCurDir ? 0 : 1;
  for (int i = 0; i<4; i++) {
    for (int j = 0; j<4; j++) {
      temp[i][j] = motorSeq[i][j];
      motorSeq[i][j] = motorSeqRev[i][j];
      motorSeqRev[i][j] = temp[i][j];
    }
  }
}


void setup() {
  // Setup lcd
  lcd.begin(16,2);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("ZigBee//SerialCT");
  lcd.setCursor(1,1);
  lcd.print("|000|");
  lcd.setCursor(7,1);
  lcd.print("12345678");
  // Setup Serial
  Serial.begin(9600);
  // Setup Device GPIO
  for( int i = 0; i < relayTotal; i++ ) {
    pinMode(relayPins[i], OUTPUT); 
    digitalWrite(relayPins[i], LOW);
  }
  for( int i = 0; i < 4; i++) {
    pinMode(motorPins[i], OUTPUT);
    digitalWrite(motorPins[i], LOW);  
  }
}
 
void loop() {
  while (running) {
    if ( Serial.available() ) {
      // Implemented Commands:
      // 0-7 - Toggle Relays
      // F - All Relays O[F]F
      // N - All Relays O[N]
      // M - Toggle Motor
      // D - Toggle Motor Dir
      // A - Actuate Motor
      // R - Reset Motor Home Pos
      // S - Light[S]how
      // Z - Report Status (If you know why this is z, than you are awesome)
      char cmd = Serial.read();
      if (cmd == 'M') {
         if (motorSpinToggle) {
          motorSpinToggle = 0;
          motorSetPhase(motorSeq[4]);
          relayDelayPeriod = 15000;
         }
         else {
          motorSpinToggle = 1;
          relayDelayPeriod = 50;
         }
         ackCmd(cmd);
      } 
      else if ( cmd - '0' >=0 && cmd -'0' < 8) {
        int device = cmd - '0';
        
        if (relayStates[device] == HIGH ) {
          relayStates[device] = LOW;
          digitalWrite(relayPins[device], LOW);
          ackCmd(cmd);
          lcd.setCursor(7+device,1);
          lcd.print(device+1);
        } 
        else {
          relayStates[device] = HIGH;
          digitalWrite(relayPins[device], HIGH);
          ackCmd(cmd);
          lcd.setCursor(7+device,1);
          lcd.print("_");
        }
      }
      else if (cmd == 'R') {
        motorCurPos = 0;
        float motorCurDeg = motorCurPos / 512.0 * 360;
        lcd.setCursor(2,1);
        lcd.print("   ");
        lcd.setCursor(2,1);
        lcd.print((int) motorCurDeg);
        ackCmd(cmd); 
      }
      else if (cmd =='S') {
       if (relayShowToggle) {
        relayShowToggle = 0;
        relaySetPat(relayStates);
        lcd.setCursor(6,1);
        lcd.print(" ");
        ackCmd(cmd);
       } else {
        relayShowPhase = 0;
        relayShowToggle = 1;
        lcd.setCursor(6,1);
        lcd.print("$");
        ackCmd(cmd);
       } 
      }
      else if (cmd == 'N') {
        for (int i = 0; i < 8; i++) {
         digitalWrite(relayPins[i], LOW);
         relayStates[i] = LOW; 
        }
        printRelays();
        ackCmd(cmd);
      }
      else if (cmd == 'F') {
        for (int i = 0; i < 8; i++) {
         digitalWrite(relayPins[i], HIGH);
         relayStates[i] = HIGH; 
        }
        printRelays();
        ackCmd(cmd);
      }
      else if (cmd == 'T') {
        for (int i = 0; i < 8; i++) {
         if (relayStates[i] == HIGH) {
          digitalWrite(relayPins[i], LOW);
          relayStates[i] = LOW; 
         } else {
          digitalWrite(relayPins[i], HIGH);
          relayStates[i] = HIGH; 
         }
        }
        printRelays();
        ackCmd(cmd); 
      }
      else if (cmd == 'D') {
        reverseMotorSeq();
        ackCmd(cmd);
      }
      else if (cmd == 'A') {
       motorActToggle = 1;
       motorSpinToggle = 1; 
       ackCmd(cmd);
      }
      else if (cmd == 'Z') {
       ackCmd(cmd);
       sendState();
      }
      else if (cmd == 'O') {
       motorOscToggle = motorOscToggle ? 0 : 1;
       ackCmd(cmd); 
      }
      else {
       ackErr(cmd);
      } 
    }
   
   if (motorSpinToggle) {
     if (motorCurPhase >= 4) {
       motorCurPhase = 0;
       motorSetPhase(motorSeq[motorCurPhase]);
     }
     else if (motorCurPhase == 3) {
       motorCurPhase = 0;
       motorSetPhase(motorSeq[motorCurPhase]);
       if (motorCurDir)
         motorCurPos--;
       else
         motorCurPos++;
       if (motorCurPos > 512) {
        motorCurPos = 0; 
       }
       else if (motorCurPos < 0) {
        motorCurPos = 512; 
       }
       motorCurDeg = motorCurPos / 512.0 * 360;
       lcd.setCursor(2,1);
       lcd.print("   ");
       lcd.setCursor(2,1);
       lcd.print((int) motorCurDeg);
     }
     else {
       motorCurPhase++;
       motorSetPhase(motorSeq[motorCurPhase]);
     }
     delay(programDelay);
   }
   
   if (motorActToggle) {
    if (motorCurPos == 0 && motorCurPhase == 3) {
      if (!motorOscToggle) {
        motorActToggle = 0;
        motorSpinToggle = 0;
        motorSetPhase(motorSeq[4]);
      }
      reverseMotorSeq();
    }
   }
   
   if (relayShowToggle) {
    relayDelayPhase++;
    if (relayDelayPhase > relayDelayPeriod) {
     relayShowPhase++;
     int lastIter;
     
     if (relayShowPhase >= relayShowPeriod) {
       relayShowPhase = 0;
     }
     relaySetPat(relayShowSeq[relayShowPhase]);
     
     relayDelayPhase = 0; 
    }
   }
  }
}
