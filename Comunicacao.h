#ifndef comunicacao_h
#define comunicacao_h

bool StringContemLetras(String txt){
  for(int i = 0; i < txt.length(); i++)  {
    if(txt[i] == 'A'  || txt[i] == 'B' || txt[i] == 'C' || txt[i] == 'D'){
      return true;
    }
  }
  return false;
}
#endif    comunicacao_h
