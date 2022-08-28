#include <Arduino.h>
#include <WiFi.h>
#include "BluetoothSerial.h"
#include "HTTP.h"
#include "DB.h"

#define PUSHBUTTON 4
#define LOCK 25
#define OPEN LOW
#define CLOSE HIGH

WiFiClient client;
BluetoothSerial SerialBT;
hw_timer_t * Door_timer = NULL;
hw_timer_t * BT_timer = NULL;

const char* Device_Name = "Door";
const char* ssid = "IoT-LAB";
const char* password = "80796640";
bool WiFi_connect = false;
bool BT_connect = false;
int version = 0;
// bool intrpt_flag = false;

void IRAM_ATTR Door_onTimer(){
  digitalWrite(LOCK, CLOSE);
  Serial.println("The Door is Locked");
  timerStop(Door_timer);
}

void IRAM_ATTR BT_onTimer(){
  Serial.println("Bluetooth time out");
  SerialBT.println("Bluetooth time out");
  SerialBT.disconnect();
  timerStop(BT_timer);
  BT_connect = false;
}

void IRAM_ATTR push_button(){
  intrpt_flag = true;
}

void Push_button_fnuc(){
  digitalWrite(LOCK, OPEN);
  Serial.println("The Door is Unlocked");
  timerSetAutoReload(Door_timer, false);
  timerAlarmWrite(Door_timer, 1000000 * 10, true);
  timerRestart(Door_timer);
}

void BT_login(){
  String BTaccount;
  String BTuser = "";
  String BTpassword = "";
  int i = 0;
  while (SerialBT.available()){
    BTaccount = SerialBT.readString();
    Serial.println("Serial Bluetooth received ["+BTaccount+"] as an account\n");
  }

  while (i < BTaccount.length() && BTaccount.charAt(i) != ' ')
  {
    BTuser = BTuser + BTaccount.charAt(i);
    i++;
  }
  i++;
  while (i < BTaccount.length() && BTaccount.charAt(i) != '\r')
  {
    BTpassword = BTpassword + BTaccount.charAt(i);
    i++;
  }
  Serial.println("User Name: "+BTuser+"\tPassword: "+BTpassword);
  
  if (BTuser.equals("") || BTpassword.equals("")){
    Serial.println("Username or Password is missed");
    SerialBT.println("Username or Password is missed");
    return;
  }

  if (getPssword(SPIFFS,findUser(SPIFFS,BTuser)).equals(BTpassword)){     
    digitalWrite(LOCK, OPEN);
    Serial.println("The Door is Unlocked");
    SerialBT.println("The Door is Unlocked");
    timerSetAutoReload(Door_timer, false);
    timerAlarmWrite(Door_timer, 1000000 * 10, true);
    timerRestart(Door_timer);
  }
  else { 
    Serial.println("Username or Password is incorrect");
    SerialBT.println("Username or Password is incorrect");
  }
  SerialBT.disconnect();
  timerStop(BT_timer);
  BT_connect = false;
}

void update_database(bool door_state){
 String db_users = getInfo("https://192.168.0.10:4040/iotdevice/door/users",door_state);      // get username from the server
 writeFile(SPIFFS,"/User.txt",db_users.c_str());      // write the username in the file
 appendFile(SPIFFS,"/User.txt","Osamah\n");      
 String db_psword = getInfo("https://192.168.0.10:4040/iotdevice/door/tkns",door_state);     // get password from the server
 writeFile(SPIFFS,"/Password.txt",db_psword.c_str()); // write the password in the file
 appendFile(SPIFFS,"/Password.txt","1998\n"); // write the password in the file
 readFile(SPIFFS, "/User.txt");
 readFile(SPIFFS, "/Password.txt");
}


void setup() {
  Serial.begin(115200);

  // Push Button Congifurations
  pinMode(PUSHBUTTON, INPUT);
  // attachInterrupt(PUSHBUTTON, push_button, HIGH);

  // Lock Congifurations
  pinMode(LOCK, OUTPUT);
  digitalWrite(LOCK, CLOSE);

  // WiFi Congifurations
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(Device_Name);
  WiFi.begin(ssid, password);

  // SPIFFS Congifurations
  if(!SPIFFS.begin(true)){
    Serial.println("ERROR: mounting SPIFFS");
    return;
  }
//  listDir(SPIFFS, "/", 0);

  // Door's Timer Congifurations
  Door_timer = timerBegin(0, 80, true);  // timer 0, MWDT clock period = 12.5 ns * TIMGn_Tx_WDT_CLK_PRESCALE -> 12.5 ns * 80 -> 1000 ns = 1 us, countUp
  timerAttachInterrupt(Door_timer, &Door_onTimer, true); // edge (not level) triggered 
  timerAlarmWrite(Door_timer, 1000000 * 10, true); // 1000000 * 1 us = 1 s * duration, autoreload true
  timerStop(Door_timer);
  timerAlarmEnable(Door_timer); // enable

  // Bluetooth's Timer Congifurations
  BT_timer = timerBegin(1, 80, true);  // timer 0, MWDT clock period = 12.5 ns * TIMGn_Tx_WDT_CLK_PRESCALE -> 12.5 ns * 80 -> 1000 ns = 1 us, countUp
  timerAttachInterrupt(BT_timer, &BT_onTimer, true); // edge (not level) triggered 
  timerAlarmWrite(BT_timer, 1000000 * 30, true); // 1000000 * 1 us = 1 s * duration, autoreload true
  timerStop(BT_timer);
  timerAlarmEnable(BT_timer); // enable

}

void loop() {
  // push button behavior
  if (intrpt_flag){
    digitalWrite(LOCK, OPEN);
    Serial.println("The Door is Unlocked");
    timerSetAutoReload(Door_timer, false);
    timerAlarmWrite(Door_timer, 1000000 * 10, true);
    timerRestart(Door_timer);
    intrpt_flag = false;
  }

  // check and try to connect to wifi periodically
  while (WiFi.status() != WL_CONNECTED) {
    if (!WiFi_connect){
      Serial.println("WiFi disconnected.");
      WiFi.begin(ssid, password);
    }
    WiFi_connect = true;
    delay(1000);
    Serial.print(".");

    if ( digitalRead(PUSHBUTTON) == HIGH ) Push_button_fnuc();
    SerialBT.begin(Device_Name);
    if (SerialBT.hasClient() && !BT_connect) {
      timerRestart(BT_timer);
      BT_connect = true;
    }
    if (!SerialBT.hasClient() && BT_connect) BT_connect = false;
    if (SerialBT.available()){
      BT_login();
    }
  }

  if (WiFi_connect) {
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    WiFi_connect = false;
  }


  // After this line the wifi will be connected
  SerialBT.end();
  String cmds = getInfo("https://192.168.0.10:4040/iotdevice/door/status",timerStarted(Door_timer));
  if (!cmds.equals("ERROR")){
    char json[500];
    cmds.replace(" ","");
    cmds.replace("\n","");
    cmds.trim();
    cmds.toCharArray(json,500);
    StaticJsonDocument<200> doc;
    deserializeJson(doc,json);

    bool open = doc["open_command"];
    int new_version = doc["acces_list_counter"];
    int time_open = doc["time_seconds"];
    bool close = doc["force_close"];

    if (open){
      Serial.println("Update Timer");
      timerSetAutoReload(Door_timer, false);
      timerAlarmWrite(Door_timer, 1000000 * time_open, true);
      digitalWrite(LOCK, OPEN);
      Serial.println("The Door is Unlocked");
      timerRestart(Door_timer);
      open = false;
    }

    if(close){
      digitalWrite(LOCK, CLOSE);
      Serial.println("The Door is Locked");
      timerStop(Door_timer);
      close = false;
    }

    if (version != new_version){
      update_database(timerStarted(Door_timer));
      version = new_version;
    }
  }
  else{
    SerialBT.begin(Device_Name);
    Serial.println("BT ON");
    for (int i = 0; i < 5; i++){
      if ( digitalRead(PUSHBUTTON) == HIGH ) Push_button_fnuc();
      delay(2000);
      if (SerialBT.hasClient() && !BT_connect) {
        timerRestart(BT_timer);
        BT_connect = true;
      }
      if (!SerialBT.hasClient() && BT_connect) BT_connect = false;
      if (SerialBT.available()){
        BT_login();
        break;
      }
    }
  }
  for (int i = 0; i < 5; i++){
    if ( digitalRead(PUSHBUTTON) == HIGH ) Push_button_fnuc();
    delay(200);
  }
}