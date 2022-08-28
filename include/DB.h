#include "SPIFFS.h"

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return;
    }

    Serial.println("- read from file:");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("- file written");
    } else {
        Serial.println("- write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    file.seek(4);
    if(file.print(message)){
        Serial.println("- message appended");
    } else {
        Serial.println("- append failed");
    }
    file.close();
}

int findUser(fs::FS &fs, String user){

    File file = fs.open("/User.txt");
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return -1;
    }

    int count = 0;
    char chr;
    String line;
    while(file.available()){
        chr = file.read();
        if (chr == '\n'){
          count++;
          if (line.equals(user)){
            file.close();
            return count;
          }
          line = "";
        }
        else line.concat(chr);
    }
    file.close();
    Serial.println("ERROR: Cannot find the user");
    return -1;
}

String getPssword(fs::FS &fs, int userNumber){

    File file = fs.open("/Password.txt");
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return "";
    }

    int count = 0;
    char chr;
    String line;
    while(file.available()){
        chr = file.read();
        if (chr == '\n'){
          count++;
          if (count == userNumber){
            file.close();
            return line;
          }
          line = "";
        }
        else line.concat(chr);
    }
    file.close();
    Serial.println("ERROR: Cannot find the password");
    return "";
}
