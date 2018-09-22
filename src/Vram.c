/******************************************************************************/
/* src/Vram.c                                                                 */
/*                                                                 2018/09/12 */
/* Copyright (C) 2018 Mochi.                                                  */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdint.h>
#include <string.h>
#include <kernel/library.h>

/* モジュール内ヘッダ */
#include "vga.h"


/******************************************************************************/
/* グローバル変数定義                                                         */
/******************************************************************************/
/** VRAM */
static uint8_t *pgVram = NULL;


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       VRAM初期化
 * @detail      VRAM領域を割り当てる。
 */
/******************************************************************************/
void VramInit( void )
{
    uint32_t errNo;     /* エラー番号 */
    uint32_t row;       /* 行番号     */
    uint32_t column;    /* 列番号     */
    
    /* 初期化 */
    errNo = MK_MSG_ERR_NONE;
    
    /* VRAM領域割当 */
    pgVram = MkIoMemAlloc( ( void * ) 0x000B8000, 0x00008000, &errNo );
    
    /* 割当結果判定 */
    if ( pgVram == NULL ) {
        /* 失敗 */
        
        /* [TODO]アボート */
        while( 1 );
    }
    
    /* VRAM初期化 */
    for ( row = 0; row < 25; row++ ) {
        for ( column = 0; column < 80; column++ ) {
            pgVram[ row * 160 + column * 2     ] = ' ';
            pgVram[ row * 160 + column * 2 + 1 ] =
                VGA_TEXT_ATTR_FG_WHITE  |   /* 白色文字属性 */
                VGA_TEXT_ATTR_FG_BRIGHT |   /* 明色文字属性 */
                VGA_TEXT_ATTR_BG_BLACK;     /* 黒色背景属性 */
        }
    }
    
    return;
}


/******************************************************************************/
/**
 * @brief       VRAM書込み
 * @details     VRAMに書き込む。
 * 
 * @param[in]   *pBuffer データ
 * @param[in]   size     データサイズ
 */
/******************************************************************************/
void VramWrite( uint8_t *pBuffer,
                size_t  size      )
{
    /* サイズチェック */
    if ( size > ( 25 * 80 * 2 ) ) {
        /* 上限越え */
        
        size = 25 * 80 * 2;
    }
    
    /* 書き込み */
    memcpy( pgVram, pBuffer, size );
    
    return;
}


/******************************************************************************/
