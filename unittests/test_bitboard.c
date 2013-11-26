/* Make sure assert is not disabled */
#ifdef NDEBUG
#undef NDEBUG
#endif

#include <stdio.h>
#include <assert.h>
#include "bitboard.h"

void test_bitboard_size()
{
    assert(sizeof(bitboard_t) == 8);
}

void test_bitboard_unsigned()
{
    bitboard_t b;
    b = 0;
    b--;
    assert(b > 0);
}

void test_bitboard_count_bits()
{
    assert(bitboard_count_bits((bitboard_t)0xFFFFFFFFFFFFFFFF) == 64);
    assert(bitboard_count_bits((bitboard_t)0x0000000000000000) ==  0);
    assert(bitboard_count_bits((bitboard_t)0xAAAAAAAAAAAAAAAA) == 32);
    assert(bitboard_count_bits((bitboard_t)0x8000000000000001) ==  2);
    assert(bitboard_count_bits((bitboard_t)0x0000000000000001) ==  1);
    assert(bitboard_count_bits((bitboard_t)0x8000000000000000) ==  1);
    assert(bitboard_count_bits((bitboard_t)0x0000001000000000) ==  1);
}

void test_bitboard_find_bit()
{
    assert(bitboard_find_bit((bitboard_t)0xFFFFFFFFFFFFFFFF) ==  0);
    assert(bitboard_find_bit((bitboard_t)0xFFFFFFFFFFFFFFFE) ==  1);
    assert(bitboard_find_bit((bitboard_t)0x0000000000000001) ==  0);
    assert(bitboard_find_bit((bitboard_t)0x0000000000000002) ==  1);
    assert(bitboard_find_bit((bitboard_t)0x8000000000000000) == 63);
    assert(bitboard_find_bit((bitboard_t)0x0000000001000000) == 24);
}

int main()
{
    bitboard_init();
    
    test_bitboard_size();
    test_bitboard_unsigned();
    test_bitboard_count_bits();
    test_bitboard_find_bit();
    
    return 0;
}
