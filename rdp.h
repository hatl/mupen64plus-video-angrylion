#ifndef _RDP_H_
#define _RDP_H_

#include "z64.h"
#include "../mupen64plus-core/src/rdp_common/gdp.h"

#ifdef USE_SSE_SUPPORT
#include <emmintrin.h>

#define SIGND(m)    _mm_srai_epi32(m, 31)
#define XMM_ZERO    _mm_setzero_si128()
#define setzero_si128(buf) _mm_store_si128((__m128i *)buf, XMM_ZERO)

#define mm_neg_epi32(m)     _mm_sub_epi32(XMM_ZERO, m)

#define mm_sign_epi32_nz(m1, m2) \
_mm_sub_epi32(_mm_xor_si128(m1, SIGND(m2)), SIGND(m2))

#define mm_unpacklo_epi64_hz(m, n) \
_mm_unpacklo_epi64(_mm_unpacklo_epi32(m, n), _mm_unpackhi_epi32(m, n))

#if 1
#ifndef USE_MMX_DECODES
#define USE_MMX_DECODES
#endif
#endif

INLINE extern __m128i mm_mullo_epi32_seh(__m128i dest, __m128i src);

#else
#define setzero_si128(buf) \
    { ((i64 *)buf)[0] = ((i64 *)buf)[1] = 0x0000000000000000; }
#endif

#ifdef USE_MMX_DECODES
#include <xmmintrin.h>
#define setzero_si64(buffer) { \
    *(__m64 *)(buffer) = _mm_setzero_si64(); \
}
#else
#define setzero_si64(buffer) { \
    *(i64 *)(buffer) = 0x0000000000000000; \
}
#endif

#if defined(_AMD64_) || defined(_IA64_) || defined(__x86_64__)
#define BUFFERFIFO(word, base, offset) { \
    *(i64 *)&cmd_data[word] = *(i64 *)((base) + 8*(offset)); \
}
#elif defined(USE_MMX_DECODES)
#define BUFFERFIFO(word, base, offset) { \
    *(__m64 *)&cmd_data[word] = *(__m64 *)((base) + 8*(offset)); \
}
#else
/*
 * compatibility fallback to prevent unspecified behavior from the ANSI C
 * specifications (reading unions per 32 bits when 64 bits were just written)
 */
#define BUFFERFIFO(word, base, offset) { \
    cmd_data[word].W32[0] = *(i32 *)((base) + 8*(offset) + 0); \
    cmd_data[word].W32[1] = *(i32 *)((base) + 8*(offset) + 4); \
}
#endif

#ifdef _MSC_VER
typedef __int8              i8;
typedef __int16             i16;
typedef __int32             i32;
typedef __int64             i64;
typedef signed __int8       s8;
typedef signed __int16      s16;
typedef signed __int32      s32;
typedef signed __int64      s64;
typedef unsigned __int8     u8;
typedef unsigned __int16    u16;
typedef unsigned __int32    u32;
typedef unsigned __int64    u64;
#else
typedef char                    i8;
typedef short                   i16;
typedef int                     i32;
typedef long long               i64;
typedef signed char             s8;
typedef signed short            s16;
typedef signed int              s32;
typedef signed long long        s64;
typedef unsigned char           u8;
typedef unsigned short          u16;
typedef unsigned int            u32;
typedef unsigned long long      u64;
#endif

#ifdef USE_SSE_SUPPORT
typedef __m128i     v8;
typedef __m128i     v16;
typedef __m128i     v32;
#else
typedef char*       v8;
typedef short*      v16;
typedef int*        v32;
#endif

#define SIGN16(x)       (s16)(x)
#define SIGN8(x)        (s8)(x)

#if (~0 >> 1 < 0)
#define SRA(exp, sa)    ((signed)(exp) >> (sa))
#else
#define SRA(exp, sa)    (SE((exp) >> (sa), (sa) ^ 31))
#endif

/*
 * Virtual register sign-extensions and clamps using b-bit immediates:
 */
#define CLAMP(i, b)     ((i) & ~(~0x00000000 << (b)))
#define SB(i, b)        ((i) &  ( 0x00000001 << ((b) - 1)))
#define SE(i, b)        ((i) | -SB((i), (b)))

/*
 * Forces to clamp immediate bit width AND sign-extend virtual register:
 */
#if (0)
#define SIGN(i, b)      SE(CLAMP((i), (b)), (b))
#else
#define SIGN(i, b)      SRA((i) << (32 - (b)), (32 - (b)))
#endif
#define SIGNF(x, numb)    ((x) | -((x) & (1 << (numb - 1))))

#define GET_LOW_RGBA16_TMEM(x)      replicated_rgba[((x) & 0x003F) >>  1]
#define GET_MED_RGBA16_TMEM(x)      replicated_rgba[((x) & 0x07FF) >>  6]
#define GET_HI_RGBA16_TMEM(x)       replicated_rgba[(u16)(x) >> 11]

#define f_BYTE_H(B) (!!(B&0xFF)*8 | !!(B&0xF0)*4 | !!(B&0xCC)*2 | !!(B&0xAA))
#define f_BYTE_L(B) (!!(B&0xFF)*0 | !!(B&0xF0)*4 | !!(B&0xCC)*2 | !!(B&0xAA))

typedef union {
    i64 W;
    s64 SW;
    u64 UW;
    i32 W32[2];
    s32 SW32[2];
    u32 UW32[2];
    i16 W16[4];
    s16 SW16[4];
    u16 UW16[4];
    unsigned char B[8];
    signed char SB[8];
} DP_FIFO;

typedef struct {
    INT32 r, g, b, a;
} COLOR;

typedef struct {
    int lx, rx;
    int unscrx;
    int validline;
    ALIGNED INT32 rgba[4];
    ALIGNED INT32 stwz[4];
    INT32 majorx[4];
    INT32 minorx[4];
    INT32 invalyscan[4];
} SPAN;

typedef struct {
    INT32 xl, yl, xh, yh;
} RECTANGLE;

enum {
    PIXEL_SIZE_4BIT,
    PIXEL_SIZE_8BIT,
    PIXEL_SIZE_16BIT,
    PIXEL_SIZE_32BIT
};

enum {
    CYCLE_TYPE_1,
    CYCLE_TYPE_2,
    CYCLE_TYPE_COPY,
    CYCLE_TYPE_FILL
};

enum {
    FORMAT_RGBA,
    FORMAT_YUV,
    FORMAT_CI,
    FORMAT_IA,
    FORMAT_I
};

enum {
    TEXEL_RGBA4,
    TEXEL_RGBA8,
    TEXEL_RGBA16,
    TEXEL_RGBA32,
    TEXEL_YUV4,
    TEXEL_YUV8,
    TEXEL_YUV16,
    TEXEL_YUV32,
    TEXEL_CI4,
    TEXEL_CI8,
    TEXEL_CI16,
    TEXEL_CI32,
    TEXEL_IA4,
    TEXEL_IA8,
    TEXEL_IA16,
    TEXEL_IA32,
    TEXEL_I4,
    TEXEL_I8,
    TEXEL_I16,
    TEXEL_I32
};

enum {
    NO,
    YES
};
enum {
    SHADE_NO,
    SHADE_YES = YES
};
enum {
    TEXTURE_NO,
    TEXTURE_YES = YES
};
enum {
    ZBUFFER_NO,
    ZBUFFER_YES = YES
};

extern void process_RDP_list(void);
#ifdef TRACE_DP_COMMANDS
extern void count_DP_commands(void);
#endif

extern void (*fbread1_ptr)(UINT32, UINT32*);
extern void (*fbread2_ptr)(UINT32, UINT32*);
extern void (*fbwrite_ptr)(
    UINT32, UINT32, UINT32, UINT32, UINT32, UINT32, UINT32);
extern void (*fbfill_ptr)(UINT32);

extern void fbwrite_4(
    UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b, UINT32 blend_en,
    UINT32 curpixel_cvg, UINT32 curpixel_memcvg);
extern void fbwrite_8(
    UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b, UINT32 blend_en,
    UINT32 curpixel_cvg, UINT32 curpixel_memcvg);
extern void fbwrite_16(
    UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b, UINT32 blend_en,
    UINT32 curpixel_cvg, UINT32 curpixel_memcvg);
extern void fbwrite_32(
    UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b, UINT32 blend_en,
    UINT32 curpixel_cvg, UINT32 curpixel_memcvg);
extern void fbfill_4(UINT32 curpixel);
extern void fbfill_8(UINT32 curpixel);
extern void fbfill_16(UINT32 curpixel);
extern void fbfill_32(UINT32 curpixel);
extern void fbread_4(UINT32 num, UINT32* curpixel_memcvg);
extern void fbread_8(UINT32 num, UINT32* curpixel_memcvg);
extern void fbread_16(UINT32 num, UINT32* curpixel_memcvg);
extern void fbread_32(UINT32 num, UINT32* curpixel_memcvg);
extern void fbread2_4(UINT32 num, UINT32* curpixel_memcvg);
extern void fbread2_8(UINT32 num, UINT32* curpixel_memcvg);
extern void fbread2_16(UINT32 num, UINT32* curpixel_memcvg);
extern void fbread2_32(UINT32 num, UINT32* curpixel_memcvg);

static void (*fbread_func[4])(UINT32, UINT32*) = {
    fbread_4, fbread_8, fbread_16, fbread_32
};
static void (*fbread2_func[4])(UINT32, UINT32*) = {
    fbread2_4, fbread2_8, fbread2_16, fbread2_32
};
static void (*fbwrite_func[4])(
    UINT32, UINT32, UINT32, UINT32, UINT32, UINT32, UINT32) = {
    fbwrite_4, fbwrite_8, fbwrite_16, fbwrite_32
};
static void (*fbfill_func[4])(UINT32) = {
    fbfill_4, fbfill_8, fbfill_16, fbfill_32
};

static INLINE void calculate_clamp_diffs(UINT32 i)
{
   g_gdp.tile[i].f.clampdiffs = ((g_gdp.tile[i].sh >> 2) - (g_gdp.tile[i].sl >> 2)) & 0x3ff;
   g_gdp.tile[i].f.clampdifft = ((g_gdp.tile[i].th >> 2) - (g_gdp.tile[i].tl >> 2)) & 0x3ff;
}

static INLINE void calculate_tile_derivs(UINT32 i)
{
   g_gdp.tile[i].f.clampens     = g_gdp.tile[i].cs || !g_gdp.tile[i].mask_s;
   g_gdp.tile[i].f.clampent     = g_gdp.tile[i].ct || !g_gdp.tile[i].mask_t;
   g_gdp.tile[i].f.masksclamped = g_gdp.tile[i].mask_s <= 10 ? g_gdp.tile[i].mask_s : 10;
   g_gdp.tile[i].f.masktclamped = g_gdp.tile[i].mask_t <= 10 ? g_gdp.tile[i].mask_t : 10;
   g_gdp.tile[i].f.notlutswitch = (g_gdp.tile[i].format << 2) | g_gdp.tile[i].size;
   g_gdp.tile[i].f.tlutswitch    = (g_gdp.tile[i].size << 2) | ((g_gdp.tile[i].format + 2) & 3);
}

extern void tile_tlut_common_cs_decoder(UINT32 w1, UINT32 w2);
extern void deduce_derivatives(void);

extern void render_spans_1cycle(int start, int end, int tilenum, int flip);
extern void render_spans_2cycle(int start, int end, int tilenum, int flip);
extern NOINLINE void render_spans_copy(
    int start, int end, int tilenum, int flip);
extern NOINLINE void render_spans_fill(int start, int end, int flip);

NOINLINE extern void edgewalker_for_loads(INT32* lewdata);
extern void (*render_spans_1cycle_ptr)(int, int, int, int);
extern void (*render_spans_2cycle_ptr)(int, int, int, int);

void SET_BLENDER_INPUT(
      int cycle, int which,
      INT32 **input_r,
      INT32 **input_g,
      INT32 **input_b,
      INT32 **input_a,
      int a, int b);

#ifdef _DEBUG
int render_cycle_mode_counts[4];
#endif

extern i32 spans_d_rgba[4];
extern i32 spans_d_stwz[4];
extern u16 spans_dzpix;
extern SPAN span[1024];

extern i32 spans_d_rgba_dy[4];
extern i32 spans_cd_rgba[4];
extern int spans_cdz;

extern i32 spans_d_stwz_dy[4];

extern int scfield;
extern int sckeepodd;

/* texture image */
extern int ti_format;         /* format: ARGB, IA, ... */
extern int ti_size;           /* size: 4, 8, 16, or 32-bit */
extern int ti_width;          /* used in rdp_settextureimage */
extern UINT32 ti_address;     /* address in RDRAM to load the texture from */

extern int fb_format;
extern int fb_size;
extern int fb_width;
extern UINT32 fb_address;
extern UINT32 zb_address;

extern UINT32 max_level;

extern INT32 *combiner_rgbsub_a_r[2];
extern INT32 *combiner_rgbsub_a_g[2];
extern INT32 *combiner_rgbsub_a_b[2];
extern INT32 *combiner_rgbsub_b_r[2];
extern INT32 *combiner_rgbsub_b_g[2];
extern INT32 *combiner_rgbsub_b_b[2];
extern INT32 *combiner_rgbmul_r[2];
extern INT32 *combiner_rgbmul_g[2];
extern INT32 *combiner_rgbmul_b[2];
extern INT32 *combiner_rgbadd_r[2];
extern INT32 *combiner_rgbadd_g[2];
extern INT32 *combiner_rgbadd_b[2];

extern INT32 *combiner_alphasub_a[2];
extern INT32 *combiner_alphasub_b[2];
extern INT32 *combiner_alphamul[2];
extern INT32 *combiner_alphaadd[2];

extern INT32 *blender1a_r[2];
extern INT32 *blender1a_g[2];
extern INT32 *blender1a_b[2];
extern INT32 *blender1b_a[2];
extern INT32 *blender2a_r[2];
extern INT32 *blender2a_g[2];
extern INT32 *blender2a_b[2];
extern INT32 *blender2b_a[2];

extern RECTANGLE __clip;

extern COLOR combined_color;
extern COLOR texel0_color;
extern COLOR texel1_color;
extern COLOR nexttexel_color;
extern COLOR shade_color;
extern INT32 noise;
extern INT32 one_color;
extern INT32 zero_color;
extern INT32 lod_frac;

extern int rdp_pipeline_crashed;

#endif
