#include "selector.h"
#include "merger.h"


/**
 * @brief 基于排序的两趟算法 
 * 
 * @param mergerOptions 连接选项
 * @param uiDBLKLowNumS 集合S的低块号
 * @param uiDBLKHighNumS 集合S的高块号
 * @param uiDBLKLowNumR 集合R的低块号
 * @param uiDBLKHighNumR 集合R的高块号
 * @param pBuf 内存缓冲区
 * @return uINT 元素个数 - 交、并、差 均为元组，连接为连接数
 */
uINT sortMerge(mergerOptions_t mergerOptions, uINT uiDBLKLowNumS, uINT uiDBLKHighNumS, uINT uiDBLKLowNumR, uINT uiDBLKHighNumR, uINT *puiNum, pBuffer pBuf){
    querySelector_t         querySelector;

    puChar                  puWriteBlk;
    uINT                    uiWriteBBLKNum;
    uINT                    uiWriteCurIndex;

    uINT                    uiDBLKNumS;
    uINT                    uiDBLKNumR;

    puChar                  puBlkS;
    uINT                    uiBBLKNumS;
    record_t                recordS;
    uINT                    uiKeyS;

    puChar                  puBlkR;
    uINT                    uiBBLKNumR;
    record_t                recordR;
    uINT                    uiKeyR;

    bool                    bCanBreak;
    bool                    bHasSame;
    uINT                    uiOpCnt;
    record_t                record;

    dResetGlobNextBLKNum();
    *puiNum = 0;
    uiOpCnt = 0;
    querySelector.uiAttrNum = mergerOptions.uiAttrNumR;
    querySelector.uiBasePos = SM_TEMP_R_POS;
    dSetGlobNextBLKNum(SM_TEMP_R_POS);
    tpmms(querySelector, uiDBLKLowNumR, uiDBLKHighNumR, TRUE, pBuf);

    querySelector.uiAttrNum = mergerOptions.uiAttrNumS;
    querySelector.uiBasePos = SM_TEMP_S_POS;
    dSetGlobNextBLKNum(SM_TEMP_S_POS);
    tpmms(querySelector, uiDBLKLowNumS, uiDBLKHighNumS, TRUE, pBuf);        /* 升序排列 */

    dSetGlobNextBLKNum(SM_POS);
    puWriteBlk      = getNewBlockInBuffer(pBuf);
    uiWriteBBLKNum  = bConvertBLKAddr2Num(puWriteBlk, pBuf);    
    uiWriteCurIndex = 0;
    bClearBLK(uiWriteBBLKNum, pBuf);

    uiDBLKNumS = SM_TEMP_S_POS + TABLE_S_NBLK;
    uiDBLKNumR = SM_TEMP_R_POS + TABLE_R_NBLK;
    
    printf("--------\n");
    dCheckBLKs(uiDBLKNumS, uiDBLKNumS + 31, pBuf);
    printf("--------\n");
    dCheckBLKs(uiDBLKNumR, uiDBLKNumR + 15, pBuf);

    for (size_t i = 0; i < TABLE_S_NBLK; i++)
    {
        uiDBLKNumS = SM_TEMP_S_POS + TABLE_S_NBLK + i;
        for (uINT uiIndexS = 0; uiIndexS < BLK_NRECORD; uiIndexS++)
        {
            puBlkS     = readBlockFromDisk(uiDBLKNumS, pBuf);
            uiBBLKNumS = bConvertBLKAddr2Num(puBlkS, pBuf);
            recordS    = bGetBLKRecord(uiBBLKNumS, uiIndexS, pBuf);
            uiKeyS     = getKeyAttr(recordS, mergerOptions.uiAttrNumS);

            bCanBreak  = FALSE;
            bHasSame   = FALSE;

            for (size_t j = 0; j < TABLE_R_NBLK; j++)
            {
                uiDBLKNumR = SM_TEMP_R_POS + TABLE_R_NBLK + j;
                for (uINT uiIndexR = 0; uiIndexR < BLK_NRECORD; uiIndexR++)
                {
                    puBlkR      = readBlockFromDisk(uiDBLKNumR, pBuf);
                    uiBBLKNumR  = bConvertBLKAddr2Num(puBlkR, pBuf);
                    recordR     = bGetBLKRecord(uiBBLKNumR, uiIndexR, pBuf);
                    uiKeyR      = getKeyAttr(recordR, mergerOptions.uiAttrNumS);
                    switch (mergerOptions.mergerType)
                    {
                    case JOIN:{
                        if(uiKeyR == uiKeyS) {
                            uiOpCnt++;
                            bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, recordS, pBuf);
                            uiWriteCurIndex++;
                            if(uiWriteCurIndex == BLK_NRECORD){
                                dWriteBLK(uiWriteBBLKNum, 1, pBuf);                     /* 写入后Buffer会被清除 */
                                puWriteBlk      = getNewBlockInBuffer(pBuf);            /* 重新申请写入块 */
                                uiWriteBBLKNum  = bConvertBLKAddr2Num(puWriteBlk, pBuf);
                                bClearBLK(uiWriteBBLKNum, pBuf);
                                uiWriteCurIndex = 0;
                                *puiNum = *puiNum + 1;
                            }
                            bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, recordR, pBuf);
                            uiWriteCurIndex++;
                            if(uiWriteCurIndex == BLK_NRECORD){
                                dWriteBLK(uiWriteBBLKNum, 1, pBuf);                     /* 写入后Buffer会被清除 */
                                puWriteBlk      = getNewBlockInBuffer(pBuf);            /* 重新申请写入块 */
                                uiWriteBBLKNum  = bConvertBLKAddr2Num(puWriteBlk, pBuf);
                                bClearBLK(uiWriteBBLKNum, pBuf);
                                uiWriteCurIndex = 0;
                                *puiNum = *puiNum + 1;
                            }
                        }
                        else if(uiKeyR > uiKeyS){                                        /* R排了序，可以直接跳过了 */
                            bCanBreak = TRUE;
                        }
                    }
                        break;
                    case INTER:{
                        if(recordR.attr1 == recordS.attr1 && recordR.attr2 == recordS.attr2){
                            bCanBreak = TRUE;
                        }
                        else {
                            uiOpCnt++;
                            bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, recordS, pBuf);
                            uiWriteCurIndex++;
                            if(uiWriteCurIndex == BLK_NRECORD){
                                dWriteBLK(uiWriteBBLKNum, 1, pBuf);                     /* 写入后Buffer会被清除 */
                                puWriteBlk      = getNewBlockInBuffer(pBuf);            /* 重新申请写入块 */
                                uiWriteBBLKNum  = bConvertBLKAddr2Num(puWriteBlk, pBuf);
                                bClearBLK(uiWriteBBLKNum, pBuf);
                                uiWriteCurIndex = 0;
                                *puiNum = *puiNum + 1;
                            }
                            bCanBreak = TRUE;
                        }
                    }
                        break;
                    case UNION:
                    case DIFF: {
                        if((recordR.attr1 == recordS.attr1 && recordR.attr2 == recordS.attr2)){
                            bHasSame    = TRUE;
                            bCanBreak   = TRUE;
                        }
                        else if(uiKeyR > uiKeyS){
                            bCanBreak   = TRUE;
                        }
                    }
                        break;
                    default:
                        break;
                    }
                    
                    freeBlockInBuffer(puBlkR, pBuf);
                    if(bCanBreak) {
                        break;
                    }
                }
                if(bCanBreak){
                    break;
                }
            }
            if(!bHasSame){
                switch (mergerOptions.mergerType)
                {
                case UNION:
                case DIFF:{
                    uiOpCnt++;
                    bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, recordS, pBuf);
                    uiWriteCurIndex++;
                    if(uiWriteCurIndex == BLK_NRECORD){
                        dWriteBLK(uiWriteBBLKNum, 1, pBuf);                     /* 写入后Buffer会被清除 */
                        puWriteBlk      = getNewBlockInBuffer(pBuf);            /* 重新申请写入块 */
                        uiWriteBBLKNum  = bConvertBLKAddr2Num(puWriteBlk, pBuf);
                        bClearBLK(uiWriteBBLKNum, pBuf);
                        uiWriteCurIndex = 0;
                        *puiNum = *puiNum + 1;
                    }
                }
                    break;
                default:
                    break;
                }
            }
            freeBlockInBuffer(puBlkS, pBuf);                                 /* 清除内存中读入的数据 */    
        }   
    }
    if(mergerOptions.mergerType == UNION) {
        for (size_t i = 0; i < TABLE_R_NBLK; i++)
        {
            uiDBLKNumR = SM_TEMP_R_POS + TABLE_R_NBLK + i;
            for (uINT uiIndexR = 0; uiIndexR < BLK_NRECORD; uiIndexR++)
            {
                puBlkR      = readBlockFromDisk(uiDBLKNumR, pBuf);
                uiBBLKNumR  = bConvertBLKAddr2Num(puBlkR, pBuf);
                recordR     = bGetBLKRecord(uiBBLKNumR, uiIndexR, pBuf);

                uiOpCnt++;
                bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, recordR, pBuf);
                uiWriteCurIndex++;
                if(uiWriteCurIndex == BLK_NRECORD){
                    dWriteBLK(uiWriteBBLKNum, 1, pBuf);                     /* 写入后Buffer会被清除 */
                    puWriteBlk      = getNewBlockInBuffer(pBuf);            /* 重新申请写入块 */
                    uiWriteBBLKNum  = bConvertBLKAddr2Num(puWriteBlk, pBuf);
                    bClearBLK(uiWriteBBLKNum, pBuf);
                    uiWriteCurIndex = 0;
                    *puiNum = *puiNum + 1;
                }
                freeBlockInBuffer(puBlkR, pBuf);
            }
        }
    }
    
    if(uiWriteCurIndex != 0) {
        dWriteBLK(uiWriteBBLKNum, 1, pBuf);
        *puiNum = *puiNum + 1;
    }
    else {
        freeBlockInBuffer(puWriteBlk, pBuf);
    }

    checkBuffer(pBuf);
    return uiOpCnt;
}
