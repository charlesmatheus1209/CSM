// ======================================================= Declaração das Bibliotecas Gerais =======================================================

#include <SoftwareSerial.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include "time.h"
#include <WiFiUdp.h>
#include <vector>

// ======================================================= Declaração das Variáveis Globais ========================================================
std::vector <String> Buffer;


// ===================================================== Declaração dos Arquivos de Funções ========================================================

#include "Adafruit_Thermal.h"
#include "Funcoes_Extras.h"
#include "WebInterface.h"
#include "Teclado.h"
#include "Abastecimento.h"
#include "Display.h"
#include "Comunicacao.h"

// ===================================================== Declaração das Variáveis e Definições =====================================================

// ------------- Impressão/Comunicação com a impressora -------------
#define TX_PIN 4 // Arduino transmit  YELLOW WIRE  labeled RX on printer
#define RX_PIN 23 // Arduino receive   GREEN WIRE   labeled TX on printer

SoftwareSerial mySerial(RX_PIN, TX_PIN); // Declare SoftwareSerial obj first
Adafruit_Thermal printer(&mySerial);     // Pass addr to printer constructor


// ------------------ Configurações de Data e Hora ------------------
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -3 * 3600;
const int   daylightOffset_sec = 3600;

// ------------------------ Gerenciador de WiFi ----------------------
WiFiManager wm;
long int PrevAutoConnect = 0;


// --------------------------------- Pinos ---------------------------
int PinoInterrupcao = 5; // Sensor de Fluxo utiliza o pino 5 (GPIO5)



// ---------- Variáveis utilizadas no programa principal -------------
long int T_intervalo = 0;

String Estado = "A"; // Estado Inicial
int Pulsos = 0;
int Intervalo = 100;
double volumeDesejado = 0;
double Vazao = 0;
double Acumulado = 0;

// ---------------------- Variáveis de Configuração de Armazenamento ----------------
long int TempoRevisaoBufferArquivo = 5*60*1000; //(5 min)
long int PrevTimeRevisao = 0;
String stringDeAbastecimento = "";
int Cupom = 1;
bool flagTimestamp = true;

// ============================================ FUNÇÕES GERAIS =====================================

// ISR -> Função que é chamada quando há interrupção no PinoInterrupção
void ContaPulsos() {
  Pulsos += 1;
}


String getHorarioAtual() {
  struct tm timeinfo;
  char MeuBuffer[80];
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "";
  }
  //Serial.println(&timeinfo, "%d/%m/%y %H:%M:%S");
  strftime (MeuBuffer, 80, "%d/%m/%y %H:%M:%S", &timeinfo);
  return String(MeuBuffer);
}


void setup() {

  // Iniciando o Sistema e o Serial
  Serial.begin(115200);
  Serial.println(F("Iniciando Sistema Arduino"));
  Serial.println(WiFi.macAddress());

  // Inicilização da comunicação com a Impressora
  mySerial.begin(9600);
  printer.begin();

  // Configuração da função utilizada para Contar pulsos
  attachInterrupt(digitalPinToInterrupt(PinoInterrupcao), ContaPulsos, RISING);
  //pinMode(12, OUTPUT);


  // Inicialiação do Display LCD
  DisplayLCD.Inicializa_display();


  // Configuração das variáveis de controle de tempo
  T_intervalo = millis();
  PrevAutoConnect = millis();


  // Configuração do SPIFFS -> Armazenamento interno do ESP32
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  PrevTimeRevisao = millis();

  // Funções responáveis por ler os arquivos armazenados no controlador e preencher as devidas variáveis
  atualizaBuffer(SPIFFS);
  mostraBuffer();
  atualizaConfigInicial(SPIFFS);
  ConfigInicial.Mostrar();
  Cupom = atualizaCupom(SPIFFS);

  //Limpar todos os arquivos
  //LimparTodosArquivos(SPIFFS);



  // Inicialização do Wifi
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
      if (flagTimestamp) { // Atualização e reconfiguração dos arquivos de configuração a partir da primeira comunicação com a internet
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

        String DadosSerializados = Config_Inicial();
        atualizaArquivoConfigInicial(SPIFFS, DadosSerializados);
        ConfigInicial.Mostrar();

        flagTimestamp = false;

      } else {
        getHorarioAtual();
      }
    } else {
      if (millis() - PrevAutoConnect > 10000) { // Caso haja a Desconexão, o portal iniciará em 10 seg
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
        DisplayLCD.Mostra_msg("A -> Abastecer", ""/*"D -> Manutencao"*/, "", "", 0, 0, 0, 0);
        T_intervalo = millis();
      }
      if (Teclado._keypad.getKey() == 'A') {
        Estado = "B";
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

    if (Frentista == "Cancelar") {
      Estado = "A";
    } else if (StringContemLetras(Frentista)) {
      Serial.println(Frentista);
      DisplayLCD.Mostra_msg("Erro!", "Digite apenas", "numeros", "", 5, 4, 6, 0);
      delay(2000);
    } else if (!VerificacaoExistencia(Frentista, ConfigInicial.FrentistasPossiveis)) {
      Serial.println("Dado invalido");
      DisplayLCD.Mostra_msg("Frentista Invalido", "Digite o ID correto!", "", "", 0, 0, 0, 0);
      delay(2000);
    } else if (Frentista == "") {
      Serial.println("Dado não digitado");
      DisplayLCD.Mostra_msg("Digite algo e", "Digite apenas", "numeros", "", 2, 4, 6, 0);
      delay(2000);
    } else  {
      Estado = "D";
      abastecimento.Frentista = Frentista;
      stringDeAbastecimento += Frentista + ";";
    }
  } else if (Estado == "D") { // VEICULO -- ID da Máquina
    Serial.println("Estado D");
    String Veiculo = Teclado.Ler_numero_teclado('D');

    if (Veiculo == "Cancelar") {
      Estado = "C";
    } else if (StringContemLetras(Veiculo)) {
      Serial.println(Veiculo);
      DisplayLCD.Mostra_msg("Erro!", "Digite apenas", "numeros", "", 5, 4, 6, 0);
      delay(2000);
    } else if (!VerificacaoExistencia(Veiculo, ConfigInicial.VeiculosPossiveis)) {
      Serial.println("Dado invalido");
      DisplayLCD.Mostra_msg("Veiculo Invalido", "Digite o ID correto!", "", "", 0, 0, 0, 0);
      delay(2000);
    } else if (Veiculo == "") {
      Serial.println("Dado não digitado");
      DisplayLCD.Mostra_msg("Digite algo e", "Digite apenas", "numeros", "", 2, 4, 6, 0);
      delay(2000);
    } else  {
      Estado = "E";
      abastecimento.Veiculo = Veiculo;
      stringDeAbastecimento += Veiculo + ";";
    }
  } else if (Estado == "E") { // HODÔMETRO -- HORÍMETRO
    Serial.println("Estado E");
    String Hodometro_Horimetro = Teclado.Ler_numero_teclado('E');

    if (Hodometro_Horimetro == "Cancelar") {
      Estado = "D";
    } else if (StringContemLetras(Hodometro_Horimetro)) {
      Serial.println(Hodometro_Horimetro);
      DisplayLCD.Mostra_msg("Erro!", "Digite apenas", "numeros", "", 5, 4, 6, 0);
      delay(2000);
    } else if (Hodometro_Horimetro == "") {
      Serial.println("Dado não digitado");
      DisplayLCD.Mostra_msg("Digite algo e", "Digite apenas", "numeros", "", 2, 4, 6, 0);
      delay(2000);
    } else   {
      abastecimento.Hodometro_Horimetro = Hodometro_Horimetro;
      stringDeAbastecimento += Hodometro_Horimetro + ";";

      if (ConfigInicial.ObrigatorioAtividades) {
        Estado = "F";
      } else if (ConfigInicial.ObrigatorioCultura) {
        Estado = "G";
        stringDeAbastecimento += ";";
      } else {
        Estado = "H";
        stringDeAbastecimento += ";;";
      }



    }
  } else if (Estado == "F") { // Atividade -- Opcional
    Serial.println("Estado F");
    String Atividade = Teclado.Ler_numero_teclado('F');

    if (Atividade == "Cancelar") {
      Estado = "E";
    } else if (Atividade == "") {
      Serial.println("Dado não digitado");
      DisplayLCD.Mostra_msg("Digite algo e", "Digite apenas", "numeros", "", 2, 4, 6, 0);
      delay(2000);
    } else if (StringContemLetras(Atividade)) {
      Serial.println(Atividade);
      DisplayLCD.Mostra_msg("Erro!", "Digite apenas", "numeros", "", 5, 4, 6, 0);
      delay(2000);
    } else if (!VerificacaoExistencia(Atividade, ConfigInicial.AtividadesPossiveis)) {
      Serial.println("Dado invalido");
      DisplayLCD.Mostra_msg("Atividade Invalido", "Digite o ID correto!", "", "", 0, 0, 0, 0);
      delay(2000);
    } else   {
      abastecimento.Atividade = Atividade;
      stringDeAbastecimento += Atividade + ";";

       if (ConfigInicial.ObrigatorioCultura) {
        Estado = "G";
      } else {
        Estado = "H";
        stringDeAbastecimento += ";";
      }
    }
  } else if (Estado == "G") { // Cultura -- Opcional
    Serial.println("Estado G");

    String Cultura = Teclado.Ler_numero_teclado('G');

    if (Cultura == "Cancelar") {
      Estado = "F";
    } else if (Cultura == "") {
      Serial.println("Dado não digitado");
      DisplayLCD.Mostra_msg("Digite algo e", "Digite apenas", "numeros", "", 2, 4, 6, 0);
      delay(2000);
    } else if (StringContemLetras(Cultura)) {
      Serial.println(Cultura);
      DisplayLCD.Mostra_msg("Erro!", "Digite apenas", "numeros", "", 5, 4, 6, 0);
      delay(2000);
    } else if (!VerificacaoExistencia(Cultura, ConfigInicial.CulturasPossiveis)) {
      Serial.println("Dado invalido");
      DisplayLCD.Mostra_msg("Cultura Invalido", "Digite o ID correto!", "", "", 0, 0, 0, 0);
      delay(2000);
    } else  {
      Estado = "H";
      abastecimento.Cultura = Cultura;
      stringDeAbastecimento += Cultura + ";";
    }
  } else if (Estado == "H") { // MODO de CONTROLE
    // Neste estado ocorre a inicialização do modo em que será lido o sensor de Fluxo
    Serial.println("Estado H");

    int incremento = 0;
    Pulsos = 0;
    Acumulado = 0;
    long int TempoPercorrido = millis();
    bool imprimir = false;

    //digitalWrite(12, HIGH);
    while (1) {

      if (millis() - TempoPercorrido > 60000 && Acumulado == 0) {
        TempoPercorrido = millis();
        break;
      }

      delay(Intervalo);
      Serial.print("Pulsos: ");
      Serial.println(Pulsos);

      if (Pulsos == 0) {
        incremento ++;

        if (incremento >= 10000 / Intervalo && Acumulado != 0) {
          Serial.println("Limite de 10 segundos atingido");
          Serial.println("Fim do abastecimento");
          imprimir = true;
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

    //digitalWrite(12, LOW);

    Serial.println("Chegueiii");
    stringDeAbastecimento += String(Acumulado) + ";";
    stringDeAbastecimento += String(getHorarioAtual()) + ";";
    stringDeAbastecimento += String(Cupom);

    Serial.println(stringDeAbastecimento);
    abastecimento.Litros = Acumulado;
    abastecimento.Cupom = Cupom;
    abastecimento.Timestamp = getHorarioAtual();



    if (imprimir) {
      DisplayLCD.Mostra_msg("Abastecimento Feito!", "Qtd: " + String(abastecimento.Litros), String(abastecimento.Timestamp), "", 0, 0, 0, 0);
      delay(4000);
      Imprimir(abastecimento, ConfigInicial, printer);
      Cupom++;
      Estado = "I";
    } else {
      DisplayLCD.Mostra_msg("Abastec. Cancelado", "Qtd: " + String(abastecimento.Litros), String(abastecimento.Timestamp), "", 0, 0, 0, 0);
      delay(4000);
      Estado = "A";
    }


  } else if (Estado == "I") { // Envio
    EnviaAbastecimento(stringDeAbastecimento, 0);
    Estado = "Z";
  } else if (Estado == "Z") {
    Estado = "A";
  }


  // --------------- Revisão Buffer/Arquivo ---------------
  // Nesta seção do loop, fazemos uma verficação a cada 5 min. Nessa verificação,
  // atualiza-se os arquivos e tenta-se enviar os dados, caso o buffer não esteja vazio.
  if (millis() - PrevTimeRevisao  > TempoRevisaoBufferArquivo) {
    PrevTimeRevisao = millis();

    Serial.println("Intervalo 5 min....");

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
