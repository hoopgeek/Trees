//legs 60 LEDs down each
// 1||2 3||4 5||6
//     1    2    3

/**
 * Sets an LED for a specific branch.
 *
 * @param branch The zero-indexed branch index.
 * @param ledIndex The index of the LED starting from the top of the tree. [0, SIDE_LENGTH]
 * @param color The color to set the LEDs.
 */
inline void setBranchLed(int branch, int ledIndex, CRGB color) {
  leds[ledIndex + branch * BRANCH_LENGTH] = color;
  leds[(BRANCH_LENGTH - ledIndex - 1) + branch * BRANCH_LENGTH] = color;
}

/**
 * Sets all LEDs on all branches to the same color.
 * @param ledIndex The index of the LED starting from the top of the tree. [0, SIDE_LENGTH]
 * @param color The color to set the LEDs.
 */
inline void setAllBranchLed(int ledIndex, CRGB color) {
  setBranchLed(0, ledIndex, color);
  setBranchLed(1, ledIndex, color);
  setBranchLed(2, ledIndex, color);
}

void activePattern() {
  if (millis() - startActiveTime < 3000) {
    //start with a base coat of the default blue spruce
    blueSpruce();
    //layer on growing up the tree for 3 seconds on the activated side
    //do the grow up tree routine
    for (int i = 0; i < (millis() - startActiveTime) / 50; ++i) {
      if (activeSensor == 1 || activeSensor == 2) {
        setBranchLed(1, i, CHSV(floor(256 / 60) * i, 255, 255));
      }
      if (activeSensor == 3 || activeSensor == 1) {
        setBranchLed(0, i, CHSV(floor(256 / 60) * i, 255, 255));
      }
      if (activeSensor == 2 || activeSensor == 3) {
        setBranchLed(2, i, CHSV(floor(256 / 60) * i, 255, 255));
      }
    }
    //twinkle the leading LED
    if (random8(3) == 1) {
      int lastLED = (millis() - startActiveTime) / 50 - 1;
      if (lastLED < 0) lastLED = 0;  //just in case
      if (activeSensor == 1 || activeSensor == 2) {
        setBranchLed(1, lastLED, CRGB::White);  //twinkle the leading LED
      }
      if (activeSensor == 3 || activeSensor == 1) {
        setBranchLed(0, lastLED, CRGB::White);  //twinkle the leading LED
      }
      if (activeSensor == 2 || activeSensor == 3) {
        setBranchLed(2, lastLED, CRGB::White);  //twinkle the leading LED
      }
    }
  } else {
    //what do we do after fully active?
    //color wash (offset steps every 3/60 seconds: 50,000 microseconds)
    //by using getNodeTime() all the trees will have their patterns in sync
    int offest = (mesh.getNodeTime() / 50000L) % 60;
    for (int i = 0; i < 60; ++i) {
      setAllBranchLed(i, CHSV(floor(256 / 60) * offset, 255, 255));
    }
  }
}

// colorful test pattern, different color for each leg for orentation and LED check
void testPattern() {
  for (int i = 0; i < SIDE_LENGTH; ++i) {
    setBranchLed(0, i, CHSV(0, 255, 32));
    setBranchLed(1, i, CHSV(160, 255, 32));
    setBranchLed(2, i, CHSV(96, 255, 32));
  }
}


//default blue spruce mode
void blueSpruce() {
  for (int i = 0; i < NUM_LEDS; ++i) {
    //the random blue tint is to make it shimmer
    if (random8(10) == 1) {
      leds[i] = CHSV(93, 255, 32);  //blueish
    } else {
      leds[i] = CHSV(40, 255, 32);  //greenish
    }
  }
}

//all off
void darkForest() {
  // A sexy little command to set all the LEDs to black.
  fill_solid(leds, NUM_LEDS, CRGB::Black);
}

//tree rotate
void patternRotate() {
  for (int i = 0; i < SIDE_LENGTH; ++i) {
    setBranchLed(0, i, CHSV((theClock() / 20) % 256, 255, 255));
    setBranchLed(1, i, CHSV((theClock() / 20 + 83) % 256, 255, 255));
    setBranchLed(2, i, CHSV((theClock() / 20 + 166) % 256, 255, 255));
    leds[random(NUM_LEDS)] = CRGB::White;
  }
}

//sparkle
void patternSparkle() {
  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = CRGB::Black;
    if (random8(100) == 1) {
      leds[i] = CRGB::White;
    }
  }
}

//tree strobe
void patternStrobe() {
  if (random8(100) == 1 && patternTime == 0) {
    patternTime = millis();
  }
  if (millis() > patternTime + 200) {
    patternTime = 0;
  }
  if (patternTime > 0) {
    for (int i = 0; i < NUM_LEDS; ++i) {
      leds[i] = CRGB::White;
    }
  } else {
    for (int i = 0; i < NUM_LEDS; ++i) {
      leds[i] = CRGB::Black;
    }
  }
}

//colorful
void patternColorful() {
  if (random8(100) == 1 && patternTime == 0) {
    patternTime = millis();
  }
  if (millis() > patternTime + 200) {
    patternTime = 0;
  }
  if (patternTime > 0) {
    for (int i = 0; i < NUM_LEDS; ++i) {
      leds[i] = CHSV(random8(), 255, 255);
    }
  } else {
    for (int i = 0; i < NUM_LEDS; ++i) {
      leds[i] = CRGB::Black;
    }
  }
}

//fire
void patternFire() {
  static const CHSVPalette16 firePalette(
      CHSV(0, 255, 8), CHSV(16, 249, 45), CHSV(30, 246, 113), CHSV(35, 200, 192));

  for (int b = 0; b < 3; b++) {
    for (int i = 0; i < SIDE_LENGTH; i++) {
      // This has a bunch of random primes, feel free to change them!
      uint8_t noise = perlinNoise(
        // Offset LEDs in the x domain.
        i * 40 + b * 997 + theClock() / 8,
        // Offset sampling in the y domain over time.
        b * 6899 + TREE_NUMBER * 7561 + theClock() / 2);
      setBranchLed(b, i, ColorFromPalette(firePalette, noise, 255, LINEARBLEND_NOWRAP));
    }
  }
}

//gradient wipe
void gradientWipe() {
  // A slow changing base gradient that is periodically "wiped" away with a
  // gradient of a new set of hues.
  // https://www.desmos.com/calculator/kbbzcf0hgr

  // A value that is used to map each LED index onto a shifting gradient. Higher
  // gives the LEDs a more uniform color.
  // Range: [1, NUM_LED]
  static int GRADIENT_SPREAD = 1;

  // How quickly the base gradient changes. Lower is faster.
  static int GRADIENT_SPEED = 20;

  // How far the hue should jump on the wipe.
  // Range: [0, 255]
  static int WIPE_DISTANCE = 64;

  // How frequently wipes occur where the number is the denominator of how often
  // the tree should be wiping. For example: A value of 3 indicates that a wipe
  // will be happening 1/3 of the time. The actual period time is specified by
  // GRADIENT_SPEED.
  static int WIPE_PERIOD = 3;

  // How fast the wipe moves across the tree. Lower is faster.
  static int WIPE_SPEED = 40;

  // The "value" of the HSV component.
  // Range: [0, 255]
  static int VALUE = 255;

  long t = theClock();

  for (int i = 0; i < SIDE_LENGTH; ++i) {
    // Calculate wipe effect.
    long wipe = (i - t / WIPE_SPEED) / (SIDE_LENGTH * WIPE_PERIOD) * WIPE_DISTANCE;

    // Calculate base gradient.
    long gradient = i / GRADIENT_SPEED + t / GRADIENT_SPEED;

    // Smash 'em together into one cool effect.
    int h = abs(wipe + gradient) % 256;

    setAllBranchLed(i, CHSV(h, 255, VALUE));
  }
}
