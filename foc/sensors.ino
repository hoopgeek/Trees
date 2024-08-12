byte gotSensor() {
  //check to see if there's detection from a sensor
  //gotta kinda debounce this a little too.
  //keep track of detection, time since last detection, and distance

  int trigPin[3] = { TRIG_PIN1, TRIG_PIN2, TRIG_PIN3 };
  int echoPin[3] = { ECHO_PIN1, ECHO_PIN2, ECHO_PIN3 };

  // 1: sensor 1, 2: sensor 2, 3: sensor 1 & 2
  // 4: sensor 3, 5: sensor 1 & 3, 6: sensor 2 & 3, 7: all
  byte results = 0;

  for (int i = 0; i < 3; ++i) {

    digitalWrite(13, LOW);
    delayMicroseconds(2);

    digitalWrite(trigPin[i], HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin[i], LOW);

    // Wait for pulse on echo pin
    //TODO: need to put a timeout limit on this incase the sensor breaks of is dissconnected.
    // commented because it locks up the program when there's no sensor attached.  while ( digitalRead(ECHO_PIN) == 0 );

    unsigned long pulse_width;
    float cm;
    float inches;


    // Measure how long the echo pin was held high (pulse width)
    // Note: the micros() counter will overflow after ~70 min

    pulse_width = pulseIn(echoPin[i], HIGH, 15000);

    inches = pulse_width / 148.0;
    //  Serial.println(pulse_width);
    // Serial.println(inches);

    if (inches < DETECTINCHES && inches > 1) {
      if (i == 0) results += 1;
      if (i == 1) results += 2;
      if (i == 2) results += 4;
      //Serial.println("*** DETECTION ***");
    } else {
      //Serial.println("");
      //return false;
    }
  }
  // Serial.println(results);
  return results;
}