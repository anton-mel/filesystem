// Portable 64-bit unsigned division and modulo, no std headers

typedef unsigned long long u64;

// __udivdi3: divide unsigned 64-bit integers
u64 __udivdi3(u64 n, u64 d)
{
    u64 q = 0;
    for (int i = 63; i >= 0; i--)
    {
        if ((n >> i) >= d)
        {
            n -= d << i;
            q |= (u64)1 << i;
        }
    }
    return q;
}

// __umoddi3: modulo of unsigned 64-bit integers
u64 __umoddi3(u64 n, u64 d)
{
    return n - __udivdi3(n, d) * d;
}
