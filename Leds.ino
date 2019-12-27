void timeToLeds(uint8_t x, uint8_t y, uint8_t z) { //x - hours, y - minutes, z - seconds

  leds.clear();

  uint8_t v;// tens of hours
  uint8_t w;//ones of hours

  w = x % 10;
  v = (x / 10) % 10;
  Serial.print("Tens of hours: ");
  Serial.println(v);
  Serial.print("Ones of hours: ");
  Serial.println(w);


  for (int i = 0; i <= sizeof(DH) - 1; i++) {
    if (i < v) {
      leds.setPixelColor(DH[i], colors[0]);
    }
    else {
      leds.setPixelColor(DH[i], colors[1]);
    }
  }

  for (int i = 0; i <= sizeof(JH) - 1; i++) {
    if (i < w) {
      leds.setPixelColor(JH[i], colors[0]);
    }
    else {
      leds.setPixelColor(JH[i], colors[1]);
    }
  }

  uint8_t t;//tens of minutes
  uint8_t u;//ones of minutes

  u = y % 10;
  t = (y / 10) % 10;
  Serial.print("Tens of minutes: ");
  Serial.println(t);
  Serial.print("Ones of minutes: ");
  Serial.println(u);

  for (int i = 0; i <= sizeof(DM) - 1; i++) {
    if (i < t) {
      leds.setPixelColor(DM[i], colors[2]);
    }
    else {
      leds.setPixelColor(DM[i], colors[3]);
    }
  }

  for (int i = 0; i <= sizeof(JM) - 1 ; i++) {
    if (i < u) {
      leds.setPixelColor(JM[i], colors[2]);
    }
    else {
      leds.setPixelColor(JM[i], colors[3]);
    }
  }

  uint8_t r;//tens of seconds
  uint8_t s;//ones of seconds

  s = z % 10;
  r = (z / 10) % 10;
  Serial.print("Tens of minutes: ");
  Serial.println(r);
  Serial.print("Ones of seconds: ");
  Serial.println(s);

  for (int i = 0; i <= sizeof(DS) - 1; i++) {
    if (i < r) {
      leds.setPixelColor(DS[i], colors[4]);
    }
    else {
      leds.setPixelColor(DS[i], colors[5]);
    }
  }

  for (int i = 0; i <= sizeof(JS) - 1 ; i++) {
    if (i < s) {
      leds.setPixelColor(JS[i], colors[4]);
    }
    else {
      leds.setPixelColor(JS[i], colors[5]);
    }
  }

  leds.show();


}
