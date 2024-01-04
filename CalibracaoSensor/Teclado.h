

#ifndef Teclado_h
#define Teclado_h

#include <Keypad.h>

class TecladoMatricial {
  public:
    const static int _ROWS = 4; //four _ROWS
    const static int _COLS = 4; //four columns
    //define the cymbols on the buttons of the keypads
    char hexaKeys[_ROWS][_COLS] = {
      {'1', '2', '3', 'A'},
      {'4', '5', '6', 'B'},
      {'7', '8', '9', 'C'},
      {'*', '0', '#', 'D'}
    };
    byte colPins[_COLS] = {33, 32, 18, 19};
    byte rowPins[_ROWS] = {14, 27, 26, 25};

    //initialize an instance of class NewKeypad
    Keypad _keypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, _ROWS, _COLS);

    String Ler_numero_teclado(char tp) {

      String NUMERO = "";

      if (tp == 'C') {
        Serial.println("Digite o ID Frentista: ");
        //DisplayLCD.Mostra_msg("", "ID do  Frentista", "Digite: ", "", 0, 0, 0, 0);
      } else if (tp == 'D') {
        Serial.println("Digite o Veiculo: ");
        //DisplayLCD.Mostra_msg("", "ID Veiculo", "Digite: ", "", 0, 0, 0, 0);
      } else if (tp == 'E') {
        Serial.println("Digite o Hodometro Horimetro: ");
        //DisplayLCD.Mostra_msg("", "Hodometro/Horimetro", "Digite: ", "", 0, 0, 0, 0);
      } else if (tp == 'F') {
        Serial.println("Digite a Atividade: ");
        //DisplayLCD.Mostra_msg("", "Atividade (Opcional)", "Digite: ", "", 0, 0, 0, 0);
      } else if (tp == 'G') {
        Serial.println("Digite a Cultura: ");
        //DisplayLCD.Mostra_msg("", "Cultura (Opcional)", "Digite: ", "", 0, 0, 0, 0);
      }

      char customKey = _keypad.getKey();

      while (customKey != '#') {
        Serial.print(customKey);
        delay(1);
        customKey = _keypad.getKey();
        if (customKey) {
          if (customKey != '#' && customKey != '*') {
            NUMERO += customKey;
            if (tp == 'C') {
              //DisplayLCD.Mostra_msg("", "ID do  Frentista", "Digite: " + NUMERO, "", 0, 0, 0, 0);
            } else if (tp == 'D') {
              //DisplayLCD.Mostra_msg("", "ID Veiculo", "Digite: " + NUMERO, "", 0, 0, 0, 0);
            } else if (tp == 'E') {
              if (NUMERO.length() > 4) {
                int index = NUMERO.lastIndexOf(".");
                char x = NUMERO[index + 1];
                NUMERO[index] = x;
                NUMERO[index + 1] = '.';
                if (NUMERO[0] == '0') {
                  //NUMERO[0] = NUMERO[1];
                  NUMERO.remove(0, 1);
                }
              } else if (NUMERO.length() == 1) {
                NUMERO = "0." + NUMERO;
              }

              //DisplayLCD.Mostra_msg("", "Hodometro/Horimetro", "Digite: " + NUMERO, "", 0, 0, 0, 0);
            } else if (tp == 'F') {
              //DisplayLCD.Mostra_msg("", "Atividade (Opcional)", "Digite: " + NUMERO, "", 0, 0, 0, 0);
            } else if (tp == 'G') {
              //DisplayLCD.Mostra_msg("", "Cultura (Opcional)", "Digite: " + NUMERO, "", 0, 0, 0, 0);
            } 
          }
          if (customKey == '*') {
            NUMERO = "";
            Serial.println();
            return "";
          } else if (customKey == 'C') {
            return "Cancelar";
          }
        }

      }
      Serial.println();
      if (NUMERO == "") {
        return "";
      }

      Serial.print("O numero digitado foi: ");
      Serial.print(NUMERO);
      Serial.println();

      return NUMERO;
    }

};





TecladoMatricial Teclado = TecladoMatricial();



#endif
