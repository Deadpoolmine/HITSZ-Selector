#include "selector.h"


#define GET_CUR_DBLK_NUM(basePos, segmentIndex) basePos + segmentIndex * uiNumPerSegment \
                                                + puiDNextIndex[segmentIndex]

#define GET_CUR_BBLK_NUM(segmentIndex)          uiBaseSegmentBBLKNum + segmentIndex

#define GET_NEXT_RECORD(segmentIndex)           puiBNextIndex[segmentIndex]++
#define GET_CUR_RECORD_INDEX(segmentIndex)      puiBNextIndex[segmentIndex]
#define RESET_CUR_RECORD(segmentIndex)          puiBNextIndex[segmentIndex] = 0

#define GET_NEXT_DBLK(segmentIndex)             puiDNextIndex[segmentIndex]++
#define GET_CUR_DBLK_INDEX(segmentIndex)        puiDNextIndex[segmentIndex]
#define RESET_CUR_DBLK(segmentIndex)            puiDNextIndex[segmentIndex] = 0

/**
 * @brief ��������
 * 
 * @param querySelector ������ 
 * @param uiDBLKLowNum �����ϵͿ��
 * @param uiDBLKHighNum �����ϸ߿��
 * @param pBuf �ڴ滺����
 */
void linearSearch(querySelector_t querySelector, uINT uiDBLKLowNum, uINT uiDBLKHighNum, pBuffer pBuf) {
    uINT        uiBBLKNum;
    uINT        uiNum;
    
    puChar      puBlk;
    puChar      puWriteBlk;
    uINT        uiWriteBBLKNum;
    uINT        uiWriteCurIndex;

    record_t    record;

    uINT        uiRecordNum;

    
    DISPLAY_TIPS("��������������ѡ���㷨 - S.C = 50");
    uiNum               = uiDBLKHighNum - uiDBLKLowNum + 1;
    uiRecordNum         = 0;
    INIT_IO_COUNTER();
    INIT_WRITE_BLK();

    for (size_t i = 0; i < uiNum; i++)
    {
        puBlk = readBlockFromDisk(uiDBLKLowNum + i, pBuf);              /* ��һ�����ڴ� */
        uiBBLKNum = bConvertBLKAddr2Num(puBlk, pBuf);
#ifdef OUTPUT_ON
        printf("�������ݿ� %ld\n", uiDBLKLowNum + i);
#endif // OUTPUT_ON
        for (uINT uiIndex = 0; uiIndex < BLK_NRECORD; uiIndex++)
        {
            record = bGetBLKRecord(uiBBLKNum, uiIndex, pBuf);
            WRITE_TILL_BLK_FILL(printf(FONT_COLOR_RED "���д����̿� %ld \n" FONT_COLOR_END, uiNextWriteBLK - 1));
            switch (querySelector.uiAttrNum)
            {
            case 1:{
                if(record.attr1 == querySelector.uiValue){
                    bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex,record, pBuf);
                    uiWriteCurIndex++;     
                    uiRecordNum++;
#ifdef OUTPUT_ON
                    DISPLAY_RECORD(record);      
#endif // OUTPUT_ON
                }
            }
                break;
            case 2:{
                if(record.attr2 == querySelector.uiValue){
                    bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex,record, pBuf);
                    uiWriteCurIndex++;
                    uiRecordNum++;
#ifdef OUTPUT_ON
                    DISPLAY_RECORD(record);      
#endif // OUTPUT_ON
                }
            }
                break;
            default:
                break;
            }   
        }
        freeBlockInBuffer(puBlk, pBuf);                                 /* ����ڴ��ж�������� */
    }
    if(uiWriteCurIndex != 0) {
        uINT uiNextWriteBLK = dWriteBLK(uiWriteBBLKNum, 1, pBuf);
#ifdef OUTPUT_ON
        printf(FONT_COLOR_RED "���д����̿� %ld \n" FONT_COLOR_END, uiNextWriteBLK - 1);
#endif // OUTPUT_ON
    }
    else {
        freeBlockInBuffer(puWriteBlk, pBuf);
    }
    
#ifdef OUTPUT_ON
    printf("����������Ԫ�鹲 %ld ��\n", uiRecordNum);
    DISPLAY_IO_CNT();
#endif // OUTPUT_ON
}

/**
 * @brief ���׶ζ�·������Compare BLK���ж��Ƿ����
 * 
 * @param uiCompareBBLKNum Compare BLK��Buffer�е����
 * @param uiSegment �ֳ��˼�·�����飿
 * @param pBuf �ڴ滺����
 * @return true ���
 * @return false δ��� 
 */
bool __tpmmsCheckIsOver(uINT uiCompareBBLKNum, uINT uiSegment, pBuffer pBuf){
    bool        bIsOver = TRUE;
    record_t    record;
    for (uINT uiIndex = 0; uiIndex < uiSegment; uiIndex++)
    {
        record = bGetBLKRecord(uiCompareBBLKNum, uiIndex, pBuf);
        if(record.attr1 != INVALID_ATTR || record.attr2 != INVALID_ATTR){
            bIsOver = FALSE;
            break;
        }
    }
    return bIsOver;
}

/**
 * @brief ����������߽���ѡ�����ֵ����Сֵ���ڵ����
 * 
 * @param uiCompareBBLKNum Compare BLK��
 * @param uiSegment ����
 * @param uiAttrNum �ؼ���
 * @param bIsAscend �Ƿ�����
 * @param pBuf �ڴ滺����
 * @return uINT ���
 */
uINT __tpmmsSelectMaxMinIndex(uINT uiCompareBBLKNum, uINT uiSegment, uINT uiAttrNum, bool bIsAscend, pBuffer pBuf){
    int         iKeyMaxMin;
    uINT        uiKey;
    uINT        uiMaxMinIndex;
    record_t    record;
    if(bIsAscend){
        iKeyMaxMin    = INT_MAX;
    }
    else {
        iKeyMaxMin    = INT_MIN;
    }
    for (uINT uiIndex = 0; uiIndex < uiSegment; uiIndex++)
    {
        record = bGetBLKRecord(uiCompareBBLKNum, uiIndex, pBuf);
        if(record.attr1 != INVALID_ATTR && record.attr2 != INVALID_ATTR){
            uiKey = getKeyAttr(record, uiAttrNum);
            if(bIsAscend){
                if((int)uiKey < iKeyMaxMin){
                    iKeyMaxMin = uiKey;
                    uiMaxMinIndex = uiIndex;
                }
            }
            else {
                if((int)uiKey > iKeyMaxMin){
                    iKeyMaxMin = uiKey;
                    uiMaxMinIndex = uiIndex;
                }
            }
        }
    }
    return uiMaxMinIndex;
}
/**
 * @brief ���׶ζ�·�鲢����
 * 
 * @param querySelector ������ 
 * @param uiDBLKLowNum �����ϵͿ��
 * @param uiDBLKHighNum �����ϸ߿��
 * @param pBuf �ڴ滺����
 */
void tpmms(querySelector_t querySelector, uINT uiDBLKLowNum, uINT uiDBLKHighNum, bool bIsAscend, pBuffer pBuf){
    uINT        uiNum;

    uINT        uiSegment;
    uINT        uiNumPerSegment;
    
    uINT        uiBaseSegmentBBLKNum;
    
    puChar      puBlk;
    uINT        uiBBLKNum;

    uINT        uiDBLKNum;

    
    puChar      puCompareBlk;
    uINT        uiCompareBBLKNum;

    uINT*       puiBNextIndex;                                      /* �� Buffer Recordָ��*/
    uINT*       puiDNextIndex;                                      /* �� ���̶�ָ�� */
    
    puChar      puWriteBlk;
    uINT        uiWriteBBLKNum;
    uINT        uiWriteCurIndex;                                    /* д�� ָ�� */

    record_t    record;
    uINT        uiMaxMinSegmentIndex;

    uiNum = uiDBLKHighNum - uiDBLKLowNum + 1;
    uiBaseSegmentBBLKNum = -1;
    
    if(uiNum < BUF_NBLK * BUF_NBLK){                                /* ���׶ζ�·�鲢���� */
        uiNumPerSegment = BUF_NBLK;                                 /* ÿ��8���� */
        uiSegment = uiNum / uiNumPerSegment;                        /* �ֳ�uiSegment�� */
        
        if(uiSegment > BLK_NRECORD){
            printf(TIPS_ERROR "[tpmms: Compare BLK can't hold #%d segments]\n", uiSegment);
            return;    
        }

        for (size_t i = 0; i < uiSegment; i++)                      /* ��һ��ɨ�� */
        {
            uiDBLKNum = uiDBLKLowNum + i * uiNumPerSegment;         /* �����ڴ����е�λ�� */
            for (size_t j = 0; j < uiNumPerSegment; j++)
            {
                readBlockFromDisk(uiDBLKNum + j, pBuf);
            }
            sortInBuf(1, uiNumPerSegment, bIsAscend, querySelector.uiAttrNum, pBuf);
            dWriteBLK(1, uiNumPerSegment, pBuf);
        }
        

        puCompareBlk     = getNewBlockInBuffer(pBuf);
        uiCompareBBLKNum = bConvertBLKAddr2Num(puCompareBlk, pBuf);
        bClearBLK(uiCompareBBLKNum, pBuf);

        INIT_WRITE_BLK();

        puiBNextIndex = (uINT *)malloc(uiSegment * sizeof(uINT));
        puiDNextIndex = (uINT *)malloc(uiSegment * sizeof(uINT));
        for (uINT uiSegmentIndex = 0; uiSegmentIndex < uiSegment; uiSegmentIndex++)
        {
            RESET_CUR_RECORD(uiSegmentIndex);
            RESET_CUR_DBLK(uiSegmentIndex);
        }

        
        for (uINT uiSegmentIndex = 0; uiSegmentIndex < uiSegment; uiSegmentIndex++)                                       /* ��ʼÿ�����鶼����һ�����ڴ� */
        {
            uiDBLKNum = GET_CUR_DBLK_NUM(querySelector.uiBasePos, uiSegmentIndex);
            puBlk     = readBlockFromDisk(uiDBLKNum, pBuf);  
            uiBBLKNum = bConvertBLKAddr2Num(puBlk, pBuf);
            
            if(uiBaseSegmentBBLKNum == -1){
                uiBaseSegmentBBLKNum = uiBBLKNum;
            }
            
            record = bGetBLKRecord(uiBBLKNum, GET_CUR_RECORD_INDEX(uiSegmentIndex), pBuf);
            bSetBLKRecord(uiCompareBBLKNum, uiSegmentIndex, record, pBuf);              /* ��ʼ��Compare BLK */
        }
        

        while (!__tpmmsCheckIsOver(uiCompareBBLKNum, uiSegment, pBuf))                  /* �ж�TPMMS�Ƿ���� */
        {
            WRITE_TILL_BLK_FILL(NULL);
                                                                                        /* ѡ����Сֵ����Segment������ */
            uiMaxMinSegmentIndex = __tpmmsSelectMaxMinIndex(uiCompareBBLKNum, uiSegment, querySelector.uiAttrNum, bIsAscend, pBuf);
            if((bIsAscend && uiMaxMinSegmentIndex != INT_MAX) || 
               (!bIsAscend && uiMaxMinSegmentIndex != INT_MIN)){                        /* ������� */
                record = bGetBLKRecord(uiCompareBBLKNum, uiMaxMinSegmentIndex, pBuf);   /* ��ȡ��������Ӧ��record */
                
                bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, record, pBuf);           /* д��Write BLK�� */
                uiWriteCurIndex++;                                                      /* д��ָ��++ */

                GET_NEXT_RECORD(uiMaxMinSegmentIndex);                                  /* ��Segment�Ķ�ָ��++ */
                if(GET_CUR_RECORD_INDEX(uiMaxMinSegmentIndex) != BLK_NRECORD){          /* ��û�ж��� */
                                                                                        /* ֱ�ӽ���һ��ֵ����Compare BLK�� */
                    uiBBLKNum = GET_CUR_BBLK_NUM(uiMaxMinSegmentIndex);
                    record = bGetBLKRecord(uiBBLKNum, GET_CUR_RECORD_INDEX(uiMaxMinSegmentIndex), pBuf);
                    bSetBLKRecord(uiCompareBBLKNum, uiMaxMinSegmentIndex, record, pBuf);
                }
                else {                                                                  /* ���� */
                                                                                        /* �ͷŸ�Segment����Ŀ� */
                    uiBBLKNum =  GET_CUR_BBLK_NUM(uiMaxMinSegmentIndex);
                    freeBlockInBuffer(GET_BUF_DATA(pBuf, uiBBLKNum), pBuf);
                    GET_NEXT_DBLK(uiMaxMinSegmentIndex);
                                                                                        /* �����Segment����һ�� */
                    if(GET_CUR_DBLK_INDEX(uiMaxMinSegmentIndex) != uiNumPerSegment){    /* ����Segment������һ�� */
                        uiDBLKNum = GET_CUR_DBLK_NUM(querySelector.uiBasePos, uiMaxMinSegmentIndex);
                        puBlk = readBlockFromDisk(uiDBLKNum, pBuf);
                        RESET_CUR_RECORD(uiMaxMinSegmentIndex);   
                        uiBBLKNum = bConvertBLKAddr2Num(puBlk, pBuf);
                        record = bGetBLKRecord(uiBBLKNum, GET_CUR_RECORD_INDEX(uiMaxMinSegmentIndex), pBuf);
                    }
                    else {                                                  
                        record.attr1 = INVALID_ATTR;
                        record.attr2 = INVALID_ATTR;
                    }

                    bSetBLKRecord(uiCompareBBLKNum, uiMaxMinSegmentIndex, record, pBuf);
                }
            }
        }
        
        if(uiWriteCurIndex != 0) {
            dWriteBLK(uiWriteBBLKNum, 1, pBuf);
        }
        else {
            freeBlockInBuffer(puWriteBlk, pBuf);
        }

        freeBlockInBuffer(puCompareBlk, pBuf);

        free(puiBNextIndex);
        free(puiDNextIndex);
    }
}


/**
 * @brief ������������������ 
 * 
 * @param querySelector ������
 * @param uiDBLKLowNum �����ļ��ĵͿ��
 * @param uiDBLKHighNum �����ļ��ĸ߿��
 * @param pBuf �ڴ滺����
 */
void indexSearch(querySelector_t querySelector, uINT uiDBLKLowNum, uINT uiDBLKHighNum, pBuffer pBuf){
    puChar      puBlk;
    uINT        uiBBLKNum;

    uINT        uiNum;

    uINT        uiDBLKHighBound;
    uINT        uiDBLKLowBound;

    record_t    record1;
    record_t    record2;
    uINT        uiKey;
    
    uiNum = uiDBLKHighNum - uiDBLKLowNum + 1;
    

    record1.attr1 = INVALID_ATTR;
    record1.attr2 = INVALID_ATTR;
    
    record2.attr1 = INVALID_ATTR;
    record2.attr2 = INVALID_ATTR;
    
    for (size_t i = 0; i < uiNum; i++)
    {
        puBlk       = readBlockFromDisk(uiDBLKLowNum + i, pBuf);
        uiBBLKNum   = bConvertBLKAddr2Num(puBlk, pBuf);
        for (uINT uiIndex = 0; uiIndex < BLK_NRECORD; uiIndex++)
        {
            record1 = bGetBLKRecord(uiBBLKNum, uiIndex, pBuf);
            if(uiIndex == BLK_NRECORD - 1 && i == uiNum - 1){
                uiDBLKHighBound = TPMMS_S_POS + 32 + 31;
                uiDBLKLowBound  = record1.attr2;                  /* �����ļ�Record (Attr, BlockPtr) */
                break;
            }
            else {
                if(record2.attr1 != INVALID_ATTR && record2.attr2 != INVALID_ATTR){
                    uiKey = querySelector.uiValue;
                    if(abs(uiKey - record1.attr1) <= abs(record2.attr1 - record1.attr1)){
                        uiDBLKHighBound = record1.attr2;
                        uiDBLKLowBound = record2.attr2;
                        break;
                    }
                }
                record2.attr1 = record1.attr1;
                record2.attr2 = record1.attr2;
            }
        }
        freeBlockInBuffer(puBlk, pBuf);
    }
    linearSearch(querySelector, uiDBLKLowBound, uiDBLKHighBound, pBuf);
}