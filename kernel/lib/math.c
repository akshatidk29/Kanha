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

int64_t __divdi3(int64_t dividend, int64_t divisor)
{
    if (divisor == 0)
        return 0;

    int negative = (dividend < 0) != (divisor < 0);

    uint64_t a = dividend < 0
        ? 0 - (uint64_t)dividend : (uint64_t)dividend;

    uint64_t b = divisor < 0
        ? 0 - (uint64_t)divisor : (uint64_t)divisor;

    uint64_t q = __udivdi3(a, b);

    if (!negative)
        return (int64_t)q;

    if (q == (UINT64_C(1) << 63))
        return INT64_MIN;

    return -(int64_t)q;
}

int64_t __moddi3(int64_t dividend, int64_t divisor)
{
    if (divisor == 0)
        return 0;

    uint64_t a = dividend < 0
        ? 0 - (uint64_t)dividend : (uint64_t)dividend;

    uint64_t b = divisor < 0
        ? 0 - (uint64_t)divisor : (uint64_t)divisor;

    uint64_t r = __umoddi3(a, b);

    if (dividend < 0)
        return -(int64_t)r;

    return (int64_t)r;
}