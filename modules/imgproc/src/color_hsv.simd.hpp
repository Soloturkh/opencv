// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html

#include "precomp.hpp"
#include "opencv2/core/hal/intrin.hpp"

namespace cv {
namespace hal {
CV_CPU_OPTIMIZATION_NAMESPACE_BEGIN
// forward declarations

void cvtBGRtoHSV(const uchar * src_data, size_t src_step,
                 uchar * dst_data, size_t dst_step,
                 int width, int height,
                 int depth, int scn, bool swapBlue, bool isFullRange, bool isHSV);
void cvtHSVtoBGR(const uchar * src_data, size_t src_step,
                 uchar * dst_data, size_t dst_step,
                 int width, int height,
                 int depth, int dcn, bool swapBlue, bool isFullRange, bool isHSV);

#ifndef CV_CPU_OPTIMIZATION_DECLARATIONS_ONLY

#if defined(CV_CPU_BASELINE_MODE)
// included in color.hpp
#else
#include "color.simd_helpers.hpp"
#endif

namespace {
////////////////////////////////////// RGB <-> HSV ///////////////////////////////////////


struct RGB2HSV_b
{
    typedef uchar channel_type;

    RGB2HSV_b(int _srccn, int _blueIdx, int _hrange)
    : srccn(_srccn), blueIdx(_blueIdx), hrange(_hrange)
    {
        CV_Assert( hrange == 180 || hrange == 256 );
    }

    void operator()(const uchar* src, uchar* dst, int n) const
    {
        CV_INSTRUMENT_REGION();

        int i, bidx = blueIdx, scn = srccn;
        const int hsv_shift = 12;

        static int sdiv_table[256];
        static int hdiv_table180[256];
        static int hdiv_table256[256];
        static volatile bool initialized = false;

        int hr = hrange;
        const int* hdiv_table = hr == 180 ? hdiv_table180 : hdiv_table256;

        if( !initialized )
        {
            sdiv_table[0] = hdiv_table180[0] = hdiv_table256[0] = 0;
            for( i = 1; i < 256; i++ )
            {
                sdiv_table[i] = saturate_cast<int>((255 << hsv_shift)/(1.*i));
                hdiv_table180[i] = saturate_cast<int>((180 << hsv_shift)/(6.*i));
                hdiv_table256[i] = saturate_cast<int>((256 << hsv_shift)/(6.*i));
            }
            initialized = true;
        }

        i = 0;

#if CV_SIMD
        const int vsize = v_uint8::nlanes;
        for ( ; i <= n - vsize;
              i += vsize, src += scn*vsize, dst += 3*vsize)
        {
            v_uint8 b, g, r;
            if(scn == 4)
            {
                v_uint8 a;
                v_load_deinterleave(src, b, g, r, a);
            }
            else
            {
                v_load_deinterleave(src, b, g, r);
            }

            if(bidx)
                swap(b, r);

            v_uint8 h, s, v;
            v_uint8 vmin;
            v = v_max(b, v_max(g, r));
            vmin = v_min(b, v_min(g, r));

            v_uint8 diff, vr, vg;
            diff = v - vmin;
            v_uint8 v255 = vx_setall_u8(0xff), vz = vx_setzero_u8();
            vr = v_select(v == r, v255, vz);
            vg = v_select(v == g, v255, vz);

            // sdiv = sdiv_table[v]
            v_int32 sdiv[4];
            v_uint16 vd[2];
            v_expand(v, vd[0], vd[1]);
            v_int32 vq[4];
            v_expand(v_reinterpret_as_s16(vd[0]), vq[0], vq[1]);
            v_expand(v_reinterpret_as_s16(vd[1]), vq[2], vq[3]);
            {
                int32_t CV_DECL_ALIGNED(CV_SIMD_WIDTH) storevq[vsize];
                for (int k = 0; k < 4; k++)
                {
                    v_store_aligned(storevq + k*vsize/4, vq[k]);
                }

                for(int k = 0; k < 4; k++)
                {
                    sdiv[k] = vx_lut(sdiv_table, storevq + k*vsize/4);
                }
            }

            // hdiv = hdiv_table[diff]
            v_int32 hdiv[4];
            v_uint16 diffd[2];
            v_expand(diff, diffd[0], diffd[1]);
            v_int32 diffq[4];
            v_expand(v_reinterpret_as_s16(diffd[0]), diffq[0], diffq[1]);
            v_expand(v_reinterpret_as_s16(diffd[1]), diffq[2], diffq[3]);
            {
                int32_t CV_DECL_ALIGNED(CV_SIMD_WIDTH) storediffq[vsize];
                for (int k = 0; k < 4; k++)
                {
                    v_store_aligned(storediffq + k*vsize/4, diffq[k]);
                }

                for (int k = 0; k < 4; k++)
                {
                    hdiv[k] = vx_lut((int32_t*)hdiv_table, storediffq + k*vsize/4);
                }
            }

            // s = (diff * sdiv + (1 << (hsv_shift-1))) >> hsv_shift;
            v_int32 sq[4];
            v_int32 vdescale = vx_setall_s32(1 << (hsv_shift-1));
            for (int k = 0; k < 4; k++)
            {
                sq[k] = (diffq[k]*sdiv[k] + vdescale) >> hsv_shift;
            }
            v_int16 sd[2];
            sd[0] = v_pack(sq[0], sq[1]);
            sd[1] = v_pack(sq[2], sq[3]);
            s = v_pack_u(sd[0], sd[1]);

            // expand all to 16 bits
            v_uint16 bdu[2], gdu[2], rdu[2];
            v_expand(b, bdu[0], bdu[1]);
            v_expand(g, gdu[0], gdu[1]);
            v_expand(r, rdu[0], rdu[1]);
            v_int16 bd[2], gd[2], rd[2];
            bd[0] = v_reinterpret_as_s16(bdu[0]);
            bd[1] = v_reinterpret_as_s16(bdu[1]);
            gd[0] = v_reinterpret_as_s16(gdu[0]);
            gd[1] = v_reinterpret_as_s16(gdu[1]);
            rd[0] = v_reinterpret_as_s16(rdu[0]);
            rd[1] = v_reinterpret_as_s16(rdu[1]);

            v_int16 vrd[2], vgd[2];
            v_expand(v_reinterpret_as_s8(vr), vrd[0], vrd[1]);
            v_expand(v_reinterpret_as_s8(vg), vgd[0], vgd[1]);
            v_int16 diffsd[2];
            diffsd[0] = v_reinterpret_as_s16(diffd[0]);
            diffsd[1] = v_reinterpret_as_s16(diffd[1]);

            v_int16 hd[2];
            // h before division
            for (int k = 0; k < 2; k++)
            {
                v_int16 gb = gd[k] - bd[k];
                v_int16 br = bd[k] - rd[k] + (diffsd[k] << 1);
                v_int16 rg = rd[k] - gd[k] + (diffsd[k] << 2);
                hd[k] = (vrd[k] & gb) + ((~vrd[k]) & ((vgd[k] & br) + ((~vgd[k]) & rg)));
            }

            // h div and fix
            v_int32 hq[4];
            v_expand(hd[0], hq[0], hq[1]);
            v_expand(hd[1], hq[2], hq[3]);
            for(int k = 0; k < 4; k++)
            {
                hq[k] = (hq[k]*hdiv[k] + vdescale) >> hsv_shift;
            }
            hd[0] = v_pack(hq[0], hq[1]);
            hd[1] = v_pack(hq[2], hq[3]);
            v_int16 vhr = vx_setall_s16(hr);
            v_int16 vzd = vx_setzero_s16();
            hd[0] += v_select(hd[0] < vzd, vhr, vzd);
            hd[1] += v_select(hd[1] < vzd, vhr, vzd);
            h = v_pack_u(hd[0], hd[1]);

            v_store_interleave(dst, h, s, v);
        }
#endif

        for( ; i < n; i++, src += scn, dst += 3 )
        {
            int b = src[bidx], g = src[1], r = src[bidx^2];
            int h, s, v = b;
            int vmin = b;
            int vr, vg;

            CV_CALC_MAX_8U( v, g );
            CV_CALC_MAX_8U( v, r );
            CV_CALC_MIN_8U( vmin, g );
            CV_CALC_MIN_8U( vmin, r );

            uchar diff = saturate_cast<uchar>(v - vmin);
            vr = v == r ? -1 : 0;
            vg = v == g ? -1 : 0;

            s = (diff * sdiv_table[v] + (1 << (hsv_shift-1))) >> hsv_shift;
            h = (vr & (g - b)) +
                (~vr & ((vg & (b - r + 2 * diff)) + ((~vg) & (r - g + 4 * diff))));
            h = (h * hdiv_table[diff] + (1 << (hsv_shift-1))) >> hsv_shift;
            h += h < 0 ? hr : 0;

            dst[0] = saturate_cast<uchar>(h);
            dst[1] = (uchar)s;
            dst[2] = (uchar)v;
        }
    }

    int srccn, blueIdx, hrange;
};


struct RGB2HSV_f
{
    typedef float channel_type;

    RGB2HSV_f(int _srccn, int _blueIdx, float _hrange)
    : srccn(_srccn), blueIdx(_blueIdx), hrange(_hrange)
    { }

    #if CV_SIMD
    inline void process(const v_float32& v_r, const v_float32& v_g, const v_float32& v_b,
                        v_float32& v_h, v_float32& v_s, v_float32& v_v,
                        float hscale) const
    {
        v_float32 v_min_rgb = v_min(v_min(v_r, v_g), v_b);
        v_float32 v_max_rgb = v_max(v_max(v_r, v_g), v_b);

        v_float32 v_eps = vx_setall_f32(FLT_EPSILON);
        v_float32 v_diff = v_max_rgb - v_min_rgb;
        v_s = v_diff / (v_abs(v_max_rgb) + v_eps);

        v_float32 v_r_eq_max = v_r == v_max_rgb;
        v_float32 v_g_eq_max = v_g == v_max_rgb;
        v_h = v_select(v_r_eq_max, v_g - v_b,
              v_select(v_g_eq_max, v_b - v_r, v_r - v_g));
        v_float32 v_res = v_select(v_r_eq_max, (v_g < v_b) & vx_setall_f32(360.0f),
                          v_select(v_g_eq_max, vx_setall_f32(120.0f), vx_setall_f32(240.0f)));
        v_float32 v_rev_diff = vx_setall_f32(60.0f) / (v_diff + v_eps);
        v_h = v_muladd(v_h, v_rev_diff, v_res) * vx_setall_f32(hscale);

        v_v = v_max_rgb;
    }
    #endif

    void operator()(const float* src, float* dst, int n) const
    {
        CV_INSTRUMENT_REGION();

        int i = 0, bidx = blueIdx, scn = srccn;
        float hscale = hrange*(1.f/360.f);
        n *= 3;

#if CV_SIMD
        const int vsize = v_float32::nlanes;
        for ( ; i <= n - 3*vsize; i += 3*vsize, src += scn * vsize)
        {
            v_float32 r, g, b, a;
            if(scn == 4)
            {
                v_load_deinterleave(src, r, g, b, a);
            }
            else // scn == 3
            {
                v_load_deinterleave(src, r, g, b);
            }

            if(bidx)
                swap(b, r);

            v_float32 h, s, v;
            process(b, g, r, h, s, v, hscale);

            v_store_interleave(dst + i, h, s, v);
        }
#endif

        for( ; i < n; i += 3, src += scn )
        {
            float b = src[bidx], g = src[1], r = src[bidx^2];
            float h, s, v;

            float vmin, diff;

            v = vmin = r;
            if( v < g ) v = g;
            if( v < b ) v = b;
            if( vmin > g ) vmin = g;
            if( vmin > b ) vmin = b;

            diff = v - vmin;
            s = diff/(float)(fabs(v) + FLT_EPSILON);
            diff = (float)(60./(diff + FLT_EPSILON));
            if( v == r )
                h = (g - b)*diff;
            else if( v == g )
                h = (b - r)*diff + 120.f;
            else
                h = (r - g)*diff + 240.f;

            if( h < 0 ) h += 360.f;

            dst[i] = h*hscale;
            dst[i+1] = s;
            dst[i+2] = v;
        }
    }

    int srccn, blueIdx;
    float hrange;
};


#if CV_SIMD
inline void HSV2RGB_simd(const v_float32& h, const v_float32& s, const v_float32& v,
                         v_float32& b, v_float32& g, v_float32& r, float hscale)
{
    v_float32 v_h = h;
    v_float32 v_s = s;
    v_float32 v_v = v;

    v_h = v_h * vx_setall_f32(hscale);

    v_float32 v_pre_sector = v_cvt_f32(v_trunc(v_h));
    v_h = v_h - v_pre_sector;
    v_float32 v_tab0 = v_v;
    v_float32 v_one = vx_setall_f32(1.0f);
    v_float32 v_tab1 = v_v * (v_one - v_s);
    v_float32 v_tab2 = v_v * (v_one - (v_s * v_h));
    v_float32 v_tab3 = v_v * (v_one - (v_s * (v_one - v_h)));

    v_float32 v_one_sixth = vx_setall_f32(1.0f / 6.0f);
    v_float32 v_sector = v_pre_sector * v_one_sixth;
    v_sector = v_cvt_f32(v_trunc(v_sector));
    v_float32 v_six = vx_setall_f32(6.0f);
    v_sector = v_pre_sector - (v_sector * v_six);

    v_float32 v_two = vx_setall_f32(2.0f);
    v_h = v_tab1 & (v_sector < v_two);
    v_h = v_h | (v_tab3 & (v_sector == v_two));
    v_float32 v_three = vx_setall_f32(3.0f);
    v_h = v_h | (v_tab0 & (v_sector == v_three));
    v_float32 v_four = vx_setall_f32(4.0f);
    v_h = v_h | (v_tab0 & (v_sector == v_four));
    v_h = v_h | (v_tab2 & (v_sector > v_four));

    v_s = v_tab3 & (v_sector < v_one);
    v_s = v_s | (v_tab0 & (v_sector == v_one));
    v_s = v_s | (v_tab0 & (v_sector == v_two));
    v_s = v_s | (v_tab2 & (v_sector == v_three));
    v_s = v_s | (v_tab1 & (v_sector > v_three));

    v_v = v_tab0 & (v_sector < v_one);
    v_v = v_v | (v_tab2 & (v_sector == v_one));
    v_v = v_v | (v_tab1 & (v_sector == v_two));
    v_v = v_v | (v_tab1 & (v_sector == v_three));
    v_v = v_v | (v_tab3 & (v_sector == v_four));
    v_v = v_v | (v_tab0 & (v_sector > v_four));

    b = v_h;
    g = v_s;
    r = v_v;
}
#endif


inline void HSV2RGB_native(float h, float s, float v,
                           float& b, float& g, float& r,
                           const float hscale)
{
    if( s == 0 )
        b = g = r = v;
    else
    {
        static const int sector_data[][3]=
            {{1,3,0}, {1,0,2}, {3,0,1}, {0,2,1}, {0,1,3}, {2,1,0}};
        float tab[4];
        int sector;
        h *= hscale;
        h = fmod(h, 6.f);
        sector = cvFloor(h);
        h -= sector;
        if( (unsigned)sector >= 6u )
        {
            sector = 0;
            h = 0.f;
        }

        tab[0] = v;
        tab[1] = v*(1.f - s);
        tab[2] = v*(1.f - s*h);
        tab[3] = v*(1.f - s*(1.f - h));

        b = tab[sector_data[sector][0]];
        g = tab[sector_data[sector][1]];
        r = tab[sector_data[sector][2]];
    }
}


struct HSV2RGB_f
{
    typedef float channel_type;

    HSV2RGB_f(int _dstcn, int _blueIdx, float _hrange)
    : dstcn(_dstcn), blueIdx(_blueIdx), hscale(6.f/_hrange)
    { }

    void operator()(const float* src, float* dst, int n) const
    {
        CV_INSTRUMENT_REGION();

        int i = 0, bidx = blueIdx, dcn = dstcn;
        float alpha = ColorChannel<float>::max();
        float hs = hscale;
        n *= 3;

#if CV_SIMD
        const int vsize = v_float32::nlanes;
        v_float32 valpha = vx_setall_f32(alpha);
        for (; i <= n - vsize*3; i += vsize*3, dst += dcn * vsize)
        {
            v_float32 h, s, v, b, g, r;
            v_load_deinterleave(src + i, h, s, v);

            HSV2RGB_simd(h, s, v, b, g, r, hs);

            if(bidx)
                swap(b, r);

            if(dcn == 4)
            {
                v_store_interleave(dst, b, g, r, valpha);
            }
            else // dcn == 3
            {
                v_store_interleave(dst, b, g, r);
            }
        }
#endif
        for( ; i < n; i += 3, dst += dcn )
        {
            float h = src[i + 0], s = src[i + 1], v = src[i + 2];
            float b, g, r;
            HSV2RGB_native(h, s, v, b, g, r, hs);

            dst[bidx] = b;
            dst[1] = g;
            dst[bidx^2] = r;
            if(dcn == 4)
                dst[3] = alpha;
        }
    }

    int dstcn, blueIdx;
    float hscale;
};


struct HSV2RGB_b
{
    typedef uchar channel_type;

    HSV2RGB_b(int _dstcn, int _blueIdx, int _hrange)
    : dstcn(_dstcn), blueIdx(_blueIdx), hscale(6.0f / _hrange)
    { }

    void operator()(const uchar* src, uchar* dst, int n) const
    {
        CV_INSTRUMENT_REGION();

        int j = 0, dcn = dstcn;
        uchar alpha = ColorChannel<uchar>::max();

#if CV_SIMD
        const int vsize = v_float32::nlanes;

        for (j = 0; j <= (n - vsize*4) * 3; j += 3 * 4 * vsize, dst += dcn * 4 * vsize)
        {
            v_uint8 h_b, s_b, v_b;
            v_uint16 h_w[2], s_w[2], v_w[2];
            v_uint32 h_u[4], s_u[4], v_u[4];
            v_load_deinterleave(src + j, h_b, s_b, v_b);
            v_expand(h_b, h_w[0], h_w[1]);
            v_expand(s_b, s_w[0], s_w[1]);
            v_expand(v_b, v_w[0], v_w[1]);
            v_expand(h_w[0], h_u[0], h_u[1]);
            v_expand(h_w[1], h_u[2], h_u[3]);
            v_expand(s_w[0], s_u[0], s_u[1]);
            v_expand(s_w[1], s_u[2], s_u[3]);
            v_expand(v_w[0], v_u[0], v_u[1]);
            v_expand(v_w[1], v_u[2], v_u[3]);

            v_int32 b_i[4], g_i[4], r_i[4];
            v_float32 v_coeff0 = vx_setall_f32(1.0f / 255.0f);
            v_float32 v_coeff1 = vx_setall_f32(255.0f);

            for( int k = 0; k < 4; k++ )
            {
                v_float32 h = v_cvt_f32(v_reinterpret_as_s32(h_u[k]));
                v_float32 s = v_cvt_f32(v_reinterpret_as_s32(s_u[k]));
                v_float32 v = v_cvt_f32(v_reinterpret_as_s32(v_u[k]));

                s *= v_coeff0;
                v *= v_coeff0;
                v_float32 b, g, r;
                HSV2RGB_simd(h, s, v, b, g, r, hscale);

                b *= v_coeff1;
                g *= v_coeff1;
                r *= v_coeff1;
                b_i[k] = v_trunc(b);
                g_i[k] = v_trunc(g);
                r_i[k] = v_trunc(r);
            }

            v_uint16 r_w[2], g_w[2], b_w[2];
            v_uint8 r_b, g_b, b_b;

            r_w[0] = v_pack_u(r_i[0], r_i[1]);
            r_w[1] = v_pack_u(r_i[2], r_i[3]);
            r_b = v_pack(r_w[0], r_w[1]);
            g_w[0] = v_pack_u(g_i[0], g_i[1]);
            g_w[1] = v_pack_u(g_i[2], g_i[3]);
            g_b = v_pack(g_w[0], g_w[1]);
            b_w[0] = v_pack_u(b_i[0], b_i[1]);
            b_w[1] = v_pack_u(b_i[2], b_i[3]);
            b_b = v_pack(b_w[0], b_w[1]);

            if( dcn == 3 )
            {
                if( blueIdx == 0 )
                    v_store_interleave(dst, b_b, g_b, r_b);
                else
                    v_store_interleave(dst, r_b, g_b, b_b);
            }
            else
            {
                v_uint8 alpha_b = vx_setall_u8(alpha);
                if( blueIdx == 0 )
                    v_store_interleave(dst, b_b, g_b, r_b, alpha_b);
                else
                    v_store_interleave(dst, r_b, g_b, b_b, alpha_b);
            }
        }
#endif

        for( ; j < n * 3; j += 3, dst += dcn )
        {
            float h, s, v, b, g, r;
            h = src[j];
            s = src[j+1] * (1.0f / 255.0f);
            v = src[j+2] * (1.0f / 255.0f);
            HSV2RGB_native(h, s, v, b, g, r, hscale);

            dst[blueIdx]   = saturate_cast<uchar>(b * 255.0f);
            dst[1]         = saturate_cast<uchar>(g * 255.0f);
            dst[blueIdx^2] = saturate_cast<uchar>(r * 255.0f);

            if( dcn == 4 )
                dst[3] = alpha;
        }
    }

    int dstcn;
    int blueIdx;
    float hscale;
};


///////////////////////////////////// RGB <-> HLS ////////////////////////////////////////

struct RGB2HLS_f
{
    typedef float channel_type;

    RGB2HLS_f(int _srccn, int _blueIdx, float _hrange)
    : srccn(_srccn), blueIdx(_blueIdx), hscale(_hrange/360.f) {
        #if CV_SIMD128
        hasSIMD = hasSIMD128();
        #endif
    }

    #if CV_SIMD128
    inline void process(v_float32x4& v_r, v_float32x4& v_g,
                        v_float32x4& v_b, v_float32x4& v_hscale) const
    {
        v_float32x4 v_max_rgb = v_max(v_max(v_r, v_g), v_b);
        v_float32x4 v_min_rgb = v_min(v_min(v_r, v_g), v_b);

        v_float32x4 v_diff = v_max_rgb - v_min_rgb;
        v_float32x4 v_sum = v_max_rgb + v_min_rgb;
        v_float32x4 v_half = v_setall_f32(0.5f);
        v_float32x4 v_l = v_sum * v_half;

        v_float32x4 v_s = v_diff / v_select(v_l < v_half, v_sum, v_setall_f32(2.0f) - v_sum);

        v_float32x4 v_r_eq_max = v_max_rgb == v_r;
        v_float32x4 v_g_eq_max = v_max_rgb == v_g;
        v_float32x4 v_h = v_select(v_r_eq_max, v_g - v_b,
                          v_select(v_g_eq_max, v_b - v_r, v_r - v_g));
        v_float32x4 v_res = v_select(v_r_eq_max, (v_g < v_b) & v_setall_f32(360.0f),
                            v_select(v_g_eq_max, v_setall_f32(120.0f), v_setall_f32(240.0f)));
        v_float32x4 v_rev_diff = v_setall_f32(60.0f) / v_diff;
        v_h = v_muladd(v_h, v_rev_diff, v_res) * v_hscale;

        v_float32x4 v_diff_gt_eps = v_diff > v_setall_f32(FLT_EPSILON);
        v_r = v_diff_gt_eps & v_h;
        v_g = v_l;
        v_b = v_diff_gt_eps & v_s;
    }
    #endif

    void operator()(const float* src, float* dst, int n) const
    {
        int i = 0, bidx = blueIdx, scn = srccn;
        n *= 3;

        #if CV_SIMD128
        if (hasSIMD)
        {
            v_float32x4 v_hscale = v_setall_f32(hscale);
            if (scn == 3) {
                if (bidx) {
                    for ( ; i <= n - 12; i += 12, src += scn * 4)
                    {
                        v_float32x4 v_r;
                        v_float32x4 v_g;
                        v_float32x4 v_b;
                        v_load_deinterleave(src, v_r, v_g, v_b);
                        process(v_r, v_g, v_b, v_hscale);
                        v_store_interleave(dst + i, v_r, v_g, v_b);
                    }
                } else {
                    for ( ; i <= n - 12; i += 12, src += scn * 4)
                    {
                        v_float32x4 v_r;
                        v_float32x4 v_g;
                        v_float32x4 v_b;
                        v_load_deinterleave(src, v_r, v_g, v_b);
                        process(v_b, v_g, v_r, v_hscale);
                        v_store_interleave(dst + i, v_b, v_g, v_r);
                    }
                }
            } else { // scn == 4
                if (bidx) {
                    for ( ; i <= n - 12; i += 12, src += scn * 4)
                    {
                        v_float32x4 v_r;
                        v_float32x4 v_g;
                        v_float32x4 v_b;
                        v_float32x4 v_a;
                        v_load_deinterleave(src, v_r, v_g, v_b, v_a);
                        process(v_r, v_g, v_b, v_hscale);
                        v_store_interleave(dst + i, v_r, v_g, v_b);
                    }
                } else {
                    for ( ; i <= n - 12; i += 12, src += scn * 4)
                    {
                        v_float32x4 v_r;
                        v_float32x4 v_g;
                        v_float32x4 v_b;
                        v_float32x4 v_a;
                        v_load_deinterleave(src, v_r, v_g, v_b, v_a);
                        process(v_b, v_g, v_r, v_hscale);
                        v_store_interleave(dst + i, v_b, v_g, v_r);
                    }
                }
            }
        }
        #endif

        for( ; i < n; i += 3, src += scn )
        {
            float b = src[bidx], g = src[1], r = src[bidx^2];
            float h = 0.f, s = 0.f, l;
            float vmin, vmax, diff;

            vmax = vmin = r;
            if( vmax < g ) vmax = g;
            if( vmax < b ) vmax = b;
            if( vmin > g ) vmin = g;
            if( vmin > b ) vmin = b;

            diff = vmax - vmin;
            l = (vmax + vmin)*0.5f;

            if( diff > FLT_EPSILON )
            {
                s = l < 0.5f ? diff/(vmax + vmin) : diff/(2 - vmax - vmin);
                diff = 60.f/diff;

                if( vmax == r )
                    h = (g - b)*diff;
                else if( vmax == g )
                    h = (b - r)*diff + 120.f;
                else
                    h = (r - g)*diff + 240.f;

                if( h < 0.f ) h += 360.f;
            }

            dst[i] = h*hscale;
            dst[i+1] = l;
            dst[i+2] = s;
        }
    }

    int srccn, blueIdx;
    float hscale;
    #if CV_SIMD128
    bool hasSIMD;
    #endif
};


struct RGB2HLS_b
{
    typedef uchar channel_type;

    RGB2HLS_b(int _srccn, int _blueIdx, int _hrange)
    : srccn(_srccn), cvt(3, _blueIdx, (float)_hrange)
    {
        #if CV_NEON
        v_scale_inv = vdupq_n_f32(1.f/255.f);
        v_scale = vdupq_n_f32(255.f);
        v_alpha = vdup_n_u8(ColorChannel<uchar>::max());
        #elif CV_SSE2
        v_scale_inv = _mm_set1_ps(1.f/255.f);
        v_zero = _mm_setzero_si128();
        haveSIMD = checkHardwareSupport(CV_CPU_SSE2);
        #endif
    }

    #if CV_SSE2
    void process(const float * buf,
                 __m128 & v_coeffs, uchar * dst) const
    {
        __m128 v_l0f = _mm_load_ps(buf);
        __m128 v_l1f = _mm_load_ps(buf + 4);
        __m128 v_u0f = _mm_load_ps(buf + 8);
        __m128 v_u1f = _mm_load_ps(buf + 12);

        v_l0f = _mm_mul_ps(v_l0f, v_coeffs);
        v_u1f = _mm_mul_ps(v_u1f, v_coeffs);
        v_coeffs = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v_coeffs), 0x92));
        v_u0f = _mm_mul_ps(v_u0f, v_coeffs);
        v_coeffs = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v_coeffs), 0x92));
        v_l1f = _mm_mul_ps(v_l1f, v_coeffs);

        __m128i v_l = _mm_packs_epi32(_mm_cvtps_epi32(v_l0f), _mm_cvtps_epi32(v_l1f));
        __m128i v_u = _mm_packs_epi32(_mm_cvtps_epi32(v_u0f), _mm_cvtps_epi32(v_u1f));
        __m128i v_l0 = _mm_packus_epi16(v_l, v_u);

        _mm_storeu_si128((__m128i *)(dst), v_l0);
    }
    #endif

    void operator()(const uchar* src, uchar* dst, int n) const
    {
        int i, j, scn = srccn;
        float CV_DECL_ALIGNED(16) buf[3*BLOCK_SIZE];
        #if CV_SSE2
        __m128 v_coeffs = _mm_set_ps(1.f, 255.f, 255.f, 1.f);
        #endif

        for( i = 0; i < n; i += BLOCK_SIZE, dst += BLOCK_SIZE*3 )
        {
            int dn = std::min(n - i, (int)BLOCK_SIZE);
            j = 0;

            #if CV_NEON
            for ( ; j <= (dn - 8) * 3; j += 24, src += 8 * scn)
            {
                uint16x8_t v_t0, v_t1, v_t2;

                if (scn == 3)
                {
                    uint8x8x3_t v_src = vld3_u8(src);
                    v_t0 = vmovl_u8(v_src.val[0]);
                    v_t1 = vmovl_u8(v_src.val[1]);
                    v_t2 = vmovl_u8(v_src.val[2]);
                }
                else
                {
                    uint8x8x4_t v_src = vld4_u8(src);
                    v_t0 = vmovl_u8(v_src.val[0]);
                    v_t1 = vmovl_u8(v_src.val[1]);
                    v_t2 = vmovl_u8(v_src.val[2]);
                }

                float32x4x3_t v_dst;
                v_dst.val[0] = vmulq_f32(vcvtq_f32_u32(vmovl_u16(vget_low_u16(v_t0))), v_scale_inv);
                v_dst.val[1] = vmulq_f32(vcvtq_f32_u32(vmovl_u16(vget_low_u16(v_t1))), v_scale_inv);
                v_dst.val[2] = vmulq_f32(vcvtq_f32_u32(vmovl_u16(vget_low_u16(v_t2))), v_scale_inv);
                vst3q_f32(buf + j, v_dst);

                v_dst.val[0] = vmulq_f32(vcvtq_f32_u32(vmovl_u16(vget_high_u16(v_t0))), v_scale_inv);
                v_dst.val[1] = vmulq_f32(vcvtq_f32_u32(vmovl_u16(vget_high_u16(v_t1))), v_scale_inv);
                v_dst.val[2] = vmulq_f32(vcvtq_f32_u32(vmovl_u16(vget_high_u16(v_t2))), v_scale_inv);
                vst3q_f32(buf + j + 12, v_dst);
            }
            #elif CV_SSE2
            if (scn == 3 && haveSIMD)
            {
                for ( ; j <= (dn * 3 - 16); j += 16, src += 16)
                {
                    __m128i v_src = _mm_loadu_si128((__m128i const *)src);

                    __m128i v_src_p = _mm_unpacklo_epi8(v_src, v_zero);
                    _mm_store_ps(buf + j, _mm_mul_ps(_mm_cvtepi32_ps(_mm_unpacklo_epi16(v_src_p, v_zero)), v_scale_inv));
                    _mm_store_ps(buf + j + 4, _mm_mul_ps(_mm_cvtepi32_ps(_mm_unpackhi_epi16(v_src_p, v_zero)), v_scale_inv));

                    v_src_p = _mm_unpackhi_epi8(v_src, v_zero);
                    _mm_store_ps(buf + j + 8, _mm_mul_ps(_mm_cvtepi32_ps(_mm_unpacklo_epi16(v_src_p, v_zero)), v_scale_inv));
                    _mm_store_ps(buf + j + 12, _mm_mul_ps(_mm_cvtepi32_ps(_mm_unpackhi_epi16(v_src_p, v_zero)), v_scale_inv));
                }

                int jr = j % 3;
                if (jr)
                    src -= jr, j -= jr;
            }
            else if (scn == 4 && haveSIMD)
            {
                for ( ; j <= (dn * 3 - 12); j += 12, src += 16)
                {
                    __m128i v_src = _mm_loadu_si128((__m128i const *)src);

                    __m128i v_src_lo = _mm_unpacklo_epi8(v_src, v_zero);
                    __m128i v_src_hi = _mm_unpackhi_epi8(v_src, v_zero);
                    _mm_storeu_ps(buf + j, _mm_mul_ps(_mm_cvtepi32_ps(_mm_unpacklo_epi16(v_src_lo, v_zero)), v_scale_inv));
                    _mm_storeu_ps(buf + j + 3, _mm_mul_ps(_mm_cvtepi32_ps(_mm_unpackhi_epi16(v_src_lo, v_zero)), v_scale_inv));
                    _mm_storeu_ps(buf + j + 6, _mm_mul_ps(_mm_cvtepi32_ps(_mm_unpacklo_epi16(v_src_hi, v_zero)), v_scale_inv));
                    float tmp = buf[j + 8];
                    _mm_storeu_ps(buf + j + 8, _mm_mul_ps(_mm_cvtepi32_ps(_mm_shuffle_epi32(_mm_unpackhi_epi16(v_src_hi, v_zero), 0x90)), v_scale_inv));
                    buf[j + 8] = tmp;
                }

                int jr = j % 3;
                if (jr)
                    src -= jr, j -= jr;
            }
            #endif
            for( ; j < dn*3; j += 3, src += scn )
            {
                buf[j] = src[0]*(1.f/255.f);
                buf[j+1] = src[1]*(1.f/255.f);
                buf[j+2] = src[2]*(1.f/255.f);
            }
            cvt(buf, buf, dn);

            j = 0;
            #if CV_NEON
            for ( ; j <= (dn - 8) * 3; j += 24)
            {
                float32x4x3_t v_src0 = vld3q_f32(buf + j), v_src1 = vld3q_f32(buf + j + 12);

                uint8x8x3_t v_dst;
                v_dst.val[0] = vqmovn_u16(vcombine_u16(vqmovn_u32(cv_vrndq_u32_f32(v_src0.val[0])),
                                                       vqmovn_u32(cv_vrndq_u32_f32(v_src1.val[0]))));
                v_dst.val[1] = vqmovn_u16(vcombine_u16(vqmovn_u32(cv_vrndq_u32_f32(vmulq_f32(v_src0.val[1], v_scale))),
                                                       vqmovn_u32(cv_vrndq_u32_f32(vmulq_f32(v_src1.val[1], v_scale)))));
                v_dst.val[2] = vqmovn_u16(vcombine_u16(vqmovn_u32(cv_vrndq_u32_f32(vmulq_f32(v_src0.val[2], v_scale))),
                                                       vqmovn_u32(cv_vrndq_u32_f32(vmulq_f32(v_src1.val[2], v_scale)))));
                vst3_u8(dst + j, v_dst);
            }
            #elif CV_SSE2
            if (haveSIMD)
            {
                for ( ; j <= (dn - 16) * 3; j += 48)
                {
                    process(buf + j,
                            v_coeffs, dst + j);

                    process(buf + j + 16,
                            v_coeffs, dst + j + 16);

                    process(buf + j + 32,
                            v_coeffs, dst + j + 32);
                }
            }
            #endif
            for( ; j < dn*3; j += 3 )
            {
                dst[j] = saturate_cast<uchar>(buf[j]);
                dst[j+1] = saturate_cast<uchar>(buf[j+1]*255.f);
                dst[j+2] = saturate_cast<uchar>(buf[j+2]*255.f);
            }
        }
    }

    int srccn;
    RGB2HLS_f cvt;
    #if CV_NEON
    float32x4_t v_scale, v_scale_inv;
    uint8x8_t v_alpha;
    #elif CV_SSE2
    __m128 v_scale_inv;
    __m128i v_zero;
    bool haveSIMD;
    #endif
};


struct HLS2RGB_f
{
    typedef float channel_type;

    HLS2RGB_f(int _dstcn, int _blueIdx, float _hrange)
    : dstcn(_dstcn), blueIdx(_blueIdx), hscale(6.f/_hrange)
    { }

#if CV_SIMD
    inline void process(const v_float32& h, const v_float32& l, const v_float32& s,
                        v_float32& b, v_float32& g, v_float32& r) const
    {
        v_float32 v1 = vx_setall_f32(1.0f), v2 = vx_setall_f32(2.0f), v4 = vx_setall_f32(4.0f);

        v_float32 lBelowHalfMask = l <= vx_setall_f32(0.5f);
        v_float32 ls = l * s;
        v_float32 elem0 = v_select(lBelowHalfMask, ls, s - ls);

        v_float32 hsRaw = h * vx_setall_f32(hscale);
        v_float32 preHs = v_cvt_f32(v_trunc(hsRaw));
        v_float32 hs = hsRaw - preHs;
        v_float32 sector = preHs - vx_setall_f32(6.0f) * v_cvt_f32(v_trunc(hsRaw * vx_setall_f32(1.0f / 6.0f)));
        v_float32 elem1 = hs + hs;

        v_float32 tab0 = l + elem0;
        v_float32 tab1 = l - elem0;
        v_float32 tab2 = l + elem0 - elem0 * elem1;
        v_float32 tab3 = l - elem0 + elem0 * elem1;

        b = v_select(sector <  v2, tab1,
            v_select(sector <= v2, tab3,
            v_select(sector <= v4, tab0, tab2)));

        g = v_select(sector <  v1, tab3,
            v_select(sector <= v2, tab0,
            v_select(sector <  v4, tab2, tab1)));

        r = v_select(sector <  v1, tab0,
            v_select(sector <  v2, tab2,
            v_select(sector <  v4, tab1,
            v_select(sector <= v4, tab3, tab0))));
    }
#endif

    void operator()(const float* src, float* dst, int n) const
    {
        CV_INSTRUMENT_REGION();

        int i = 0, bidx = blueIdx, dcn = dstcn;
        float alpha = ColorChannel<float>::max();

#if CV_SIMD
        static const int vsize = v_float32::nlanes;
        for (; i <= n - vsize; i += vsize, src += 3*vsize, dst += dcn*vsize)
        {
            v_float32 h, l, s, r, g, b;
            v_load_deinterleave(src, h, l, s);

            process(h, l, s, b, g, r);

            if(bidx)
                swap(b, r);

            if(dcn == 3)
            {
                v_store_interleave(dst, b, g, r);
            }
            else
            {
                v_float32 a = vx_setall_f32(alpha);
                v_store_interleave(dst, b, g, r, a);
            }
        }
#endif

        for( ; i < n; i++, src += 3, dst += dcn )
        {
            float h = src[0], l = src[1], s = src[2];
            float b, g, r;

            if( s == 0 )
                b = g = r = l;
            else
            {
                static const int sector_data[][3]=
                {{1,3,0}, {1,0,2}, {3,0,1}, {0,2,1}, {0,1,3}, {2,1,0}};
                float tab[4];
                int sector;

                float p2 = l <= 0.5f ? l*(1 + s) : l + s - l*s;
                float p1 = 2*l - p2;

                h *= hscale;
                if( h < 0 )
                    do h += 6; while( h < 0 );
                else if( h >= 6 )
                    do h -= 6; while( h >= 6 );

                assert( 0 <= h && h < 6 );
                sector = cvFloor(h);
                h -= sector;

                tab[0] = p2;
                tab[1] = p1;
                tab[2] = p1 + (p2 - p1)*(1-h);
                tab[3] = p1 + (p2 - p1)*h;

                b = tab[sector_data[sector][0]];
                g = tab[sector_data[sector][1]];
                r = tab[sector_data[sector][2]];
            }

            dst[bidx] = b;
            dst[1] = g;
            dst[bidx^2] = r;
            if( dcn == 4 )
                dst[3] = alpha;
        }
    }

    int dstcn, blueIdx;
    float hscale;
};


struct HLS2RGB_b
{
    typedef uchar channel_type;

    HLS2RGB_b(int _dstcn, int _blueIdx, int _hrange)
    : dstcn(_dstcn), cvt(3, _blueIdx, (float)_hrange)
    {
        #if CV_NEON
        v_scale_inv = vdupq_n_f32(1.f/255.f);
        v_scale = vdupq_n_f32(255.f);
        v_alpha = vdup_n_u8(ColorChannel<uchar>::max());
        #elif CV_SSE2
        v_scale = _mm_set1_ps(255.f);
        v_alpha = _mm_set1_ps(ColorChannel<uchar>::max());
        v_zero = _mm_setzero_si128();
        haveSIMD = checkHardwareSupport(CV_CPU_SSE2);
        #endif
    }

    #if CV_SSE2
    void process(__m128i v_r, __m128i v_g, __m128i v_b,
                 const __m128& v_coeffs_,
                 float * buf) const
    {
        __m128 v_r0 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(v_r, v_zero));
        __m128 v_g0 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(v_g, v_zero));
        __m128 v_b0 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(v_b, v_zero));

        __m128 v_r1 = _mm_cvtepi32_ps(_mm_unpackhi_epi16(v_r, v_zero));
        __m128 v_g1 = _mm_cvtepi32_ps(_mm_unpackhi_epi16(v_g, v_zero));
        __m128 v_b1 = _mm_cvtepi32_ps(_mm_unpackhi_epi16(v_b, v_zero));

        __m128 v_coeffs = v_coeffs_;

        v_r0 = _mm_mul_ps(v_r0, v_coeffs);
        v_g1 = _mm_mul_ps(v_g1, v_coeffs);

        v_coeffs = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v_coeffs), 0x49));

        v_r1 = _mm_mul_ps(v_r1, v_coeffs);
        v_b0 = _mm_mul_ps(v_b0, v_coeffs);

        v_coeffs = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v_coeffs), 0x49));

        v_g0 = _mm_mul_ps(v_g0, v_coeffs);
        v_b1 = _mm_mul_ps(v_b1, v_coeffs);

        _mm_store_ps(buf, v_r0);
        _mm_store_ps(buf + 4, v_r1);
        _mm_store_ps(buf + 8, v_g0);
        _mm_store_ps(buf + 12, v_g1);
        _mm_store_ps(buf + 16, v_b0);
        _mm_store_ps(buf + 20, v_b1);
    }
    #endif

    void operator()(const uchar* src, uchar* dst, int n) const
    {
        int i, j, dcn = dstcn;
        uchar alpha = ColorChannel<uchar>::max();
        float CV_DECL_ALIGNED(16) buf[3*BLOCK_SIZE];
        #if CV_SSE2
        __m128 v_coeffs = _mm_set_ps(1.f, 1.f/255.f, 1.f/255.f, 1.f);
        #endif

        for( i = 0; i < n; i += BLOCK_SIZE, src += BLOCK_SIZE*3 )
        {
            int dn = std::min(n - i, (int)BLOCK_SIZE);
            j = 0;

            #if CV_NEON
            for ( ; j <= (dn - 8) * 3; j += 24)
            {
                uint8x8x3_t v_src = vld3_u8(src + j);
                uint16x8_t v_t0 = vmovl_u8(v_src.val[0]),
                           v_t1 = vmovl_u8(v_src.val[1]),
                           v_t2 = vmovl_u8(v_src.val[2]);

                float32x4x3_t v_dst;
                v_dst.val[0] = vcvtq_f32_u32(vmovl_u16(vget_low_u16(v_t0)));
                v_dst.val[1] = vmulq_f32(vcvtq_f32_u32(vmovl_u16(vget_low_u16(v_t1))), v_scale_inv);
                v_dst.val[2] = vmulq_f32(vcvtq_f32_u32(vmovl_u16(vget_low_u16(v_t2))), v_scale_inv);
                vst3q_f32(buf + j, v_dst);

                v_dst.val[0] = vcvtq_f32_u32(vmovl_u16(vget_high_u16(v_t0)));
                v_dst.val[1] = vmulq_f32(vcvtq_f32_u32(vmovl_u16(vget_high_u16(v_t1))), v_scale_inv);
                v_dst.val[2] = vmulq_f32(vcvtq_f32_u32(vmovl_u16(vget_high_u16(v_t2))), v_scale_inv);
                vst3q_f32(buf + j + 12, v_dst);
            }
            #elif CV_SSE2
            if (haveSIMD)
            {
                for ( ; j <= (dn - 8) * 3; j += 24)
                {
                    __m128i v_src0 = _mm_loadu_si128((__m128i const *)(src + j));
                    __m128i v_src1 = _mm_loadl_epi64((__m128i const *)(src + j + 16));

                    process(_mm_unpacklo_epi8(v_src0, v_zero),
                            _mm_unpackhi_epi8(v_src0, v_zero),
                            _mm_unpacklo_epi8(v_src1, v_zero),
                            v_coeffs,
                            buf + j);
                }
            }
            #endif
            for( ; j < dn*3; j += 3 )
            {
                buf[j] = src[j];
                buf[j+1] = src[j+1]*(1.f/255.f);
                buf[j+2] = src[j+2]*(1.f/255.f);
            }
            cvt(buf, buf, dn);

            j = 0;
            #if CV_NEON
            for ( ; j <= (dn - 8) * 3; j += 24, dst += dcn * 8)
            {
                float32x4x3_t v_src0 = vld3q_f32(buf + j), v_src1 = vld3q_f32(buf + j + 12);
                uint8x8_t v_dst0 = vqmovn_u16(vcombine_u16(vqmovn_u32(cv_vrndq_u32_f32(vmulq_f32(v_src0.val[0], v_scale))),
                                                           vqmovn_u32(cv_vrndq_u32_f32(vmulq_f32(v_src1.val[0], v_scale)))));
                uint8x8_t v_dst1 = vqmovn_u16(vcombine_u16(vqmovn_u32(cv_vrndq_u32_f32(vmulq_f32(v_src0.val[1], v_scale))),
                                                           vqmovn_u32(cv_vrndq_u32_f32(vmulq_f32(v_src1.val[1], v_scale)))));
                uint8x8_t v_dst2 = vqmovn_u16(vcombine_u16(vqmovn_u32(cv_vrndq_u32_f32(vmulq_f32(v_src0.val[2], v_scale))),
                                                           vqmovn_u32(cv_vrndq_u32_f32(vmulq_f32(v_src1.val[2], v_scale)))));

                if (dcn == 4)
                {
                    uint8x8x4_t v_dst;
                    v_dst.val[0] = v_dst0;
                    v_dst.val[1] = v_dst1;
                    v_dst.val[2] = v_dst2;
                    v_dst.val[3] = v_alpha;
                    vst4_u8(dst, v_dst);
                }
                else
                {
                    uint8x8x3_t v_dst;
                    v_dst.val[0] = v_dst0;
                    v_dst.val[1] = v_dst1;
                    v_dst.val[2] = v_dst2;
                    vst3_u8(dst, v_dst);
                }
            }
            #elif CV_SSE2
            if (dcn == 3 && haveSIMD)
            {
                for ( ; j <= (dn * 3 - 16); j += 16, dst += 16)
                {
                    __m128 v_src0 = _mm_mul_ps(_mm_load_ps(buf + j), v_scale);
                    __m128 v_src1 = _mm_mul_ps(_mm_load_ps(buf + j + 4), v_scale);
                    __m128 v_src2 = _mm_mul_ps(_mm_load_ps(buf + j + 8), v_scale);
                    __m128 v_src3 = _mm_mul_ps(_mm_load_ps(buf + j + 12), v_scale);

                    __m128i v_dst0 = _mm_packs_epi32(_mm_cvtps_epi32(v_src0),
                                                     _mm_cvtps_epi32(v_src1));
                    __m128i v_dst1 = _mm_packs_epi32(_mm_cvtps_epi32(v_src2),
                                                     _mm_cvtps_epi32(v_src3));

                    _mm_storeu_si128((__m128i *)dst, _mm_packus_epi16(v_dst0, v_dst1));
                }

                int jr = j % 3;
                if (jr)
                    dst -= jr, j -= jr;
            }
            else if (dcn == 4 && haveSIMD)
            {
                for ( ; j <= (dn * 3 - 12); j += 12, dst += 16)
                {
                    __m128 v_buf0 = _mm_mul_ps(_mm_load_ps(buf + j), v_scale);
                    __m128 v_buf1 = _mm_mul_ps(_mm_load_ps(buf + j + 4), v_scale);
                    __m128 v_buf2 = _mm_mul_ps(_mm_load_ps(buf + j + 8), v_scale);

                    __m128 v_ba0 = _mm_unpackhi_ps(v_buf0, v_alpha);
                    __m128 v_ba1 = _mm_unpacklo_ps(v_buf2, v_alpha);

                    __m128i v_src0 = _mm_cvtps_epi32(_mm_shuffle_ps(v_buf0, v_ba0, 0x44));
                    __m128i v_src1 = _mm_shuffle_epi32(_mm_cvtps_epi32(_mm_shuffle_ps(v_ba0, v_buf1, 0x4e)), 0x78);
                    __m128i v_src2 = _mm_cvtps_epi32(_mm_shuffle_ps(v_buf1, v_ba1, 0x4e));
                    __m128i v_src3 = _mm_shuffle_epi32(_mm_cvtps_epi32(_mm_shuffle_ps(v_ba1, v_buf2, 0xee)), 0x78);

                    __m128i v_dst0 = _mm_packs_epi32(v_src0, v_src1);
                    __m128i v_dst1 = _mm_packs_epi32(v_src2, v_src3);

                    _mm_storeu_si128((__m128i *)dst, _mm_packus_epi16(v_dst0, v_dst1));
                }

                int jr = j % 3;
                if (jr)
                    dst -= jr, j -= jr;
            }
            #endif

            for( ; j < dn*3; j += 3, dst += dcn )
            {
                dst[0] = saturate_cast<uchar>(buf[j]*255.f);
                dst[1] = saturate_cast<uchar>(buf[j+1]*255.f);
                dst[2] = saturate_cast<uchar>(buf[j+2]*255.f);
                if( dcn == 4 )
                    dst[3] = alpha;
            }
        }
    }

    int dstcn;
    HLS2RGB_f cvt;
    #if CV_NEON
    float32x4_t v_scale, v_scale_inv;
    uint8x8_t v_alpha;
    #elif CV_SSE2
    __m128 v_scale;
    __m128 v_alpha;
    __m128i v_zero;
    bool haveSIMD;
    #endif
};

} // namespace anon

// 8u, 32f
void cvtBGRtoHSV(const uchar * src_data, size_t src_step,
                 uchar * dst_data, size_t dst_step,
                 int width, int height,
                 int depth, int scn, bool swapBlue, bool isFullRange, bool isHSV)
{
    CV_INSTRUMENT_REGION();

    int hrange = depth == CV_32F ? 360 : isFullRange ? 256 : 180;
    int blueIdx = swapBlue ? 2 : 0;
    if(isHSV)
    {
        if(depth == CV_8U)
            CvtColorLoop(src_data, src_step, dst_data, dst_step, width, height, RGB2HSV_b(scn, blueIdx, hrange));
        else
            CvtColorLoop(src_data, src_step, dst_data, dst_step, width, height, RGB2HSV_f(scn, blueIdx, static_cast<float>(hrange)));
    }
    else
    {
        if( depth == CV_8U )
            CvtColorLoop(src_data, src_step, dst_data, dst_step, width, height, RGB2HLS_b(scn, blueIdx, hrange));
        else
            CvtColorLoop(src_data, src_step, dst_data, dst_step, width, height, RGB2HLS_f(scn, blueIdx, static_cast<float>(hrange)));
    }
}

// 8u, 32f
void cvtHSVtoBGR(const uchar * src_data, size_t src_step,
                 uchar * dst_data, size_t dst_step,
                 int width, int height,
                 int depth, int dcn, bool swapBlue, bool isFullRange, bool isHSV)
{
    CV_INSTRUMENT_REGION();

    int hrange = depth == CV_32F ? 360 : isFullRange ? 255 : 180;
    int blueIdx = swapBlue ? 2 : 0;
    if(isHSV)
    {
        if( depth == CV_8U )
            CvtColorLoop(src_data, src_step, dst_data, dst_step, width, height, HSV2RGB_b(dcn, blueIdx, hrange));
        else
            CvtColorLoop(src_data, src_step, dst_data, dst_step, width, height, HSV2RGB_f(dcn, blueIdx, static_cast<float>(hrange)));
    }
    else
    {
        if( depth == CV_8U )
            CvtColorLoop(src_data, src_step, dst_data, dst_step, width, height, HLS2RGB_b(dcn, blueIdx, hrange));
        else
            CvtColorLoop(src_data, src_step, dst_data, dst_step, width, height, HLS2RGB_f(dcn, blueIdx, static_cast<float>(hrange)));
    }
}

#endif
CV_CPU_OPTIMIZATION_NAMESPACE_END
}} // namespace
