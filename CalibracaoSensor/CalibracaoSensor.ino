#include "Funcoes_Extras.h"
#include "Teclado.h"
int PinoInterrupcao = 5;


int Pulsos = 0;
int Intervalo = 100;
double volumeDesejado = 0;
double Vazao = 0;
double Acumulado = 0;

void ContaPulsos() {
  Pulsos += 1;
}


void setup() {
  Serial.begin(115200);
  Serial.println(F("Iniciando Sistema Arduino"));

  attachInterrupt(digitalPinToInterrupt(PinoInterrupcao), ContaPulsos, RISING);

}

void loop() {
  int incremento = 0;
  Acumulado = 0;
  Pulsos = 0;
  String valor = "";
  bool c = false;

  while (Serial.available()) {
    char input = Serial.read();
    if(input == '\n'){
      Serial.println("Deu bom");
      c = true;
    }
    valor += input;
  }
  

  if (c) {
    Serial.println("valor foi: " + String(valor));
    delay(2000);
    CONST_CONVERSAO = String(valor).toDouble();
    while (1) {
      delay(Intervalo);
      Serial.print("Pulsos: ");
      Serial.println(Pulsos);
      Serial.println("Fator de Conversão: " + String(CONST_CONVERSAO));

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
    }
    Serial.println("Chegueiii");
  }

}
