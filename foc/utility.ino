//prune any tree from the forest that hasn't talked in 5 minutes
void pruneForest() {
  for (int i = 0; i < NUM_TREES; ++i) {
    if (forestLastAlive[i] < millis() - 300000) {
      //he's dead Jim, remove the node
      forestNodes[i] = 0;
    }
  }
}

//return the total number of tree that are currently alive (active nodes)
int aliveTreesCount() {
  int aliveTrees = 0;
  for (int i = 0; i < NUM_TREES; ++i) {
    if (forestNodes[i] > 0) {
      ++aliveTrees;
    }
  }
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
  return theID;
}

//check the forestState array to see if all the trees are active
//to account for the possibility of a tree being offline, 
// the forest will activate then the number of trees that are connected to the mesh = active trees
void checkForest() {
  int numberOfActiveTrees = 0;
  int numberOfLiveTrees = aliveTreesCount();
  for (int i=1; i <= NUM_TREES; ++i) {
    if (!forestState[i]) ++numberOfActiveTrees;
  }
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
  pullTime = millis();
  treeState = DRAW;  
}


int nextState(long seed) {
  return (seed % FORESTPATTERENS) + 4 ;
}