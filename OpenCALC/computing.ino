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

void simpleMulDiv() {  
  for (cyclepointer = 1; cyclepointer <= pointer; cyclepointer++) {
    if ((oprbafr[cyclepointer] == 'N') and (oprbafr[cyclepointer + 2] == 'N') and ((oprbafr[cyclepointer + 1] == '/') or (oprbafr[cyclepointer +1] == '*'))) {
      if (oprbafr[cyclepointer + 1] == '*') {
        numbafr[cyclepointer + 2] = numbafr[cyclepointer + 2] * numbafr[cyclepointer];
        numbafr[cyclepointer] = 0;
        oprbafr[cyclepointer] = 'X';
        oprbafr[cyclepointer + 1] = 'X';
        if (cyclecheck == 1) {
          lcd.setCursor(5, 1);
          lcd.print("MulDiv*");
          cycleCheck();
        }
      } else if (oprbafr[cyclepointer + 1] == '/') {
        numbafr[cyclepointer + 2] = numbafr[cyclepointer] / numbafr[cyclepointer + 2];
        numbafr[cyclepointer] = 0;
        oprbafr[cyclepointer] = 'X';
        oprbafr[cyclepointer + 1] = 'X';
        if (cyclecheck == 1) {
          lcd.setCursor(5, 1);
          lcd.print("MulDiv/");
          cycleCheck();
        }
      }
      break; // End simpleMulDiv() function prematuraly anc continue with ComputeAns() cycle
    }
  }
  formCompress();
}