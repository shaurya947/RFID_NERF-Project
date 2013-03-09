//code for Arduino board, written by Verneil Mesecher

int pir = 2;                                                //General variable setup
int servopin = 4;
int ledpin = 6;
char val = 0;
String id;
boolean pirB = false;
int calibSeconds = 5;
boolean fired = false;
int startMillis = 0;
int currMillis = 0;
boolean allow = false;
boolean testForChange = false;
boolean ledaccess = false;
boolean leddenial = false;
boolean test = false;
String allowedIDs[2] = {"55004DEFC93E", "550049E744BF"};  //allowed tags in the system

void fireGun() {
  for (int i = 0; i < 40; i++) {                //pull the trigger
    digitalWrite(servopin, HIGH);
    delayMicroseconds(0);
    digitalWrite(servopin, LOW);
    delay(20);
  }
  for (int i = 0; i < 60; i++) {                //release the trigger
    digitalWrite(servopin, HIGH);
    delayMicroseconds(1);
    digitalWrite(servopin, LOW);
    delay(20);
  }
}

void readRFID() {
    if (Serial.available() > 12) {              //if a tag has been scanned
    id = "";
    Serial.read();                              //clear the useless first character
    for (int i = 0; i < 12; i++) {              //construct the string that represents an RFID tag's unique identifier
      char val = Serial.read();
      id = id + val;
    }
    
    while (Serial.read() != -1);                 //clear the buffer
    
    for (int k = 0; k < 2; k++) {
      if (id.equalsIgnoreCase(allowedIDs[k])) {  //if the read id is in the list of allowed ids
        allow = true;
        testForChange = false;        //start watching for a person to walk through
        ledaccess = true;
        leddenial = false;
        break;
      }
      else {
        leddenial = true;
        ledaccess = false;
      }
    }
    digitalWrite(ledpin,HIGH);
    startMillis = millis();
    currentMillis = startMillis;
  }
}

void setup() {
  Serial.begin(9600);                              //general arduino set up, serial, pins
  pinMode(pir,INPUT);
  pinMode(servopin, OUTPUT);
  pinMode(ledpin,OUTPUT);
  digitalWrite(pir,LOW);
  
  for (int i = 0; i < calibSeconds; i++) {        //allow a calibration time period for the pir sensor
    delay(1000);
  }
  
}

void loop() {                                            //main logic loop
  readRFID();                                            //read the RFID number if a tag has been scanned
  if (test != digitalRead(pir)){
      Serial.write('a');
      test = !test;
  }
  if (digitalRead(pir) == HIGH && allow == false) {      //if a person is detected, but the grace period is not activated
      fireGun();                                         //fire the gun
  }
  else if (digitalRead(pir) == HIGH && allow == true) {  //if a person is detected, but the grace period is activated
    testForChange = true;                                //update the system to realize that it has allowed one person in
  }
  else if (digitalRead(pir) == LOW && allow == true) {    //if a person is not detected, but the grace period is activated
    if (testForChange == true) {                          //if the system has already let someone in during this grace period
      allow = false;                                      //end the grace period because the system has already let someone through
    }
  }
  
  if (startMillis != 0) {
    currMillis = millis();                  //if the grace period is activated, update the current time 
  }
  
  if ((currMillis - startMillis) > 5000) {  //if it has been 5 seconds since the start of the grace period
    allow = false;                          //deactivate the grace period
    startMillis = 0;
    currMillis = 0;
  }
  
  if ((currMillis - startMillis) > 600 && (currMillis - startMillis) < 1200 && ledaccess) {  //if access granted, make led blink twice for 600 milliseconds each time
    digitalWrite(ledpin,LOW);
  }
  else if ((currMillis - startMillis) > 1200 && (currMillis - startMillis) < 1800 && ledaccess) {  
    digitalWrite(ledpin,HIGH);
  }
  else if ((currMillis - startMillis) > 1800 && ledaccess) {  
    digitalWrite(ledpin,LOW);
  }
  
  else if ((currMillis - startMillis) > 3000 && leddenial) {  //if access denied, make led turn on straight for 3 seconds
    digitalWrite(ledpin,LOW);
  }
  
  delay(200);                                //poll the system five times a second
}
