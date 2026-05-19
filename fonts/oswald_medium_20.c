/*******************************************************************************
 * Size: 20 px
 * Bpp: 1
 * Opts: --bpp 1 --size 20 --no-compress --stride 1 --align 1 --font Oswald-Medium.ttf --symbols .0123456789 --format lvgl -o oswald_medium_20.c
 ******************************************************************************/

#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif



#ifndef OSWALD_MEDIUM_20
#define OSWALD_MEDIUM_20 1
#endif

#if OSWALD_MEDIUM_20

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+002E "." */
    0xff, 0x80,

    /* U+0030 "0" */
    0x3e, 0x3f, 0xb8, 0xfc, 0x7e, 0x3f, 0x1f, 0x8f,
    0xc7, 0xe3, 0xf1, 0xf8, 0xfc, 0x7e, 0x3f, 0x1f,
    0x9e, 0xfe, 0x3e, 0x0,

    /* U+0031 "1" */
    0x1d, 0xff, 0xf7, 0x1c, 0x71, 0xc7, 0x1c, 0x71,
    0xc7, 0x1c, 0x71, 0xc7, 0x1c,

    /* U+0032 "2" */
    0x3e, 0x3f, 0xbc, 0xfc, 0x7e, 0x3f, 0x1c, 0x1e,
    0xe, 0xf, 0x7, 0x7, 0x7, 0x83, 0x83, 0x83,
    0xc1, 0xfe, 0xff, 0x0,

    /* U+0033 "3" */
    0x3e, 0x3f, 0xb8, 0xfc, 0x7e, 0x38, 0x1c, 0x1c,
    0x1e, 0xf, 0x3, 0xc0, 0xe0, 0x7e, 0x3f, 0x1f,
    0x8e, 0xfe, 0x3e, 0x0,

    /* U+0034 "4" */
    0xf, 0x3, 0xc1, 0xf0, 0x7c, 0x1f, 0xd, 0xc3,
    0x71, 0xdc, 0x67, 0x19, 0xce, 0x73, 0xff, 0xff,
    0xc1, 0xc0, 0x70, 0x1c, 0x7, 0x0,

    /* U+0035 "5" */
    0xfe, 0xfe, 0xe0, 0xe0, 0xe0, 0xe0, 0xfc, 0xfe,
    0xe7, 0x7, 0x7, 0x7, 0xe7, 0xe7, 0xe7, 0x7e,
    0x3c,

    /* U+0036 "6" */
    0x3c, 0x7e, 0xe7, 0xe7, 0xe7, 0xe0, 0xe0, 0xfe,
    0xfe, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0x7e,
    0x3c,

    /* U+0037 "7" */
    0xff, 0xfc, 0x38, 0x70, 0xc1, 0x87, 0xe, 0x1c,
    0x30, 0x61, 0xc3, 0x87, 0xc, 0x38, 0x70,

    /* U+0038 "8" */
    0x3c, 0x7e, 0xe7, 0xe7, 0xe7, 0xe7, 0x66, 0x3e,
    0x7e, 0x66, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0x7e,
    0x3c,

    /* U+0039 "9" */
    0x3c, 0x7e, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7,
    0x7f, 0x7f, 0x7, 0x7, 0xe7, 0xe7, 0xe7, 0x7e,
    0x3c
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 69, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 2, .adv_w = 171, .box_w = 9, .box_h = 17, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 22, .adv_w = 122, .box_w = 6, .box_h = 17, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 35, .adv_w = 159, .box_w = 9, .box_h = 17, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 55, .adv_w = 159, .box_w = 9, .box_h = 17, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 75, .adv_w = 161, .box_w = 10, .box_h = 17, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 97, .adv_w = 158, .box_w = 8, .box_h = 17, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 114, .adv_w = 167, .box_w = 8, .box_h = 17, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 131, .adv_w = 132, .box_w = 7, .box_h = 17, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 146, .adv_w = 163, .box_w = 8, .box_h = 17, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 163, .adv_w = 167, .box_w = 8, .box_h = 17, .ofs_x = 1, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint8_t glyph_id_ofs_list_0[] = {
    0, 0, 1, 2, 3, 4, 5, 6,
    7, 8, 9, 10
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 46, .range_length = 12, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = glyph_id_ofs_list_0, .list_length = 12, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Pair left and right glyphs for kerning*/
static const uint8_t kern_pair_glyph_ids[] =
{
    2, 9,
    3, 9,
    4, 6,
    6, 3,
    6, 9,
    7, 9,
    8, 3,
    8, 4,
    8, 9,
    8, 11,
    9, 6,
    10, 9,
    11, 9
};

/* Kerning between the respective left and right glyphs
 * 4.4 format which needs to scaled with `kern_scale`*/
static const int8_t kern_pair_values[] =
{
    -2, -3, -4, -4, -4, -4, -2, -2,
    -3, -3, -9, -4, -5
};

/*Collect the kern pair's data in one place*/
static const lv_font_fmt_txt_kern_pair_t kern_pairs =
{
    .glyph_ids = kern_pair_glyph_ids,
    .values = kern_pair_values,
    .pair_cnt = 13,
    .glyph_ids_size = 0
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_pairs,
    .kern_scale = 16,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif

};

extern const lv_font_t lv_font_montserrat_14;


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t oswald_medium_20 = {
#else
lv_font_t oswald_medium_20 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 17,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif
    .static_bitmap = 0,
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = &lv_font_montserrat_14,
#endif
    .user_data = NULL,
};



#endif /*#if OSWALD_MEDIUM_20*/
