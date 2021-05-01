#include "selector.h"

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
    uINT        uiKey;

    
    DISPLAY_TIPS(querySelector.cTips);
    uiNum               = uiDBLKHighNum - uiDBLKLowNum + 1;
    uiRecordNum         = 0;
    INIT_IO_COUNTER();
    INIT_WRITE_BLK();

    for (size_t i = 0; i < uiNum; i++)
    {
        puBlk = readBlockFromDisk(uiDBLKLowNum + i, pBuf);              /* ��һ�����ڴ� */
        uiBBLKNum = bConvertBLKAddr2Num(puBlk, pBuf);
#ifdef  OUTPUT_DETAIL_ON
        printf("�������ݿ� %ld\n", uiDBLKLowNum + i);
#endif //  OUTPUT_DETAIL_ON
        for (uINT uiIndex = 0; uiIndex < BLK_NRECORD; uiIndex++)
        {
            record = bGetBLKRecord(uiBBLKNum, uiIndex, pBuf);
            WRITE_TILL_BLK_FILL(NULL);
            uiKey = getKeyAttr(record, querySelector.uiAttrNum);
            
            if(querySelector.uiValue == uiKey){
                bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex,record, pBuf);
                uiWriteCurIndex++;     
                uiRecordNum++;
                DISPLAY_RECORD(record); 
            }
        }
        freeBlockInBuffer(puBlk, pBuf);                                 /* ����ڴ��ж�������� */
    }
    if(uiWriteCurIndex != 0) {
        uINT uiNextWriteBLK = dWriteBLK(uiWriteBBLKNum, 1, pBuf);
#ifdef  OUTPUT_DETAIL_ON
        printf(FONT_COLOR_RED "���д����̿� %ld \n" FONT_COLOR_END, uiNextWriteBLK - 1);
#endif //  OUTPUT_DETAIL_ON
    }
    else {
        freeBlockInBuffer(puWriteBlk, pBuf);
    }
    
#ifdef OUTPUT_DETAIL_ON
    printf("����������Ԫ�鹲 %ld ��\n", uiRecordNum);
#endif // OUTPUT_DETAIL_ON
    DISPLAY_IO_CNT();
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
    
    DISPLAY_TIPS(querySelector.cTips);
    INIT_IO_COUNTER();
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
        

        while (!tpmmCheckIsOver(uiCompareBBLKNum, uiSegment, pBuf))                  /* �ж�TPMMS�Ƿ���� */
        {
            WRITE_TILL_BLK_FILL(NULL);
                                                                                        /* ѡ����Сֵ����Segment������ */
            uiMaxMinSegmentIndex = tpmmSelectMaxMinIndex(uiCompareBBLKNum, uiSegment, querySelector.uiAttrNum, bIsAscend, pBuf);
            if((bIsAscend && uiMaxMinSegmentIndex != INT_MAX) || 
               (!bIsAscend && uiMaxMinSegmentIndex != INT_MIN)){                        /* ������� */
                record = bGetBLKRecord(uiCompareBBLKNum, uiMaxMinSegmentIndex, pBuf);   /* ��ȡ��������Ӧ��record */
                
                bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, record, pBuf);           /* д��Write BLK�� */
                uiWriteCurIndex++;                                                      /* д��ָ��++ */
                tpmmShiftSegmentRecord(querySelector.uiBasePos, uiCompareBBLKNum, uiBaseSegmentBBLKNum, uiMaxMinSegmentIndex, 
                                       puiBNextIndex, puiDNextIndex, uiNumPerSegment, pBuf);
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
    DISPLAY_IO_CNT();
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

    DISPLAY_TIPS(querySelector.cTips);
    INIT_IO_COUNTER();
    uiNum = uiDBLKHighNum - uiDBLKLowNum + 1;
    

    record1.attr1 = INVALID_ATTR;
    record1.attr2 = INVALID_ATTR;
    
    record2.attr1 = INVALID_ATTR;
    record2.attr2 = INVALID_ATTR;
    
    for (size_t i = 0; i < uiNum; i++)
    {
        puBlk       = readBlockFromDisk(uiDBLKLowNum + i, pBuf);
        uiBBLKNum   = bConvertBLKAddr2Num(puBlk, pBuf);
#ifdef  OUTPUT_DETAIL_ON
        printf(FONT_COLOR_PURPLE "���������� %ld\n" FONT_COLOR_END, uiDBLKLowNum + i);
#endif //  OUTPUT_DETAIL_ON
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

#ifdef  OUTPUT_DETAIL_ON
        printf(FONT_COLOR_GREEN "�ҵ��߽�� [%ld, %ld]\n" FONT_COLOR_END, uiDBLKLowBound, uiDBLKHighBound);
#endif //  OUTPUT_DETAIL_ON
    GENERATE_TIPS("���������ļ��ҵ��ı߽��������������� - S.C = 50")
    linearSearch(querySelector, uiDBLKLowBound, uiDBLKHighBound, pBuf);
    DISPLAY_IO_CNT();
}