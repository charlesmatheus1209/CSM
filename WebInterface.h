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

    bool ObrigatorioCultura;
    bool ObrigatorioAtividades;

    double saldo = 0;

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
      Serial.println("Obrigatorio Cultura: " + String(this->ObrigatorioCultura));
      Serial.println("Obrigatorio Atividades: " + String(this->ObrigatorioAtividades));
      Serial.println("Saldo: " + String(this->saldo));
    }

    void Limpar() {
      this->produto = "";
      this->empresa = "";
      this->posto  = "";
      this->ObrigatorioCultura = false;
      this->ObrigatorioAtividades = false;
      this->saldo = 0;
      VeiculosPossiveis.clear();
      FrentistasPossiveis.clear();
      AtividadesPossiveis.clear();
      CulturasPossiveis.clear();
    }
};

ConfiguracaoInicial ConfigInicial = ConfiguracaoInicial();


bool EnviaParaServidor(String Abastecimento) {
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi 

    HTTPClient http;

    http.begin("http://34.95.251.59:8080/gsa/controlador/envia_registros");  //Especifica destination for HTTP request
    http.addHeader("Content-Type", "text/plain");             //Especifica content-type header

    Serial.println("\"" + Abastecimento + ";" + WiFi.macAddress()  + "\"");
    int httpResponseCode = http.POST(Abastecimento + ";" + WiFi.macAddress());   // Envia requisição
    Serial.println(httpResponseCode);
    if (httpResponseCode == 200) {

      String response = http.getString();                       //Get resposta da requisição

      Serial.println(httpResponseCode);   //Print codigo
      Serial.println(response);           //Print resposta
      delay(5000);
      return true;
    } else  {
      String payload = http.getString();
      Serial.println("Status Code: " + String(httpResponseCode));
      Serial.println(payload);
      return false;
    }
    http.end();  //Free resources
  } else {
    Serial.println("Erro na conexao WiFi");
    delay(5000);
    return false;
  }
}

void DeserializarConfigInicial(String txt) {
  ConfigInicial.Limpar();
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, txt);

  if (error)
    Serial.println(F("Falhs ao ler o Json"));

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
  bool obCultura = doc["obriga_cultura"];
  ConfigInicial.ObrigatorioCultura = obCultura;
  bool obAtivid = doc["obriga_atividade"];
  ConfigInicial.ObrigatorioAtividades = obAtivid;
  double saldo = doc["saldo"];
  ConfigInicial.saldo = saldo;

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

  
  ConfigInicial.Mostrar();
}

String Config_Inicial() {
  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;
    http.begin("http://34.95.251.59:8080/gsa/controlador/busca_registros");
    http.addHeader("Content-Type", "text/json");             //Especifica content-type header

    int httpResponseCode = http.POST("{ \"serial\": \""+ WiFi.macAddress() +"\"}");   //Envia o POST

    if (httpResponseCode == 200) { //Check o código da resposta
      String payload = http.getString();
      Serial.println("Status Code: " + String(httpResponseCode));
      Serial.println("payload: " + payload);

      DeserializarConfigInicial(payload);

      return String(payload);

    } else if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println("Status Code: " + String(httpResponseCode));
      Serial.println(payload);
    }
    else {
      Serial.println("Erro na requisição HTTP ");
    }

    http.end(); //Free the resources
    return "";
  } else {
    Serial.println("Erro na conexão WiFi");
    delay(5000);
    return "";
  }
}

#endif    WebInterface_h
