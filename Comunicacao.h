#ifndef comunicacao_h
#define comunicacao_h

#include "Abastecimento.h";

bool StringContemLetras(String txt) {
  for (int i = 0; i < txt.length(); i++)  {
    if (txt[i] == 'A'  || txt[i] == 'B' || txt[i] == 'C' || txt[i] == 'D') {
      return true;
    }
  }
  return false;
}


bool VerificacaoExistencia(String Dado, std::vector<String> Vetor) {
  for (String d : Vetor) {
    if (d == Dado) {
      return true;
    }
  }
  return false;
}



void Imprimir(Abastecimento abastecimento, ConfiguracaoInicial Config_Inicial) {
  Serial.println("Imprimindo");
  printer.setFont('B');
  printer.justify('C');
  printer.println("------------------------------");
  printer.println("Abastecimento\n" + Config_Inicial.empresa + "\nPosto: " + Config_Inicial.posto + "\nHorario: " + abastecimento.Timestamp + "\n" );
  Serial.println("Abastecimento\n" + Config_Inicial.empresa + "\nPosto: " + Config_Inicial.posto + "\nHorario: " + abastecimento.Timestamp + "\n" );
  printer.println("------------------------------");
  /*printer.println(Config_Inicial.empresa);
  printer.println("Posto: " + Config_Inicial.posto);
  printer.println("Horario: " + abastecimento.Timestamp );*/
  
  printer.justify('L');
  printer.println("Cupom: " + abastecimento.Cupom );
  printer.println("Frentista: " + abastecimento.Frentista);
  printer.println("Veiculo: " + abastecimento.Veiculo );
  printer.println("Horimetro_Hodometrto: " + abastecimento.Hodometro_Horimetro);
  printer.println("Atividade: " + abastecimento.Atividade );
  printer.println("Cultura: " + abastecimento.Cultura);
  printer.println("Produto: " + Config_Inicial.produto);
  printer.println("Quantidade: " + abastecimento.Litros + "L");
  printer.println("------------------------------");
  printer.println("------------------------------");
  printer.println("------------------------------");
  printer.println("------------------------------");
  
  delay(10000);
}


#endif    comunicacao_h
