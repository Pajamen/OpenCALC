#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

// Constants
const int maxop = 100;     // maximum pointer
const bool pcheck = 0;     // '1' enables pointerCheck function
const bool nbcheck = 0;    // '1' enables numbafrCheck function
const bool numcheck = 0;   // '1' enables numCheck function
const bool skipintro = 1;  // '1' skips boot animation
bool cyclecheck = 1;       // '1' enables cycleCheck function
bool obcheck = 1;          // '1' enables oprbafrCheck function

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Keypad setup
const byte ROWS = 5;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  { '(', ')', 'C', '/' },
  { '7', '8', '9', '*' },
  { '4', '5', '6', '-' },
  { '1', '2', '3', '+' },
  { '0', '.', 'F', '=' }
};
byte rowPins[ROWS] = { 7, 8, 9, 10, 11 };
byte colPins[COLS] = { 2, 3, 4, 5 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Status LED setup
const int READYled = 12;
const int WORKINGled = 13;

// Variables
char key;      // pressed key
char backkey;  // previous key for clear function
char ansbafr[10];
char oprbafr[maxop];  // oprbafr - operand buffer - X=nothing, N=NUM
int numbafr[maxop];   // numbafr - number buffer
int bracketcount = 0;
int pointer;       // main pointer identifying length of formula
int cyclepointer;  // Common pointer for use when cycling through arrays
int NUMcounter;    // number of NUMs in equation
int bracketstart, bracketend = 0;
bool isbracket = 0;
bool computed, numstat, Fstat = 0;
bool workingstat = 0;
bool eqflag = 0;
bool keyback = 0;
float ans = 0;  // variables for computing ANS
int loopfault = 0;

//Debug variables

int obpointer = 0;
int nbpointer = 0;

// Functions (internal)

// DEBUG functions

void oprbafrCheck() {  // oprbafrCheck - shows raw oprbafr instead of top row
  if (eqflag == 1) {
    lcd.setCursor(4, 0);
    for (obpointer = 0; (obpointer < 12); obpointer++) {
      if (obpointer > pointer) lcd.print(' ');
      else if (oprbafr[obpointer] == 'X') lcd.print('_');
      else (lcd.print(oprbafr[obpointer]));
    }
  }
}

void numbafrCheck() {  // shows raw numbafr instead of bottom row
  lcd.setCursor(0, 1);
  for (nbpointer = 0; (nbpointer <= 6); nbpointer++) {
    lcd.print(numbafr[nbpointer]);
    lcd.print(' ');
  }
}

/*
void oprbafrCheck() {  // oprbafrCheck - shows raw oprbafr instead of top row
  if (eqflag == 1) {
    lcd.setCursor(0, 0);
    for (cyclepointer = 0; (cyclepointer < 16); cyclepointer++) {
      lcd.print(oprbafr[cyclepointer]);
    }
  }
}

void numbafrCheck() {  // shows raw numbafr instead of bottom row
  lcd.setCursor(0, 1);
  for (cyclepointer = 0; (cyclepointer <= 6); cyclepointer++) {
    lcd.print(numbafr[cyclepointer]);
    lcd.print(' ');
  }
}
*/

void pointerCheck() {  // Shows lvl of Pointer variable
  lcd.setCursor(0, 0);
  lcd.print('P');
  lcd.print(pointer - 1);
  lcd.print(' ');
}

void numCheck() {  // Shows current NUM value
  lcd.setCursor(4, 1);
  lcd.print(numbafr[pointer - 1]);
}

void cycleCheck() {  // shows top row every cycle with 1000ms delay
  writeKey(1);
  lcd.setCursor(0, 1);
  lcd.print("LF:");
  lcd.print(loopfault);
  delay(2000);
  lcd.setCursor(0, 1);
  lcd.print("                ");
}

// GUI Functions
void working(bool worktemp) {
  if (worktemp == 0) {
    digitalWrite(WORKINGled, LOW);
    digitalWrite(READYled, HIGH);
  } else if (worktemp == 1) {
    digitalWrite(WORKINGled, HIGH);
    digitalWrite(READYled, LOW);
  }
}

void writeKey(bool DBG) {  // Writes top row ... selfexplanatory (bool DBG because writelength issues when cyclecheck)
  int lengthpointer, numtest, steppointer, cycluspointer;
  steppointer = pointer - 1;
  if (DBG) steppointer = steppointer + 2;
  cycluspointer = 15;
  lcd.setCursor(0, 0);
  lcd.print("                ");
  while ((cycluspointer >= 0) and (steppointer > 0)) {
    if (oprbafr[steppointer] == 'N') {
      lengthpointer = 0;
      numtest = numbafr[(steppointer)];
      do {
        lengthpointer++;
        numtest /= 10;
      } while (numtest);
      if (DBG) {
        lcd.setCursor((cycluspointer - lengthpointer), 0);
        if (numbafr[steppointer] >= 0) lcd.print('+');
        lcd.print(numbafr[steppointer]);
        cycluspointer = cycluspointer - lengthpointer - 1;
        steppointer--;
      } else {
        lcd.setCursor((cycluspointer - lengthpointer + 1), 0);
        lcd.print(numbafr[steppointer]);
        cycluspointer = cycluspointer - lengthpointer;
        steppointer--;
      }
    } else {
      lcd.setCursor((cycluspointer), 0);
      lcd.print(oprbafr[steppointer]);
      cycluspointer--;
      steppointer--;
    }
  }
  if (obcheck == 1) oprbafrCheck();  // Prints oprbafr to the top row
  if (pcheck == 1) pointerCheck();   // Top left shows pointer count
  if (nbcheck == 1) numbafrCheck();  // Prints numbafr to the bottom right
  if (numcheck == 1) numCheck();     // Prints actual NUM value to the bottom middle
  if (cyclecheck == 1) {
    lcd.setCursor(0, 0);
    lcd.print("DBG|");
  }
}


void bracketCheck() {  // DONE Prints number of open brackets
  lcd.setCursor(0, 1);
  if (bracketcount > 0) {
    lcd.print("(x");
    lcd.print(bracketcount);
    lcd.print(" ");
  } else
    lcd.print("   ");
}

// Parsing/cleaning functions
void formCompress() {  // DONE removes blank spaces in oprbafr and numbafr
  int movepointer, emptypointer;
  bool notemptyflag = 0;
  for (cyclepointer = 1; cyclepointer <= pointer + 1; cyclepointer++) {  // runs through whole oprbafr
    if (oprbafr[cyclepointer] == 'X') {
      notemptyflag = 0;
      for (emptypointer = cyclepointer ;emptypointer <= pointer + 1 ;emptypointer++) {
        if (oprbafr[emptypointer] != 'X') notemptyflag = 1;
      }
      if (notemptyflag == 0) {
        cyclepointer = pointer + 1;
      }
      else {
        for (movepointer = cyclepointer; movepointer <= pointer + 1; movepointer++) {  // moves all after the blank space to the left
          numbafr[movepointer] = numbafr[movepointer + 1];
          oprbafr[movepointer] = oprbafr[movepointer + 1];
        }
        if (cyclecheck == 1) {
          lcd.setCursor(5, 1);
          lcd.print("FormComp");
          cycleCheck();
        }
      } 
    }
  }
  pointerCompress();
}

void pointerCompress() {  // TODO Moves pointer to last USED position
  for (cyclepointer = pointer; oprbafr[cyclepointer] == 'X'; cyclepointer--) {
    if (oprbafr[cyclepointer] == 'X') {
      pointer--;
      if (cyclecheck == 1) {
        lcd.setCursor(5, 1);
        lcd.print("PointComp");
        cycleCheck();
      }
    }
  }
}

/*
void pointerCompress() {  // TODO Moves pointer to last USED position
  for (cyclepointer = pointer; cyclepointer > 0; cyclepointer--) {
    if (oprbafr[cyclepointer] == 'X') {
      pointer--;
      if (cyclecheck == 1) {
        lcd.setCursor(5, 1);
        lcd.print("PointComp");
        cycleCheck();
      }
    }
  }
}
*/

void plusMinus() {                                                                // removes '+' and '-' signs in front of NUMs
  for (cyclepointer = 1; cyclepointer <= pointer; cyclepointer++) {               // runs through whole oprbafr
    if ((oprbafr[cyclepointer - 1] == '-') and (oprbafr[cyclepointer] == 'N')) {  // removes '-' sign
      numbafr[cyclepointer] = (-numbafr[cyclepointer]);
      oprbafr[cyclepointer - 1] = 'X';
      if (cyclecheck == 1) {
            lcd.setCursor(5, 1);
            lcd.print("PlusMin-");
            cycleCheck();
      }
    } 
    else if ((oprbafr[cyclepointer - 1] == '+') and (oprbafr[cyclepointer] == 'N')) {  // removes '+' sign
      oprbafr[cyclepointer - 1] = 'X';
      if (cyclecheck == 1) {
            lcd.setCursor(5, 1);
            lcd.print("PlusMin+");
            cycleCheck();
      }
    }
  }
  formCompress();
}

void bracketMinus() {  // removes '-' before brackets (all numbers excluding multiplicators and dividers in bracket gets *(-1))
  int changepointer, brcount;
  for (cyclepointer = 1; cyclepointer <= pointer; cyclepointer++) {  // run through whole oprbafr
    if ((oprbafr[cyclepointer] == '-') and (oprbafr[cyclepointer + 1] == '(')) {
      brcount = 1;
      for (changepointer = cyclepointer + 2; brcount > 0; changepointer++) {  // change all signs at NUMs after '-(' until ')'
        if ((oprbafr[changepointer] == 'N') and (oprbafr[changepointer - 1] != '/') and (oprbafr[changepointer - 1] != '*')) {
          numbafr[changepointer] = (-numbafr[changepointer]);
          if (cyclecheck == 1) {
            lcd.setCursor(5, 1);
            lcd.print("BrackNum-");
            cycleCheck();
          }
        }
        if (oprbafr[changepointer] == '(') brcount++;
        if (oprbafr[changepointer] == ')') brcount--;
      }
      oprbafr[cyclepointer] = 'X';  // removes '-' sign from oprbafr
      if (cyclecheck == 1) {
        lcd.setCursor(5, 1);
        lcd.print("Bracket-");
        cycleCheck();
      }
    } else if ((oprbafr[cyclepointer] == '+') and (oprbafr[cyclepointer + 1] == '(')) {
      oprbafr[cyclepointer] = 'X';  // removes '+' sign from oprbafr
      if (cyclecheck == 1) {
        lcd.setCursor(5, 1);
        lcd.print("Bracket+");
        cycleCheck();
      }
    }
  }
  formCompress();
}

/*
void bracketMinus() {  // removes '-' before brackets (all numbers excluding multiplicators and dividers in bracket gets *(-1))
  int changepointer, brmovepointer, brcount, minbrcount;
  for (cyclepointer = 1; cyclepointer <= pointer; cyclepointer++) {  // run through whole oprbafr
    if ((oprbafr[cyclepointer] == '-') and (oprbafr[cyclepointer + 1] == '(')) {
      brcount = 1;
      for (changepointer = cyclepointer + 2; brcount > 0; changepointer++) {  // change all signs at NUMs after '-(' until ')'
        if (oprbafr[changepointer] == '(') { // add - at the beginning of bracket
          pointer++;
          for (brmovepointer = pointer - 1; brmovepointer >= changepointer; brmovepointer--){
            numbafr[brmovepointer + 1] = numbafr[brmovepointer];
            oprbafr[brmovepointer + 1] = oprbafr[brmovepointer];
          };
          oprbafr[changepointer] = '-';
          numbafr[changepointer] = 0;
          if (cyclecheck == 1) {
            lcd.setCursor(5, 1);
            lcd.print("BrackMov-");
            cycleCheck();
          };
          // cycle to the end of modified bracket
          minbrcount = 1;
          for (brmovepointer = changepointer + 2; (oprbafr[brmovepointer] != ')') and (minbrcount >= 1);brmovepointer++) {
            if (oprbafr[brmovepointer] == '(') minbrcount++;
            else if (oprbafr[brmovepointer] == ')') minbrcount--;
          }
          changepointer++;
        };
        if ((oprbafr[changepointer] == 'N') and (oprbafr[changepointer - 1] != '/') and (oprbafr[changepointer - 1] != '*')) {
          numbafr[changepointer] = (-numbafr[changepointer]);
          if (cyclecheck == 1) {
            lcd.setCursor(5, 1);
            lcd.print("BrackNum-");
            cycleCheck();
          }
        }
        if (oprbafr[changepointer] == '(') brcount++;
        if (oprbafr[changepointer] == ')') brcount--;
      }
      oprbafr[cyclepointer] = 'X';  // removes '-' sign from oprbafr
      if (cyclecheck == 1) {
        lcd.setCursor(5, 1);
        lcd.print("Bracket-");
        cycleCheck();
      }
    } else if ((oprbafr[cyclepointer] == '+') and (oprbafr[cyclepointer + 1] == '(')) {
      oprbafr[cyclepointer] = 'X';  // removes '+' sign from oprbafr
      if (cyclecheck == 1) {
        lcd.setCursor(5, 1);
        lcd.print("Bracket+");
        cycleCheck();
      }
    }
  }
  formCompress();
}
*/

void bracketRemove() {
  for (cyclepointer = pointer; cyclepointer > 1; cyclepointer--) {
    if ((oprbafr[cyclepointer] == ')') and (oprbafr[cyclepointer - 1] == 'N') and (oprbafr[cyclepointer - 2] == '(')) {
      oprbafr[cyclepointer] = 'X';
      oprbafr[cyclepointer - 2] = 'X';
      if (cyclecheck == 1) {
        lcd.setCursor(5, 1);
        lcd.print("BracketRem");
        cycleCheck();
      }
    }
  }
  formCompress();
}

void NUMcount() {
  NUMcounter = 0;
  for (cyclepointer = 0; cyclepointer <= pointer; cyclepointer++) {
    if (oprbafr[cyclepointer] == 'N') {
      NUMcounter++;
    }
  }
}

void printAns() {
  int ansstart = 0;
  int bafrans = abs(ans);
  for (ansstart = 15; bafrans > 9; ansstart--) bafrans = bafrans / 10;
  if (ans < 0) ansstart--;
  lcd.setCursor(ansstart, 1);
  lcd.print(ans);
}

// Computing functions
void computeAns() {  // TODO Computing ANS
  // TODO COMPUTING METHOD
  loopfault = 0;
  plusMinus();     // adds + or - to the number it belongs to
  bracketMinus();  // removes - before brackets
  NUMcount();      // counts number of NUMs in equation
  while (pointer > 1) {
    loopfault++;
    simpleMulDiv();
    simpleAdd();      // Addition of numbers next to each other
    bracketRemove();  // removes brackets around lonely NUM
    NUMcount();
    if (loopfault > maxop) {
      lcd.setCursor(0, 0);
      lcd.print("LoopFault ERROR ");
      cyclecheck = 1;  // Overflow triggers cyclecheck
      return;
    }
  }
  ans = numbafr[1];  // defining ANS
}

void simpleAdd() {  // Adds NUMs in reverse order
  for (cyclepointer = pointer; cyclepointer >= 1; cyclepointer--) {
    if ((oprbafr[cyclepointer] == 'N') and (oprbafr[cyclepointer - 1] == 'N')) {
      if (((cyclepointer >= pointer - 1) or (/*(cyclepointer < pointer) and */ ((oprbafr[cyclepointer + 1] != '/') and (oprbafr[cyclepointer + 1] != '*')))) and ((cyclepointer < 3) or ((oprbafr[cyclepointer - 2] != '/') and (oprbafr[cyclepointer - 2] != '*')))) {
        numbafr[cyclepointer - 1] = numbafr[cyclepointer - 1] + numbafr[cyclepointer];
        numbafr[cyclepointer] = 0;
        oprbafr[cyclepointer] = 'X';
        if (cyclecheck == 1) {
          lcd.setCursor(5, 1);
          lcd.print("SimpleAdd");
          cycleCheck();
        }
      }
    }
  }
  formCompress();
}

void simpleMulDiv() {  // TODO Multiplies NUMs in reverse order
  for (cyclepointer = pointer; cyclepointer > 1; cyclepointer--) {
    if ((oprbafr[cyclepointer] == 'N') and (oprbafr[cyclepointer - 2] == 'N') and ((oprbafr[cyclepointer - 1] == '/') or (oprbafr[cyclepointer - 1] == '*'))) {
      if (oprbafr[cyclepointer - 1] == '*') {
        numbafr[cyclepointer - 2] = numbafr[cyclepointer - 2] * numbafr[cyclepointer];
        numbafr[cyclepointer] = 0;
        oprbafr[cyclepointer] = 'X';
        oprbafr[cyclepointer - 1] = 'X';
        if (cyclecheck == 1) {
          lcd.setCursor(5, 1);
          lcd.print("MulDiv*");
          cycleCheck();
        }
      } else if (oprbafr[cyclepointer - 1] == '/') {
        numbafr[cyclepointer - 2] = numbafr[cyclepointer - 2] / numbafr[cyclepointer];
        numbafr[cyclepointer] = 0;
        oprbafr[cyclepointer] = 'X';
        oprbafr[cyclepointer - 1] = 'X';
        if (cyclecheck == 1) {
          lcd.setCursor(5, 1);
          lcd.print("MulDiv/");
          cycleCheck();
        }
      }
    }
  }
  formCompress();
}

// -----------------------------------------------------------SETUP FROM HERE-----------------------------------------------------------
// LONG SETUP (fancy)
void setup() {
  working(1);
  for (cyclepointer = 0; cyclepointer < maxop; cyclepointer++) numbafr[cyclepointer] = 0;
  for (cyclepointer = 0; cyclepointer < maxop; cyclepointer++) oprbafr[cyclepointer] = 'X';
  randomSeed(analogRead(0));
  pinMode(READYled, OUTPUT);
  pinMode(WORKINGled, OUTPUT);
  lcd.init();
  lcd.noCursor();
  lcd.noBlink();
  lcd.backlight();       //Backlight on
  if (skipintro == 0) {  // Long fancy boot
    char intro[] = { "OpenCALC" };
    lcd.setCursor(0, 0);
    lcd.print("|              |");
    delay(200);
    lcd.setCursor(0, 1);
    lcd.print("|              |");
    delay(500);
    lcd.setCursor(4, 0);
    for (pointer = 0; pointer < 8; pointer++) {
      delay(50);
      lcd.print(intro[pointer]);
    }
    delay(500);
    for (pointer = 1; pointer < 15; pointer++) {  // progress bar
      lcd.setCursor(pointer, 1);
      delay(random(700));
      lcd.print(char(255));  // print blackbox
    }
    delay(random(700));
    lcd.setCursor(0, 1);
    lcd.print("|      OK      |");
    delay(1500);  // Wait for display to show info
  } else {        // Short efficient boot
    lcd.setCursor(0, 0);
    lcd.print("------BOOT------");
    lcd.setCursor(0, 1);
    lcd.print("-------OK-------");
    delay(1000);
  }
  lcd.clear();  // Clear display
  pointer = 1;
  keypad.setHoldTime(500);
  keypad.setDebounceTime(25);
  working(0);
  writeKey(0);
}

//-----------------------------------------------------------LOOP FROM HERE-----------------------------------------------------------------
void loop()  // Main code block
{
  key = keypad.getKey();
  if ((key) or (backkey != 'X')) {
    working(1);
    if (backkey != 'X') key = backkey;
    backkey = 'X';

    if ((key == 'C')) {  // TODO Clear last action (add bracket support)
      if (pointer > 1) {

        oprbafr[pointer] = 'X';
        numbafr[pointer] = 0;
        backkey = oprbafr[pointer - 1];
        oprbafr[pointer - 1] = 'X';
        if (oprbafr[pointer - 2] == '(') bracketcount--;
        if (oprbafr[pointer - 2] == ')') bracketcount++;
        pointer--;
        pointer--;
        writeKey(0);
        bracketCheck();
      }
    }

    if (computed == 1) {  // TODO Clear whole display
      for (cyclepointer = 0; cyclepointer < maxop; cyclepointer++) numbafr[cyclepointer] = 0;
      for (cyclepointer = 0; cyclepointer < maxop; cyclepointer++) oprbafr[cyclepointer] = 'X';
      pointer = 1;
      computed = 0;
      numstat = 0;
      Fstat = 0;
      bracketcount = 0;
      lcd.clear();
      writeKey(0);
    }

    if (key == '=') {  // DONE Compute output
      if ((bracketcount == 0 and (pointer > 1)) and ((oprbafr[pointer - 1] == 'N') or (oprbafr[pointer - 1] == ')'))) {
        eqflag = 1;
        pointer--;
        computeAns();
        computed = 1;
        if (pcheck == 1) pointerCheck();  // Pointer check - top left shows pointer count
        lcd.setCursor(0, 1);
        lcd.print("Ans =           ");
        printAns();
        if (obcheck == 1) oprbafrCheck();
        if (nbcheck == 1) numbafrCheck();
        if (pcheck == 1) pointerCheck();  // Pointer check - top left shows pointer count
        eqflag = 0;
        delay(200);
      }
    }

    // Operators
    else if (key == '-') {  // DONE substraction
      if ((pointer == 1) or (oprbafr[pointer - 1] == 'N') or (oprbafr[pointer - 1] == '(') or (oprbafr[pointer - 1] == ')')) {
        oprbafr[pointer] = key;
        numbafr[pointer] = 0;
        numstat = 0;
        Fstat = 0;
        pointer++;
        writeKey(0);
      }
    }

    else if ((key == '+') or (key == '*') or (key == '/')) {  // DONE addition, multiplication ,division
      if (((oprbafr[pointer - 1] == 'N') or (oprbafr[pointer - 1] == ')')) and (pointer > 1)) {
        oprbafr[pointer] = key;
        numbafr[pointer] = 0;
        numstat = 0;
        Fstat = 0;
        pointer++;
        writeKey(0);
      }
    }

    /*
      // Decimal point - TODO .................
      else if (key=='.'){

      }
    */

    /*
      // Function - TODO .................
      else if (key=='F'){

      }
    */

    else if (key == 'F') {  // Function used as cyclecheck switch until NUM errors solved
      if (cyclecheck == 1) cyclecheck = 0;
      else
        cyclecheck = 1;
      writeKey(0);
    }

    // Numbers
    else if ((key == '0') or (key == '1') or (key == '2') or (key == '3') or (key == '4') or (key == '5') or (key == '6') or (key == '7') or (key == '8') or (key == '9')) {
      if ((oprbafr[pointer - 1] != ')') or (pointer == 1)) {  // TODO number after number after backspace
        if (numstat == 1) {
          numbafr[pointer - 1] = ((numbafr[pointer - 1] * 10) + (key - '0'));
        }
        if (numstat == 0) {
          numbafr[pointer] = numbafr[pointer] + (key - '0');
          oprbafr[pointer] = 'N';
          numstat = 1;
          pointer++;
        }
        Fstat = 0;
        writeKey(0);
      }
    }

    else if (key == 'N') {
      oprbafr[pointer] = 'N';
      numstat = 1;
      pointer++;
      Fstat = 0;
      writeKey(0);
    }

    else if (key == '(') {  // DONE Open bracket
      if (((pointer == 1) || (oprbafr[pointer - 1] != 'N')) && (oprbafr[pointer - 1] != ')')) {
        bracketcount++;
        oprbafr[pointer] = '(';
        numbafr[pointer] = 0;
        numstat = 0;
        Fstat = 0;
        pointer++;
        writeKey(0);
        bracketCheck();
      }
    }

    else if (key == ')') {  // DONE Close bracket
      if ((bracketcount > 0) && ((oprbafr[pointer - 1] == 'N') || (oprbafr[pointer - 1] == ')'))) {
        bracketcount--;
        oprbafr[pointer] = ')';
        numbafr[pointer] = 0;
        numstat = 0;
        Fstat = 0;
        pointer++;
        writeKey(0);
        bracketCheck();
      }
    }
    working(0);
  }
}
