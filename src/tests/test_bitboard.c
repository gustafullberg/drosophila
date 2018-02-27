/* Make sure assert is not disabled */
#ifdef NDEBUG
#undef NDEBUG
#endif

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

void test_BITBOARD_count_bits()
{
    assert(BITBOARD_count_bits((bitboard_t)0xFFFFFFFFFFFFFFFF) == 64);
    assert(BITBOARD_count_bits((bitboard_t)0x0000000000000000) ==  0);
    assert(BITBOARD_count_bits((bitboard_t)0xAAAAAAAAAAAAAAAA) == 32);
    assert(BITBOARD_count_bits((bitboard_t)0x8000000000000001) ==  2);
    assert(BITBOARD_count_bits((bitboard_t)0x0000000000000001) ==  1);
    assert(BITBOARD_count_bits((bitboard_t)0x8000000000000000) ==  1);
    assert(BITBOARD_count_bits((bitboard_t)0x0000001000000000) ==  1);
}

void test_BITBOARD_find_bit()
{
    assert(BITBOARD_find_bit((bitboard_t)0xFFFFFFFFFFFFFFFF) ==  0);
    assert(BITBOARD_find_bit((bitboard_t)0xFFFFFFFFFFFFFFFE) ==  1);
    assert(BITBOARD_find_bit((bitboard_t)0x0000000000000001) ==  0);
    assert(BITBOARD_find_bit((bitboard_t)0x0000000000000002) ==  1);
    assert(BITBOARD_find_bit((bitboard_t)0x8000000000000000) == 63);
    assert(BITBOARD_find_bit((bitboard_t)0x0000000001000000) == 24);
}

int main()
{
    BITBOARD_init();
    
    test_bitboard_size();
    test_bitboard_unsigned();
    test_BITBOARD_count_bits();
    test_BITBOARD_find_bit();
    
    return 0;
}
