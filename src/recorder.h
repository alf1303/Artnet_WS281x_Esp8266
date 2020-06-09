#pragma once
#include <LittleFS.h>

class Recorder {
    bool fileNameSetted;
    bool fileOpened;
    bool _writing;
    bool _reading;
    bool _first;
    bool _stopped;
    char filename[5];
    int packetCount;
    File file;
    uint8_t* firstPacket; 
    uint8_t bytesToSave;

    public:
    Recorder(uint8_t pixel_num) {
        init();
        bytesToSave = pixel_num*3;
        _stopped = false;
    };

    void init() {
        fileNameSetted = false;
        fileOpened = false;
        _writing = false;
        _reading = false;
        _first = true;
        packetCount = 0;
    }

    void setFile(int num) {
        sprintf(filename, "rec%d", num);
        fileNameSetted = true;
        printf("*rec* setted filename: %s\n", filename);
    }

    void openReadFile() {
        if (fileNameSetted) {
            if (LittleFS.exists(filename)) {
                file = LittleFS.open(filename, "r");
                fileOpened = true;
                firstPacket = (uint8_t*)calloc(bytesToSave, sizeof(uint8_t));
                printf("*rec* opened file for reading: %s\n", filename);
            }
        }
        else {
            printf("*rec* fileName not setted: %s\n", filename);
        }
    }

    void openWriteFile() {
        if (fileNameSetted) {
            file = LittleFS.open(filename, "w");
            fileOpened = true;
            firstPacket = (uint8_t*)calloc(bytesToSave, sizeof(uint8_t));
            printf("*rec* opened file for writing: %s\n", filename);
        }
        else {
            printf("*rec* fail to open file for writing: %s\n", filename);
        }
    }
    
    void closeFile() {
        if(LittleFS.exists(filename)) {
            file.close();
            fileOpened = false;
            fileNameSetted = false;
            printf("*rec* closed file: %s\n", filename);
        }
        else {
            printf("*rec* error while closing file\n");
        }
    }
    
    bool comparePackets(uint8_t* first, uint8_t* current, size_t p_size) {
        while(p_size > 0) {
            if(*first++ != *current++) {
                return false;
            }
            p_size--;
        }
        return true;
    }

    void stopWriting(uint8_t cause) {
        printf("*rec* stop writing: %d (1 match, 2 stop, 3 clear) | count: %d\n", cause, packetCount);
        _stopped = true;
        closeFile();
        free(firstPacket);
        init();
    }

    void writePacket(uint8_t* data, uint8_t f_index, uint8_t stop, uint8_t start) {
        if(start == 0 && stop == 0) {
            _stopped = false;
            if(_writing) {
                stopWriting(3);
            }
        }
        if (start == 255 && !_stopped) {
            if (!_writing) {
                setFile(f_index);
                openWriteFile();
                memcpy(firstPacket, data, bytesToSave);
                _writing = true;
            }
            if (stop == 255) {
                stopWriting(2);
            }
            else {
                bool match = comparePackets(firstPacket, data, bytesToSave);
                if(!_first && match) {
                    stopWriting(1);
                }
                else {
                   file.write(data, bytesToSave);
                   packetCount++;
                  _first = false;
                }
            }
        }
    }

    uint8_t* readPacket(uint8_t f_index, uint8_t speed) {
        if(!_reading) {
            setFile(f_index);
            openReadFile();
            _reading = true;
            printf("*rec* start reading\n");
        }
        if(file.available()) {
            file.read(firstPacket, bytesToSave);
            delay(35 - speed/12);
        }
        else {
            file.seek(0, SeekSet);
        }
        return firstPacket;
    }

    void tryStopReading() {
        if (_reading) {
            stopReading();
        }
    }

    void stopReading() {
        closeFile();
        free(firstPacket);
        _reading = false;
        printf("*rec* reading stopped\n");
    }

};