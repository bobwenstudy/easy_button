#ifndef _BIT_ARRAY_H_
#define _BIT_ARRAY_H_

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// #define BIT_ARRAY_CONFIG_64

// here can change to uint64_t, if you system is 64bit.
#ifdef BIT_ARRAY_CONFIG_64
typedef uint64_t bit_array_t;
#define BIT_ARRAY_BIT(n) (1ULL << (n))
#else
typedef uint32_t bit_array_t;
#define BIT_ARRAY_BIT(n) (1UL << (n))
#endif
typedef bit_array_t bit_array_val_t;

#define BIT_ARRAY_BITS (sizeof(bit_array_val_t) * 8)

#define BIT_ARRAY_BIT_WORD(bit)  ((bit) / BIT_ARRAY_BITS)
#define BIT_ARRAY_BIT_INDEX(bit) ((bit_array_val_t)(bit) & (BIT_ARRAY_BITS - 1U))

#define BIT_ARRAY_MASK(bit)       BIT_ARRAY_BIT(BIT_ARRAY_BIT_INDEX(bit))
#define BIT_ARRAY_ELEM(addr, bit) ((addr)[BIT_ARRAY_BIT_WORD(bit)])

// word of all 1s
#define BIT_ARRAY_WORD_MAX (~(bit_array_val_t)0)

#define BIT_ARRAY_SUB_MASK(nbits) ((nbits) ? BIT_ARRAY_WORD_MAX >> (BIT_ARRAY_BITS - (nbits)) : (bit_array_val_t)0)

// A possibly faster way to combine two words with a mask
// #define bitmask_merge(a,b,abits) ((a & abits) | (b & ~abits))
#define bitmask_merge(a, b, abits) (b ^ ((a ^ b) & abits))

/**
 * @brief This macro computes the number of bit array variables necessary to
 * represent a bitmap with @a num_bits.
 *
 * @param num_bits Number of bits.
 */
#define BIT_ARRAY_BITMAP_SIZE(num_bits) (1 + ((num_bits)-1) / BIT_ARRAY_BITS)

/**
 * @brief Define an array of bit array variables.
 *
 * This macro defines an array of bit array variables containing at least
 * @a num_bits bits.
 *
 * @note
 * If used from file scope, the bits of the array are initialized to zero;
 * if used from within a function, the bits are left uninitialized.
 *
 * @cond INTERNAL_HIDDEN
 * @note
 * This macro should be replicated in the PREDEFINED field of the documentation
 * Doxyfile.
 * @endcond
 *
 * @param name Name of array of bit array variables.
 * @param num_bits Number of bits needed.
 */
#define BIT_ARRAY_DEFINE(name, num_bits) bit_array_t name[BIT_ARRAY_BITMAP_SIZE(num_bits)]

#if 1
// See http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
static inline bit_array_val_t _windows_popcount(bit_array_val_t w)
{
    w = w - ((w >> 1) & (bit_array_val_t) ~(bit_array_val_t)0 / 3);
    w = (w & (bit_array_val_t) ~(bit_array_val_t)0 / 15 * 3) + ((w >> 2) & (bit_array_val_t) ~(bit_array_val_t)0 / 15 * 3);
    w = (w + (w >> 4)) & (bit_array_val_t) ~(bit_array_val_t)0 / 255 * 15;
    return (bit_array_val_t)(w * ((bit_array_val_t) ~(bit_array_val_t)0 / 255)) >> (sizeof(bit_array_val_t) - 1) * 8;
}

#define POPCOUNT(x) _windows_popcount(x)
#else
#define POPCOUNT(x) (unsigned)__builtin_popcountll(x)
#endif

#define bits_in_top_word(nbits) ((nbits) ? BIT_ARRAY_BIT_INDEX((nbits)-1) + 1 : 0)

static inline void _bit_array_mask_top_word(bit_array_t *target, int num_bits)
{
    // Mask top word
    int num_of_words = BIT_ARRAY_BITMAP_SIZE(num_bits);
    int bits_active = bits_in_top_word(num_bits);
    target[num_of_words - 1] &= BIT_ARRAY_SUB_MASK(bits_active);
}

/**
 * @brief Bit Array test a bit.
 *
 * This routine tests whether bit number @a bit of @a target is set or not.
 *
 * @param target Address of bit array variable or array.
 * @param bit Bit number (starting from 0).
 *
 * @return true if the bit was set, false if it wasn't.
 */
static inline int bit_array_get(const bit_array_t *target, int bit)
{
    bit_array_val_t val = BIT_ARRAY_ELEM(target, bit);

    return (1 & (val >> (bit & (BIT_ARRAY_BITS - 1)))) != 0;
}

/**
 * @brief Bit Array clear a bit.
 *
 * Bit Array clear bit number @a bit of @a target.
 *
 * @param target Address of bit array variable or array.
 * @param bit Bit number (starting from 0).
 */
static inline void bit_array_clear(bit_array_t *target, int bit)
{
    bit_array_val_t mask = BIT_ARRAY_MASK(bit);

    BIT_ARRAY_ELEM(target, bit) &= ~mask;
}

/**
 * @brief Bit Array set a bit.
 *
 * Bit Array set bit number @a bit of @a target.
 *
 * @param target Address of bit array variable or array.
 * @param bit Bit number (starting from 0).
 */
static inline void bit_array_set(bit_array_t *target, int bit)
{
    bit_array_val_t mask = BIT_ARRAY_MASK(bit);

    BIT_ARRAY_ELEM(target, bit) |= mask;
}

/**
 * @brief Bit Array toggle a bit.
 *
 * Bit Array toggle bit number @a bit of @a target.
 *
 * @param target Address of bit array variable or array.
 * @param bit Bit number (starting from 0).
 */
static inline void bit_array_toggle(bit_array_t *target, int bit)
{
    bit_array_val_t mask = BIT_ARRAY_MASK(bit);

    BIT_ARRAY_ELEM(target, bit) ^= mask;
}

/**
 * @brief Bit Array set a bit to a given value.
 *
 * Bit Array set bit number @a bit of @a target to value @a val.
 *
 * @param target Address of bit array variable or array.
 * @param bit Bit number (starting from 0).
 * @param val true for 1, false for 0.
 */
static inline void bit_array_assign(bit_array_t *target, int bit, int val)
{
    bit_array_val_t mask = BIT_ARRAY_MASK(bit);

    if (val)
    {
        BIT_ARRAY_ELEM(target, bit) |= mask;
    }
    else
    {
        BIT_ARRAY_ELEM(target, bit) &= ~mask;
    }
}

static inline void bit_array_clear_all(bit_array_t *target, int num_bits)
{
    memset((void *)target, 0, BIT_ARRAY_BITMAP_SIZE(num_bits) * sizeof(bit_array_val_t));
}

static inline void bit_array_set_all(bit_array_t *target, int num_bits)
{
    memset((void *)target, 0xff, BIT_ARRAY_BITMAP_SIZE(num_bits) * sizeof(bit_array_val_t));
    _bit_array_mask_top_word(target, num_bits);
}

static inline void bit_array_toggle_all(bit_array_t *target, int num_bits)
{
    for (int i = 0; i < BIT_ARRAY_BITMAP_SIZE(num_bits); i++)
    {
        target[i] ^= BIT_ARRAY_WORD_MAX;
    }
    _bit_array_mask_top_word(target, num_bits);
}

//
// Strings and printing
//

// Construct a BIT_ARRAY from a substring with given on and off characters.

// From string method
static inline void bit_array_from_str(bit_array_t *bitarr, const char *str)
{
    int i, index;
    int space = 0;
    int len = strlen(str);

    for (i = 0; i < len; i++)
    {
        index = i - space;
        if (strchr("1", str[i]) != NULL)
        {
            bit_array_set(bitarr, index);
        }
        else if (strchr("0", str[i]) != NULL)
        {
            bit_array_clear(bitarr, index);
        }
        else
        {
            // error.
            space++;
        }
    }
}

// Takes a char array to write to.  `str` must be bitarr->num_of_bits+1 in length
// Terminates string with '\0'
static inline char *bit_array_to_str(const bit_array_t *bitarr, int num_bits, char *str)
{
    int i;

    for (i = 0; i < num_bits; i++)
    {
        str[i] = bit_array_get(bitarr, i) ? '1' : '0';
    }

    str[num_bits] = '\0';

    return str;
}

// Takes a char array to write to.  `str` must be bitarr->num_of_bits+1 in length
// Terminates string with '\0'
static inline char *bit_array_to_str_8(const bit_array_t *bitarr, int num_bits, char *str)
{
    int i;
    int space = 0;

    for (i = 0; i < num_bits; i++)
    {
        str[i + space] = bit_array_get(bitarr, i) ? '1' : '0';

        if ((i + 1) % 8 == 0)
        {
            space++;
            str[i + space] = ' ';
        }
    }

    str[num_bits + space] = '\0';

    return str;
}

//
// Get and set words (internal use only -- no bounds checking)
//

static inline bit_array_val_t _bit_array_get_word(const bit_array_t *target, int num_bits, int start)
{
    int word_index = BIT_ARRAY_BIT_WORD(start);
    int word_offset = BIT_ARRAY_BIT_INDEX(start);

    bit_array_val_t result = target[word_index] >> word_offset;

    int bits_taken = BIT_ARRAY_BITS - word_offset;

    // word_offset is now the number of bits we need from the next word
    // Check the next word has at least some bits
    if (word_offset > 0 && start + bits_taken < num_bits)
    {
        result |= target[word_index + 1] << (BIT_ARRAY_BITS - word_offset);
    }

    return result;
}

// Set 64 bits from a particular start position
// Doesn't extend bit array
static inline void _bit_array_set_word(bit_array_t *target, int num_bits, int start, bit_array_val_t word)
{
    int word_index = BIT_ARRAY_BIT_WORD(start);
    int word_offset = BIT_ARRAY_BIT_INDEX(start);

    if (word_offset == 0)
    {
        target[word_index] = word;
    }
    else
    {
        target[word_index] = (word << word_offset) | (target[word_index] & BIT_ARRAY_SUB_MASK(word_offset));

        if (word_index + 1 < BIT_ARRAY_BITMAP_SIZE(num_bits))
        {
            target[word_index + 1] = (word >> (BIT_ARRAY_BITS - word_offset)) | (target[word_index + 1] & (BIT_ARRAY_WORD_MAX << word_offset));
        }
    }

    // Mask top word
    _bit_array_mask_top_word(target, num_bits);
}

//
// Fill a region (internal use only)
//

// FillAction is fill with 0 or 1 or toggle
typedef enum
{
    ZERO_REGION,
    FILL_REGION,
    SWAP_REGION
} FillAction;

static inline void _bit_array_set_region(bit_array_t *target, int start, int length, FillAction action)
{
    if (length == 0)
        return;

    int first_word = BIT_ARRAY_BIT_WORD(start);
    int last_word = BIT_ARRAY_BIT_WORD(start + length - 1);
    int foffset = BIT_ARRAY_BIT_INDEX(start);
    int loffset = BIT_ARRAY_BIT_INDEX(start + length - 1);

    if (first_word == last_word)
    {
        bit_array_val_t mask = BIT_ARRAY_SUB_MASK(length) << foffset;

        switch (action)
        {
        case ZERO_REGION:
            target[first_word] &= ~mask;
            break;
        case FILL_REGION:
            target[first_word] |= mask;
            break;
        case SWAP_REGION:
            target[first_word] ^= mask;
            break;
        }
    }
    else
    {
        // Set first word
        switch (action)
        {
        case ZERO_REGION:
            target[first_word] &= BIT_ARRAY_SUB_MASK(foffset);
            break;
        case FILL_REGION:
            target[first_word] |= ~BIT_ARRAY_SUB_MASK(foffset);
            break;
        case SWAP_REGION:
            target[first_word] ^= ~BIT_ARRAY_SUB_MASK(foffset);
            break;
        }

        int i;

        // Set whole words
        switch (action)
        {
        case ZERO_REGION:
            for (i = first_word + 1; i < last_word; i++)
                target[i] = (bit_array_val_t)0;
            break;
        case FILL_REGION:
            for (i = first_word + 1; i < last_word; i++)
                target[i] = BIT_ARRAY_WORD_MAX;
            break;
        case SWAP_REGION:
            for (i = first_word + 1; i < last_word; i++)
                target[i] ^= BIT_ARRAY_WORD_MAX;
            break;
        }

        // Set last word
        switch (action)
        {
        case ZERO_REGION:
            target[last_word] &= ~BIT_ARRAY_SUB_MASK(loffset + 1);
            break;
        case FILL_REGION:
            target[last_word] |= BIT_ARRAY_SUB_MASK(loffset + 1);
            break;
        case SWAP_REGION:
            target[last_word] ^= BIT_ARRAY_SUB_MASK(loffset + 1);
            break;
        }
    }
}

// Get the number of bits set (hamming weight)
static inline int bit_array_num_bits_set(bit_array_t *target, int num_bits)
{
    int i;

    int num_of_bits_set = 0;

    for (i = 0; i < BIT_ARRAY_BITMAP_SIZE(num_bits); i++)
    {
        if (target[i] > 0)
        {
            num_of_bits_set += POPCOUNT(target[i]);
        }
    }

    return num_of_bits_set;
}

// Get the number of bits not set (1 - hamming weight)
static inline int bit_array_num_bits_cleared(bit_array_t *target, int num_bits)
{
    return num_bits - bit_array_num_bits_set(target, num_bits);
}

// Copy bits from one array to another
// Note: use MACRO bit_array_copy
// Destination and source can be the same bit_array and
// src/dst regions can overlap
static inline void bit_array_copy(bit_array_t *dst, int dstindx, const bit_array_t *src, int srcindx, int length, int src_num_bits, int dst_num_bits)
{
    // Num of full words to copy
    int num_of_full_words = length / BIT_ARRAY_BITS;
    int i;

    int bits_in_last_word = bits_in_top_word(length);

    if (dst == src && srcindx > dstindx)
    {
        // Work left to right
        for (i = 0; i < num_of_full_words; i++)
        {
            bit_array_val_t word = _bit_array_get_word(src, src_num_bits, srcindx + i * BIT_ARRAY_BITS);
            _bit_array_set_word(dst, dst_num_bits, dstindx + i * BIT_ARRAY_BITS, word);
        }

        if (bits_in_last_word > 0)
        {
            bit_array_val_t src_word = _bit_array_get_word(src, src_num_bits, srcindx + i * BIT_ARRAY_BITS);
            bit_array_val_t dst_word = _bit_array_get_word(dst, dst_num_bits, dstindx + i * BIT_ARRAY_BITS);

            bit_array_val_t mask = BIT_ARRAY_SUB_MASK(bits_in_last_word);
            bit_array_val_t word = bitmask_merge(src_word, dst_word, mask);

            _bit_array_set_word(dst, dst_num_bits, dstindx + num_of_full_words * BIT_ARRAY_BITS, word);
        }
    }
    else
    {
        // Work right to left
        for (i = 0; i < num_of_full_words; i++)
        {
            bit_array_val_t word = _bit_array_get_word(src, src_num_bits, srcindx + length - (i + 1) * BIT_ARRAY_BITS);
            _bit_array_set_word(dst, dst_num_bits, dstindx + length - (i + 1) * BIT_ARRAY_BITS, word);
        }

        if (bits_in_last_word > 0)
        {
            bit_array_val_t src_word = _bit_array_get_word(src, src_num_bits, srcindx);
            bit_array_val_t dst_word = _bit_array_get_word(dst, dst_num_bits, dstindx);

            bit_array_val_t mask = BIT_ARRAY_SUB_MASK(bits_in_last_word);
            bit_array_val_t word = bitmask_merge(src_word, dst_word, mask);
            _bit_array_set_word(dst, dst_num_bits, dstindx, word);
        }
    }

    _bit_array_mask_top_word(dst, dst_num_bits);
}

// copy all of src to dst. dst is resized to match src.
static inline void bit_array_copy_all(bit_array_t *dst, const bit_array_t *src, int num_bits)
{
    for (int i = 0; i < BIT_ARRAY_BITMAP_SIZE(num_bits); i++)
    {
        dst[i] = src[i];
    }
}

//
// Logic operators
//

// Destination can be the same as one or both of the sources
static inline void bit_array_and(bit_array_t *dest, const bit_array_t *src1, const bit_array_t *src2, int num_bits)
{
    for (int i = 0; i < BIT_ARRAY_BITMAP_SIZE(num_bits); i++)
    {
        dest[i] = src1[i] & src2[i];
    }
}

static inline void bit_array_or(bit_array_t *dest, const bit_array_t *src1, const bit_array_t *src2, int num_bits)
{
    for (int i = 0; i < BIT_ARRAY_BITMAP_SIZE(num_bits); i++)
    {
        dest[i] = src1[i] | src2[i];
    }
}

static inline void bit_array_xor(bit_array_t *dest, const bit_array_t *src1, const bit_array_t *src2, int num_bits)
{
    for (int i = 0; i < BIT_ARRAY_BITMAP_SIZE(num_bits); i++)
    {
        dest[i] = src1[i] ^ src2[i];
    }
}

static inline void bit_array_not(bit_array_t *dest, const bit_array_t *src, int num_bits)
{
    for (int i = 0; i < BIT_ARRAY_BITMAP_SIZE(num_bits); i++)
    {
        dest[i] = ~src[i];
    }
}

//
// Shift array left/right.  If fill is zero, filled with 0, otherwise 1
//

// Shift towards LSB / lower index
static inline void bit_array_shift_right(bit_array_t *target, int num_bits, int shift_dist, int fill)
{
    if (shift_dist >= num_bits)
    {
        fill ? bit_array_set_all(target, num_bits) : bit_array_clear_all(target, num_bits);
        return;
    }
    else if (shift_dist == 0)
    {
        return;
    }

    FillAction action = fill ? FILL_REGION : ZERO_REGION;

    int cpy_length = num_bits - shift_dist;
    bit_array_copy(target, 0, target, shift_dist, cpy_length, num_bits, num_bits);

    _bit_array_set_region(target, cpy_length, shift_dist, action);
}

// Shift towards MSB / higher index
static inline void bit_array_shift_left(bit_array_t *target, int num_bits, int shift_dist, int fill)
{
    if (shift_dist >= num_bits)
    {
        fill ? bit_array_set_all(target, num_bits) : bit_array_clear_all(target, num_bits);
        return;
    }
    else if (shift_dist == 0)
    {
        return;
    }

    FillAction action = fill ? FILL_REGION : ZERO_REGION;

    int cpy_length = num_bits - shift_dist;
    bit_array_copy(target, shift_dist, target, 0, cpy_length, num_bits, num_bits);
    _bit_array_set_region(target, 0, shift_dist, action);
}

//
// Comparisons
//

// Compare two bit arrays by value stored, with index 0 being the Least
// Significant Bit (LSB). Arrays must have the same length.
// returns:
//  >0 iff bitarr1 > bitarr2
//   0 iff bitarr1 == bitarr2
//  <0 iff bitarr1 < bitarr2
static inline int bit_array_cmp(const bit_array_t *bitarr1, const bit_array_t *bitarr2, int num_bits)
{
    return memcmp(bitarr1, bitarr2, BIT_ARRAY_BITMAP_SIZE(num_bits) * sizeof(bit_array_val_t));
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _BIT_ARRAY_H_ */
