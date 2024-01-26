#include <SoftwareSerial.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include "time.h"
#include <WiFiUdp.h>
#include <vector>

std::vector <String> Buffer;

#include "Adafruit_Thermal.h"
#include "Funcoes_Extras.h"
#include "Teclado.h"
#include "WebInterface.h"
#include "Abastecimento.h"
#include "Display.h"

// ------------- Impressão/Comunicação com a impressora -------------
#define TX_PIN 4 // Arduino transmit  YELLOW WIRE  labeled RX on printer
#define RX_PIN 23 // Arduino receive   GREEN WIRE   labeled TX on printer

SoftwareSerial mySerial(RX_PIN, TX_PIN); // Declare SoftwareSerial obj first
Adafruit_Thermal printer(&mySerial);     // Pass addr to printer constructor
#include "Comunicacao.h"

// ------------- Data e Hora -------------
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -3 * 3600;
const int   daylightOffset_sec = 3600;

// ------------- Gerenciador de WiFi -------------
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

int Cupom = 0;

bool flagTimestamp = true;


// -------- WiFi ---------
long int PrevAutoConnect = 0;



// ISR -> Função que é chamada quando há interrupção no PinoInterrupção
void ContaPulsos() {
  Pulsos += 1;
  //  Serial.print("Contando");
  //  Serial.println(Pulsos);
}

String getLocalTime() {
  struct tm timeinfo;
  char MeuBuffer[80];
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "";
  }
  //Serial.println(&timeinfo, "%d/%m/%y %H:%M:%S");
  strftime (MeuBuffer, 80, "%d/%m/%y %H:%M:%S.", &timeinfo);
  return String(MeuBuffer);
}


void setup() {

  // Inicilização da comunicação com a Impressora
  mySerial.begin(9600);
  printer.begin();


  Serial.begin(115200);
  Serial.println(F("Iniciando Sistema Arduino"));



  attachInterrupt(digitalPinToInterrupt(PinoInterrupcao), ContaPulsos, RISING);


  DisplayLCD.Inicializa_display();


  T_intervalo = millis();
  PrevAutoConnect = millis();


  // ------------ SPIFFS -------------
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  PrevTimeRevisao = millis();

  atualizaBuffer(SPIFFS);
  mostraBuffer();
  atualizaConfigInicial(SPIFFS);
  ConfigInicial.Mostrar();

  Cupom = atualizaCupom(SPIFFS);

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
  wm.process();


  // --------------- WiFi ---------------
  if (Estado == "A") {
    if (WiFi.status() == WL_CONNECTED) {

      //Serial.println("Conectado ao WiFi");
      if (flagTimestamp) {
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

        String DadosSerializados = Config_Inicial();
        atualizaArquivoConfigInicial(SPIFFS, DadosSerializados);
        ConfigInicial.Mostrar();

        flagTimestamp = false;

      } else {
        getLocalTime();
      }
    } else {
      if (millis() - PrevAutoConnect > 10000) {
        WiFi.mode(WIFI_STA);
        wm.setConfigPortalBlocking(false);
        wm.setConfigPortalTimeout(60);
        if (!wm.autoConnect()) {
          Serial.println("Falha ao reconectar ao Wi-Fi");
        } else {
          Serial.println("Reconexão bem-sucedida!");
        }
        PrevAutoConnect = millis();
      }
    }
  }

  // --------------- Controle Geral ---------------
  if (Estado == "A") { // ESTADO DE ESPERA
    abastecimento.Limpar();
    if (flagTimestamp) {
      DisplayLCD.Mostra_msg("", "Configure o WiFi", "", "", 0, 0, 0, 0);
      delay(100);
    } else {
      if (millis() - T_intervalo > 1000) {
        Serial.println("Estado A");
        Serial.println(xPortGetCoreID());
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




    if (!VerificacaoExistencia(Frentista, ConfigInicial.FrentistasPossiveis)) {
      Serial.println("Dado invalido");
      DisplayLCD.Mostra_msg("Frentista Invalido", "Digite o ID correto!", "", "", 0, 0, 0, 0);
      delay(2000);
    } else if (Frentista == "") {
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
      abastecimento.Frentista = Frentista;
      stringDeAbastecimento += Frentista + ";";
    }
  } else if (Estado == "D") { // VEICULO -- ID da Máquina
    Serial.println("Estado D");
    String Veiculo = Teclado.Ler_numero_teclado('D');


    if (!VerificacaoExistencia(Veiculo, ConfigInicial.VeiculosPossiveis)) {
      Serial.println("Dado invalido");
      DisplayLCD.Mostra_msg("Veiculo Invalido", "Digite o ID correto!", "", "", 0, 0, 0, 0);
      delay(2000);
    } else if (Veiculo == "") {
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
      abastecimento.Veiculo = Veiculo;
      stringDeAbastecimento += Veiculo + ";";
    }
  } else if (Estado == "E") { // HODÔMETRO -- HORÍMETRO
    Serial.println("Estado E");
    String Hodometro_Horimetro = Teclado.Ler_numero_teclado('E');


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
      abastecimento.Hodometro_Horimetro = Hodometro_Horimetro;
      stringDeAbastecimento += Hodometro_Horimetro + ";";
    }
  } else if (Estado == "F") { // Atividade -- Opcional
    Serial.println("Estado F");
    String Atividade = Teclado.Ler_numero_teclado('F');

    if (Atividade == "") {
      Estado = "G";
    } else if (!VerificacaoExistencia(Atividade, ConfigInicial.AtividadesPossiveis)) {
      Serial.println("Dado invalido");
      DisplayLCD.Mostra_msg("Atividade Invalido", "Digite o ID correto!", "", "", 0, 0, 0, 0);
      delay(2000);
    } else if (StringContemLetras(Atividade)) {
      Serial.println(Atividade);
      DisplayLCD.Mostra_msg("Erro!", "Digite apenas", "numeros", "", 5, 4, 6, 0);
      delay(2000);
    } else if (Atividade == "Cancelar") {
      Estado = "E";
    } else {
      Estado = "G";
      abastecimento.Atividade = Atividade;
      stringDeAbastecimento += Atividade + ";";
    }
  } else if (Estado == "G") { // Cultura -- Opcional
    Serial.println("Estado G");
    String Cultura = Teclado.Ler_numero_teclado('G');

    if (Cultura == "") {
      Estado = "H";
    } else if (!VerificacaoExistencia(Cultura, ConfigInicial.CulturasPossiveis)) {
      Serial.println("Dado invalido");
      DisplayLCD.Mostra_msg("Cultura Invalido", "Digite o ID correto!", "", "", 0, 0, 0, 0);
      delay(2000);
    } else if (StringContemLetras(Cultura)) {
      Serial.println(Cultura);
      DisplayLCD.Mostra_msg("Erro!", "Digite apenas", "numeros", "", 5, 4, 6, 0);
      delay(2000);
    } else if (Cultura == "Cancelar") {
      Estado = "F";
    } else {
      Estado = "H";
      abastecimento.Cultura = Cultura;
      stringDeAbastecimento += Cultura + ";";
    }
  } else if (Estado == "H") { // MODO de CONTROLE
    Serial.println("Estado H");

    int incremento = 0;
    int incrementodifzero = 0; //utilizado para contabilizar tempo despois que o acumulado é diferente de zero
    bool imprime = false;

    Pulsos = 0;
    Acumulado = 0;
    int AcumuladoAnterior = 0; //utilizado para contabilizar tempo despois que o acumulado é diferente de zero
    while (1) {
      delay(Intervalo);
      Serial.print("Pulsos: ");
      Serial.println(Pulsos);

      if (Pulsos == 0 && Acumulado == AcumuladoAnterior && Acumulado != 0) { // deve contar 10 segundo sem haver pulsos depois que o acumulado é igual do ultimo acumulado contabilizado
                                                                             // conta tempo para encerrar qualquer abastecimento
        incrementodifzero ++;

        if (incrementodifzero >= 10000 / Intervalo) {
          Serial.println("Limite de 10 segundos atingido");
          Serial.println("Fim do abastecimento");
          imprime = true;
          break;
        }
      } else {
        incrementodifzero = 0;
      }

      if (Pulsos == 0) { // contar 10 segundo do início do abastecimento -- implementar voltar ao início
        incremento ++;

        if (incremento >= 10000 / Intervalo) {
          Serial.println("Limite de 10 segundos atingido");
          Serial.println("Fim do abastecimento");
          imprime = false;
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
    if (imprime == true){ //verifica se houve abastecimento 
        
      Serial.println("Chegueiii");
      stringDeAbastecimento += String(Acumulado) + ";";
      stringDeAbastecimento += String(getLocalTime()) + ";";
  
      Serial.println(stringDeAbastecimento);
      abastecimento.Litros = Acumulado;
      abastecimento.Cupom = Cupom;
      abastecimento.Timestamp = getLocalTime();
  
      Imprimir(abastecimento, ConfigInicial);
      Cupom++;
      Estado = "I";
    }
    else {
      Estado = "A";
    }
      
  } else if (Estado == "I") { // Envio
    EnviaAbastecimento(stringDeAbastecimento, 0);
    Estado = "Z";
  } else if (Estado == "Z") {
    Estado = "A";
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

    atualizaArquivoCupom (SPIFFS, Cupom);

    Serial.println("Cupom: " + String(Cupom));
    mostraBuffer();
    ConfigInicial.Mostrar();
  }


}
