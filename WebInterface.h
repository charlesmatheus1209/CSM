#ifndef WebInterface_h
#define WebInterface_h

#include <HTTPClient.h>

#include <ArduinoJson.h>

class ConfiguracaoInicial {
  public:
    std::vector<String> VeiculosPossiveis;
    std::vector<String> FrentistasPossiveis;

    String produto;
    String empresa;
    String posto;

    std::vector<String> AtividadesPossiveis;
    std::vector<String> CulturasPossiveis;

    void Mostrar() {
      Serial.println("Veiculos Possíveis");
      for (String x : this->VeiculosPossiveis) {
        Serial.println(x);
      }

      Serial.println("Frentistas Possiveis");
      for (String x : this->FrentistasPossiveis) {
        Serial.println(x);
      }

      Serial.println("Atividades Possiveis");
      for (String x : this->AtividadesPossiveis) {
        Serial.println(x);
      }

      Serial.println("Culturas Possiveis");
      for (String x : this->CulturasPossiveis) {
        Serial.println(x);
      }

      Serial.println("Produto: " + this->produto);
      Serial.println("Empresa: " + this->empresa);
      Serial.println("Posto: " + this->posto);
    }

    void Limpar() {
      this->produto = "";
      this->empresa = "";
      this->posto  = "";
      VeiculosPossiveis.clear();
      FrentistasPossiveis.clear();
      AtividadesPossiveis.clear();
      CulturasPossiveis.clear();
    }
};

ConfiguracaoInicial ConfigInicial = ConfiguracaoInicial();


bool EnviaParaServidor(String Abastecimento) {
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

    HTTPClient http;

    http.begin("http://gsatestes.somee.com/PostTeste");  //Specify destination for HTTP request
    http.addHeader("Content-Type", "text/json");             //Specify content-type header

    int httpResponseCode = http.POST("\"" + Abastecimento + "\"");   //Send the actual POST request
    Serial.println("HTTP response code");
    Serial.println(httpResponseCode);
    if (httpResponseCode == 200) {

      String response = http.getString();                       //Get the response to the request

      Serial.println(httpResponseCode);   //Print return code
      Serial.println(response);           //Print request answer
      delay(5000);
      return true;
    } else if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("Status Code: " + String(httpCode));
      Serial.println(payload);
    }
    else {
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

void DeserializarConfigInicial(String txt) {
  ConfigInicial.Limpar();
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, txt);

  if (error)
    Serial.println(F("Failed to read Json, using default configuration"));

  JsonArray veiculos = doc["veiculos"];
  JsonArray frentistas = doc["frentistas"];
  String produto = doc["produto"];
  ConfigInicial.produto = produto;
  String empresa = doc["empresa"];
  ConfigInicial.empresa = empresa;
  String posto = doc["posto"];
  ConfigInicial.posto = posto;
  JsonArray atividades = doc["atividades"];
  JsonArray culturas = doc["culturas"];

  for (String veiculo : veiculos) {
    ConfigInicial.VeiculosPossiveis.push_back(veiculo);
  }

  for (String frentista : frentistas) {
    ConfigInicial.FrentistasPossiveis.push_back(frentista);
  }

  for (String atividade : atividades) {
    ConfigInicial.AtividadesPossiveis.push_back(atividade);
  }

  for (String cultura : culturas) {
    ConfigInicial.CulturasPossiveis.push_back(cultura);
  }

}

String Config_Inicial() {
  HTTPClient http;
  http.begin("http://gsatestes.somee.com/InicializacaoControlador?id=" + String(ESP.getEfuseMac()));
  int httpCode = http.GET();

  if (httpCode == 200) { //Check for the returning code
    String payload = http.getString();
    Serial.println("Status Code: " + String(httpCode));

    DeserializarConfigInicial(payload);

    return String(payload);

  } else if (httpCode > 0) {
    String payload = http.getString();
    Serial.println("Status Code: " + String(httpCode));
    Serial.println(payload);
  }
  else {
    Serial.println("Error on HTTP request");
  }

  http.end(); //Free the resources
  return "";
}


#endif    WebInterface_h
