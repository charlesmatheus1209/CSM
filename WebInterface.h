#ifndef WebInterface_h
#define WebInterface_h

#include <HTTPClient.h>

bool EnviaParaServidor(String Abastecimento) {
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

    HTTPClient http;

    http.begin("http://gsa123123.somee.com/PostTeste?informacao=" + Abastecimento);  //Specify destination for HTTP request
    http.addHeader("Content-Type", "text/plain");             //Specify content-type header

    int httpResponseCode = http.POST("Posting");   //Send the actual POST request
    Serial.println("HTTP response code");
    Serial.println(httpResponseCode);
    if (httpResponseCode > 0) {

      String response = http.getString();                       //Get the response to the request

      Serial.println(httpResponseCode);   //Print return code
      Serial.println(response);           //Print request answer
      delay(5000);
      return true;
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
      delay(5000);
      return false;
    }
    http.end();  //Free resources
  } else {
    Serial.println("Error in WiFi connection");
    delay(5000);
    return false;
  }

  
}


#endif    WebInterface_h
