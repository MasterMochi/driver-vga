/******************************************************************************/
/* src/TextCtrl.c                                                             */
/*                                                                 2018/09/06 */
/* Copyright (C) 2018 Mochi.                                                  */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdint.h>
#include <stddef.h>

/* 外部モジュールヘッダ */
#include "vga.h"
#include "Vram.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** テキスト制御情報型 */
typedef struct {
    int32_t row;    /**< 行   */
    int32_t column; /**< 列   */
    uint8_t attr;   /**< 属性 */
} info_t;

/** CSI機能関数型 */
typedef void ( *escCsiFunc_t )( uint32_t n,
                                uint32_t m  );

/** CSI機能関数テーブル */
typedef struct {
    char         code;  /**< 機能コード */
    escCsiFunc_t func;  /**< 機能関数   */
} escCsiTbl_t;


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/* エスケープシーケンス(CSI)パラメータ取得 */
static uint32_t GetEscapeCsiParam( char     *pStr,
                                   uint32_t *pN,
                                   uint32_t *pM    );

/* カーソル位置最適化 */
static void OptimizeCursor( void );

/* エスケープシーケンス処理 */
static uint32_t ProcEscape( char *pStr );

/* カーソル移動（列） */
static void ProcEscapeCsiCha( uint32_t n,
                              uint32_t m  );

/* カーソル移動（下先頭） */
static void ProcEscapeCsiCnl( uint32_t n,
                              uint32_t m  );

/* カーソル移動（上先頭） */
static void ProcEscapeCsiCpl( uint32_t n,
                              uint32_t m  );

/* カーソル移動（左） */
static void ProcEscapeCsiCub( uint32_t n,
                              uint32_t m  );

/* カーソル移動（下） */
static void ProcEscapeCsiCud( uint32_t n,
                              uint32_t m  );

/* カーソル移動（右） */
static void ProcEscapeCsiCuf( uint32_t n,
                              uint32_t m  );

/* カーソル移動（任意） */
static void ProcEscapeCsiCup( uint32_t n,
                              uint32_t m  );

/* カーソル移動（上） */
static void ProcEscapeCsiCuu( uint32_t n,
                              uint32_t m  );

/* 全消去 */
static void ProcEscapeCsiEd( uint32_t n,
                             uint32_t m  );

/* 行消去 */
static void ProcEscapeCsiEl( uint32_t n,
                             uint32_t m  );

/* 文字属性設定 */
static void ProcEscapeCsiSgr( uint32_t n,
                              uint32_t m  );


/******************************************************************************/
/* グローバル変数定義                                                         */
/******************************************************************************/
/** テキスト制御情報 */
static info_t gInfo;

/** CSI機能関数テーブル */
static escCsiTbl_t gEscCsiTbl[] = {
    { 'A', ProcEscapeCsiCuu },      /* カーソル移動（上）     */
    { 'B', ProcEscapeCsiCud },      /* カーソル移動（下）     */
    { 'C', ProcEscapeCsiCuf },      /* カーソル移動（右）     */
    { 'D', ProcEscapeCsiCub },      /* カーソル移動（左）     */
    { 'E', ProcEscapeCsiCnl },      /* カーソル移動（下先頭） */
    { 'F', ProcEscapeCsiCpl },      /* カーソル移動（上先頭） */
    { 'G', ProcEscapeCsiCha },      /* カーソル移動（列）     */
    { 'H', ProcEscapeCsiCup },      /* カーソル移動（任意）   */
    { 'J', ProcEscapeCsiEd  },      /* 全消去                 */
    { 'K', ProcEscapeCsiEl  },      /* 行消去                 */
    { 'f', ProcEscapeCsiCup },      /* カーソル移動（任意）   */
    { 'm', ProcEscapeCsiSgr },      /* 文字属性設定           */
    { 0,   NULL              }  };  /* 終端                   */


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       テキスト制御初期化
 * @details     テキスト制御情報と画面を初期化する。
 */
/******************************************************************************/
void TextCtrlInit( void )
{
    uint32_t row;       /* 行カウンタ */
    uint32_t column;    /* 列カウンタ */
    
    /* テキスト制御情報初期化 */
    gInfo.row    = 1;
    gInfo.column = 1;
    gInfo.attr   = VGA_TEXT_ATTR_FG_WHITE  |    /* 白色文字属性 */
                   VGA_TEXT_ATTR_FG_BRIGHT |    /* 明色文字属性 */
                   VGA_TEXT_ATTR_BG_BLACK;      /* 黒色背景属性 */
    
    /* 画面初期化 */
    for ( row = 1; row <= 25; row++ ) {
        for ( column = 1; column <= 80; column++ ) {
            /* VRAM書き込み */
            VramWrite( row, column, ' ', gInfo.attr );
        }
    }
    
    return;
}


/******************************************************************************/
/**
 * @brief       テキスト出力
 * @details     テキストをVRAMに出力する。
 * 
 * @param[in]   pStr テキスト
 * 
 * @note        テキストは終端文字を入れること。
 */
/******************************************************************************/
void TextCtrlOutput( char *pStr )
{
    uint32_t index; /* インデックス */
    
    /* 初期化 */
    index = 0;
    
    /* 一文字毎に繰り返し */
    while ( pStr[ index ] != '\0' ) {
        /* 文字判定 */
        switch ( pStr[ index ] ) {
            case '\033':
                /* ESC */
                
                /* エスケープシーケンス処理 */
                index++;
                index += ProcEscape( &pStr[ index ] );
                
                break;
                
            default:
                /* その他 */
                
                /* VRAM書き込み */
                VramWrite( ( uint32_t ) gInfo.row,          /* 行   */
                           ( uint32_t ) gInfo.column,       /* 列   */
                           pStr[ index ],                   /* 文字 */
                           gInfo.attr                 );    /* 属性 */
                
                index++;
        }
    }
    
    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       エスケープシーケンス(CSI)パラメータ取得
 * @details     エスケープシーケンスの制御シーケンスのパラメータを取得する。
 * 
 * @param[in]   *pStr エスケープシーケンスパラメータ
 * @param[in]   *pN   パラメータN
 * @param[in]   *pM   パラメータM
 * 
 * @return      パラメータ文字数
 */
/******************************************************************************/
static uint32_t GetEscapeCsiParam( char     *pStr,
                                   uint32_t *pN,
                                   uint32_t *pM    )
{
    uint32_t index; /* インデックス */
    
    /* 初期化 */
    index = 0;
    *pN   = 0;
    *pM   = 0;
    
    /* パラメータN取得 */
    while ( ( '0' <= pStr[ index ] ) && ( pStr[ index ] <= '9' ) ) {
        *pN = *pN * 10 + ( uint32_t ) ( pStr[ index ] - '0' );
        index++;
    }
    
    /* パラメータ区切り判定 */
    if ( pStr[ index ] != ';' ) {
        /* 区切り無し */
        return index;
    } else {
        /* 区切り有り */
        index++;
    }
    
    /* パラメータM取得 */
    while ( ( '0' <= pStr[ index ] ) && ( pStr[ index ] <= '9' ) ) {
        *pM = *pM * 10 + ( uint32_t ) ( pStr[ index ] - '0' );
        index++;
    }
    
    return index;
}


/******************************************************************************/
/**
 * @brief       カーソル位置最適化
 * @details     カーソル位置が最大値および最小値を超えない様に最適化する。
 */
/******************************************************************************/
static void OptimizeCursor( void )
{
    /* 行番号チェック */
    if ( gInfo.row > 25 ) {
        /* 最大値越え */
        
        gInfo.row = 25;
        
    } else if ( gInfo.row < 1 ) {
        /* 最小値越え */
        
        gInfo.row = 1;
    }
    
    /* 列番号チェック */
    if ( gInfo.column > 80 ) {
        /* 最大値越え */
        
        gInfo.column = 80;
        
    } else if ( gInfo.column < 1 ) {
        /* 最小値越え */
        
        gInfo.column = 1;
    }
    
    return;
}


/******************************************************************************/
/**
 * @brief       エスケープシーケンス処理
 * @details     制御シーケンス(Control Sequence Introducer)を処理する。
 * 
 * @param[in]   *pStr シーケンス文字列
 * 
 * @return      エスケープシーケンス文字数を返す。
 */
/******************************************************************************/
static uint32_t ProcEscape( char *pStr )
{
    uint32_t n;         /* パラメータ1                     */
    uint32_t m;         /* パラメータ2                     */
    uint32_t index;     /* 文字列インデックス              */
    uint32_t tblIdx;    /* CSI機能関数テーブルインデックス */
    
    /* 初期化 */
    n      = 0;
    m      = 0;
    tblIdx = 0;
    
    /* 制御シーケンス判定 */
    if ( pStr[ 0 ] != '[' ) {
        /* 非制御シーケンス */
        
        return 0;
    }
    
    /* パラメータ取得 */
    index += GetEscapeCsiParam( pStr, &n, &m );
    
    /* 機能検索 */
    while ( gEscCsiTbl[ tblIdx ].code != 0 ) {
        /* コード判定 */
        if ( gEscCsiTbl[ tblIdx ].code == pStr[ index ] ) {
            /* 一致 */
            
            /* 機能呼出し */
            ( gEscCsiTbl[ tblIdx ].func )( n, m );
            
            return ( index + 1 );
        }
        
        tblIdx++;
    }
    
    return 1;
}


/******************************************************************************/
/**
 * @brief       カーソル移動（列）
 * @details     カーソルを指定した列番号に移動する。
 * 
 * @param[in]   n 指定列
 * @param[in]   m 未使用
 * 
 * @note        CHA( Cursor Horizontal Absolute )
 */
/******************************************************************************/
static void ProcEscapeCsiCha( uint32_t n,
                              uint32_t m  )
{
    /* カーソル設定 */
    gInfo.column = n;
    
    /* カーソル位置最適化 */
    OptimizeCursor();
    
    return;
}


/******************************************************************************/
/**
 * @brief       カーソル移動（下先頭）
 * @details     カーソルを現在位置から指定した行数分下に移動し、更に列番号を先
 *              頭に移動する。
 * 
 * @param[in]   n 行数
 * @param[in]   m 未使用
 * 
 * @note        CNL( Cursor Next Line )
 */
/******************************************************************************/
static void ProcEscapeCsiCnl( uint32_t n,
                              uint32_t m  )
{
    /* カーソル設定 */
    gInfo.row    += ( int32_t ) n;  /* 行番号 */
    gInfo.column  = 1;              /* 列番号 */
    
    /* カーソル位置最適化 */
    OptimizeCursor();
    
    return;
}


/******************************************************************************/
/**
 * @brief       カーソル移動（上先頭）
 * @details     カーソルを現在位置から指定した行数分上に移動し、更に列番号を先
 *              頭に移動する。
 * 
 * @param[in]   n 行数
 * @param[in]   m 未使用
 * 
 * @note        CPL( Cursor Previous Line )
 */
/******************************************************************************/
static void ProcEscapeCsiCpl( uint32_t n,
                              uint32_t m  )
{
    /* カーソル設定 */
    gInfo.row    -= ( int32_t ) n;  /* 行番号 */
    gInfo.column  = 1;              /* 列番号 */
    
    /* カーソル位置最適化 */
    OptimizeCursor();
    
    return;
}


/******************************************************************************/
/**
 * @brief       カーソル移動（左）
 * @details     カーソルを現在位置から左に移動する。
 * 
 * @param[in]   n 移動数
 * @param[in]   m 未使用
 * 
 * @note        CUB( Cursor Back )
 */
/******************************************************************************/
static void ProcEscapeCsiCub( uint32_t n,
                              uint32_t m  )
{
    /* カーソル設定 */
    gInfo.column -= ( int32_t ) n;  /* 列番号 */
    
    /* カーソル位置最適化 */
    OptimizeCursor();
    
    return;
}


/******************************************************************************/
/**
 * @brief       カーソル移動（下）
 * @details     カーソルを現在位置から指定した行数分下に移動する。
 * 
 * @param[in]   n 行数
 * @param[in]   m 未使用
 * 
 * @note        CUD( Cursor Down )
 */
/******************************************************************************/
static void ProcEscapeCsiCud( uint32_t n,
                              uint32_t m  )
{
    /* カーソル設定 */
    gInfo.row += ( int32_t ) n; /* 行番号 */
    
    /* カーソル位置最適化 */
    OptimizeCursor();
    
    return;
}


/******************************************************************************/
/**
 * @brief       カーソル移動（右）
 * @details     カーソルを現在位置から右に移動する。
 * 
 * @param[in]   n 移動数
 * @param[in]   m 未使用
 * 
 * @note        CUF( Cursor Forward )
 */
/******************************************************************************/
static void ProcEscapeCsiCuf( uint32_t n,
                              uint32_t m  )
{
    /* カーソル設定 */
    gInfo.column += ( int32_t ) n;  /* 列番号 */
    
    /* カーソル位置最適化 */
    OptimizeCursor();
    
    return;
}


/******************************************************************************/
/**
 * @brief       カーソル移動（任意）
 * @details     カーソルを指定した行と列に移動する。
 * 
 * @param[in]   n 行
 * @param[in]   m 列
 * 
 * @note        CUP( Cursor Position )
 */
/******************************************************************************/
static void ProcEscapeCsiCup( uint32_t n,
                              uint32_t m  )
{
    /* カーソル設定 */
    gInfo.row    = ( int32_t ) n;  /* 行番号 */
    gInfo.column = ( int32_t ) m;  /* 列番号 */
    
    /* カーソル位置最適化 */
    OptimizeCursor();
    
    return;
}


/******************************************************************************/
/**
 * @brief       カーソル移動（上）
 * @details     カーソルを現在位置から指定した行数分上に移動する。
 * 
 * @param[in]   n 移動数
 * @param[in]   m 未使用
 * 
 * @note        CUU( Cursor Up )
 */
/******************************************************************************/
static void ProcEscapeCsiCuu( uint32_t n,
                              uint32_t m  )
{
    /* カーソル設定 */
    gInfo.row -= ( int32_t ) n; /* 行番号 */
    
    /* カーソル位置最適化 */
    OptimizeCursor();
    
    return;
}


/******************************************************************************/
/**
 * @brief       全消去
 * @details     指定した消去方法で消去する。
 * 
 * @param[in]   n 消去方法
 *                  - 0 カーソルから最後まで
 *                  - 1 先頭からカーソルまで
 *                  - 2 全て
 * @param[in]   m 未使用
 * 
 * @note        ED( Erase in Display )
 */
/******************************************************************************/
static void ProcEscapeCsiEd( uint32_t n,
                             uint32_t m  )
{
    uint32_t row;       /* 消去行番号   */
    uint32_t column;    /* 消去列番号   */
    uint32_t end;       /* 消去終端位置 */
    
    /* 消去方法判定 */
    if ( n == 0 ) {
        /* カーソルから最後まで */
        
        row    = ( uint32_t ) gInfo.row;
        column = ( uint32_t ) gInfo.column;
        end    = 25 * 80 + 80;
        
    } else if ( n == 1 ) {
        /* 先頭からカーソルまで */
        
        row    = 1;
        column = 1;
        end    = ( uint32_t ) ( gInfo.row * 80 + gInfo.column );
        
    } else if ( n == 2 ) {
        /* 全て */
        
        row    = 1;
        column = 1;
        end    = 25 * 80 + 80;
        
    } else {
        /* 未定義 */
        
        return;
    }
    
    do {
        /* VRAM書き込み */
        VramWrite( row,             /* 行番号   */
                   column,          /* 列番号   */
                   ' ',             /* 文字     */
                   gInfo.attr );    /* 文字属性 */
        
        /* 列番号更新 */
        column++;
        
        /* 行番号更新 */
        if ( column > 80 ) {
            row++;
            column = 1;
        }
        
    /* 終端判定 */
    } while ( ( row * 80 + column ) <= end );
    
    return;
}


/******************************************************************************/
/**
 * @brief       行消去
 * @details     指定した消去方法で現在位置の行を消去する。
 * 
 * @param[in]   n 消去方法
 *                  - 0 カーソルから最後まで
 *                  - 1 先頭からカーソルまで
 *                  - 2 全て
 * @param[in]   m 未使用
 * 
 * @note        EL( Erase in Line )
 */
/******************************************************************************/
static void ProcEscapeCsiEl( uint32_t n,
                             uint32_t m  )
{
    uint32_t column;    /* 消去列番号   */
    uint32_t end;       /* 消去終端位置 */
    
    /* 消去方法判定 */
    if ( n == 0 ) {
        /* カーソルから最後まで */
        
        column = ( uint32_t ) gInfo.column;
        end    = 80;
        
    } else if ( n == 1 ) {
        /* 先頭からカーソルまで */
        
        column = 1;
        end    = ( uint32_t ) gInfo.column;
        
    } else if ( n == 2 ) {
        /* 全て */
        
        column = 1;
        end    = 80;
        
    } else {
        /* 未定義 */
        
        return;
    }
    
    do {
        /* VRAM書き込み */
        VramWrite( ( uint32_t ) gInfo.row,      /* 行番号   */
                   column,                      /* 列番号   */
                   ' ',                         /* 文字     */
                   gInfo.attr              );   /* 文字属性 */
        
        /* 列番号更新 */
        column++;
        
    /* 終端判定 */
    } while ( column <= end );
    
    return;
}


/******************************************************************************/
/**
 * @brief       文字属性設定
 * @details     指定した文字属性を設定する。以降の文字はこの文字属性により出力
 *              される。
 * 
 * @param[in]   n 文字属性
 * @param[in]   m 未使用
 * 
 * @note        SGR( Select Graphic Rendition )
 */
/******************************************************************************/
static void ProcEscapeCsiSgr( uint32_t n,
                              uint32_t m  )
{
    uint8_t attr;   /* 属性 */
    
    /* 文字属性判定 */
    switch ( n ) {
        case 0:
            /* 属性初期化 */
            
            gInfo.attr = VGA_TEXT_ATTR_FG_WHITE  |  /* 白色文字属性 */
                         VGA_TEXT_ATTR_FG_BRIGHT |  /* 明色文字属性 */
                         VGA_TEXT_ATTR_BG_BLACK;    /* 黒色背景属性 */
            
            break;
            
        case 7:
            /* 反転 */
            
            /* 反転 */
            attr  = ( VGA_TEXT_ATTR_BG( gInfo.attr ) >> 4 ) |
                    VGA_TEXT_ATTR_FG_BRIGHT;
            attr &= ( VGA_TEXT_ATTR_FG( gInfo.attr ) << 4 ) &
                    ~VGA_TEXT_ATTR_BLINK;
            
            /* 設定 */
            gInfo.attr = attr;
            
            break;
            
        case 30:
            /* 黒色文字 */
            gInfo.attr =
                VGA_TEXT_ATTR_FG_CHG(
                    gInfo.attr,
                    VGA_TEXT_ATTR_FG_BLACK | VGA_TEXT_ATTR_FG_BRIGHT );
            break;
            
        case 31:
            /* 赤色文字 */
            gInfo.attr =
                VGA_TEXT_ATTR_FG_CHG(
                    gInfo.attr,
                    VGA_TEXT_ATTR_FG_RED | VGA_TEXT_ATTR_FG_BRIGHT );
            break;
            
        case 32:
            /* 緑色文字 */
            gInfo.attr =
                VGA_TEXT_ATTR_FG_CHG(
                    gInfo.attr,
                    VGA_TEXT_ATTR_FG_GREEN | VGA_TEXT_ATTR_FG_BRIGHT );
            break;
            
        case 33:
            /* 黄色文字 */
            gInfo.attr =
                VGA_TEXT_ATTR_FG_CHG(
                    gInfo.attr,
                    VGA_TEXT_ATTR_FG_BROWN | VGA_TEXT_ATTR_FG_BRIGHT );
            break;
            
        case 34:
            /* 青色文字 */
            gInfo.attr =
                VGA_TEXT_ATTR_FG_CHG(
                    gInfo.attr,
                    VGA_TEXT_ATTR_FG_BLUE | VGA_TEXT_ATTR_FG_BRIGHT );
            break;
            
        case 35:
            /* 紫色文字 */
            gInfo.attr =
                VGA_TEXT_ATTR_FG_CHG(
                    gInfo.attr,
                    VGA_TEXT_ATTR_FG_PURPLE | VGA_TEXT_ATTR_FG_BRIGHT );
            break;
            
        case 36:
            /* 水色文字 */
            gInfo.attr =
                VGA_TEXT_ATTR_FG_CHG(
                    gInfo.attr,
                    VGA_TEXT_ATTR_FG_CYAN | VGA_TEXT_ATTR_FG_BRIGHT );
            break;
            
        case 37:
            /* 白色文字 *//* FALL THROUGH */
        case 39:
            /* 標準色文字 */
            gInfo.attr =
                VGA_TEXT_ATTR_FG_CHG(
                    gInfo.attr,
                    VGA_TEXT_ATTR_FG_WHITE | VGA_TEXT_ATTR_FG_BRIGHT );
            break;
            
        case 40:
            /* 黒色背景 *//* FALL THROUGH */
        case 49:
            /* 標準色背景 */
            gInfo.attr =
                VGA_TEXT_ATTR_BG_CHG(
                    gInfo.attr,
                    VGA_TEXT_ATTR_BG_BLACK );
            break;
            
        case 41:
            /* 赤色背景 */
            gInfo.attr =
                VGA_TEXT_ATTR_BG_CHG(
                    gInfo.attr,
                    VGA_TEXT_ATTR_BG_RED );
            break;
            
        case 42:
            /* 緑色背景 */
            gInfo.attr =
                VGA_TEXT_ATTR_BG_CHG(
                    gInfo.attr,
                    VGA_TEXT_ATTR_BG_RED );
            break;
            
        case 43:
            /* 黄色背景 */
            gInfo.attr =
                VGA_TEXT_ATTR_BG_CHG(
                    gInfo.attr,
                    VGA_TEXT_ATTR_BG_BROWN );
            break;
            
        case 44:
            /* 青色背景 */
            gInfo.attr =
                VGA_TEXT_ATTR_BG_CHG(
                    gInfo.attr,
                    VGA_TEXT_ATTR_BG_BLUE );
            break;
            
        case 45:
            /* 紫色背景 */
            gInfo.attr =
                VGA_TEXT_ATTR_BG_CHG(
                    gInfo.attr,
                    VGA_TEXT_ATTR_BG_PURPLE );
            break;
            
        case 46:
            /* 水色背景 */
            gInfo.attr =
                VGA_TEXT_ATTR_BG_CHG(
                    gInfo.attr,
                    VGA_TEXT_ATTR_BG_CYAN );
            break;
            
        case 47:
            /* 白色背景 */
            gInfo.attr =
                VGA_TEXT_ATTR_BG_CHG(
                    gInfo.attr,
                    VGA_TEXT_ATTR_BG_WHITE );
            break;
            
        default:
            /* 他 */
            break;
    }
    
    return;
}


/******************************************************************************/
