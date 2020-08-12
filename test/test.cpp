#include <stdio.h>
#include <math.h>

bool comparePackets(uint8_t* first, uint8_t* second, size_t size);
void tryToStop(uint8_t *stopVal);

int main() {
    uint8_t vl = 127;
    float f = vl*1.0/255;
    printf("%f\n", f);
    uint8_t first[514];
    uint8_t second[514];
    for(int i = 0; i < 514; i++) {
        first[i] = i%255;
        second[i] = (i)%255;
        if(i == 500) second[i] = 15;
    }
    uint8_t val = 15;
    tryToStop(&val);
    printf("changed stopVal: %d\n", val);
    //printf("result: %d\n", comparePackets(first, second, 514));

    size_t s = 10000;
    printf("length of num: %d\n", (int)log10(s)+1);
    char size_str[7];
    sprintf(size_str, "%d\0", s);
    printf("size: %s\n", size_str);
    printf("length: %d\n", sizeof(size_str));
}

bool comparePackets(uint8_t* first, uint8_t* second, size_t size) {
    while (size > 0) {
        //printf("%d : %d\n", *first++, *second++);
        //printf("%d\n", *first++ == *second++);
        if (*first++ != *second++) {
            printf("%d %d %d\n", *--first, *--second, size);
            return false;
        }
        size--;
    }
    return true;
}

void tryToStop(uint8_t* stopVal) {
    printf("stopVal: %d\n", *stopVal);
    *stopVal = 26;
}