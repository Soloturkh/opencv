// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html


#include "precomp.hpp"

namespace cv {

typedef void (*PatchNanFunc)(uchar* tptr, size_t len, double newVal);
typedef void (*FiniteMaskFunc)(const uchar *src, uchar *dst, size_t total);

CV_CPU_OPTIMIZATION_NAMESPACE_BEGIN

PatchNanFunc getPatchNanFunc(bool isDouble);
FiniteMaskFunc getFiniteMaskFunc(bool isDouble, int cn);

#ifndef CV_CPU_OPTIMIZATION_DECLARATIONS_ONLY

static void patchNaNs_32f(uchar* ptr, size_t ulen, double newVal)
{
    CV_INSTRUMENT_REGION();

    int32_t* tptr = (int32_t*)ptr;
    Cv32suf val;
    val.f = (float)newVal;

    int j = 0;
    int len = (int)ulen;

#if (CV_SIMD || CV_SIMD_SCALABLE)
    v_int32 v_pos_mask = vx_setall_s32(0x7fffffff), v_exp_mask = vx_setall_s32(0x7f800000);
    v_int32 v_val = vx_setall_s32(val.i);

    int cWidth = VTraits<v_int32>::vlanes();
    for (; j < len - cWidth*2 + 1; j += cWidth*2)
    {
        v_int32 v_src0 = vx_load(tptr + j);
        v_int32 v_src1 = vx_load(tptr + j + cWidth);

        v_int32 v_cmp_mask0 = v_lt(v_exp_mask, v_and(v_src0, v_pos_mask));
        v_int32 v_cmp_mask1 = v_lt(v_exp_mask, v_and(v_src1, v_pos_mask));

        if (v_check_any(v_or(v_cmp_mask0, v_cmp_mask1)))
        {
            v_int32 v_dst0 = v_select(v_cmp_mask0, v_val, v_src0);
            v_int32 v_dst1 = v_select(v_cmp_mask1, v_val, v_src1);

            v_store(tptr + j, v_dst0);
            v_store(tptr + j + cWidth, v_dst1);
        }
    }
#endif

    for (; j < len; j++)
    {
        if ((tptr[j] & 0x7fffffff) > 0x7f800000)
        {
            tptr[j] = val.i;
        }
    }
}


static void patchNaNs_64f(uchar* ptr, size_t ulen, double newVal)
{
    CV_INSTRUMENT_REGION();

    int64_t* tptr = (int64_t*)ptr;
    Cv64suf val;
    val.f = newVal;

    int j = 0;
    int len = (int)ulen;

#if (CV_SIMD || CV_SIMD_SCALABLE)
    v_int64 v_exp_mask = vx_setall_s64(0x7FF0000000000000);
    v_int64 v_pos_mask = vx_setall_s64(0x7FFFFFFFFFFFFFFF);
    v_int64 v_val = vx_setall_s64(val.i);

    int cWidth = VTraits<v_int64>::vlanes();
    for (; j < len - cWidth * 2 + 1; j += cWidth*2)
    {
        v_int64 v_src0 = vx_load(tptr + j);
        v_int64 v_src1 = vx_load(tptr + j + cWidth);

        v_int64 v_cmp_mask0 = v_lt(v_exp_mask, v_and(v_src0, v_pos_mask));
        v_int64 v_cmp_mask1 = v_lt(v_exp_mask, v_and(v_src1, v_pos_mask));

        if (v_check_any(v_cmp_mask0) || v_check_any(v_cmp_mask1))
        {
            // v_select is not available for v_int64, emulating it
            v_int32 val32 = v_reinterpret_as_s32(v_val);
            v_int64 v_dst0 = v_reinterpret_as_s64(v_select(v_reinterpret_as_s32(v_cmp_mask0), val32, v_reinterpret_as_s32(v_src0)));
            v_int64 v_dst1 = v_reinterpret_as_s64(v_select(v_reinterpret_as_s32(v_cmp_mask1), val32, v_reinterpret_as_s32(v_src1)));

            v_store(tptr + j, v_dst0);
            v_store(tptr + j + cWidth, v_dst1);
        }
    }
#endif

    for (; j < len; j++)
        if ((tptr[j] & 0x7FFFFFFFFFFFFFFF) > 0x7FF0000000000000)
            tptr[j] = val.i;
}

PatchNanFunc getPatchNanFunc(bool isDouble)
{
    return isDouble ? (PatchNanFunc)GET_OPTIMIZED(patchNaNs_64f)
                    : (PatchNanFunc)GET_OPTIMIZED(patchNaNs_32f);
}

////// finiteMask //////

#if (CV_SIMD || CV_SIMD_SCALABLE)

template <typename _Tp, int cn>
int finiteMaskSIMD_(const _Tp *src, uchar *dst, size_t total);

template <>
int finiteMaskSIMD_<float, 1>(const float *fsrc, uchar *dst, size_t utotal)
{
    const uint32_t* src = (const uint32_t*)fsrc;
    const int osize = VTraits<v_uint8>::vlanes();
    v_uint32 vmaskExp = vx_setall_u32(0x7f800000);

    int i = 0;
    int total = (int)utotal;
    for(; i < total - osize + 1; i += osize )
    {
        v_uint32 vv0, vv1, vv2, vv3;
        vv0 = v_ne(v_and(vx_load(src + i              ), vmaskExp), vmaskExp);
        vv1 = v_ne(v_and(vx_load(src + i +   (osize/4)), vmaskExp), vmaskExp);
        vv2 = v_ne(v_and(vx_load(src + i + 2*(osize/4)), vmaskExp), vmaskExp);
        vv3 = v_ne(v_and(vx_load(src + i + 3*(osize/4)), vmaskExp), vmaskExp);

        v_store(dst + i, v_pack_b(vv0, vv1, vv2, vv3));
    }

    return i;
}


template <>
int finiteMaskSIMD_<float, 2>(const float *fsrc, uchar *dst, size_t utotal)
{
    const uint32_t* src = (const uint32_t*)fsrc;
    const int size8 = VTraits<v_uint8>::vlanes();
    v_uint32 vmaskExp = vx_setall_u32(0x7f800000);
    v_uint16 vmaskBoth = vx_setall_u16(0xffff);

    int i = 0;
    int total = (int)utotal;
    for(; i < total - (size8 / 2) + 1; i += (size8 / 2) )
    {
        v_uint32 vv0, vv1, vv2, vv3;
        vv0 = v_ne(v_and(vx_load(src + i*2                ), vmaskExp), vmaskExp);
        vv1 = v_ne(v_and(vx_load(src + i*2 +   (size8 / 4)), vmaskExp), vmaskExp);
        vv2 = v_ne(v_and(vx_load(src + i*2 + 2*(size8 / 4)), vmaskExp), vmaskExp);
        vv3 = v_ne(v_and(vx_load(src + i*2 + 3*(size8 / 4)), vmaskExp), vmaskExp);
        v_uint8 velems = v_pack_b(vv0, vv1, vv2, vv3);
        v_uint16 vfinite = v_eq(v_reinterpret_as_u16(velems), vmaskBoth);

        // 2nd argument in vfinite is useless
        v_store_low(dst + i, v_pack(vfinite, vfinite));
    }

    return i;
}


template <>
int finiteMaskSIMD_<float, 3>(const float *fsrc, uchar *dst, size_t utotal)
{
    const uint32_t* src = (const uint32_t*)fsrc;
    const int npixels = VTraits<v_float32>::vlanes();
    v_uint32 vmaskExp = vx_setall_u32(0x7f800000);
    v_uint32 z = vx_setzero_u32();

    int i = 0;
    int total = (int)utotal;
    for (; i < total - npixels + 1; i += npixels)
    {
        v_uint32 vv0, vv1, vv2;
        vv0 = v_ne(v_and(vx_load(src + i*3            ), vmaskExp), vmaskExp);
        vv1 = v_ne(v_and(vx_load(src + i*3 +   npixels), vmaskExp), vmaskExp);
        vv2 = v_ne(v_and(vx_load(src + i*3 + 2*npixels), vmaskExp), vmaskExp);

        v_uint8 velems = v_pack_b(vv0, vv1, vv2, z);

        // 2nd arg is useless
        v_uint8 vsh1 = v_extract<1>(velems, velems);
        v_uint8 vsh2 = v_extract<2>(velems, velems);

        v_uint8 vres3 = v_and(v_and(velems, vsh1), vsh2);
        for (int j = 0; j < npixels; j++)
        {
            dst[i + j] = v_get0(vres3);
            // 2nd arg is useless
            vres3 = v_extract<3>(vres3, vres3);
        }
    }

    return i;
}


template <>
int finiteMaskSIMD_<float, 4>(const float *fsrc, uchar *dst, size_t utotal)
{
    const uint32_t* src = (const uint32_t*)fsrc;
    CV_StaticAssert(VTraits<v_uint8>::max_nlanes*8 <= 1024, "SIMD registers should be not more than 1024 bit wide");
    const int npixels = VTraits<v_uint32>::vlanes()/4;

    v_uint32 vmaskExp = vx_setall_u32(0x7f800000);
    v_uint32 z = vx_setzero_u32();
    v_uint32 vmaskAll4 = vx_setall_u32(0xFFFFFFFF);

    int i = 0;
    int total = (int)utotal;
    for(; i < total - npixels + 1; i += npixels )
    {
        v_uint32 vv = v_ne(v_and(vx_load(src + i*4), vmaskExp), vmaskExp);
        v_uint8 velems = v_pack_b(vv, z, z, z);

        v_uint32 vresWide = v_eq(v_reinterpret_as_u32(velems), vmaskAll4);

        v_uint8 vres = v_pack_b(vresWide, z, z, z);
        if (npixels == 1) // 128 bit wide
        {
            dst[i] = v_get0(vres);
        }
        else if (npixels == 2) // 256 bit wide
        {
            *((uint16_t*)(dst + i)) = v_get0(v_reinterpret_as_u16(vres));
        }
        else if (npixels == 4) // 512 bit wide
        {
            *((uint32_t*)(dst + i)) = v_get0(v_reinterpret_as_u32(vres));
        }
        else if (npixels == 8) // 1024 bit wide
        {
            *((uint64_t*)(dst + i)) = v_get0(v_reinterpret_as_u64(vres));
        }
    }

    return i;
}


template <>
int finiteMaskSIMD_<double, 1>(const double *dsrc, uchar *dst, size_t utotal)
{
    const uint64_t* src = (const uint64_t*)dsrc;
    const int size8 = VTraits<v_uint8>::vlanes();
    int i = 0;
    int total = (int)utotal;

    v_uint64 vmaskExp = vx_setall_u64(0x7ff0000000000000);
    v_uint64 z = vx_setzero_u64();

    for(; i < total - (size8 / 2) + 1; i += (size8 / 2) )
    {
        v_uint64 vv0, vv1, vv2, vv3;
        vv0 = v_ne(v_and(vx_load(src + i                ), vmaskExp), vmaskExp);
        vv1 = v_ne(v_and(vx_load(src + i +   (size8 / 8)), vmaskExp), vmaskExp);
        vv2 = v_ne(v_and(vx_load(src + i + 2*(size8 / 8)), vmaskExp), vmaskExp);
        vv3 = v_ne(v_and(vx_load(src + i + 3*(size8 / 8)), vmaskExp), vmaskExp);

        v_uint8 v = v_pack_b(vv0, vv1, vv2, vv3, z, z, z, z);

        v_store_low(dst + i, v);
    }

    return i;
}

template <>
int finiteMaskSIMD_<double, 2>(const double *dsrc, uchar *dst, size_t utotal)
{
    const uint64_t* src = (const uint64_t*)dsrc;
    const int npixels = VTraits<v_uint8>::vlanes() / 2;
    const int ndoubles = VTraits<v_uint64>::vlanes();
    v_uint64 vmaskExp = vx_setall_u64(0x7ff0000000000000);
    v_uint16 vmaskBoth = vx_setall_u16(0xffff);

    int i = 0;
    int total = (int)utotal;
    for(; i < total - npixels + 1; i += npixels )
    {
        v_uint64 vv0 = v_ne(v_and(vx_load(src + i*2 + 0*ndoubles), vmaskExp), vmaskExp);
        v_uint64 vv1 = v_ne(v_and(vx_load(src + i*2 + 1*ndoubles), vmaskExp), vmaskExp);
        v_uint64 vv2 = v_ne(v_and(vx_load(src + i*2 + 2*ndoubles), vmaskExp), vmaskExp);
        v_uint64 vv3 = v_ne(v_and(vx_load(src + i*2 + 3*ndoubles), vmaskExp), vmaskExp);
        v_uint64 vv4 = v_ne(v_and(vx_load(src + i*2 + 4*ndoubles), vmaskExp), vmaskExp);
        v_uint64 vv5 = v_ne(v_and(vx_load(src + i*2 + 5*ndoubles), vmaskExp), vmaskExp);
        v_uint64 vv6 = v_ne(v_and(vx_load(src + i*2 + 6*ndoubles), vmaskExp), vmaskExp);
        v_uint64 vv7 = v_ne(v_and(vx_load(src + i*2 + 7*ndoubles), vmaskExp), vmaskExp);

        v_uint8 velems0 = v_pack_b(vv0, vv1, vv2, vv3, vv4, vv5, vv6, vv7);

        v_uint16 vfinite0 = v_eq(v_reinterpret_as_u16(velems0), vmaskBoth);

        // 2nd arg is useless
        v_uint8 vres = v_pack(vfinite0, vfinite0);
        v_store_low(dst + i, vres);
    }

    return i;
}


template <>
int finiteMaskSIMD_<double, 3>(const double *fsrc, uchar *dst, size_t utotal)
{
    const uint64_t* src = (const uint64_t*)fsrc;
    const int npixels = VTraits<v_uint64>::vlanes();
    v_uint64 vmaskExp = vx_setall_u64(0x7ff0000000000000);
    v_uint64 z = vx_setzero_u64();

    int i = 0;
    int total = (int)utotal;
    for (; i < total - npixels + 1; i += npixels)
    {
        v_uint64 vv0, vv1, vv2;
        vv0 = v_ne(v_and(vx_load(src + i*3            ), vmaskExp), vmaskExp);
        vv1 = v_ne(v_and(vx_load(src + i*3 +   npixels), vmaskExp), vmaskExp);
        vv2 = v_ne(v_and(vx_load(src + i*3 + 2*npixels), vmaskExp), vmaskExp);

        v_uint8 velems = v_pack_b(vv0, vv1, vv2, z, z, z, z, z);

        // 2nd arg is useless
        v_uint8 vsh1 = v_extract<1>(velems, velems);
        v_uint8 vsh2 = v_extract<2>(velems, velems);

        v_uint8 vres3 = v_and(v_and(velems, vsh1), vsh2);
        for (int j = 0; j < npixels; j++)
        {
            dst[i + j] = v_get0(vres3);
            // 2nd arg is useless
            vres3 = v_extract<3>(vres3, vres3);
        }
    }

    return i;
}

template <>
int finiteMaskSIMD_<double, 4>(const double *dsrc, uchar *dst, size_t utotal)
{
    //TODO: vectorize it properly

    const uint64_t* src = (const uint64_t*)dsrc;
    const int npixels = VTraits<v_uint8>::vlanes();
    uint64_t maskExp = 0x7ff0000000000000;

    int i = 0;
    int total = (int)utotal;
    for(; i < total - npixels + 1; i += npixels )
    {
        for (int j = 0; j < npixels; j++)
        {
            uint64_t val0 = src[i * 4 + j * 4 + 0];
            uint64_t val1 = src[i * 4 + j * 4 + 1];
            uint64_t val2 = src[i * 4 + j * 4 + 2];
            uint64_t val3 = src[i * 4 + j * 4 + 3];

            bool finite = ((val0 & maskExp) != maskExp) &&
                          ((val1 & maskExp) != maskExp) &&
                          ((val2 & maskExp) != maskExp) &&
                          ((val3 & maskExp) != maskExp);

            dst[i + j] = finite ? 255 : 0;
        }
    }

    return i;
}

#endif


template <typename _Tp, int cn>
void finiteMask_(const uchar *src, uchar *dst, size_t total)
{
    CV_INSTRUMENT_REGION();
    size_t i = 0;
    const _Tp* tsrc = (const _Tp*) src;

#if (CV_SIMD || CV_SIMD_SCALABLE)
    i = finiteMaskSIMD_<_Tp, cn>(tsrc, dst, total);
#endif

    for(; i < total; i++ )
    {
        bool finite = true;
        for (int c = 0; c < cn; c++)
        {
            _Tp val = tsrc[i * cn + c];
            finite = finite && !cvIsNaN(val) && !cvIsInf(val);
        }
        dst[i] = finite ? 255 : 0;
    }
}

FiniteMaskFunc getFiniteMaskFunc(bool isDouble, int cn)
{
    static FiniteMaskFunc tab[] =
    {
        (FiniteMaskFunc)GET_OPTIMIZED((finiteMask_<float,  1>)),
        (FiniteMaskFunc)GET_OPTIMIZED((finiteMask_<float,  2>)),
        (FiniteMaskFunc)GET_OPTIMIZED((finiteMask_<float,  3>)),
        (FiniteMaskFunc)GET_OPTIMIZED((finiteMask_<float,  4>)),
        (FiniteMaskFunc)GET_OPTIMIZED((finiteMask_<double, 1>)),
        (FiniteMaskFunc)GET_OPTIMIZED((finiteMask_<double, 2>)),
        (FiniteMaskFunc)GET_OPTIMIZED((finiteMask_<double, 3>)),
        (FiniteMaskFunc)GET_OPTIMIZED((finiteMask_<double, 4>)),
    };

    int idx = (isDouble ? 4 : 0) + cn - 1;
    return tab[idx];
}

#endif
CV_CPU_OPTIMIZATION_NAMESPACE_END
} // namespace cv
