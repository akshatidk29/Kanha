#include <stdint.h>

uint64_t __udivdi3(uint64_t dividend, uint64_t divisor) {

    if (divisor == 0) 
        return 0; 
    
    uint64_t quotient = 0;
    uint64_t remainder = 0;
    
    for (int i = 63; i >= 0; i--) {
        remainder = (remainder << 1) | ((dividend >> i) & 1);
        if (remainder >= divisor) {
            remainder -= divisor;
            quotient |= ((uint64_t)1 << i);
        }
    }
    return quotient;
}

uint64_t __umoddi3(uint64_t dividend, uint64_t divisor) {
   
    if (divisor == 0) 
        return 0;
    
    uint64_t remainder = 0;
    
    for (int i = 63; i >= 0; i--) {
        remainder = (remainder << 1) | ((dividend >> i) & 1);
        if (remainder >= divisor) {
            remainder -= divisor;
        }
    }
    return remainder;
}
