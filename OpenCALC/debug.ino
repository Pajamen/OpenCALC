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
