// Processing functions

void printAns() {
  int ansstart = 0;
  int bafrans = abs(ans);
  for (ansstart = 15; bafrans > 9; ansstart--) bafrans = bafrans / 10;
  if (ans < 0) ansstart--;
  lcd.setCursor(ansstart, 1);
  lcd.print(ans);
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
