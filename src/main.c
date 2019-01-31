/******************************************************************************/
/* src/main.c                                                                 */
/*                                                                 2019/01/31 */
/* Copyright (C) 2018-2019 Mochi.                                             */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdbool.h>
#include <string.h>
#include <kernel/library.h>

/* モジュールヘッダ */
#include "drv-vga.h"
#include "Vram.h"


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
void main( void )
{
    char           buffer[ MK_MSG_SIZE_MAX + 1 ];   /* 受信メッセージバッファ */
    int32_t        size;                            /* 受信メッセージサイズ   */
    uint32_t       errNo;                           /* エラー番号             */
    DrvVgaMsgHdr_t *pMsg;                           /* メッセージ             */

    /* 初期化 */
    memset( buffer, 0, sizeof ( buffer ) );
    size  = MK_MSG_RET_FAILURE;
    errNo = MK_MSG_ERR_NONE;
    pMsg  = ( DrvVgaMsgHdr_t * ) buffer;

    /* VRAM初期化 */
    VramInit();

    /* メインループ */
    while ( true ) {
        /* メッセージ受信 */
        size = MkMsgReceive( MK_CONFIG_TASKID_NULL,     /* 受信待ちタスクID   */
                             pMsg,                      /* メッセージバッファ */
                             MK_MSG_SIZE_MAX,           /* バッファサイズ     */
                             NULL,                      /* 送信元タスクID     */
                             &errNo                 );  /* エラー番号         */

        /* メッセージ受信結果判定 */
        if ( size == MK_MSG_RET_FAILURE ) {
            /* 失敗 */

            continue;
        }

        /* 機能ID判定 */
        if ( pMsg->funcId == DRVVGA_FUNC_WRITE ) {
            /* VRAM書き込み */
            VramWrite( ( DrvVgaMsgWrite_t * ) pMsg );
        }
    }
}


/******************************************************************************/
