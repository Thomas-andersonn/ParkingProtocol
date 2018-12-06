#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUDP.h>
#include <EEPROM.h> 
#include <ESP8266HTTPClient.h>

int localUdpPort = 8888; // the destination port
MDNSResponder mdns;
String parentIP = "";
 String broadcastSSID = "";
int pin_OP = 4;
int key_addr = 514;
WiFiUDP Udp;
//Packet Buffer 
const int packetSize = 20;
byte packetBuffer[packetSize]; 
int key =0;
// Replace with your network credentials
String startString = "swag";
char* ssid = "swag";
const char* password = "ishusing";
int currentLevel =10000;
int childNumber = 1;
ESP8266WebServer server(80);

String webPage = "";

 extern "C" {
#include <user_interface.h>
}

 
int decrypt(String action,String salt){
  Serial.println(action.toInt());
  Serial.println(key);
  return action.toInt()-salt.toInt()-key;
}
int toggle(String action,String salt){
  Serial.println(salt.toInt());
  int act = decrypt(action,salt);
  if(act == 1){
    Serial.println("One");
    digitalWrite(pin_OP,LOW);
  }
  else{
    Serial.println("Two"); 
    digitalWrite(pin_OP,HIGH); 
  }
    return act;
}
String sendHTTP(String type){
  HTTPClient http; 
  
  http.begin("http://"+parentIP+"/"+type);
  Serial.println("http://"+parentIP+"/"+type);
//http.begin("http://192.168.4.1/getChild");

  
  int httpCode = http.GET();
   Serial.println("PArent IP is: "+parentIP);
   String payload = "0";
   Serial.println("htttpcode :"+String(httpCode));
    if (httpCode > 0) { //Check the returning code
 
      payload = http.getString();   //Get the request response payload
      Serial.println(payload);                     //Print the response payload
 
    }
 
    http.end();   //Close connection
     Serial.println("Returning Payload");  
    return payload;
}
void startAP(String ssid){
  WiFi.mode(WIFI_STA);
  String payload = sendHTTP("getChild");
  Serial.println("Payload :"+payload);
int childNumber = payload.toInt();
 broadcastSSID = startString+currentLevel+"_"+childNumber;
WiFi.mode(WIFI_AP_STA);
   boolean conn = WiFi.softAP(broadcastSSID.c_str(), "ishusing");
  
    Serial.print("AP Started" + conn );
 Serial.println(" SSID: "+ broadcastSSID);

    
}
void startServer(){
    webPage += "ESP8266 Connected"; 
    
 mdns.begin("esp8266", WiFi.localIP());
 server.on("/setKey", [](){
  String qsid =server.arg("key"); 
//  key = (server.arg("key"));
  Serial.println("Key received "+ qsid);
    for (int i = 0; i < qsid.length(); ++i)
      {
        EEPROM.write(i, qsid[i]);
        Serial.print("Wrote: ");
        Serial.println(qsid[i]);
      }
      Serial.println(qsid);
      Serial.println("");
//  EEPROM.write(0, 'c'); //write the first half
EEPROM.commit();
    server.send(200, "text/html", qsid);  
  });
   server.on("/getKey", [](){
  char str = EEPROM.read(0);
  Serial.println(str);
String epass= "";
  for (int i = 0; i < 1; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);
  key = epass.toInt();
    server.send(200, "text/html", epass);
  });
   server.on("/status", [](){
    
    server.send(200, "text/html", webPage);
  });
  server.on("/getChild", [](){
    Serial.println("getChild request Received: " +String(childNumber));
    childNumber++;
    server.send(200, "text/html", String(childNumber));
  });
   server.on("/start", [](){
    String id = server.arg("id");
    String ssid = "espwifi_"+id;
     startAP(ssid);
//    Serial.println("Was the status");
    server.send(200, "text/html", "");
  });
   server.on("/stop", [](){
  
      boolean conn =WiFi.softAPdisconnect (true);
   Serial.println(conn);
    Serial.println("AP stopped" + conn);
//    Serial.println("Was the status");
    server.send(200, "text/html", webPage);
  });
   server.on("/heartbeat", [](){
    server.send(200, "text/html", "Good");
  });
  server.on("/", [](){
    server.send(200, "text/html", webPage);
  });
  server.on("/sendToRoot", [](){
    String dev_id = server.arg("dev_id");
    String dev_status = server.arg("status");
    String broadcastSSID = startString+currentLevel+"_"+childNumber;
    String payload = sendHTTP("sendToRoot?dev_id="+dev_id+"&status="+dev_status);
    Serial.println("sendToRoot "+dev_id +" "+dev_status);
    server.send(200, "text/html", "sendToRoot "+dev_id +" "+dev_status);
  });
  
  server.on("/socket1On", [](){ 
    String action = server.arg("action");
    String salt= server.arg("salt");
    Serial.println("SOCKET ON "+action +" "+salt);
    int ret = toggle(action,salt);
    server.send(200, "text/html", "Return Received was "+ret);
  
    delay(100);
  });
 pinMode(pin_OP,OUTPUT);
 digitalWrite(pin_OP,LOW);
  server.begin();
  Serial.println("HTTP server started");
}
boolean isHigherLevel(String id){
 int indexOflevel = id.indexOf(startString) + startString.length();
 int indexOfUnderScore = id.indexOf("_");
 
 int parentLevel = (id.substring(indexOflevel,indexOfUnderScore).toInt());
 if(parentLevel < currentLevel){
  Serial.println("True" +String(parentLevel)+ " " + String(currentLevel));
  return true;
 }
  Serial.println("False" +String(parentLevel)+ " " + String(currentLevel));
 return false;
}
   String getWifiNetworks(){
   int n = WiFi.scanNetworks();
   String nets[n]; 
   String ret = "";
   int min = -1000;
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      nets[i] = WiFi.SSID(i);
//      Serial.println("******************************");
//      Serial.println(nets[i].startsWith(startString));
//      Serial.println(WiFi.RSSI(i)> min);
//      Serial.println(isHigherLevel(nets[i]));
//      Serial.println("---------------------------------");
      
      if(nets[i].startsWith(startString) && WiFi.RSSI(i)> min && isHigherLevel(nets[i])){
        String id = nets[i];
        min = WiFi.RSSI(i);
         

        ret = nets[i];
      }
      Serial.println("Min: "+ String(min));
      Serial.print(" (");
      Serial.println("NET: "+ nets[i]);
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("Closest ssid is "+ ret);
  int indexOflevel = ret.indexOf(startString) + startString.length();
 int indexOfUnderScore = ret.indexOf("_");
   int parentLevel = (ret.substring(indexOflevel,indexOfUnderScore).toInt());
 currentLevel = parentLevel  +1;
  if(ret.equals("")){
    delay(2000);
    getWifiNetworks();
  }
  return ret;
}
void setup(void){
  
  delay(1000);
  

  Serial.begin(115200);
     String ret= getWifiNetworks();
    EEPROM.begin(512);
String epass= "";
  for (int i = 0; i < 1; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);
  key = epass.toInt(); 
  char s[ret.length()+1];
  ret.toCharArray(s, ret.length()+1);
  Serial.print("Converted array ");
  Serial.println(s);
//  for(int i=0;i<ret.length();i++){
//    s[i] = ret.
//  }
  WiFi.begin(s, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ret);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
    Serial.println(WiFi.localIP());
    parentIP = WiFi.gatewayIP().toString();
    startAP("swag_1") ;

     startServer();  
 
}
int missedHeartBeats = 0;
int heartbeatcounter = 0;
int failcounter = 0;
void checkheartbeat() {
  heartbeatcounter = heartbeatcounter + 1;
  if (heartbeatcounter == 250000) {
    heartbeatcounter = 0;
    String payload = sendHTTP("heartbeat");
//    String broadcastSSID = startString+currentLevel+"_"+childNumber;
    sendHTTP("sendToRoot?dev_id="+broadcastSSID+"&status=83");
    Serial.println("Payload received for heartbeat: " + payload);
    if (payload != "Good") {
      failcounter = failcounter + 1;
      if (failcounter == 5) {
        failcounter = 0;
      //Disconnect and scan for other networks
      //Serial.print("Heartbeat Failed \n");
//      WiFi.disconnect();
      boolean conn =WiFi.softAPdisconnect (true);
     Serial.println(conn);
      Serial.println("AP stopped" + conn);
      Serial.print("**********************************API Stopped.. Restarting ***************** \n");
      setup();
      }
      else if (failcounter < 5) {
        Serial.print("**********************************Heartbeat Failed.. Retrying ***************** \n");
      }
    }
    else {
      failcounter = 0;
    }
    
  }
}
void loop(void){
  server.handleClient();


    checkheartbeat();
}
