#include "utils.h"

/**
 * @brief 将Buffer中BLK号uiBBLKLowNum到uiBBLKHighNum的BLK按照某种方式排序
 * 
 * @param uiBBLKLowNum Buffer BLK低块号 
 * @param uiBBLKHighNum Buffer BLK高块号
 * @param bIsAscend 是否升序排列
 * @param uiAttrNum 基于哪个字段
 * @param pBuf 内存缓冲区
 */
void sortInBuf(uINT uiBBLKLowNum, uINT uiBBLKHighNum, bool bIsAscend, uINT uiAttrNum, pBuffer pBuf){
    uINT        uiNum;

    uINT        uiBBLKNumi;
    uINT        uiIndexi;
    uINT        uiKeyi;
    record_t    recordi;

    uINT        uiBBLKNumj;
    uINT        uiIndexj;
    uINT        uiKeyj;
    record_t    recordj;


    uiNum = uiBBLKHighNum - uiBBLKLowNum + 1;
    
    if(uiBBLKHighNum > BUF_NBLK) {
        printf(TIPS_ERROR "[ sortInBuf: try to sort [%d, %d] BLKs in Buffer [1, 8].]\n",
               uiBBLKLowNum, uiBBLKHighNum);
        return;
    }
    
    for (uiBBLKNumi = 1; uiBBLKNumi <= uiNum; uiBBLKNumi++)
    {
        for (uiIndexi = 0; uiIndexi < BLK_NRECORD; uiIndexi++)
        {
            for (uiBBLKNumj = 1; uiBBLKNumj <= uiNum; uiBBLKNumj++)
            {
                for (uiIndexj = 0; uiIndexj < BLK_NRECORD; uiIndexj++)
                {
                    if(!(uiBBLKNumi == uiBBLKNumj && uiIndexi == uiIndexj)){            /* 保证重复计算 */
                        recordi = bGetBLKRecord(uiBBLKNumi, uiIndexi, pBuf);
                        recordj = bGetBLKRecord(uiBBLKNumj, uiIndexj, pBuf);
                        
                        uiKeyi  = getKeyAttr(recordi, uiAttrNum);
                        uiKeyj  = getKeyAttr(recordj, uiAttrNum);

                        if(bIsAscend){                                                  /* 升序排列 */
                            if(uiKeyi < uiKeyj){
                                bSetBLKRecord(uiBBLKNumi, uiIndexi, recordj, pBuf);
                                bSetBLKRecord(uiBBLKNumj, uiIndexj, recordi, pBuf);
                            }
                            if(uiKeyi == uiKeyj){
                                uiKeyi = getAnotherKeyAttr(recordi, uiAttrNum);
                                uiKeyj = getAnotherKeyAttr(recordj, uiAttrNum);
                                if(uiKeyi < uiKeyj){
                                    bSetBLKRecord(uiBBLKNumi, uiIndexi, recordj, pBuf);
                                    bSetBLKRecord(uiBBLKNumj, uiIndexj, recordi, pBuf);
                                }
                            }
                        }
                        else {
                            if(uiKeyi > uiKeyj){                                        /* 降序排列 */
                                bSetBLKRecord(uiBBLKNumi, uiIndexi, recordj, pBuf);
                                bSetBLKRecord(uiBBLKNumj, uiIndexj, recordi, pBuf);
                            }
                            if(uiKeyi == uiKeyj){
                                uiKeyi = getAnotherKeyAttr(recordi, uiAttrNum);
                                uiKeyj = getAnotherKeyAttr(recordj, uiAttrNum);
                                if(uiKeyi > uiKeyj){
                                    bSetBLKRecord(uiBBLKNumi, uiIndexi, recordj, pBuf);
                                    bSetBLKRecord(uiBBLKNumj, uiIndexj, recordi, pBuf);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
}