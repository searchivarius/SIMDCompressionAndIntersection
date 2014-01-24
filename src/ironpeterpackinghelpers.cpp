#include "ironpeterpackinghelpers.h"

/*
 * Originally written by Peter Popov in Sept 2012
 * Bug-fixed and refactored by Leonid Boytsov in March 2013.
 * Source was taken from https://github.com/IronPeter/groupvarint
*/

__m128i IronPeterPackingHelpers::shuffles4[256];
__m128i IronPeterPackingHelpers::shuffles2[256];
__m128i IronPeterPackingHelpers::shuffle16 =  _mm_set_epi8(0xf, 0xe, 0xf, 0xe,
                                  0xf, 0xe, 0xf, 0xe,
                                  0xf, 0xe, 0xf, 0xe,
                                  0xf, 0xe, 0xf, 0xe);

size_t  IronPeterPackingHelpers::sizes4[256];
size_t  IronPeterPackingHelpers::sizes2[256];

// This structure is solely to force initialization at the program start
struct AutoInit {
    AutoInit() {
        IronPeterPackingHelpers::Init();
    }
} Do;

void IronPeterPackingHelpers::Init() {
    for (size_t i = 0; i < 256; ++i) {
        {
        uint8_t buff[16];
        size_t k = 0;
        for (size_t j = 0; j < 4; ++j) {
            size_t size = (i >> (j * 2)) & 0x3;
            //buff[j * 4 + 0] = size >= 0 ? k : 0xff;
            buff[j * 4 + 0] = static_cast<uint8_t>(k);
            //k += size >= 0;
            k++;
            buff[j * 4 + 1] = static_cast<uint8_t>(size >= 1 ? k : 0xff);
            k += size >= 1;
            buff[j * 4 + 2] = static_cast<uint8_t>(size >= 2 ? k : 0xff);
            k += size >= 2;
            buff[j * 4 + 3] = static_cast<uint8_t>(size >= 3 ? k : 0xff);
            k += size >= 3;
        }
        sizes4[i] = k;
        shuffles4[i] = _mm_loadu_si128(reinterpret_cast<__m128i *>(buff));
        }
        {
        uint8_t buff[16];
        size_t k = 0;
        for (size_t j = 0; j < 8; ++j) {
            size_t size = (i >> (j)) & 0x1;
            //buff[j * 2 + 0] = size >= 0 ? k : 0xff;
            buff[j * 2 + 0] = static_cast<uint8_t>(k);
            //k += size >= 0;
            k++;
            buff[j * 2 + 1] = static_cast<uint8_t>(size >= 1 ? k : 0xff);
            k += size >= 1;
        }
        sizes2[i] = k;
        shuffles2[i] = _mm_loadu_si128(reinterpret_cast<__m128i *>(buff));
        }
    }
}

