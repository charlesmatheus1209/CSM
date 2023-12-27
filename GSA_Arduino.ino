#include <SoftwareSerial.h>

#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "Comunicacao.h"
#include "Display.h"
#include "Funcoes_Extras.h"
#include "Teclado.h"
#include "WebInterface.h"
#include "Abastecimento.h"


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

WiFiManager wm;

// Declarações de pinos

int PinoInterrupcao = 5;


// Variáveis utilizadas no programa principal
long int T_intervalo = 0;

String Estado = "A";
int Pulsos = 0;
int Intervalo = 100;
double volumeDesejado = 0;
double Vazao = 0;
double Acumulado = 0;

double ValorDoLitro = 5.5;


// Armazenamento
long int TempoRevisaoBufferArquivo = 10000; //(10 seg)
long int PrevTimeRevisao = 0;


String stringDeAbastecimento = "";


bool flagTimestamp = true;

// ISR -> Função que é chamada quando há interrupção no PinoInterrupção
void ContaPulsos() {
  Pulsos += 1;
  //  Serial.print("Contando");
  //  Serial.println(Pulsos);
}

void setup() {

  Serial.begin(115200);
  Serial.println(F("Iniciando Sistema Arduino"));

  attachInterrupt(  digitalPinToInterrupt(PinoInterrupcao), ContaPulsos, RISING);

  DisplayLCD.Inicializa_display();


  T_intervalo = millis();


  // ------------ SPIFFS -------------
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  PrevTimeRevisao = millis();
  atualizaBuffer(SPIFFS);
  mostraBuffer();



  WiFi.mode(WIFI_STA);
  wm.setConfigPortalBlocking(true/*false*/);
  wm.setConfigPortalTimeout(60);
  //automatically connect using saved credentials if they exist
  //If connection fails it starts an access point with the specified name
  if (wm.autoConnect("AutoConnectAP")) {
    Serial.println("connected...yeey :)");
  }
  else {
    Serial.println("Configportal running");
    DisplayLCD.Mostra_msg("", "Configure o WiFi", "", "", 0, 0, 0, 0);
  }

}

void loop() {

  // --------------- WiFi ---------------
  if (WiFi.status() == WL_CONNECTED) {
    //Serial.println("Conectado ao WiFi");
    if (flagTimestamp) {
      timeClient.begin();
      timeClient.setTimeOffset(-3 * 3600);
      flagTimestamp = false;
    } else {
      timeClient.update();
      //Serial.println(timeClient.getFormattedTime());
    }
  }

  // --------------- Controle Geral ---------------
  if (Estado == "A") { // ESTADO DE ESPERA

    if (flagTimestamp) {
      DisplayLCD.Mostra_msg("", "Configure o WiFi", "", "", 0, 0, 0, 0);
      delay(100);
    } else {
      if (millis() - T_intervalo > 1000) {
        Serial.println("Estado A");
        DisplayLCD.Mostra_msg("A -> Abastecer", "D -> Manutencao", "", "", 0, 0, 0, 0);
        T_intervalo = millis();
      }
      if (Teclado._keypad.getKey() == 'A') {
        Estado = "B";
      } else if (Teclado._keypad.getKey() == 'B') {
        Estado = "X";
      }
    }

  } else if (Estado == "B") { // ESTADO DE EXPLICAÇÃO
    Serial.println("Estado B");
    DisplayLCD.Mostra_msg("Tecla # - Confirmar ", "Tecla * - Apagar", "Tecla C - Cancelar", "", 0, 0, 0, 0);
    stringDeAbastecimento = "";
    delay(3000);
    Estado = "C";
  } else if (Estado == "C") { // FRENTISTA
    Serial.println("Estado C");
    String Frentista = Teclado.Ler_numero_teclado('C');
    stringDeAbastecimento += Frentista + ";";
    if (Frentista == "") {
      Serial.println("Dado não digitado");
      DisplayLCD.Mostra_msg("Digite algo e", "Digite apenas", "numeros", "", 2, 4, 6, 0);
      delay(2000);
    } else if (StringContemLetras(Frentista)) {
      Serial.println(Frentista);
      DisplayLCD.Mostra_msg("Erro!", "Digite apenas", "numeros", "", 5, 4, 6, 0);
      delay(2000);
    } else if (Frentista == "Cancelar") {
      Estado = "A";
    } else {
      Estado = "D";
    }
  } else if (Estado == "D") { // VEICULO -- ID da Máquina
    Serial.println("Estado D");
    String Veiculo = Teclado.Ler_numero_teclado('D');
    stringDeAbastecimento += Veiculo + ";";

    if (Veiculo == "") {
      Serial.println("Dado não digitado");
      DisplayLCD.Mostra_msg("Digite algo e", "Digite apenas", "numeros", "", 2, 4, 6, 0);
      delay(2000);
    } else if (StringContemLetras(Veiculo)) {
      Serial.println(Veiculo);
      DisplayLCD.Mostra_msg("Erro!", "Digite apenas", "numeros", "", 5, 4, 6, 0);
      delay(2000);
    } else if (Veiculo == "Cancelar") {
      Estado = "C";
    } else {
      Estado = "E";
    }
  } else if (Estado == "E") { // HODÔMETRO -- HORÍMETRO
    Serial.println("Estado E");
    String Hodometro_Horimetro = Teclado.Ler_numero_teclado('E');
    stringDeAbastecimento += Hodometro_Horimetro + ";";

    if (Hodometro_Horimetro == "") {
      Serial.println("Dado não digitado");
      DisplayLCD.Mostra_msg("Digite algo e", "Digite apenas", "numeros", "", 2, 4, 6, 0);
      delay(2000);
    } else if (StringContemLetras(Hodometro_Horimetro)) {
      Serial.println(Hodometro_Horimetro);
      DisplayLCD.Mostra_msg("Erro!", "Digite apenas", "numeros", "", 5, 4, 6, 0);
      delay(2000);
    } else if (Hodometro_Horimetro == "Cancelar") {
      Estado = "D";
    } else {
      Estado = "F";
    }
  } else if (Estado == "F") { // Atividade -- Opcional
    Serial.println("Estado F");
    String Atividade = Teclado.Ler_numero_teclado('F');
    stringDeAbastecimento += Atividade + ";";

    if (StringContemLetras(Atividade)) {
      Serial.println(Atividade);
      DisplayLCD.Mostra_msg("Erro!", "Digite apenas", "numeros", "", 5, 4, 6, 0);
      delay(2000);
    } else if (Atividade == "Cancelar") {
      Estado = "E";
    } else {
      Estado = "G";
    }
  } else if (Estado == "G") { // Cultura -- Opcional
    Serial.println("Estado G");
    String Cultura = Teclado.Ler_numero_teclado('G');
    stringDeAbastecimento += Cultura + ";";

    if (StringContemLetras(Cultura)) {
      Serial.println(Cultura);
      DisplayLCD.Mostra_msg("Erro!", "Digite apenas", "numeros", "", 5, 4, 6, 0);
      delay(2000);
    } else if (Cultura == "Cancelar") {
      Estado = "F";
    } else {
      Estado = "H";
    }
  } else if (Estado == "H") { // MODO de CONTROLE
    Serial.println("Estado H");

    int incremento = 0;

    Pulsos = 0;
    while (1) {
      delay(Intervalo);
      Serial.print("Pulsos: ");
      Serial.println(Pulsos);

      if (Pulsos == 0) {
        incremento ++;

        if (incremento >= 10000 / Intervalo) {
          Serial.println("Limite de 10 segundos atingido");
          Serial.println("Fim do abastecimento");
          break;
        }
      } else {
        incremento = 0;
      }

      Vazao = CalculaVazao(Pulsos, Intervalo);

      Serial.print("Vazao: ");
      Serial.print(Vazao);
      Serial.println("L/s");

      Acumulado += CalculaVolume(Vazao, Intervalo);

      Serial.print("Acumulado: ");
      Serial.println(Acumulado);
      Pulsos = 0;
      DisplayLCD.Mostra_msg("Volume: ", String(Acumulado), "", "", 0, 0, 0, 0);

    }
    Serial.println("Chegueiii");
    stringDeAbastecimento += String(Acumulado) + ";";
    stringDeAbastecimento += String(timeClient.getFormattedTime()) + ";";

    Serial.println(stringDeAbastecimento);
    Estado = "I";
  } else if (Estado == "I") { // Envio
    EnviaAbastecimento(stringDeAbastecimento, 0);
    Estado = "J";
  }


  // --------------- Revisão Buffer/Arquivo ---------------
  if (millis() - PrevTimeRevisao  > TempoRevisaoBufferArquivo) {
    PrevTimeRevisao = millis();

    Serial.println("Intervalo 10 seg....");
    
    if (Buffer.size() > 0) {
      Serial.println("Buffer Maior que zero");
      if (EnviaAbastecimento(Buffer[0], 1)) {
        Buffer.erase(Buffer.begin());
      }
    }

    deleteFile(SPIFFS, NomeArquivoArmazenamento.c_str());
    for (int i = 0; i < Buffer.size(); i++) {
      String insert = Buffer[i] + "\n";
      appendFile(SPIFFS, "/Abastecimentos.txt", insert.c_str());
      Serial.println(i);
    }

    mostraBuffer();


  }


}
