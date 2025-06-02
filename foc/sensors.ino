byte gotSensor() {
  //check to see if there's detection from a sensor
  //gotta kinda debounce this a little too.
  //keep track of detection, time since last detection, and distance

  int trigPin[3] = { TRIG_PIN1, TRIG_PIN2, TRIG_PIN3 };
  int echoPin[3] = { ECHO_PIN1, ECHO_PIN2, ECHO_PIN3 };

  // 1: sensor 1, 2: sensor 2, 3: sensor 1 & 2
  // 4: sensor 3, 5: sensor 1 & 3, 6: sensor 2 & 3, 7: all
  byte results = 100;


//TODO: Change mack to 3!

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
     //Serial.print("inches = ");Serial.println(inches);

    if (inches < 72 && inches > 1) {
      if (inches - 12 > maxSensor[i]) {
        maxSensor[i] = inches - 12;
        Serial.print("Max Sensor "); Serial.print(i+1); Serial.print(" = "); Serial.println(maxSensor[i]);
      } else {
        //Serial.print("Sensor "); Serial.print(i+1); Serial.print(" = "); Serial.print(results);
        //if this sensor is closer than the others then set the result inches we're returning and the currentSensor index 
        //that's used in the ledpatters to compute % up the tree
        
        //if the maxSensor for this sensor is 20 then the sensor isn't working right and we need to ignore it
        //greater than 20 we're good
        if (maxSensor[i] > 20) {
          if (results > inches) {
            results = inches;
            currentSensor = i;
          }
        }
        //Serial.print("- "); Serial.println(results);
        
        //Serial.println("*** DETECTION ***");
      }
    } 
    
  }
  //Serial.println(results);
  //reset the maxInches every once (about every 3 minutes) in a while incase something changes about fixed objects in front of the sensor
  if (random(1000) == 5) {
    maxSensor[0] = 40;
    maxSensor[1] = 40;
    maxSensor[2] = 40;
    Serial.println("Sensor Reset");
  }
  //Serial.println(results);
  if (results == 100) results = 0;
  return results;
}