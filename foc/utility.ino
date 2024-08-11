//prune any tree from the forest that hasn't talked in 5 minutes
void pruneForest() {
  for (int i = 1; i < NUM_TREES; ++i) {
    if (forestLastAlive[i] < millis() - 300000) {
      //he's dead Jim, remove the node
      forestNodes[i] = 0;
    }
  }
}

int activeTreesCount() {  
  int numberOfActiveTrees = 0;
  for (int i = 1; i <= NUM_TREES; ++i) {
    if (forestState[i] == true) {
      ++numberOfActiveTrees;
      //Serial.println(i);
    }
  }
  return numberOfActiveTrees;
}

//return the total number of tree that are currently alive (active nodes)
int aliveTreesCount() {
  int aliveTrees = 0;
  for (int i = 1; i < NUM_TREES; ++i) {
    if (forestNodes[i] != 0) {
      ++aliveTrees;
    }
  }
  ++aliveTrees;  //because this tree is also alive
  return aliveTrees;
}

//function will return the index of the forestNode or NUM_TREES if not found
int getTreeIndexByNodeId(uint32_t nodeID) {
  int theID = NUM_TREES;
  for (int i = 0; i < NUM_TREES; ++i) {
    if (forestNodes[i] == nodeID) {
      theID = i;
    }
  }
  return theID + 1;
}

//check the forestState array to see if all the trees are active
//to account for the possibility of a tree being offline,
// the forest will activate then the number of trees that are connected to the mesh = active trees
void checkForest() {
  int numberOfActiveTrees = activeTreesCount();
  int numberOfLiveTrees = aliveTreesCount();
  
  //save the forest state to the 0 array position
  if (numberOfActiveTrees >= numberOfLiveTrees) {
    //party time, is this a new state?
    if (forestState[0]) {
      //we're already active
    } else {
      //newly active
      forestState[0] = true;
      //how do we know what to do?
      activateForest();  //this will know what to do
    }
  } else {
    forestState[0] = false;
  }
}

void activateForest() {
  //tree will join forest activation on even 5 seconds
  //first shut off LEDs
  // pullTime = millis();
  long temp = mesh.getNodeTime() / 1000000L;
  temp = temp / 5;
  temp = temp * 5 * 1000000L + 5000000L;
  activateTime = temp;  // 5 to 10 seconds from now on a second divisible by 5;
  treeState = DRAW;
}

void clearForestActivity() {
  for (int i = 0; i < NUM_TREES; ++i) {
    forestState[i] = false;
  }
}

int nextState(long seed) {
  return (seed % FORESTPATTERENS) + 4;
}

boolean isValidNumber(String str) {
  for (byte i = 0; i < str.length(); i++) {
    if (isDigit(str.charAt(i))) return true;
  }
  return false;
}