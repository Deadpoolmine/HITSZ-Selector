#include "selector.h"
#include "merger.h"


/**
 * @brief ��������������㷨 
 * 
 * @param mergerOptions ����ѡ��
 * @param uiDBLKLowNumS ����S�ĵͿ��
 * @param uiDBLKHighNumS ����S�ĸ߿��
 * @param uiDBLKLowNumR ����R�ĵͿ��
 * @param uiDBLKHighNumR ����R�ĸ߿��
 * @param pBuf �ڴ滺����
 * @return uINT Ԫ�ظ��� - ���������� ��ΪԪ�飬����Ϊ������
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

    uINT                    uiDBLKNumLastR;            /* R��S��һ���ɱȿ� - �Ż��� */

    bool                    bCanBreak;
    bool                    bHasSame;
    uINT                    uiOpCnt;
    record_t                record;

    switch (mergerOptions.mergerType)
    {
    case JOIN:
        DISPLAY_TIPS("��������������㷨");
        break;
    case UNION:
        DISPLAY_TIPS("��������ļ��ϲ�����");
        break;
    case INTER:
        DISPLAY_TIPS("��������ļ��Ͻ�����");
        break;
    case DIFF:
        DISPLAY_TIPS("��������ļ��ϲ�����");
        break; 
    default:
        break;
    }

    dResetGlobNextBLKNum();
    *puiNum = 0;
    uiOpCnt = 0;
    querySelector.uiAttrNum = mergerOptions.uiAttrNumR;
    querySelector.uiBasePos = SM_TEMP_R_POS;
    GENERATE_TIPS("mergeǰ������TPMMS�Թ�ϵR����");
    dSetGlobNextBLKNum(SM_TEMP_R_POS);
    tpmms(querySelector, uiDBLKLowNumR, uiDBLKHighNumR, TRUE, pBuf);

    querySelector.uiAttrNum = mergerOptions.uiAttrNumS;
    querySelector.uiBasePos = SM_TEMP_S_POS;
    GENERATE_TIPS("mergeǰ������TPMMS�Թ�ϵS����");
    dSetGlobNextBLKNum(SM_TEMP_S_POS);
    tpmms(querySelector, uiDBLKLowNumS, uiDBLKHighNumS, TRUE, pBuf);        /* �������� */

    dSetGlobNextBLKNum(SM_POS);
    INIT_WRITE_BLK();
    INIT_IO_COUNTER();
    uiDBLKNumLastR = 0;

    uiDBLKNumS = SM_TEMP_S_POS + TABLE_S_NBLK;
    uiDBLKNumR = SM_TEMP_R_POS + TABLE_R_NBLK;
    // printf("--------\n");
    // dCheckBLKs(uiDBLKNumS, uiDBLKNumS + 31, pBuf);
    // printf("--------\n");
    // dCheckBLKs(uiDBLKNumR, uiDBLKNumR + 15, pBuf);

    for (size_t i = 0; i < TABLE_S_NBLK; i++)
    {
        uiDBLKNumS = SM_TEMP_S_POS + TABLE_S_NBLK + i;
        for (uINT uiIndexS = 0; uiIndexS < BLK_NRECORD; uiIndexS++)
        {
            puBlkS     = readBlockFromDisk(uiDBLKNumS, pBuf);
            uiBBLKNumS = bConvertBLKAddr2Num(puBlkS, pBuf);
            recordS    = bGetBLKRecord(uiBBLKNumS, uiIndexS, pBuf);
            uiKeyS     = getKeyAttr(recordS, mergerOptions.uiAttrNumS);

            bCanBreak  = FALSE;                 /* �Ƿ��S����һ����¼ */
            bHasSame   = FALSE;

            for (size_t j = 0; j < TABLE_R_NBLK; j++)
            {
                uiDBLKNumR = SM_TEMP_R_POS + TABLE_R_NBLK + j;
                if(uiDBLKNumR < uiDBLKNumLastR){                            /* ֱ��������һ���ɱȿ� */
                    continue;
                }
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
                            WRITE_TILL_BLK_FILL(*puiNum = *puiNum + 1);

                            bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, recordR, pBuf);
                            uiWriteCurIndex++;
                            WRITE_TILL_BLK_FILL(*puiNum = *puiNum + 1);
                        }
                        else if(uiKeyR > uiKeyS){                                        /* R�����򣬿���ֱ�������� */
                            bCanBreak = TRUE;
                        }
                        else if(uiKeyR < uiKeyS){
                            if(mergerOptions.bIsOptimise)
                                uiDBLKNumLastR = uiDBLKNumR;
                        }
                    }
                        break;
                    case INTER:{
                        if(recordR.attr1 == recordS.attr1 && recordR.attr2 == recordS.attr2){
                            uiOpCnt++;

                            bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, recordS, pBuf);
                            uiWriteCurIndex++;
                            
                            WRITE_TILL_BLK_FILL(*puiNum = *puiNum + 1);
                            bCanBreak = TRUE;
                        }
                        else if(uiKeyR > uiKeyS) {
                            bCanBreak = TRUE;
                        }
                        else if(uiKeyR < uiKeyS){
                            if(mergerOptions.bIsOptimise)
                                uiDBLKNumLastR = uiDBLKNumR;
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
                        else if(uiKeyR < uiKeyS){
                            if(mergerOptions.bIsOptimise)
                                uiDBLKNumLastR = uiDBLKNumR;
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
                
                    WRITE_TILL_BLK_FILL(*puiNum = *puiNum + 1);
                }
                    break;
                default:
                    break;
                }
            }
            freeBlockInBuffer(puBlkS, pBuf);                                 /* ����ڴ��ж�������� */    
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
                
                WRITE_TILL_BLK_FILL(*puiNum = *puiNum + 1);
                
                freeBlockInBuffer(puBlkR, pBuf);
            }
        }
    }
    
    if(uiWriteCurIndex != 0) {
        uINT uiNextWriteBLK = dWriteBLK(uiWriteBBLKNum, 1, pBuf);
        *puiNum = *puiNum + 1; 
#ifdef  OUTPUT_DETAIL_ON
        printf(FONT_COLOR_RED "���д����̿� %ld \n" FONT_COLOR_END, uiNextWriteBLK - 1);
#endif //  OUTPUT_DETAIL_ON
    }
    else {
        freeBlockInBuffer(puWriteBlk, pBuf);
    }
    DISPLAY_IO_CNT();
    //checkBuffer(pBuf);
    return uiOpCnt;
}


/**
 * @brief ����ɨ���㷨��ʵ�ּ��϶�Ԫ����
 * 
 * @param mergerOptions ��Ԫ����ѡ��
 * @param uiDBLKLowNumS ��ϵS�ʹ��̿��
 * @param uiDBLKHighNumS ��ϵS�ߴ��̿��
 * @param uiDBLKLowNumR ��ϵR�ʹ��̿��
 * @param uiDBLKHighNumR ��ϵR�ߴ��̿��
 * @param puiNum �ܹ������Ŀ��ָ��
 * @param pBuf �ڴ滺����
 * @return uINT 
 */
uINT  tpmm(mergerOptions_t mergerOptions, uINT uiDBLKLowNumS, uINT uiDBLKHighNumS, uINT uiDBLKLowNumR, uINT uiDBLKHighNumR, uINT *puiNum, pBuffer pBuf){
    uINT        uiNumS;
    uINT        uiNumR;

    uINT        uiSegmentS;
    uINT        uiSegmentR;

    uINT        uiSegmentIndexi;
    uINT        uiSegmentIndexj;

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
    record_t    recordi;
    record_t    recordj;
    bool        bHasSame;
    uINT        uiMaxMinSegmentIndex;

    uINT        uiOpCnt;

    switch (mergerOptions.mergerType)
    {
    case UNION:
        DISPLAY_TIPS("��������ļ��ϲ�����");
        break;
    case INTER:
        DISPLAY_TIPS("��������ļ��Ͻ�����");
        break;
    case DIFF:
        DISPLAY_TIPS("��������ļ��ϲ�����");
        break; 
    default:
        break;
    }

    uiOpCnt = 0;
    uiNumS = uiDBLKHighNumS - uiDBLKLowNumS + 1;
    uiNumR = uiDBLKHighNumR - uiDBLKLowNumR + 1;

    uiBaseSegmentBBLKNum = -1;
    
    INIT_IO_COUNTER();
    if(uiNumS < BUF_NBLK * BUF_NBLK){                               /* ���׶ζ�·�鲢���� */
        uiNumPerSegment = BUF_NBLK;                                 /* ÿ��8���� */
        uiSegmentS = uiNumS / uiNumPerSegment;                      /* �ֳ�uiSegmentS�� */
        uiSegmentR = uiNumR / uiNumPerSegment;
        
        if(uiSegmentS + uiSegmentR > BLK_NRECORD){
            printf(TIPS_ERROR "[tpmm: Compare BLK can't hold #%d segments]\n", uiSegmentS);
            return uiOpCnt;    
        }

        dSetGlobNextBLKNum(SM_TEMP_S_POS);
        for (size_t i = 0; i < uiSegmentS; i++)                      /* ��һ��ɨ�� */
        {
            uiDBLKNum = uiDBLKLowNumS + i * uiNumPerSegment;         /* S�и����ڴ����е�λ�� */
            for (size_t j = 0; j < uiNumPerSegment; j++)
            {
                readBlockFromDisk(uiDBLKNum + j, pBuf);
            }
            sortInBuf(1, uiNumPerSegment, TRUE, 1, pBuf);
            dWriteBLK(1, uiNumPerSegment, pBuf);
        }

        for (size_t i = 0; i < uiSegmentR; i++)                      /* ��һ��ɨ�� */
        {
            uiDBLKNum = uiDBLKLowNumR + i * uiNumPerSegment;         /* R�и����ڴ����е�λ�� */
            for (size_t j = 0; j < uiNumPerSegment; j++)
            {
                readBlockFromDisk(uiDBLKNum + j, pBuf);
            }
            sortInBuf(1, uiNumPerSegment, TRUE, 1, pBuf);
            dWriteBLK(1, uiNumPerSegment, pBuf);
        }

        dSetGlobNextBLKNum(SM_POS);
        puCompareBlk     = getNewBlockInBuffer(pBuf);
        uiCompareBBLKNum = bConvertBLKAddr2Num(puCompareBlk, pBuf);
        bClearBLK(uiCompareBBLKNum, pBuf);

        INIT_WRITE_BLK();

        puiBNextIndex = (uINT *)malloc((uiSegmentS + uiSegmentR) * sizeof(uINT));
        puiDNextIndex = (uINT *)malloc((uiSegmentS + uiSegmentR) * sizeof(uINT));
        for (uINT uiSegmentIndex = 0; uiSegmentIndex < uiSegmentS + uiSegmentR; uiSegmentIndex++)
        {
            RESET_CUR_RECORD(uiSegmentIndex);
            RESET_CUR_DBLK(uiSegmentIndex);
        }

        for (uINT uiSegmentIndex = 0; uiSegmentIndex < uiSegmentS + uiSegmentR; uiSegmentIndex++)                                       /* ��ʼÿ�����鶼����һ�����ڴ� */
        {
            uiDBLKNum = GET_CUR_DBLK_NUM(SM_TEMP_S_POS, uiSegmentIndex);
            puBlk     = readBlockFromDisk(uiDBLKNum, pBuf);  
            uiBBLKNum = bConvertBLKAddr2Num(puBlk, pBuf);
            
            if(uiBaseSegmentBBLKNum == -1){
                uiBaseSegmentBBLKNum = uiBBLKNum;
            }
            
            record = bGetBLKRecord(uiBBLKNum, GET_CUR_RECORD_INDEX(uiSegmentIndex), pBuf);
            bSetBLKRecord(uiCompareBBLKNum, uiSegmentIndex, record, pBuf);              /* ��ʼ��Compare BLK */
        }

        while (!tpmmCheckIsOver(uiCompareBBLKNum, uiSegmentS + uiSegmentR, pBuf))     /* �ж�TPMMS�Ƿ���� */
        {
            WRITE_TILL_BLK_FILL(*puiNum = *puiNum + 1);
            bHasSame = tpmmCheckIfSame(uiCompareBBLKNum, uiSegmentR + uiSegmentS, &uiSegmentIndexi, &uiSegmentIndexj, pBuf);
            if(bHasSame){                                                                        /* ������� */
                recordi = bGetBLKRecord(uiCompareBBLKNum, uiSegmentIndexi, pBuf);                /* ��ȡ��������Ӧ��record */
                recordj = bGetBLKRecord(uiCompareBBLKNum, uiSegmentIndexj, pBuf);
                if(uiSegmentIndexi + 1 <= uiSegmentS && uiSegmentIndexj + 1 > uiSegmentS){        /* uiSegemnti���ڹ�ϵS��uiSegemnti���ڹ�ϵR*/
                        
                    if(mergerOptions.mergerType == INTER || mergerOptions.mergerType == UNION){
                        bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, recordi, pBuf);            /* д��Write BLK�� */
                        uiWriteCurIndex++;    
                        uiOpCnt++;  
                        
                    }
                    tpmmShiftSegmentRecord(SM_TEMP_S_POS, uiCompareBBLKNum, uiBaseSegmentBBLKNum,uiSegmentIndexi, puiBNextIndex, 
                                                 puiDNextIndex, uiNumPerSegment, pBuf);
                    tpmmShiftSegmentRecord(SM_TEMP_S_POS,uiCompareBBLKNum, uiBaseSegmentBBLKNum, uiSegmentIndexj, puiBNextIndex, 
                                                puiDNextIndex, uiNumPerSegment, pBuf);
                }
                else {
                    tpmmShiftSegmentRecord(SM_TEMP_S_POS, uiCompareBBLKNum, uiBaseSegmentBBLKNum, uiSegmentIndexj, puiBNextIndex, 
                                             puiDNextIndex, uiNumPerSegment, pBuf);
                }
            }
            else {
                uiMaxMinSegmentIndex = tpmmSelectMaxMinIndex(uiCompareBBLKNum, uiSegmentS + uiSegmentR, 1, TRUE, pBuf);
                if(uiMaxMinSegmentIndex != INT_MAX){                           /* ������� */
                    record = bGetBLKRecord(uiCompareBBLKNum, uiMaxMinSegmentIndex, pBuf);   /* ��ȡ��������Ӧ��record */
                    if(mergerOptions.mergerType == UNION){
                        bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, record, pBuf);            /* д��Write BLK�� */
                        uiOpCnt++;
                        uiWriteCurIndex++;
                    }
                    if(mergerOptions.mergerType == DIFF){
                        if(uiMaxMinSegmentIndex + 1 <= uiSegmentS){
                            bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, record, pBuf);            /* д��Write BLK�� */
                            uiOpCnt++;
                            uiWriteCurIndex++;
                        }
                    }
                    tpmmShiftSegmentRecord(SM_TEMP_S_POS,uiCompareBBLKNum, uiBaseSegmentBBLKNum, uiMaxMinSegmentIndex, puiBNextIndex, 
                                             puiDNextIndex, uiNumPerSegment, pBuf);
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
    DISPLAY_IO_CNT();
    return uiOpCnt;
}