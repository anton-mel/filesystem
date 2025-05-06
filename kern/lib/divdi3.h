#ifndef _KERN_LIB_DIVDI3_H_
#define _KERN_LIB_DIVDI3_H_

#ifdef _KERN_

typedef unsigned long long u64;

// Performs 64-bit unsigned integer division: returns n / d
u64 __udivdi3(u64 n, u64 d);

// Performs 64-bit unsigned integer modulo: returns n % d
u64 __umoddi3(u64 n, u64 d);

#endif /* _KERN_ */

#endif /* !_KERN_LIB_DIVDI3_H_ */
