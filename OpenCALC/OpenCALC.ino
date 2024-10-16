// Main program file

// Requires:
// LiquidCrystal_I2C V 1.12 by Frank de Brabander
// Keypad V 3.1.1 by Mark Stanley, Alexander Brevig

#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

// Constants
const int maxop = 100;     // maximum pointer
const bool pcheck = 0;     // '1' enables pointerCheck function
const bool nbcheck = 0;    // '1' enables numbafrCheck function
const bool numcheck = 0;   // '1' enables numCheck function
const bool skipintro = 1;  // '1' skips boot animation
bool cyclecheck = 0;       // '1' enables cycleCheck function
bool obcheck = 0;          // '1' enables oprbafrCheck function

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
char key;                           // pressed key
char backkey;                       // previous key for clear function
char ansbafr[10];                   
char oprbafr[maxop];                // oprbafr - operand buffer (X=nothing, N=NUM)
int numbafr[maxop];                 // numbafr - number buffer
int bracketcount = 0;               // current deepness of brackets
int pointer;                        // main pointer identifying length of formula
int cyclepointer;                   // common pointer for use when cycling through arrays
int NUMcounter;                     // number of NUMs in equation
int bracketstart, bracketend = 0;
bool isbracket = 0;
bool computed, numstat, Fstat = 0;
bool workingstat = 0;
bool eqflag = 0;
bool keyback = 0;
float ans = 0;                      // variables for computing ANS
int loopfault = 0;

// Debug variables
int obpointer = 0;
int nbpointer = 0;

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

    if (key == '=') {  // Compute output
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
    else if (key == '-') {  // Substraction
      if ((pointer == 1) or (oprbafr[pointer - 1] == 'N') or (oprbafr[pointer - 1] == '(') or (oprbafr[pointer - 1] == ')')) {
        oprbafr[pointer] = key;
        numbafr[pointer] = 0;
        numstat = 0;
        Fstat = 0;
        pointer++;
        writeKey(0);
      }
    }

    else if ((key == '+') or (key == '*') or (key == '/')) {  // Addition, Multiplication, Division
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
