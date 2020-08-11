#pragma once
#include <LittleFS.h>
#include <math.h>

class Recorder {
    bool fileOpened;
    bool _reading;
    bool _first;
    int first;
    bool _stopped;
    int packetCount;
    uint8_t* wr_ext_flag;
    File file;
    uint8_t* firstPacket; 
    int bytesToSave;
    void (*funStart)();
    void (*funStop)();

    public:
    char filename[5];
    bool fileNameSetted;
    bool _writing;
    Recorder(uint8_t pixel_num, uint8_t *wr_flag) {
        init();
        wr_ext_flag = wr_flag;
        bytesToSave = pixel_num*3;
        _stopped = false;
    }

    void setFunc(void (*funcSt)(), void (*funcFin)()) {
        funStart = funcSt;
        funStop = funcFin;
    }

    void init() {
        fileNameSetted = false;
        fileOpened = false;
        _writing = false;
        _reading = false;
        _first = true;
        first = 0;
        packetCount = 0;
    }

    void setFile(int num) {
        sprintf(filename, "rec%d", num);
        fileNameSetted = true;
        printf("*rec* setted filename: %s\n", filename); //// ????? When commented wifi crashes, when selecting chase which have no file in FS
    }

    void openReadFile() {
        if (fileNameSetted) {
            if (LittleFS.exists(filename)) {
                packetCount = 0;
                file = LittleFS.open(filename, "r");
                fileOpened = true;
                firstPacket = (uint8_t*)calloc(bytesToSave, sizeof(uint8_t));
                printf("*rec* opened file for reading: %s\n", filename);
            }
        }
        else {
            firstPacket = (uint8_t*)calloc(bytesToSave, sizeof(uint8_t));
            printf("*rec* fileName not setted: %s\n", filename);
        }
    }

    void openWriteFile() {
        if (fileNameSetted) {
            packetCount = 0;
            file = LittleFS.open(filename, "w");
            *wr_ext_flag = 1;
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
    
    bool comparePackets(uint8_t* first, uint8_t* current, size_t p_size, uint8_t delta) {
        while(p_size > 0) {
            if(abs(*first++ - *current++) > delta) {
                //printf("*rec* stop compare: %d\n", pcount);
                return false;
            }
            p_size--;
        }
        return true;
    }

    void stopWriting(uint8_t cause) {
        printf("*rec* stop writing: %d (1 match, 2 stop, 3 clear) | count: %d\n", cause, packetCount);
        //printf("*rec* bytesToSave: %d\n", bytesToSave);
        _stopped = true;
        *wr_ext_flag = 0;
        closeFile();
        free(firstPacket);
        init();
        funStop();
    }

    void writePacket(uint8_t* data, uint8_t f_index, uint8_t stop, uint8_t start) {
        if(start == 0 && stop == 0) {
            _stopped = false;
            if(_writing) {
                stopWriting(3);
            }
        }
        if (start >= 250 && !_stopped) {
            first++;
            if (first == 20) {
                //printf("*rec* store firstPacket\n");
                memcpy(firstPacket, data, bytesToSave);
            }

            if (first == 80) {
                //printf("*rec* allow to compare \n");
                _first = false;
            }

            if (!_writing) {
                setFile(f_index);
                openWriteFile();
                //memcpy(firstPacket, data, bytesToSave);
                _writing = true;
                funStart();
            }
            if (stop == 255) {
                stopWriting(2);
            }
            else {
                bool match;
                if (start != 250) {
                    uint8_t delta = start%250;
                   match = comparePackets(firstPacket, data, bytesToSave, delta);
                }
                else {
                    match = false;
                }
                if(!_first && match) {
                    stopWriting(1);
                }
                else {
                    if (first >= 30) {
                        file.write(data, bytesToSave);
                    }
                   packetCount++;
                  //_first = false;
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
            packetCount++;
            file.read(firstPacket, bytesToSave);
            //delay(35 - speed/12);
            delay(50 - speed/6);
        }
        else {
            file.seek(0, SeekSet);
            printf("File size read: %d\n", file.size());
            printf("*rec* read packet count: %d\n",packetCount);
            packetCount = 0;
        }
        return firstPacket;
    }

    void tryStopReading() {
        //printf("*rec* _reading: %d\n", _reading);
        if (_reading) {
            printf("*rec* trying to stop\n");
            stopReading();
        }
    }

    void stopReading() {
        closeFile();
        free(firstPacket);
        _reading = false;
        //printf("*rec* reading stopped\n");
    }

};