#include <stdio.h>
#include <math.h>
main() {
    int a = 120;
    int b = 144;
    for(int i = 1; i < 30; i++) {
        if(((a%i) == 0) && (b%i) == 0) {
            printf("dig: %d\n", i);
        }
    }
}