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
