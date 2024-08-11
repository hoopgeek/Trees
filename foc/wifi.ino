bool gotCommand() {
  //check to see if there's a command for this tree on the wifi buffer
  //if command {
  //return true;
  //} else {
  return false;
  //}
}


void tellForest(String status) {

  // Serial.println();
  // Serial.println("Start Sending....");

  // Serialize the message
  DynamicJsonDocument doc(1024);
  doc["status"] = status;
  String msg;
  serializeJson(doc, msg);  //
  mesh.sendBroadcast(msg);
  Serial.print("Sending: ");
  Serial.println(msg);
}

void sendParty(int stateNumber) {
  Serial.println("Announcing Party Mode");

  // Serialize the message
  DynamicJsonDocument doc(1024);
  doc["status"] = "PARTY";
  doc["state"] = String(stateNumber);
  String msg;
  serializeJson(doc, msg);
  mesh.sendBroadcast(msg);
  Serial.println("Message ");
  Serial.println(msg);
}

// Deserialize the message
void receivedCallback(uint32_t from, String &msg) {

  Serial.print("# ");
  Serial.print(from);
  Serial.print(" : ");
  Serial.println(msg);
  String json = msg.c_str();

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
  } else {
    updateAlive(from);
    if (doc["status"] == "ACTIVATED") {
      int treeNumber = getTreeIndexByNodeId(from);
      if (treeNumber > 0 && treeNumber <= 25) {
        //Serial.printf("Tree Activated: %d\n", treeNumber);
        forestState[treeNumber] = true;
        // checkForest();
      }
      // treeState = ACTIVATED;
      // activeTime = millis();
    }
    if (doc["status"] == "DEACTIVATED") {
      //get the ID of the node sending the data and translate that to the tree index
      int treeNumber = getTreeIndexByNodeId(from);
      if (treeNumber > 0 && treeNumber <= 25) {
        //Serial.printf("Tree Deactivated: %d\n", treeNumber);
        forestState[treeNumber] = false;
        // checkForest();
      }
    }
    if (doc["status"] == "IMATREE") {
      //tree checking in to say they alive
      //Serial.printf("\nIMATREE %i", from);
    }
    if (doc["status"] == "PARTY") {
      //Serial.printf("\nPARTY %i", from);
      ++partyCount;
      if (partyCount > 1 && treeState < DRAW) {
        String thisState = doc["state"];
        if (isValidNumber(thisState)) {
          int intState = thisState.toInt();
          if (intState > DRAW && intState <= (FORESTPATTERENS + DRAW)) {
            treeState = intState;
            pullTime = millis();
          }
        }
      }
    }
  }
}

void updateAlive(uint32_t from) {
  int theTree = getTreeIndexByNodeId(from);
  if (theTree < NUM_TREES) {
    forestLastAlive[theTree] = millis();
    //Serial.printf("\n..Tree %i", theTree);
  } else {
    //this tree needs added
    addNewTree(from);
    //Serial.printf("\nAdding");
  }
}

void ImAlive() {
  //send IMATREE
  if (treeState >= ACTIVATED) {
    tellForest("ACTIVATED");
  } else {
    tellForest("IMATREE");
  }
}



void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
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
    //check for the next empty slot
    if (forestNodes[i] == 0 && nextSlot > NUM_TREES) {
      nextSlot = i;
    }
    if (!gotTree && nextSlot < NUM_TREES) {
      //new tree, welcome to the forest
      forestNodes[nextSlot] = nodeId;
      Serial.printf("\nTree %i added\n", nextSlot);
    }
  }
}
void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}
void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

//not using
// String httpGETRequest(const char* serverName) {
//   WiFiClient client;
//   HTTPClient http;

//   // Your Domain name with URL path or IP address with path
//   http.begin(client, serverName);

//   // Send HTTP POST request
//   int httpResponseCode = http.GET();

//   String payload = "--";

//   if (httpResponseCode>0) {
//     Serial.print("HTTP Response code: ");
//     Serial.println(httpResponseCode);
//     payload = http.getString();
//   }
//   else {
//     Serial.print("Error code: ");
//     Serial.println(httpResponseCode);
//   }
//   // Free resources
//   http.end();

//   return payload;
// }