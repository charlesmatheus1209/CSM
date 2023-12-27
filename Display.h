#ifndef Display_h
#define Display_h

#include <LiquidCrystal_I2C.h>

class Display{
  public:
    LiquidCrystal_I2C Tela = LiquidCrystal_I2C(0x27,20,4);  // set the Display address to 0x27 for a 16 chars and 2 line display

    void Inicializa_display(){
      this->Tela.init();                      // initialize the Display 
      this->Tela.init();
      // Print a message to the Display.
      this->Tela.backlight();
      this->Tela.setCursor(1,0);
      this->Tela.print(F("Inicializando"));
    } 

    void Mostra_msg(String s1, String s2, String s3, String s4,int pos1, int pos2, int pos3, int pos4){
      if(WiFi.status() == WL_CONNECTED){
        this->Tela.clear();
        this->Tela.setCursor(pos1,0);
        this->Tela.print(s1);
        this->Tela.setCursor(pos2,1);
        this->Tela.print(s2);
        this->Tela.setCursor(pos3,2);
        this->Tela.print(s3);
        this->Tela.setCursor(pos4,3);
        this->Tela.print(s4 + "W");
      }else{
        this->Tela.clear();
        this->Tela.setCursor(pos1,0);
        this->Tela.print(s1);
        this->Tela.setCursor(pos2,1);
        this->Tela.print(s2);
        this->Tela.setCursor(pos3,2);
        this->Tela.print(s3);
        this->Tela.setCursor(pos4,3);
        this->Tela.print(s4);
      }
    }
    
};

Display DisplayLCD = Display();


#endif    Display_h
