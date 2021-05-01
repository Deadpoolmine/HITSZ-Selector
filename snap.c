#ifdef SNAP
INIT_WRITE_BLK();
INIT_IO_COUNTER();
uiDBLKNumLastR = 0;

for (size_t i = 0; i < TABLE_S_NBLK; i++)
{
    uiDBLKNumS = SM_TEMP_S_POS + TABLE_S_NBLK + i;
    for (uINT uiIndexS = 0; uiIndexS < BLK_NRECORD; uiIndexS++)
    {
        puBlkS     = readBlockFromDisk(uiDBLKNumS, pBuf);
        uiBBLKNumS = bConvertBLKAddr2Num(puBlkS, pBuf);
        recordS    = bGetBLKRecord(uiBBLKNumS, uiIndexS, pBuf);
        uiKeyS     = getKeyAttr(recordS, mergerOptions.uiAttrNumS);

        bCanBreak  = FALSE;                                             /* 是否读S的下一条记录 */
        bHasSame   = FALSE;

        for (size_t j = 0; j < TABLE_R_NBLK; j++)
        {
            uiDBLKNumR = SM_TEMP_R_POS + TABLE_R_NBLK + j;
            if(uiDBLKNumR < uiDBLKNumLastR){                            /* 直接跳到第一个可比块 */
                continue;
            }
            for (uINT uiIndexR = 0; uiIndexR < BLK_NRECORD; uiIndexR++)
            {
                puBlkR      = readBlockFromDisk(uiDBLKNumR, pBuf);
                uiBBLKNumR  = bConvertBLKAddr2Num(puBlkR, pBuf);
                recordR     = bGetBLKRecord(uiBBLKNumR, uiIndexR, pBuf);
                uiKeyR      = getKeyAttr(recordR, mergerOptions.uiAttrNumS);
               if(uiKeyR == uiKeyS) {
                    uiOpCnt++;
                    
                    bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, recordS, pBuf);
                    uiWriteCurIndex++;
                    WRITE_TILL_BLK_FILL(*puiNum = *puiNum + 1);

                    bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, recordR, pBuf);
                    uiWriteCurIndex++;
                    WRITE_TILL_BLK_FILL(*puiNum = *puiNum + 1);
                }
                else if(uiKeyR > uiKeyS){                                        /* R排了序，可以直接跳过了 */
                    bCanBreak = TRUE;
                }
                else if(uiKeyR < uiKeyS){
                    if(mergerOptions.bIsOptimise)
                        uiDBLKNumLastR = uiDBLKNumR;
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
        freeBlockInBuffer(puBlkS, pBuf);                                 /* 清除内存中读入的数据 */    
    }   
}
if(uiWriteCurIndex != 0) {
    dWriteBLK(uiWriteBBLKNum, 1, pBuf);
    *puiNum = *puiNum + 1; 
}
else {
    freeBlockInBuffer(puWriteBlk, pBuf);
}
DISPLAY_IO_CNT();






#endif // DEBUG

