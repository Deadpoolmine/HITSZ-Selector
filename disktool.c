#include "utils.h"
#include "windows.h"
#include "selector.h"

uINT _G_DBLKNextAvailableNum;

/**
 * @brief ��ʼ�����̹��ߣ�
 *        1. ��ʼ����һ��������д��Ŀ���
 * 
 */
void initDTool(){
    const char * sDir = ".\\data";
    WIN32_FIND_DATA fdFile;
    HANDLE hFind = NULL;
    size_t fileLen, nameLen;
    char sPath[2048];
    int maxBLKNum = -1;
    puChar fileNameBuf;

    sprintf(sPath, "%s\\*.blk", sDir);
    
    if((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
    {
        printf("Path not found: [%s]\n", sDir);
        return;
    }

    do
    {
        if(strcmp(fdFile.cFileName, ".") != 0
                && strcmp(fdFile.cFileName, "..") != 0)
        {
            fileLen = strlen(fdFile.cFileName);
            nameLen = fileLen - 4;
            fileNameBuf = (puChar)malloc(fileLen + 1);
            for (size_t i = 0; i < nameLen; i++)
            {
                *(fileNameBuf + i) = fdFile.cFileName[i];
            }
            *(fileNameBuf + nameLen) = '\0';
            if(atoi(fileNameBuf) > maxBLKNum){
                maxBLKNum = atoi(fileNameBuf);
            }

            if(atoi(fileNameBuf) > TABLE_R_NBLK + TABLE_S_NBLK){
                dropBlockOnDisk(atoi(fileNameBuf));    
            }
        }
    }
    while(FindNextFile(hFind, &fdFile)); 
    FindClose(hFind); 
    
    _G_DBLKNextAvailableNum = maxBLKNum + 1;
    
    if(fileNameBuf){
        free(fileNameBuf);
    }
}

/**
 * @brief ��ȡ��һ��������д��Ŀ��ţ���ʹ֮ !!��һ
 * 
 * @return uINT ���ú�Ŀ��
 */
uINT dGetBLKNextGlobNum(){
    return _G_DBLKNextAvailableNum++;
}

/**
 * @brief ������һ��������д��Ŀ���Ϊ��ǰ����ź�һ��
 * 
 * @return uINT ���ú�Ŀ��
 */
uINT dResetGlobNextBLKNum(){
    initDTool();
    return _G_DBLKNextAvailableNum;
}

/**
 * @brief ������һ��������д��Ŀ�
 * 
 * @param uiDBLKNum ������һ����д��Ŀ�Ŀ�� 
 */
void dSetGlobNextBLKNum(uINT uiDBLKNum){
    _G_DBLKNextAvailableNum = uiDBLKNum;
}

/**
 * @brief �����д��Buffer�е����ݿ飬д�����Щ�鱻�ͷ�
 * 
 * @param uiBBLKNum Buffer BLK��
 * @param uiNum ����д��uiNum����
 * @param pBuf �ڴ滺����
 * @return uINT ���һ��д��Ŀ��
 */
uINT dWriteBLK(uINT uiBBLKNum, uINT uiNum, pBuffer pBuf){
    uINT    uiDBLKNextNum; 
    uINT    uiRemainNum;
    puChar  puBBlk;

    
    if(uiBBLKNum + uiNum - 1 > BUF_NBLK){
        printf(TIPS_ERROR "[dWriteBLK: uiNum = %d out of bounds = %d]\n", 
               uiNum, (BUF_NBLK - uiBBLKNum + 1));
    }
    
    for (size_t i = 0; i < uiNum; i++)
    {
        uiDBLKNextNum = dGetBLKNextGlobNum();
        bSetBLKNextBLK(uiBBLKNum + i, uiDBLKNextNum + 1, pBuf);
        puBBlk = GET_BUF_DATA(pBuf, uiBBLKNum + i);
        writeBlockToDisk(puBBlk, uiDBLKNextNum, pBuf);
    }

    return _G_DBLKNextAvailableNum;
}

/**
 * @brief �鿴�����ϵĿ���Ϣ
 * 
 * @param uiDBLKLowNum �����ϵͿ��
 * @param uiDBLKHighNum �����ϸ߿��
 * @param pBuf �ڴ滺����
 */
void dCheckBLKs(uINT uiDBLKLowNum,uINT uiDBLKHighNum, pBuffer pBuf){
    puChar      puBlk;
    uINT        uiNum;

#ifdef OUTPUT_ON
    printf("\n\n��[%ld, %ld]��Ϣ��¼:\n\n", uiDBLKLowNum, uiDBLKHighNum);
#endif // OUTPUT_ON
    
    uiNum = uiDBLKHighNum - uiDBLKLowNum + 1;
    for (size_t i = 0; i < uiNum; i++)
    {
        puBlk = readBlockFromDisk(uiDBLKLowNum + i, pBuf);
        printf("[DBLK %d]: ", uiDBLKLowNum + i);
        for (size_t j = 0; j < BLK_NRECORD; j++)
        {
            record_t record = bGetBLKRecord(bConvertBLKAddr2Num(puBlk, pBuf), j, pBuf);
            printf("(%d, %d)\t", record.attr1, record.attr2);
        }
        printf("\n");
        freeBlockInBuffer(puBlk, pBuf);
    }
}
/**
 * @brief �鿴��ϵR��S��ϵ
 * 
 * @param pBuf �ڴ��ȡ������
 */
void dCheckTables(pBuffer pBuf) {
    printf("\n------------------------------Table R:------------------------------\n");
    dCheckBLKs(1, TABLE_R_NBLK, pBuf);

    printf("\n------------------------------Table S:------------------------------\n");
    dCheckBLKs(TABLE_R_NBLK + 1, TABLE_R_NBLK + TABLE_S_NBLK, pBuf);
}

/**
 * @brief �鿴TPMMS��S��ϵ
 * 
 * @param uiTPMMSRes ����TPMMS_S_POS
 * @param pBuf �ڴ滺����
 */
void dCheckTpmmsS(uINT uiTPMMSRes, pBuffer pBuf){
    printf("\n------------------------------Table S After TPMMS:------------------------------\n");
    dCheckBLKs(uiTPMMSRes + 32, uiTPMMSRes + 32 + 31, pBuf);
}

/**
 * @brief �����̿�[uiDBLKLowNum, uiDBLKHighNum]���������Ƿ�����
 * 
 * @param uiDBLKLowNum ���̿�Ϳ��
 * @param uiDBLKHighNum ���̿�߿��
 * @param uiAttrNum ��������
 * @return true ������
 * @return false δ����
 */
bool __dCheckIsSort(uINT uiDBLKLowNum,uINT uiDBLKHighNum, uINT uiAttrNum, pBuffer pBuf){
    uINT        uiNum;
    uINT        uiBBLKNum;

    uINT        uiDBLKNum;
    puChar      puBlk;
    
    bool        bIsSort;
    bool        bIsAscend;
    
    
    uINT        uiKey1;
    uINT        uiKey2;
    record_t    record1;
    record_t    record2;

    record1.attr1 = INVALID_ATTR;
    record1.attr2 = INVALID_ATTR;

    record2.attr1 = INVALID_ATTR;
    record2.attr2 = INVALID_ATTR;
    
    uiNum = uiDBLKHighNum - uiDBLKLowNum + 1;
    
    for (size_t i = 0; i < uiNum; i++)
    {
        uiDBLKNum   = i + uiDBLKLowNum;
        puBlk       = readBlockFromDisk(uiDBLKNum, pBuf);
        uiBBLKNum   = bConvertBLKAddr2Num(puBlk, pBuf);
        for (uINT uiIndex = 0; uiIndex < BLK_NRECORD; uiIndex++)
        {
            record1 = bGetBLKRecord(uiBBLKNum, uiIndex, pBuf);
            if(record2.attr1 != INVALID_ATTR && record2.attr2 != INVALID_ATTR){
                uiKey1 = getKeyAttr(record1, uiAttrNum);
                uiKey2 = getKeyAttr(record2, uiAttrNum);
                // switch (uiAttrNum)
                // {
                // case 1:{
                //     uiKey1 = record1.attr1;
                //     uiKey2 = record2.attr1;
                // }
                //     break;
                // case 2:{
                //     uiKey1 = record1.attr2;
                //     uiKey2 = record2.attr2;
                // }
                //     break;
                // default:
                //     break;
                // }
                if(uiKey1 > uiKey2){
                    bIsAscend = TRUE;
                    bIsSort = TRUE;
                }
                else {
                    bIsAscend = FALSE;
                    bIsSort = TRUE;
                }

                if(bIsAscend){
                    if(uiKey1 < uiKey2) {
                        bIsSort = FALSE;
                        break;
                    }
                }
                else {
                    if(uiKey1 > uiKey2) {
                        bIsSort = FALSE;
                        break;
                    }
                }
            }        
            record2 = record1;
        }
        freeBlockInBuffer(puBlk, pBuf);
    }
    return bIsSort;
}
/**
 * @brief Ϊ���̿�[uiDBLKLowNum, uiDBLKHighNum]���������ļ�
 * 
 * @param uiDBLKLowNum ���̿�Ϳ��
 * @param uiDBLKHighNum ���̿�߿��
 * @param uiAttrNum �������ֻ�������
 * @param uiGap ���������������ļ��
 * @param puiNum �����ļ�ռ�ö��ٴ��̿�
 * @param pBuf �ڴ滺����
 * @return uINT �����ļ���ʼ���
 */
uINT dBuildIndexFile(uINT uiDBLKLowNum,uINT uiDBLKHighNum, uINT uiAttrNum, uINT uiGap, uINT *puiNum, pBuffer pBuf){
    querySelector_t         querySelector;
    uINT                    uiNum;

    record_t                record;
    record_t                recordIndex;

    puChar                  puWriteBlk;
    uINT                    uiWriteBBLKNum;
    uINT                    uiWriteCurIndex;

    puChar                  puBlk;
    uINT                    uiBBLKNum;
    
    uINT                    BuiBLKPerGap;                    

    DISPLAY_TIPS("Ϊ����TPMMS�����Ĺ�ϵS���������ļ�\n�����ļ�λ�ڴ��̿�2000");

    if(!__dCheckIsSort(uiDBLKLowNum, uiDBLKHighNum, uiAttrNum, pBuf)){
        dSetGlobNextBLKNum(TPMMS_S_POS);
        querySelector.uiAttrNum = 1;
        querySelector.uiValue = INVALID_ATTR;
        tpmms(querySelector, TABLE_R_NBLK + 1, TABLE_R_NBLK + TABLE_S_NBLK, FALSE, pBuf);
    }

    dSetGlobNextBLKNum(INDEX_FILE_POS);

    uiNum           = uiDBLKHighNum - uiDBLKLowNum + 1;
    BuiBLKPerGap    = uiNum / uiGap;    

    puWriteBlk      = getNewBlockInBuffer(pBuf);
    uiWriteBBLKNum  = bConvertBLKAddr2Num(puWriteBlk, pBuf);
    uiWriteCurIndex = 0;
    bClearBLK(uiWriteBBLKNum, pBuf);

    for (size_t i = 0; i < BuiBLKPerGap; i++)
    {
        if(uiWriteCurIndex == BLK_NRECORD){
            dWriteBLK(uiWriteBBLKNum, 1, pBuf);                                     /* д���Buffer�ᱻ��� */
            puWriteBlk      = getNewBlockInBuffer(pBuf);                            /* ��������д��� */
            uiWriteBBLKNum  = bConvertBLKAddr2Num(puWriteBlk, pBuf);
            bClearBLK(uiWriteBBLKNum, pBuf);
            uiWriteCurIndex = 0;
            *puiNum++;
        }
        puBlk       = readBlockFromDisk(uiDBLKLowNum + i * uiGap, pBuf);
        uiBBLKNum   = bConvertBLKAddr2Num(puBlk, pBuf);
        record      = bGetBLKRecord(uiBBLKNum, 0, pBuf);

        recordIndex.attr1 = getKeyAttr(record, uiAttrNum);

        recordIndex.attr2 = uiDBLKLowNum + i * uiGap;
        bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, recordIndex, pBuf);
        freeBlockInBuffer(puBlk, pBuf);
        uiWriteCurIndex++;
    }

    if(uiWriteCurIndex != 0) {
        *puiNum++;
        while (uiWriteCurIndex != BLK_NRECORD)
        {
            recordIndex.attr1 = INVALID_ATTR;
            recordIndex.attr2 = INVALID_ATTR;
            bSetBLKRecord(uiWriteBBLKNum, uiWriteCurIndex, recordIndex, pBuf);
            uiWriteCurIndex++;
        }
        dWriteBLK(uiWriteBBLKNum, 1, pBuf);
    }
    else {
        freeBlockInBuffer(puWriteBlk, pBuf);
    }
    
    checkBuffer(pBuf);
}