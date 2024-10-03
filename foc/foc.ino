#include <FastLED.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <painlessMesh.h>

#define MESH_PREFIX "FoC"
#define MESH_PASSWORD "JsFS)weI9J#*ij4F~jn="
//#define MESH_PASSWORD "JsFS)weI9J#*"
#define MESH_PORT 5555  // default port
//#define MESH_PORT 7777  // default port

Scheduler userScheduler;
painlessMesh mesh;

void sendMessage() ; // callback func
Task sendTask( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

#define TREE_NUMBER 6 //each tree is numbered in order based on where it is located
#define DETECTINCHES 48
#define TREE_DEBUG true

//changed these so it didn't have to do math every time BRANCH_LENGTH and NUM_LEDS are used
//also added 10 extra LEDs to NUM_LEDS since some of the strands were cut long
#define SIDE_LENGTH 60
#define BRANCH_LENGTH 120
#define NUM_LEDS 370 
#define PARTY_MILLISECONDS 60000 
#define REST_MILLISECONDS 10000

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
#define SWINGINGLIGHTS 10
#define PSYCHEDELLIC 11
#define RACINGLIGHTS 12
#define RAINBOWFOREST 13
#define ROUNDTHETREES 14
#define FORESTPATTERENS 11

#define TRIG_PIN1 13
#define TRIG_PIN2 14
#define TRIG_PIN3 26
#define ECHO_PIN1 12
#define ECHO_PIN2 27
#define ECHO_PIN3 25

int activeTimeout = 25000;  //25 seconds to activate all the trees
long startActiveTime = 0;
long lastActiveTime = 0;
int activeSensor = 1;  //must be 1, 2, or 3
long pullTime = 0;
long activateTime = 0;
long lastPartyTime = 0;

long clockOffset = 0;
long lastSensor = 0;
long lastPruneForest = 0;
long lastCheckForest = 0;
long lastStatus = 0;
long lastDirty = 0;

bool imAlone = true;
bool freshState = false;

#define NUM_TREES 25
int forestState[NUM_TREES + 1];  //forestState[NUM_TREES] is the collective forest state, forestState[0] is me
long forestNodes[NUM_TREES];      //keep a table of the mesh node ids forestNodes[0] is me
long forestLastAlive[NUM_TREES];  //millis of the last time we heard from each tree

CRGB leds[NUM_LEDS];
// int offset = 0;
byte masterHue;
long patternTime = 0;

void setup() {

  // setCpuFrequencyMhz(240);

  Serial.begin(115200);
  pinMode(TRIG_PIN1, OUTPUT);
  pinMode(ECHO_PIN1, INPUT);
  pinMode(TRIG_PIN2, OUTPUT);
  pinMode(ECHO_PIN2, INPUT);
  pinMode(TRIG_PIN3, OUTPUT);
  pinMode(ECHO_PIN3, INPUT);



  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  //  FastLED.addLeds<SK9822, DATA_PIN, CLOCK_PIN, BGR>(leds, NUM_LEDS);

  //This is where the power is regulated.  These pebble lights are kinda weird, so it will be some trial an error....
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 800);

  mesh.setDebugMsgTypes(ERROR | STARTUP);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onDroppedConnection(&droppedConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.initOTAReceive("TREE");
  userScheduler.addTask( sendTask );
  sendTask.enable();

  //init our forest arrays
  for (int i = 0; i < NUM_TREES; ++i) {
    forestState[i] = DEFAULT;
    forestNodes[i] = 0;
    forestLastAlive[i] = 0;
  }
  forestState[NUM_TREES] = DEFAULT;  //one extra in this guy
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
  if (millis() < 1000 * 3) {
    // Serial.println(millis());
    testPattern();  //first 15 seconds
  } else {

    if (forestState[0] <= ACTIVATING) blueSpruce();
    if (forestState[0] == ACTIVATED) activePattern();
    if (forestState[0] == DRAW) darkForest();
    
    if (forestState[0] == ROTATE) patternRotate();
    if (forestState[0] == SPARKLE) patternSparkle();
    if (forestState[0] == STROBE) patternStrobe();
    if (forestState[0] == COLORFUL) patternColorful();
    if (forestState[0] == FIRE) patternFire();
    if (forestState[0] == GRADIENTWIPE) patternGradientWipe();
    if (forestState[0] == SWINGINGLIGHTS) patternSwingingLights();
    if (forestState[0] == PSYCHEDELLIC) patternPsychedellic();
    if (forestState[0] == RACINGLIGHTS) patternRacingLights();
    if (forestState[0] == RAINBOWFOREST) patternRainbowForest();
    if (forestState[0] == ROUNDTHETREES) patternRoundTheTrees();

    if (imAlone) offlineTree(); //turn the tree dark if it's not connecting to others on the network

  }

  tooManyLEDsFix();

  FastLED.show();

  //trigger activation on 5 second network time
  long meshTime = mesh.getNodeTime();
  //activate time will be 0 when the program first starts or when there's a meshTime problem with a peer presure.
  if (meshTime > activateTime && activateTime != 0) {
    if (forestState[0] == DRAW) {
      //activate
      long seed = meshTime / 10000000;  //I think if we made this 60000000 we'd get better pattern coherance 
      changeState(nextState(seed));  //this function will know what to do
      if (TREE_DEBUG) Serial.print("Start Party ");
      if (TREE_DEBUG) Serial.println(forestState[0]);
      //Monday: broadcastStatus();
      pullTime = millis();
      lastPartyTime = millis();
    }
    
  
  //shut down after 60 seconds
    if (forestState[0] > DRAW && ((pullTime < millis() - PARTY_MILLISECONDS)) ) {
      if (TREE_DEBUG) Serial.println("Party Over");
      changeState(DEFAULT);
      clearForestActivity();
    }
  }


  //check to see if the forest is active every 500ms
  if (millis() - lastCheckForest > 500) {
    checkForest();
    // pruneForest(); // REMOVED AND REPLACED WITH DROPPED CALLBACK
    lastCheckForest = millis();
  }

  //trigger a send of the status every 15 seconds
  if (millis() - lastDirty > 15000) {
    freshState = true;
    lastDirty = millis();
  }

  //output status
  if (millis() - lastStatus > 2000) {
    if (TREE_DEBUG) Serial.printf("\n** Active: %i, Live: %i **\n", activeTreesCount(), aliveTreesCount());
    // size_t i=0;
    // SimpleList<uint32_t> nl = mesh.getNodeList();
    // SimpleList<uint32_t>::iterator itr = nl.begin();
    // while(itr != nl.end()) {
 
    //     Serial.print(F("[+] MeshNode (Chip-ID) -> "));
    //     Serial.println(*itr,DEC);
      
    //   itr++;
    // }
    
     if (TREE_DEBUG) Serial.println(mesh.asNodeTree().toString());

    lastStatus = millis();
  }

  // *check for sensor detection every 200ms
  if (millis() - lastSensor > 200 && lastPartyTime < millis() - (PARTY_MILLISECONDS + REST_MILLISECONDS)) {
    byte sensors = gotSensor();
    //sensors = 3;
    if (sensors != 0) {
      //Serial.print("Sensor ");
      if (sensors > 3) {
        //Serial.print(" 3");
        activeSensor = 3;
        sensors -= 4;
      }
      if (sensors > 1) {
        //Serial.print(" 2");
        activeSensor = 2;
        sensors -= 2;
      }
      if (sensors == 1) {
        //Serial.print(" 1");
        activeSensor = 1;
      }
      //Serial.println(" ");

      if (forestState[0] == DEFAULT) {
        changeState(ACTIVATING);
      } else if (forestState[0] == ACTIVATING) {
        changeState(ACTIVATED);
        //Monday: broadcastStatus();
        startActiveTime = millis();
        lastActiveTime = millis();
      } else {
        lastActiveTime = millis();
      }

    } else {
      if (forestState[0] == ACTIVATING) changeState(DEFAULT);
    }
    //expire activation
    if (forestState[0] == ACTIVATED && lastActiveTime < millis() - activeTimeout) {
      changeState(DEFAULT);

      //Monday: broadcastStatus();
    }
    lastSensor = millis();
  }

  // delay(10);

  // ++offset;
  // if (offset >= NUM_LEDS) offset = 0;
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
