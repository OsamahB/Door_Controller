#include <HTTPClient.h>
#include <ArduinoJson.h>

HTTPClient http;

String getInfo(String URL,bool door_state){
  http.setRedirectLimit(100);
  String response;
  StaticJsonDocument<200> post_doc;
  post_doc["bluetooth_mac"] = "94:b9:7e:fb:57:1a";
  post_doc["token"] = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJibHVldG9vdGhfbWFjIjoiOTQ6Yjk6N2U6ZmI6NTc6MWEifQ.8N-MHMU8FW70mUZ874iFdDGGSydS4ePY7odgidIdSk8";
  post_doc["state"] = door_state;
  String requestBody;
  serializeJson(post_doc, requestBody);
  
  http.begin(URL); //Specify the URL
  http.addHeader("Content-Type", "application/json");        //Specify content-type header
  Serial.println(requestBody);
  int httpResponseCode = http.POST(requestBody);          //Send the actual POST request

  if(httpResponseCode>0){

  response = http.getString();                       //Get the response to the request

  Serial.println("");   //Print return code
  Serial.println(response);           //Print request answer

  }else{
  Serial.print("Error on sending POST: ");
  Serial.println(httpResponseCode);
  response = "ERROR";
  }
  http.end(); //Free the resources
  return response;
}