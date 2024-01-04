#ifndef Funcoes_h
#define Funcoes_h


float CONST_CONVERSAO = 20;

double CalculaVazao(int Pulsos, int Intervalo){

  //Freq=(6.6*Q), Q = L/min +/-3%
  double frequencia = (double)Pulsos / Intervalo;

  //retirar dps
  frequencia = frequencia;
  
  double Vazao = 60 * frequencia / CONST_CONVERSAO; 
  return Vazao;  
}

double CalculaVolume(double Vazao, int Intervalo/*ms*/){
  double intervaloSegundos = ((double)Intervalo / 1000); /*s*/
  Serial.println("CalculaVolume");
  Serial.println(intervaloSegundos);
  Serial.println(Vazao * intervaloSegundos);
  return Vazao * intervaloSegundos;  
}

#endif    Funcoes_h
