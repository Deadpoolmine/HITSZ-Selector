#include "utils.h"

/**
 * @brief ��Buffer��BLK��uiBBLKLowNum��uiBBLKHighNum��BLK����ĳ�ַ�ʽ����
 * 
 * @param uiBBLKLowNum Buffer BLK�Ϳ�� 
 * @param uiBBLKHighNum Buffer BLK�߿��
 * @param bIsAscend �Ƿ���������
 * @param uiAttrNum �����ĸ��ֶ�
 * @param pBuf �ڴ滺����
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
                    if(!(uiBBLKNumi == uiBBLKNumj && uiIndexi == uiIndexj)){            /* ��֤�ظ����� */
                        recordi = bGetBLKRecord(uiBBLKNumi, uiIndexi, pBuf);
                        recordj = bGetBLKRecord(uiBBLKNumj, uiIndexj, pBuf);
                        
                        uiKeyi  = getKeyAttr(recordi, uiAttrNum);
                        uiKeyj  = getKeyAttr(recordj, uiAttrNum);

                        if(bIsAscend){                                                  /* �������� */
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
                            if(uiKeyi > uiKeyj){                                        /* �������� */
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