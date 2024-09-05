void changeState(int newState) {
  Serial.printf("\State Change: %i => %i\n", forestState[0], newState);
  forestState[0] = newState;
}

//prune any tree from the forest that hasn't talked in 5 minutes
void pruneForest() {
  if (millis() < 4000) return;
  //start at 1 because forestNodes[0] is me
  for (int i = 1; i < NUM_TREES; ++i) {
    if (forestNodes[i] != 0) {
      if (forestLastAlive[i] < (millis() - 4000)) {
        //he's dead Jim, remove the node
        //forestState[0] is me
        Serial.printf("\nTree %i pruned @ %lu \n", i, forestNodes[i]);
        //Serial.printf(" ... forestLastAlive[] %lu < %lu \n", forestLastAlive[i], millis()-300000);
        forestState[i] = 0;
        forestNodes[i] = 0;
      }
    }
  }
}

int activeTreesCount() {  
  int numberOfActiveTrees = 0;
  for (int i = 0; i < NUM_TREES; ++i) {
    if (forestState[i] == ACTIVATED) {
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

//return the total number of tree that are currently alive (active nodes)
int partyTreesCount() {
  int partyTrees = 0;
  for (int i = 1; i <= NUM_TREES; ++i) {
    if (forestState[i] > ACTIVATED) {
      ++partyTrees;
    }
  }
  return partyTrees;
}

//function will return the index of the forestNode or 0 if not found
int getTreeIndexByNodeId(uint32_t nodeID) {
  int theID = 0;
  //start at 1, because 0 is me
  for (int i = 1; i < NUM_TREES; ++i) {
    if (forestNodes[i] == nodeID) {
      theID = i;
    }
  }
  return theID;
}

//check the forestState array to see if all the trees are active
//to account for the possibility of a tree being offline,
// the forest will activate then the number of trees that are connected to the mesh = active trees
void checkForest() {
  int numberOfActiveTrees = activeTreesCount();
  int numberOfLiveTrees = aliveTreesCount();
  
  //save the forest state to the 0 array position
  //temp changing to try to get the forest to activate, but reducing the required number of ativated trees to 5
  if (numberOfActiveTrees >= (numberOfLiveTrees / 2) && numberOfActiveTrees > 1) {
    //if (numberOfActiveTrees >= 5) {
    //party time, is this a new state?
    if (forestState[0] > ACTIVATED) {
      //we're already active
    } else {
      activateForest();  //this will know what to do
    }
  }

  if (numberOfLiveTrees == 1) {
    imAlone = true;
  } else {
    imAlone = false;
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
  changeState(DRAW);
}

void clearForestActivity() {
  changeState(DEFAULT);
  for (int i = 1; i <= NUM_TREES; ++i) {
    forestState[i] = DEFAULT;
  }
}

int nextState(long seed) {
  int tmp = abs(seed % FORESTPATTERENS);
  return tmp + 4;
}

boolean isValidNumber(String str) {
  for (byte i = 0; i < str.length(); i++) {
    if (isDigit(str.charAt(i))) return true;
  }
  return false;
}