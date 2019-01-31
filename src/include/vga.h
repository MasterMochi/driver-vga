/******************************************************************************/
/* src/vga.h                                                                  */
/*                                                                 2018/08/26 */
/* Copyright (C) 2018 Mochi.                                                  */
/******************************************************************************/
#ifndef VGA_H
#define VGA_H
/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/*--------------------*/
/* テキストモード属性 */
/*--------------------*/
/* 文字属性 */
#define VGA_TEXT_ATTR_FG_BLACK  ( 0x00 )    /** 黒色文字属性 */
#define VGA_TEXT_ATTR_FG_BLUE   ( 0x01 )    /** 青色文字属性 */
#define VGA_TEXT_ATTR_FG_GREEN  ( 0x02 )    /** 緑色文字属性 */
#define VGA_TEXT_ATTR_FG_CYAN   ( 0x03 )    /** 水色文字属性 */
#define VGA_TEXT_ATTR_FG_RED    ( 0x04 )    /** 赤色文字属性 */
#define VGA_TEXT_ATTR_FG_PURPLE ( 0x05 )    /** 紫色文字属性 */
#define VGA_TEXT_ATTR_FG_BROWN  ( 0x06 )    /** 茶色文字属性 */
#define VGA_TEXT_ATTR_FG_WHITE  ( 0x07 )    /** 白色文字属性 */
#define VGA_TEXT_ATTR_FG_BRIGHT ( 0x08 )    /** 明色文字属性 */

/* 背景属性 */
#define VGA_TEXT_ATTR_BG_BLACK  ( 0x00 )    /** 黒色背景属性 */
#define VGA_TEXT_ATTR_BG_BLUE   ( 0x10 )    /** 青色背景属性 */
#define VGA_TEXT_ATTR_BG_GREEN  ( 0x20 )    /** 緑色背景属性 */
#define VGA_TEXT_ATTR_BG_CYAN   ( 0x30 )    /** 水色背景属性 */
#define VGA_TEXT_ATTR_BG_RED    ( 0x40 )    /** 赤色背景属性 */
#define VGA_TEXT_ATTR_BG_PURPLE ( 0x50 )    /** 紫色背景属性 */
#define VGA_TEXT_ATTR_BG_BROWN  ( 0x60 )    /** 茶色背景属性 */
#define VGA_TEXT_ATTR_BG_WHITE  ( 0x70 )    /** 白色背景属性 */
#define VGA_TEXT_ATTR_BLINK     ( 0x80 )    /** 点滅文字属性 */

/** 文字色取得マクロ */
#define VGA_TEXT_ATTR_FG( __BASE ) ( ( __BASE ) & 0x0F )

/** 背景色取得マクロ */
#define VGA_TEXT_ATTR_BG( __BASE ) ( ( __BASE ) & 0xF0 )

/** 文字色変更マクロ */
#define VGA_TEXT_ATTR_FG_CHG( __BASE, __FG ) \
    ( VGA_TEXT_ATTR_BG( __BASE ) | ( __FG ) )

/** 背景色変更マクロ */
#define VGA_TEXT_ATTR_BG_CHG( __BASE, __BG ) \
    ( VGA_TEXT_ATTR_FG( __BASE ) | ( __BG ) )


/******************************************************************************/
#endif
