// Deserialize the message
void receivedCallback(uint32_t from, String &msg) {

   Serial.print("# ");
   Serial.print(from);
   Serial.print(" : ");
   Serial.println(msg);
  // String json = msg.c_str();  //now passing it stright in below to save memory

  DynamicJsonDocument doc(256);  //was 1024 but memory errors
  DeserializationError error = deserializeJson(doc, msg);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
  } else {
    updateAlive(from);
    int treeNumber = getTreeIndexByNodeId(from); //returns 0 if not found
    if (treeNumber > 0) {
      String thisState = doc["state"];
      if (isValidNumber(thisState)) {
        int intState = thisState.toInt();
        if (intState <= FORESTPATTERENS + DRAW) {
          forestState[treeNumber] = intState;
      
          //peer pressure
          if (intState > DRAW && lastPartyTime < millis() - (PARTY_MILLISECONDS * 2)) {
            //Serial.printf("\nPARTY %i", from);
            // Serial.print("# "); Serial.print(from); Serial.print(" : "); Serial.println(msg);
            if (partyTreesCount() > 1 && forestState[0] < DRAW) {
              //figure out how long to party
              String thisTimer = doc["timer"];
              if (isValidNumber(thisTimer)) {
                pullTime = millis() - (PARTY_MILLISECONDS - thisTimer.toInt());
                //pullTime = millis();  //would be better to math this out
                lastPartyTime = pullTime;
                changeState(intState);
              }
            }
          }
        }
      }
    }
  }
}

void updateAlive(uint32_t from) {
  int theTree = getTreeIndexByNodeId(from);
  if (theTree > 0) {
    forestLastAlive[theTree] = millis();
    //Serial.printf("\n..Tree %i", theTree);
  } else {
    //this tree needs added
    addNewTree(from);
    //Serial.printf("\nAdding");
  }
}

void broadcastStatus() {
  // Serialize the message
  DynamicJsonDocument doc(1024);
  doc["state"] = forestState[0];
  if (forestState[0] > DRAW) {
    long timerMilliseconds = PARTY_MILLISECONDS - (millis() - pullTime);
    if (timerMilliseconds > PARTY_MILLISECONDS) timerMilliseconds = 0; //for strange condition
    doc["timer"] = timerMilliseconds;
  }
  String msg;
  serializeJson(doc, msg);  //
  mesh.sendBroadcast(msg);
  // Serial.print("Sending: ");
  // Serial.println(msg);
}



void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> startHere: New Connection, nodeId = %ul\n", nodeId);
  //a tree has connected
  addNewTree(nodeId);
}
void addNewTree(uint32_t nodeId) {
  //do we already have this tree in the node table?
  bool gotTree = false;
  int nextSlot = NUM_TREES + 1;
  for (int i = 1; i < NUM_TREES; ++i) {
    if (forestNodes[i] == nodeId) {
      //already have it, just update last alive time
      forestLastAlive[i] = millis();
      gotTree = true;
    }
  }
  if (!gotTree) {
    int nextSlot = 1;
    while (forestNodes[nextSlot] != 0) {
      ++nextSlot;
    }
    forestNodes[nextSlot] = nodeId;
    forestLastAlive[nextSlot] = millis();
    Serial.printf("\nTree %i added @ %lu \n", nextSlot, nodeId);
  }
}
void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}
void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}
