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

void bracketCheck() {  // DONE Prints number of open brackets
  lcd.setCursor(0, 1);
  if (bracketcount > 0) {
    lcd.print("(x");
    lcd.print(bracketcount);
    lcd.print(" ");
  } else
    lcd.print("   ");
}
