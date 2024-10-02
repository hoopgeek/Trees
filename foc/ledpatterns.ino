//legs 60 LEDs down each
// 1||2 3||4 5||6
//     1    2    3

#define MAX_UINT16 65536L

/**
 * Sets an LED for a specific branch.
 *
 * @param branch The zero-indexed branch index.
 * @param ledIndex The index of the LED starting from the top of the tree. [0, SIDE_LENGTH]
 * @param color The color to set the LEDs.
 */
inline void setBranchLed(int branch, int ledIndex, CRGB color) {
  if (ledIndex >= 0 && ledIndex < SIDE_LENGTH) {
    leds[ledIndex + branch * BRANCH_LENGTH] = color;
    leds[(BRANCH_LENGTH - ledIndex - 1) + branch * BRANCH_LENGTH] = color;
  }
}

/**
 * Sets all LEDs on all branches to the same color.
 *
 * @param ledIndex The index of the LED starting from the top of the tree. [0, SIDE_LENGTH]
 * @param color The color to set the LEDs.
 */
inline void setAllBranchLed(int ledIndex, CRGB color) {
  setBranchLed(0, ledIndex, color);
  setBranchLed(1, ledIndex, color);
  setBranchLed(2, ledIndex, color);
}

/**
 * Gets a random time per branch to ensure all the branches aren't synced.
 *
 * @param branch The index of the branch to get the time for.
 */
inline uint16_t randomBranchTime(int branch) {
  return theClock() + branch * 7919 + TREE_NUMBER * 7723;
}

/**
 * Returns the theta value of a tree offset by the time.
 *
 * @param time The time. Range [0, 65536]
 * @returns The theta value. Range [0, 65536]
 */
inline uint16_t treeTheta(uint16_t time) {
  return (((TREE_NUMBER - 1) * MAX_UINT16) / min(aliveTreesCount(), 1) - time) % MAX_UINT16;
}

/**
 * Returns a brightness value in which the brightness moves around the forest
 * based on time and is spread out among spread * 2 trees.
 *
 * @param time The time. Range [0, 65536]
 * @param spread How far the high value should spread in number of trees.
 * @returns The brightness of the tree at this time.
 */
uint16_t cycleTree(uint16_t time, uint8_t spread) {
  const uint8_t aliveTrees = min(aliveTreesCount(), 1);
  return max(
    (abs(MAX_UINT16 / 2 - treeTheta(time)) * aliveTrees - (aliveTrees - spread * 2) * MAX_UINT16 / 2) / spread,
    0L);
}

void activePattern() {
  if (millis() - startActiveTime < 3000) {
    //start with a base coat of the default blue spruce
    blueSpruce();
    //layer on growing up the tree for 3 seconds on the activated side
    //do the grow up tree routine
    for (int i = 0; i < (millis() - startActiveTime) / 50; ++i) {
      setAllBranchLed(59-i, CHSV(floor(256 / 60) * i, 255, 255));
      // if (activeSensor == 1 || activeSensor == 2) {
        // setBranchLed(1, 59-i, CHSV(floor(256 / 60) * i, 255, 255));
      // }
      // if (activeSensor == 3 || activeSensor == 1) {
        // setBranchLed(0, 59-i, CHSV(floor(256 / 60) * i, 255, 255));
      // }
      // if (activeSensor == 2 || activeSensor == 3) {
        // setBranchLed(2, 59-i, CHSV(floor(256 / 60) * i, 255, 255));
      // }
    }
    //twinkle the leading LED
    if (random8(3) == 1) {
      int lastLED = 59 - ((millis() - startActiveTime) / 50 - 1);
      if (lastLED < 0) lastLED = 0;  //just in case
        setAllBranchLed(lastLED, CRGB::White);  //twinkle the leading LED
      // if (activeSensor == 1 || activeSensor == 2) {
        // setBranchLed(1, lastLED, CRGB::White);  //twinkle the leading LED
      // }
      // if (activeSensor == 3 || activeSensor == 1) {
        // setBranchLed(0, lastLED, CRGB::White);  //twinkle the leading LED
      // }
      // if (activeSensor == 2 || activeSensor == 3) {
        // setBranchLed(2, lastLED, CRGB::White);  //twinkle the leading LED
      // }
    }
  } else {
    //what do we do after fully active?
    //color wash (offset steps every 3/60 seconds: 50,000 microseconds)
    //by using getNodeTime() all the trees will have their patterns in sync
    int offset = (mesh.getNodeTime() / 50000L) % 60;
    for (int i = 0; i < 60; ++i) {
      setAllBranchLed(i, CHSV(floor(256 / 60) * ((offset + i) % 60) , 255, 255));
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
      leds[i] = CHSV(97, 255, 32);  //greenish
    } else {
      leds[i] = CHSV(134, 255, 32);  //blueish
    }
  }
}

//not conneted to other trees on the mesh,
//light up bottom LEDs as a diagnostic

void offlineTree() {
  //light the bottom LEDs
  leds[60] = CHSV(0, 255, 64);
  leds[180] = CHSV(0, 255, 64);
  leds[300] = CHSV(0, 255, 64);
}

//all off
void darkForest() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  //ground lights to prevent tripping
  leds[60] = CHSV(255, 0, 32);
  leds[180] = CHSV(255, 0, 32);
  leds[300] = CHSV(255, 0, 32);
}

//duplicate the last 10 LEDs for strands that are too long
//(AKA, coding some duct tape on it and calling it good)
void tooManyLEDsFix() {
  memcpy(&leds[NUM_LEDS-11], &leds[NUM_LEDS-21], 30);
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
  if (millis() > patternTime + 360) {
    patternTime = 0;
  }
  long pt = millis() - patternTime; 
  if (pt > 0 && pt <= 40
    || pt > 80 && pt <= 120
    || pt > 160 && pt <= 200
    || pt > 240 && pt <= 280
    || pt > 320 && pt <= 360) {
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
    byte thisColor = random8();
    for (int i = 0; i < NUM_LEDS; ++i) {
      leds[i] = CHSV(thisColor, 255, 255);
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
void patternGradientWipe() {
  // A slow changing base gradient that is periodically "wiped" away with a
  // gradient of a new set of hues.
  // https://www.desmos.com/calculator/kbbzcf0hgr

  // A value that is used to map each LED index onto a shifting gradient. Higher
  // gives the LEDs a more uniform color.
  // Range: [1, NUM_LED]
  static const int GRADIENT_SPREAD = 1;

  // How quickly the base gradient changes. Lower is faster.
  static const int GRADIENT_SPEED = 20;

  // How far the hue should jump on the wipe.
  // Range: [0, 255]
  static const int WIPE_DISTANCE = 64;

  // How frequently wipes occur where the number is the denominator of how often
  // the tree should be wiping. For example: A value of 3 indicates that a wipe
  // will be happening 1/3 of the time. The actual period time is specified by
  // GRADIENT_SPEED.
  static const int WIPE_PERIOD = 3;

  // How fast the wipe moves across the tree. Lower is faster.
  static const int WIPE_SPEED = 40;

  // The "value" of the HSV component.
  // Range: [0, 255]
  static const int VALUE = 255;

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

//swingingLights
void patternSwingingLights() {
  // The lights to include in this pattern.
  static const CRGB colors[] = { CRGB(255, 0, 0), CRGB(0, 255, 0), CRGB(0, 0, 255) };
  static const uint8_t colorsSize = 3;

  static const int PERIOD = 5000;  // The lights complete a cycle every PERIOD ms.
  static const int SPREAD = 6;     // Each light spans 1/SPREAD of the full branch.

  static CRGB scratchPad[SIDE_LENGTH];

  for (int b = 0; b < 3; b++) {
    fill_solid(scratchPad, SIDE_LENGTH, CRGB::Black);
    const fract16 angle = (randomBranchTime(b) % PERIOD) * MAX_UINT16 / PERIOD;

    for (int l = 0; l < colorsSize; ++l) {
      // Offset lights evenly.
      const fract16 lightOffset = l * MAX_UINT16 / colorsSize;
      // Figure out the center of each light.
      const fract16 lightPos = sin16(lightOffset + angle) + MAX_UINT16 / 2;

      for (int i = 0; i < SIDE_LENGTH; ++i) {
        const fract16 ledPos = i * MAX_UINT16 / SIDE_LENGTH;
        // Has scale component so it can go negative next line.
        const fract16 absDist = abs(lightPos - ledPos);
        const fract16 scaledDist = max(MAX_UINT16 / SPREAD - absDist, 0L) * SPREAD;

        CRGB lightColor = CRGB(colors[l]);
        lightColor.r = ((uint32_t)lightColor.r * scaledDist) / MAX_UINT16;
        lightColor.g = ((uint32_t)lightColor.g * scaledDist) / MAX_UINT16;
        lightColor.b = ((uint32_t)lightColor.b * scaledDist) / MAX_UINT16;

        scratchPad[i] = colorAddWithBloom(scratchPad[i], lightColor);
      }

      for (int i = 0; i < SIDE_LENGTH; ++i) {
        setBranchLed(b, i, scratchPad[i]);
      }
    }
  }
}

//Psychedellic
void patternPsychedellic() {
  // Change this to mkae it prettier.
  static const CRGBPalette16 palette(
    CRGB(0, 255, 0),
    CRGB(255, 0, 0),
    CRGB(0, 0, 255));

  for (int b = 0; b < 3; b++) {
    const uint16_t t = randomBranchTime(b) >> 4;

    for (uint32_t i = 0; i < SIDE_LENGTH; ++i) {

      // This has a bunch of random primes, feel free to change them!
      uint8_t noise = perlinNoise(
        // Offset LEDs in the x domain with a bit of time.
        i * 40 + b * 6607,
        // Offset sampling in the y domain over time.
        b * 6899 + TREE_NUMBER * 7561 + t / 2);

      setBranchLed(b, i, ColorFromPalette(palette, noise, 255, LINEARBLEND_NOWRAP));
      // setBranchLed(b, i, CHSV(noise, 255, 255));
    }
  }
}

//Racing Lights
void patternRacingLights() {
  static const uint8_t colorsSize = 3;

  static const int PERIOD = 2;  // The lights will only be on the branch 1/PERIOD of the time.
  static const int SPREAD = 6;  // Each light spans 1/SPREAD of the full branch.
  static const int NUM_LIGHTS = 6;
  static const int LIGHT_LENGTH = 8;

  static CRGB scratchPad[SIDE_LENGTH];

  for (int b = 0; b < 3; b++) {
    fill_solid(scratchPad, SIDE_LENGTH, CRGB::Black);

    for (int l = 0; l < NUM_LIGHTS; ++l) {
      // Make the lights run at different speeds.
      const uint16_t lightPos = SIDE_LENGTH - (((randomBranchTime(b) * (l + 1) + l * 997) >> 7) % (SIDE_LENGTH * PERIOD + LIGHT_LENGTH * 2) - LIGHT_LENGTH * 2);

      for (uint i = 0; i < SIDE_LENGTH; ++i) {
        const int ledPos = i;
        int intensity = max(((lightPos - ledPos) * 255) / LIGHT_LENGTH, 0);
        if (intensity > 255) {
          intensity = 0;
        }

        const uint8_t hue = b * 32 + l * 16;

        scratchPad[i] = colorAddWithBloom(scratchPad[i], CHSV(hue, 255, intensity));
      }

      for (int i = 0; i < SIDE_LENGTH; ++i) {
        setBranchLed(b, i, scratchPad[i]);
      }
    }
  }
}

//Rainbow forest
void patternRainbowForest() {
  uint8_t hue = treeTheta(theClock() * 28) / 256;
  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = CHSV(hue, 255, 255);
  }
}

//Round the trees
void patternRoundTheTrees() {
  static const uint8_t NUM_BRIGHT_SPOTS = 3;

  uint16_t brightness = 0L;
  for (int b = 0; b < NUM_BRIGHT_SPOTS; ++b) {
    brightness = max(brightness, cycleTree((MAX_UINT16 * b / NUM_BRIGHT_SPOTS) + theClock() * 28, 1));
  }

  for (int i = 0; i < SIDE_LENGTH; ++i) {
    setAllBranchLed(
      i,
      CHSV(
        treeTheta(theClock() * 8) / 256,
        255,
        brightness / 256));
  }
}
