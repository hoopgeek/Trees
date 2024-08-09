#include <FastLED.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <painlessMesh.h>

#define MESH_PREFIX "FoC"
#define MESH_PASSWORD "JsFS)weI9J#*ij4F~jn="
#define MESH_PORT 5555  // default port

Scheduler userScheduler;
painlessMesh mesh;

#define TREE_NUMBER 1  //each tree is numbered in order based on where it is located
#define DETECTINCHES 12

#define SIDE_LENGTH 60
#define BRANCH_LENGTH SIDE_LENGTH * 2
#define NUM_LEDS BRANCH_LENGTH * 3

#define DATA_PIN 23
#define CLOCK_PIN 18

#define DEFAULT 0
#define ACTIVATING 1
#define ACTIVATED 2
#define DRAW 3
#define ROTATE 4
#define SPARKLE 5
#define STROBE 6
#define COLORFUL 7
#define FIRE 8
#define GRADIENTWIPE 9
#define FORESTPATTERENS 6

#define TRIG_PIN1 13
#define TRIG_PIN2 14
#define TRIG_PIN3 26
#define ECHO_PIN1 12
#define ECHO_PIN2 27
#define ECHO_PIN3 25

int activeTimeout = 10000;  //10 seconds to activate all the trees
int treeState = 0;          //0 is default blue spruce rest state
long startActiveTime = 0;
long lastActiveTime = 0;
int activeSensor = 1;  //must be 1, 2, or 3
long pullTime = 0;
long activateTime = 0;

long clockOffset = 0;
long lastSensor = 0;
long lastImAlive = 0;
long lastPruneForest = 0;
long lastCheckForest = 0;

#define NUM_TREES 25
bool forestState[NUM_TREES + 1];  //forestState[0] is the collective forest state
long forestNodes[NUM_TREES];      //keep a table of the mesh node ids
long forestLastAlive[NUM_TREES];  //millis of the last time we heard from each tree

CRGB leds[NUM_LEDS];
int offset = 0;
byte masterHue;
long patternTime = 0;
int partyCount = 0;

void setup() {

  // setCpuFrequencyMhz(240);

  Serial.begin(115200);
  pinMode(TRIG_PIN1, OUTPUT);
  pinMode(ECHO_PIN1, INPUT);
  pinMode(TRIG_PIN2, OUTPUT);
  pinMode(ECHO_PIN2, INPUT);
  pinMode(TRIG_PIN3, OUTPUT);
  pinMode(ECHO_PIN3, INPUT);



  FastLED.addLeds<WS2812B, DATA_PIN, GBR>(leds, NUM_LEDS);
  //  FastLED.addLeds<SK9822, DATA_PIN, CLOCK_PIN, BGR>(leds, NUM_LEDS);

  //This is where the power is regulated.  These pebble lights are kinda weird, so it will be some trial an error....
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1000);

  mesh.setDebugMsgTypes(ERROR | STARTUP);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.initOTAReceive("TREE");

  //init our forest arrays
  for (int i = 0; i < NUM_TREES; ++i) {
    forestState[i] = false;
    forestNodes[i] = 0;
    forestLastAlive[i] = 0;
  }
  forestState[NUM_TREES] = false;  //one extra in this guy
  forestNodes[0] = 1;              //this tree

  leds[0] = CRGB::Blue;
  FastLED.show();
  delay(500);
  masterHue = 0;

  //set the clock offset
  // getClockOffset();
}

void loop() {

  mesh.update();

  //set the clock offset incase it isn't set
  //if (clockOffset == 0) getClockOffset();

  // LED pattern
  if (millis() < 1000 * 15) {
    // Serial.println(millis());
    testPattern();  //first 15 seconds
  } else {

    if (treeState <= ACTIVATING) blueSpruce();
    if (treeState == ACTIVATED) activePattern();
    if (treeState == DRAW) darkForest();

    if (treeState == ROTATE) patternRotate();
    if (treeState == SPARKLE) patternSparkle();
    if (treeState == STROBE) patternStrobe();
    if (treeState == COLORFUL) patternColorful();
    if (treeState == FIRE) patternFire();
    if (treeState == GRADIENTWIPE) gradientWipe();
  }

  FastLED.show();

  // *check for wifi communications
  if (gotCommand()) {
  }

  //trigger activation on 5 second network time
  long meshTime = mesh.getNodeTime();
  if (meshTime > activateTime) {
    if (treeState == DRAW) {
      //activate
      long seed = meshTime / 10000000;
      treeState = nextState(seed);  //this function will know what to do
      Serial.println("Start Party");
      Serial.println(treeState);
      sendParty(treeState);
      pullTime = millis();
    }
    //shut down after 30 seconds
    if (treeState > DRAW && (pullTime < millis() - 30000)) {
      Serial.println("Party Over");
      treeState = DEFAULT;
      forestState[0] = false;
      clearForestActivity();
    }
  }

  //brodcast IMATREE every 120 seconds
  if (millis() - lastImAlive > 120000) {
    ImAlive();
    lastImAlive = millis();
  }

  //prune forest every 5 minutes
  if (millis() - lastPruneForest > 300000) {
    pruneForest();
    lastPruneForest = millis();
  }

  //check to see if the forest is active every 500ms
  if (millis() - lastCheckForest > 500) {
    checkForest();
    //also decay party count
    if (partyCount > 0) --partyCount;
    lastCheckForest = millis();
  }

  // *check for sensor detection every 200ms
  if (millis() - lastSensor > 200) {
    byte sensors = gotSensor();
    if (sensors != 0) {
      Serial.print("Sensor ");
      if (sensors > 3) {
        Serial.print(" 3");
        activeSensor = 3;
        sensors -= 4;
      }
      if (sensors > 1) {
        Serial.print(" 2");
        activeSensor = 2;
        sensors -= 2;
      }
      if (sensors == 1) {
        Serial.print(" 1");
        activeSensor = 1;
      }
      Serial.println(" ");

      if (treeState == DEFAULT) {
        treeState = ACTIVATING;
      } else if (treeState == ACTIVATING) {
        treeState = ACTIVATED;
        forestState[1] = true;
        tellForest("ACTIVATED");
        startActiveTime = millis();
        lastActiveTime = millis();
      } else {
        lastActiveTime = millis();
      }

    } else {
      //treeState = 0;
      if (treeState == ACTIVATING) treeState = DEFAULT;
    }
    //expire activation
    if (treeState == ACTIVATED && lastActiveTime < millis() - 5000) {
      treeState = DEFAULT;
      tellForest("DEACTIVATED");
      forestState[1] = false;
    }
    lastSensor = millis();
  }

  //check for active timeout
  if (treeState == ACTIVATED && millis() - lastActiveTime > activeTimeout) {
    //go back to inacctive
    treeState = DEFAULT;
    forestState[1] = false;
    tellForest("DEACTIVATED");
    //should this tree state change be reported to other trees?
  }

  // delay(10);

  ++offset;
  if (offset >= NUM_LEDS) offset = 0;
}


long theClock() {
  return mesh.getNodeTime() / 1000;
}

//A few ideas for color patterns and interactive games:
// * 3 People, one on each side of tree.  Tree spins like the Wheel of Fortune wheel until it slows and lands on a winner.  Flashing light and marque to indicate winner.
// * Multiple people in front of trees, but not all of them.  Trees rotate which is lit until lands on winner.
// * Dark Blue Spruce color through the forest.  Lightning hits a tree (white flashing with sparkling sparks).
//    Tree catches fire, orange and red flame grown from top of tree down, then spreads tree by three through the forest.  To put the fire out, three people need to stand on all sides of each tree.  When they do the fire starts burning out until the tree is saved and goes back to spruce color.  Tree can reignite though if the ones next to it are still on fire.  Takes a group effort to stop the forest fire.
// * Graphic EQ mode: Add a microphone to the master tree and turn the forest into a graphic EQ display for music being played, each tree a different frequency by way of Fourier transform.
//
//Color patters:
// * Default Blue Spruce mode: Blue/green colored trees.
// * Tree Rotate: 3 sides rotate color to make it look like the tree is spinning.
// * Sparkle: Add glitter sparkling white LEDs to the trees.
// * Color grow up from bottom to top of trees.
// * Tree Strobe on (all as the same time, randomly, or rotating through forest)