#ifndef Abastecimento_h
#define Abastecimento_h


#include "FS.h"
#include "SPIFFS.h"

/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */
#define FORMAT_SPIFFS_IF_FAILED true

String NomeArquivoArmazenamento = "/Abastecimentos.txt";
std::vector <String> Buffer;

String listDir(fs::FS &fs, const char * dirname, uint8_t levels) {

  String TodosArquivos = "";
  Serial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("- failed to open directory");
    return "";
  }
  if (!root.isDirectory()) {
    Serial.println(" - not a directory");
    return "";
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      TodosArquivos = TodosArquivos + file.name() + ";";
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }

  return TodosArquivos;
}

void readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return;
  }

  Serial.println("- read from file:");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

String readFileToString(fs::FS &fs, const char * path) {
  String content = "";
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return "";
  }

  Serial.println("- read from file:");
  while (file.available()) {
    //Serial.write();
    content += String(file.readString());
  }
  file.close();
  return content;
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\r\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("- failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("- message appended");
  } else {
    Serial.println("- append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2) {
  Serial.printf("Renaming file %s to %s\r\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("- file renamed");
  } else {
    Serial.println("- rename failed");
  }
}

void deleteFile(fs::FS &fs, const char * path) {
  Serial.printf("Deleting file: %s\r\n", path);
  if (fs.remove(path)) {
    Serial.println("- file deleted");
  } else {
    Serial.println("- delete failed");
  }
}

void testFileIO(fs::FS &fs, const char * path) {
  Serial.printf("Testing file I/O with %s\r\n", path);

  static uint8_t buf[512];
  size_t len = 0;
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }

  size_t i;
  Serial.print("- writing" );
  uint32_t start = millis();
  for (i = 0; i < 2048; i++) {
    if ((i & 0x001F) == 0x001F) {
      Serial.print(".");
    }
    file.write(buf, 512);
  }
  Serial.println("");
  uint32_t end = millis() - start;
  Serial.printf(" - %u bytes written in %u ms\r\n", 2048 * 512, end);
  file.close();

  file = fs.open(path);
  start = millis();
  end = start;
  i = 0;
  if (file && !file.isDirectory()) {
    len = file.size();
    size_t flen = len;
    start = millis();
    Serial.print("- reading" );
    while (len) {
      size_t toRead = len;
      if (toRead > 512) {
        toRead = 512;
      }
      file.read(buf, toRead);
      if ((i++ & 0x001F) == 0x001F) {
        Serial.print(".");
      }
      len -= toRead;
    }
    Serial.println("");
    end = millis() - start;
    Serial.printf("- %u bytes read in %u ms\r\n", flen, end);
    file.close();
  } else {
    Serial.println("- failed to open file for reading");
  }
}





void atualizaBuffer(fs::FS &fs) {
  String meuArquivo = readFileToString(fs, NomeArquivoArmazenamento.c_str());
  String linha = "";
  Buffer.clear();
  for (int i = 0; i < meuArquivo.length(); i++) {
    if (meuArquivo[i] == '\n') {
      Buffer.push_back(linha);
      linha = "";
    } else {
      linha += meuArquivo[i];
    }
  }
}

void EnviaPrimeiraLinha() {
  if (Buffer.size() > 0) {
    Buffer.erase(Buffer.begin());
  }
}

void atualizaArquivoConfigInicial (fs::FS &fs, String DadosSerializados){
  deleteFile(fs, "/ConfigInicial.txt");
  appendFile(fs, "/ConfigInicial.txt", DadosSerializados.c_str());
}


void atualizaConfigInicial(fs::FS &fs){
  String meuArquivo = readFileToString(fs, "/ConfigInicial.txt");
  DeserializarConfigInicial(meuArquivo);
}

bool EnviaAbastecimento(String stringDeAbastecimento, int retentativa) {
  Serial.println("Tentando Enviar: Digite");

  if (EnviaParaServidor(stringDeAbastecimento)) { // Enviado com sucesso?
    Serial.println("Enviado com sucesso: " + stringDeAbastecimento);
    return true;
  } else { // Erro no envio
    Serial.println("Erro ao enviar");
    if (retentativa == 0) {
      Serial.println("Armazenando no buffer");
      Buffer.push_back(stringDeAbastecimento);
    }
    return false;
  }
}

void mostraBuffer() {
  Serial.println("Mostrando Buffer...");
  for (int i = 0; i < Buffer.size(); i++) {
    Serial.println("-> " + Buffer[i]);
  }
}

void atualizaArquivoCupom (fs::FS &fs, int Cupom){
  deleteFile(fs, "/Cupom.txt");
  appendFile(fs, "/Cupom.txt", String(Cupom).c_str());
}


int atualizaCupom(fs::FS &fs){
  String meuArquivo = readFileToString(fs, "/Cupom.txt");
  return meuArquivo.toInt();
}


#endif    Abastecimento_h
