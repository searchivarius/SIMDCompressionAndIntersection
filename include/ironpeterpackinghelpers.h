#ifndef IRON_PETER_PACKING_HELPERS_H
#define IRON_PETER_PACKING_HELPERS_H

/*
 * Originally written by Peter Popov in Sept 2012
 * Bug-fixed and refactored by Leonid Boytsov in March 2013.
 * Source was taken from https://github.com/IronPeter/groupvarint
*/

#include "common.h"
#include "delta.h"
#include "util.h"
#include "codecs.h"

using namespace std;




struct IronPeterPackingHelpers {
    static __m128i shuffles4[256];
    static __m128i shuffles2[256];
    static __m128i shuffle16;
    static size_t  sizes4[256];
    static size_t  sizes2[256];

    static void Init();

    static __attribute__((always_inline))
    __m128i inline Integrate4(__m128i v0, __m128i prev) {
        __m128i v1 = _mm_add_epi32(_mm_slli_si128(v0, 8), v0);
        __m128i v2 = _mm_add_epi32(_mm_slli_si128(v1, 4), v1);
        return _mm_add_epi32(v2, _mm_shuffle_epi32(prev, 0xff));
    }

    static __attribute__((always_inline))
    __m128i inline Integrate2(__m128i v0) {
        __m128i v1 = _mm_add_epi16(_mm_slli_si128(v0, 8), v0);
        __m128i v2 = _mm_add_epi16(_mm_slli_si128(v1, 4), v1);
        __m128i v3 = _mm_add_epi16(_mm_slli_si128(v2, 2), v2);
        return v3;
    }

    static __attribute__((always_inline))
    __m128i inline Integrate1(__m128i v0) {
        __m128i v1 = _mm_add_epi16(_mm_slli_si128(v0, 8), v0);
        __m128i v2 = _mm_add_epi16(_mm_slli_si128(v1, 4), v1);
        __m128i v3 = _mm_add_epi16(_mm_slli_si128(v2, 2), v2);
        __m128i v4 = _mm_add_epi16(_mm_slli_si128(v3, 1), v3);
        return v4;
    }

    __attribute__((always_inline))
    static inline int Type(uint32_t i) {
        if (i < 0xff)
            return 0;
        if (i < 0xffff)
            return 1;
        if (i < 0xffffff)
            return 2;
        return 3;
    }

    static uint8_t *Code4(const uint32_t *deltas, uint8_t *code, uint8_t *data) {
    	const int t0 = Type(deltas[0]);
    	const int t1 = Type(deltas[1]);
    	const int t2 = Type(deltas[2]);
    	const int t3 = Type(deltas[3]);
        code[0] = static_cast<uint8_t>(t0 + t1 * 4 + t2 * 16 + t3 * 64);
        (reinterpret_cast<int32_t*>(data))[0] = deltas[0];
        data += t0 + 1;
        (reinterpret_cast<int32_t*>(data))[0] = deltas[1];
        data += t1 + 1;
        (reinterpret_cast<int32_t*>(data))[0] = deltas[2];
        data += t2 + 1;
        (reinterpret_cast<int32_t*>(data))[0] = deltas[3];
        data += t3 + 1;
        return data;
    }

    static uint8_t *Code2(const uint32_t *deltas, uint8_t *code, uint8_t *data) {
    	const int t0 = Type(deltas[0]);
    	const int t1 = Type(deltas[1]);
    	const int t2 = Type(deltas[2]);
    	const int t3 = Type(deltas[3]);
    	const int t4 = Type(deltas[4]);
    	const int t5 = Type(deltas[5]);
    	const int t6 = Type(deltas[6]);
    	const int t7 = Type(deltas[7]);
        code[0] = static_cast<uint8_t>(t0 + t1 * 2 + t2 * 4 + t3 * 8 + t4 * 16 + t5 * 32 + t6 * 64 + t7 * 128);
        (reinterpret_cast<int32_t*>(data))[0] = deltas[0];
        data += t0 + 1;
        (reinterpret_cast<int32_t*>(data))[0] = deltas[1];
        data += t1 + 1;
        (reinterpret_cast<int32_t*>(data))[0] = deltas[2];
        data += t2 + 1;
        (reinterpret_cast<int32_t*>(data))[0] = deltas[3];
        data += t3 + 1;
        (reinterpret_cast<int32_t*>(data))[0] = deltas[4];
        data += t4 + 1;
        (reinterpret_cast<int32_t*>(data))[0] = deltas[5];
        data += t5 + 1;
        (reinterpret_cast<int32_t*>(data))[0] = deltas[6];
        data += t6 + 1;
        (reinterpret_cast<int32_t*>(data))[0] = deltas[7];
        data += t7 + 1;
        return data;
    }


    static uint8_t *Code16(const uint32_t *deltas, uint8_t *code) {
        size_t maxv = 0;
        for (size_t i = 0; i < 16; ++i) {
            maxv += deltas[i];
        }
        if (maxv < 0xff) {
            code[0] = 0;
            ++code;
            uint8_t add = 0;
            for (size_t i = 0; i < 16; ++i) {
                add = static_cast<uint8_t>(add + deltas[i]);
                code[i] = add;
            }
            return code + 16;
        } else if (maxv < 0xffff) {
            code[0] = 1;
            ++code;
            uint8_t *data = code + 2;
            data = Code2(deltas + 0 * 8, code + 0, data);
            data = Code2(deltas + 1 * 8, code + 1, data);
            return data;
        } else {
            code[0] = 2;
            ++code;
            uint8_t *data = code + 4;
            data = Code4(deltas + 0 * 4, code + 0, data);
            data = Code4(deltas + 1 * 4, code + 1, data);
            data = Code4(deltas + 2 * 4, code + 2, data);
            data = Code4(deltas + 3 * 4, code + 3, data);
            return data;
        }
    }


    static __attribute__((always_inline))
    const inline uint8_t *Decode16(const uint8_t *src, /* volatile why do we need volatile? */ uint32_t *dst, __m128i &last) {
        uint8_t val = src[0];
        ++src;
        if (val == 0) {
            __m128i vali = _mm_loadu_si128(reinterpret_cast<const __m128i *>(src));
            //__m128i vali = Integrate1(val);
            __m128i shuf = _mm_shuffle_epi32(last, 0xff);
            __m128i v16_0 = _mm_unpacklo_epi8(vali, _mm_setzero_si128());
            __m128i v16_1 = _mm_unpackhi_epi8(vali, _mm_setzero_si128());
            __m128i v32_0 = _mm_unpacklo_epi16(v16_0, _mm_setzero_si128());
            __m128i v32_1 = _mm_unpackhi_epi16(v16_0, _mm_setzero_si128());
            __m128i v32_2 = _mm_unpacklo_epi16(v16_1, _mm_setzero_si128());
            __m128i v32_3 = _mm_unpackhi_epi16(v16_1, _mm_setzero_si128());
            __m128i out0 = _mm_add_epi32(shuf, v32_0);
            __m128i out1 = _mm_add_epi32(shuf, v32_1);
            __m128i out2 = _mm_add_epi32(shuf, v32_2);
            __m128i out3 = _mm_add_epi32(shuf, v32_3);
            last = out3;
            _mm_store_si128(reinterpret_cast<__m128i *>(dst) + 0, out0);
            _mm_store_si128(reinterpret_cast<__m128i *>(dst) + 1, out1);
            _mm_store_si128(reinterpret_cast<__m128i *>(dst) + 2, out2);
            _mm_store_si128(reinterpret_cast<__m128i *>(dst) + 3, out3);
            return src + 16;
        } else if (val == 1) {
            size_t ind0 = src[0];
            size_t ind1 = src[1];
            size_t siz0 = sizes2[ind0];
            size_t siz1 = sizes2[ind1];
            src += 2;
            __m128i v16_0 = Integrate2(_mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i *>(src)), shuffles2[ind0]));
            src += siz0;
            __m128i v16_1 = Integrate2(_mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i *>(src)), shuffles2[ind1]));
            v16_1 = _mm_add_epi16(v16_1, _mm_shuffle_epi8(v16_0, shuffle16));
            src += siz1;
            __m128i shuf = _mm_shuffle_epi32(last, 0xff);
            __m128i v32_0 = _mm_unpacklo_epi16(v16_0, _mm_setzero_si128());
            __m128i v32_1 = _mm_unpackhi_epi16(v16_0, _mm_setzero_si128());
            __m128i v32_2 = _mm_unpacklo_epi16(v16_1, _mm_setzero_si128());
            __m128i v32_3 = _mm_unpackhi_epi16(v16_1, _mm_setzero_si128());
            __m128i out0 = _mm_add_epi32(shuf, v32_0);
            __m128i out1 = _mm_add_epi32(shuf, v32_1);
            __m128i out2 = _mm_add_epi32(shuf, v32_2);
            __m128i out3 = _mm_add_epi32(shuf, v32_3);
            last = out3;
            _mm_store_si128(reinterpret_cast<__m128i *>(dst) + 0, out0);
            _mm_store_si128(reinterpret_cast<__m128i *>(dst) + 1, out1);
            _mm_store_si128(reinterpret_cast<__m128i *>(dst) + 2, out2);
            _mm_store_si128(reinterpret_cast<__m128i *>(dst) + 3, out3);
            return src;
        } else {
            size_t ind0 = src[0];
            size_t ind1 = src[1];
            size_t ind2 = src[2];
            size_t ind3 = src[3];
            size_t siz0 = sizes4[ind0];
            size_t siz1 = sizes4[ind1];
            size_t siz2 = sizes4[ind2];
            size_t siz3 = sizes4[ind3];
            src += 4;
            __m128i out0 = Integrate4(_mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i *>(src)), shuffles4[ind0]), last);
            src += siz0;
            __m128i out1 = Integrate4(_mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i *>(src)), shuffles4[ind1]), out0);
            src += siz1;
            __m128i out2 = Integrate4(_mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i *>(src)), shuffles4[ind2]), out1);
            src += siz2;
            __m128i out3 = Integrate4(_mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i *>(src)), shuffles4[ind3]), out2);
            src += siz3;
            last = out3;
            _mm_store_si128(reinterpret_cast<__m128i *>(dst) + 0, out0);
            _mm_store_si128(reinterpret_cast<__m128i *>(dst) + 1, out1);
            _mm_store_si128(reinterpret_cast<__m128i *>(dst) + 2, out2);
            _mm_store_si128(reinterpret_cast<__m128i *>(dst) + 3, out3);
            return src;
        }
    }

    const static size_t BlockSize = 16;

    static void inline pack(uint32_t * in, const size_t Qty, uint32_t * out,  const uint32_t bit ) {
        ipack(in, Qty, out, bit);
    }

    static void inline packwithoutmask(uint32_t * in, const size_t Qty, uint32_t * out,  const uint32_t bit ) {
        ipack(in, Qty, out, bit);
    }

    static void inline unpack(const uint32_t * in, size_t Qty, uint32_t * out,  const uint32_t bit ) {
        iunpack(in, Qty, out, bit);
    }
    static void inline ipatchedunpack(const uint32_t * in, const size_t Qty, uint32_t * out,  const uint32_t  bit ) {
    	// bogus since iron does not support patching
    	iunpack(in, Qty,  out,  bit);

    }

    static void inline iunpack(const uint32_t * in, const size_t Qty, uint32_t * out,  const uint32_t /* bit */) {
        if (Qty % BlockSize) {
            throw std::logic_error("Incorrect # of entries.");
        }

	  const uint8_t *ptr = reinterpret_cast<const uint8_t*>(in);
	  uint32_t *dst = reinterpret_cast<uint32_t*>(out);

	  __m128i last = _mm_setzero_si128();
	  for (size_t i = 0; i < Qty/16; ++i) {
            ptr = Decode16(ptr, dst, last);
            dst += 16;
	  }
    }

    static void inline ipackwithoutmask(uint32_t * in, const size_t Qty, uint32_t * out,  const uint32_t bit ) {
        return ipack(in, Qty, out, bit);
    }

    static void inline ipack(uint32_t * in, const size_t Qty, uint32_t * out,  const uint32_t /* bit */) {
        // Code16 works with deltas only
        delta(uint32_t(0), in, Qty);

        if (Qty % BlockSize) {
            throw std::logic_error("Incorrect # of entries.");
        }

        uint8_t *code = reinterpret_cast<uint8_t*>(out);
        for (size_t j = 0; j < Qty /16; j ++) {
            code = Code16(in + 16 * j, code);
        }
    }

    /*static void GenRandom(std::vector<uint32_t>& data, int b) {
	    data[0] = random(b);

	    for(size_t i = 1 ; i < data.size() ; ++i )
		    data[i] = random(b) + data[i-1];
    }*/

    static void CheckMaxDiff(const std::vector<uint32_t>& refdata, unsigned bit) {
        for(size_t i = 1; i < refdata.size(); ++i ) {
        	if(gccbits(refdata[i]-refdata[i-1])>bit) throw std::runtime_error("bug");

        }
    }
};

class IronPeterPacking: public IntegerCODEC {
public:
    static const uint32_t CookiePadder = 123456;// just some made up number
    static const uint32_t MiniBlockSize = 16;
    static const uint32_t HowManyMiniBlocks = 8;
    static const uint32_t BlockSize = HowManyMiniBlocks * MiniBlockSize;


    void encodeArray(uint32_t *in, const size_t length, uint32_t *out,
            size_t &nvalue) {
        checkifdivisibleby(length, BlockSize);
        const uint32_t * const initout(out);
        if(needPaddingTo128Bits(out) or needPaddingTo128Bits(in)) throw std::runtime_error("alignment issue: pointers should be aligned on 128-bit boundaries");
        *out++ = static_cast<uint32_t>(length);
        while(needPaddingTo128Bits(out)) *out++ = CookiePadder;

        // Code16 works with deltas only
        delta(uint32_t(0), in, length);

        uint8_t *code = reinterpret_cast<uint8_t*>(out);
        for (size_t j = 0; j < length /16; j ++) {
            code = IronPeterPackingHelpers::Code16(in + 16 * j, code);
        }


        // Let's pad to the 32-bit boundary
        code = padTo32bits(code);

        out = reinterpret_cast<uint32_t*>(code);

        nvalue = out - initout;
    }

    const uint32_t * decodeArray(const uint32_t *in, const size_t /* length */,
            uint32_t *out, size_t & nvalue) {
        if(needPaddingTo128Bits(out) or needPaddingTo128Bits(in)) throw std::runtime_error("alignment issue: pointers should be aligned on 128-bit boundaries");
    	const uint32_t actuallength = *in++;
        while(needPaddingTo128Bits(in)) {
            if(in[0] != CookiePadder) throw logic_error("IronPeter unpacking alignment issue.");
            ++in;
        }
        const uint32_t * const initout(out);

        const uint8_t *ptr = reinterpret_cast<const uint8_t*>(in);
        uint32_t *dst = reinterpret_cast<uint32_t*>(out);

        __m128i last = _mm_setzero_si128();
        for (size_t i = 0; i < actuallength/128; ++i) {
            ptr = IronPeterPackingHelpers::Decode16(ptr, dst, last);
            ptr = IronPeterPackingHelpers::Decode16(ptr, dst + 16, last);
            ptr = IronPeterPackingHelpers::Decode16(ptr, dst + 32, last);
            ptr = IronPeterPackingHelpers::Decode16(ptr, dst + 48, last);
            ptr = IronPeterPackingHelpers::Decode16(ptr, dst + 64, last);
            ptr = IronPeterPackingHelpers::Decode16(ptr, dst + 80, last);
            ptr = IronPeterPackingHelpers::Decode16(ptr, dst + 96, last);
            ptr = IronPeterPackingHelpers::Decode16(ptr, dst + 112, last);
            dst += 128;
        }
        out = reinterpret_cast<uint32_t*>(dst);

        nvalue = out - initout;

        // Let's pad to the 32-bit boundary
        ptr = padTo32bits(ptr);

        in = reinterpret_cast<const uint32_t*>(ptr);

        return in;

    }

    string name() const {
        return "IronPeter";
    }
};

#endif
