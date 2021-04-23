#include "selector.h"

void linearSearch(querySelector_t querySelector, uINT uiDBLKLowNum, uINT uiDBLKHighNum, pBuffer pBuf) {
    uINT        uiBBLKNum;
    uINT        uiNum;
    
    puChar      puBlk;
    puChar      puWriteBlk;
    uINT        uiWriteBBLKNum;
    uINT        uiWriteCurIndex;

    record_t    record;

    uiNum               = uiDBLKHighNum - uiDBLKLowNum + 1;
    puWriteBlk          = getNewBlockInBuffer(pBuf);
    uiWriteBBLKNum      = bConvertBLKAddr2Num(puWriteBlk, pBuf);
    uiWriteCurIndex     = 0;
    bClearBLK(uiWriteBBLKNum, pBuf);

    for (size_t i = 0; i < uiNum; i++)
    {
        puBlk = readBlockFromDisk(uiDBLKLowNum + i, pBuf);              /* 读一块至内存 */
        uiBBLKNum = bConvertBLKAddr2Num(puBlk, pBuf);
        for (uINT uiIndex = 0; uiIndex < BLK_NRECORD; uiIndex++)
        {
            record = bGetBLKRecord(uiBBLKNum, uiIndex, pBuf);
            if(uiWriteCurIndex == BLK_NRECORD - 1) {                    /* 写满一个BLK就该写入磁盘了 */
                dWriteBLK(uiWriteBBLKNum, 1, pBuf);                     /* 写入后Buffer会被清除 */
                puWriteBlk      = getNewBlockInBuffer(pBuf);            /* 重新申请写入块 */
                uiWriteBBLKNum  = bConvertBLKAddr2Num(puWriteBlk, pBuf);
                bClearBLK(uiWriteBBLKNum, pBuf);
                uiWriteCurIndex = 0;
            }
            switch (querySelector.uiColNum)
            {
            case 1:{
                if(record.attr1 == querySelector.uiValue){
                    bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex,record, pBuf);
                    uiWriteCurIndex++;           
                }
            }
                break;
            case 2:{
                if(record.attr2 == querySelector.uiValue){
                    bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex,record, pBuf);
                    uiWriteCurIndex++;
                }
            }
                break;
            default:
                break;
            }   
        }
        freeBlockInBuffer(puBlk, pBuf);                                 /* 清除内存中读入的数据 */
    }
    if(uiWriteCurIndex != 0) {
        dWriteBLK(uiWriteBBLKNum, 1, pBuf);
    }
    
    checkBuffer(pBuf);                                                  /* 查看I/O情况 */
}