#include "utils.h"

/**
 * @brief ��ȡ Buffer BLK�� valid�ֽ�
 * 
 */
#define GET_BUF_BLK_VALID(pBuf, uiBBLKNum) *(pBuf->data + (uiBBLKNum - 1) * (DISK_BLK_PER_SZ + 1))

/**
 * @brief �ж�uiBBLKNum�Ƿ�Ϸ�
 * 
 * @param uiBBLKNum ��Ӧ�ó����Ѿ���ȡ��pBuf�е��ܸ���
 * @param pBuf �ڴ滺��ȡ
 * @return true �Ϸ�
 * @return false ���Ϸ�
 */
bool checkBBLKNum(uINT uiBBLKNum, pBuffer pBuf){
    if(uiBBLKNum > pBuf->numAllBlk){
        printf(TIPS_ERROR "[checkBBLKNum: uiBBLKNum = %d > AllBLK = %d]\n", 
               uiBBLKNum, pBuf->numAllBlk);
        return FALSE;
    }
    return TRUE;
}

/**
 * @brief �ж�Buffer BLK�ڲ������Ƿ�Ϸ�
 * 
 * @param uiIndex ȡֵӦ��Ϊ 0 ~ 6
 * @return true 
 * @return false 
 */
bool checkBIndex(uINT uiIndex){
    if (uiIndex < 0 || uiIndex > 6)
    {
        printf(TIPS_ERROR "[checkBIndex: uiIndex = %d Out of Bounds]\n", uiIndex);
        return FALSE;
    }
    return TRUE;
}


/**
 * @brief ��Buffer�еĵ�ַת��Ϊ��ӦBuffer�е�BLK��
 * 
 * @param puBlk Buffer�еĵ�ַ����ͨ��getNewBufferBLK�õ���Ҳ
 * @param pBuf 
 * @return uINT Buffer��BLK�� 
 */
uINT bConvertBLKAddr2Num(puChar puBlk, pBuffer pBuf){
    return ((puBlk - pBuf->data) / (DISK_BLK_PER_SZ + 1)) + 1;
}

/**
 * @brief ��ȡBuffer BLK�е�һ����¼����ע��һ�����ݣ���һ���Ǽ�¼����СΪ 8B��������INT��
 * 
 * @param uiBBLKNum Buffer�е�BLK��
 * @param uiIndex ��ӦBuffer BLK�ڲ�������ֻ��0 ~ 6���ã�ʣ��Ϊ��ָ�룩
 * @param pBuf �ڴ滺����
 * @return record_t ��¼��8B
 */
record_t bGetBLKRecord(uINT uiBBLKNum, uINT uiIndex, pBuffer pBuf){
    char               temp[sizeof(int) + 1];
    record_t           record;
    
    memset(temp, 0, sizeof(int) + 1);
    record.attr1 = INVALID_ATTR;
    record.attr2 = INVALID_ATTR;
    
    if(!checkBBLKNum(uiBBLKNum, pBuf)){
        return record;
    }

    if(!checkBIndex(uiIndex)){
        return record;
    }    

    for (size_t i = 0; i < sizeof(int); i++)
    {
        temp[i] = *(GET_BUF_DATA(pBuf, uiBBLKNum) + uiIndex * 8 + i);
    }
    record.attr1 = atoi(temp);

    for (size_t i = 0; i < sizeof(int); i++)
    {
        temp[i] = *(GET_BUF_DATA(pBuf, uiBBLKNum) + uiIndex * 8 + 4 + i);
    }
    record.attr2 = atoi(temp);
    return record;
}

/**
 * @brief ��Buffer BLK�е�uiIndex�ط�д��record
 * 
 * @param uiBBLKNum Buffer BLK��
 * @param uiIndex Buffer BLK�ڲ�����
 * @param record Ҫд��ļ�¼
 * @param pBuf �ڴ滺����
 * @return bError �ֱ������������ڴ��Ŵ����Լ��޴���
 */
bError bSetBLKRecord(uINT uiBBLKNum, uINT uiIndex, record_t record, pBuffer pBuf){
    char              temp[sizeof(int) + 1];
    
    memset(temp, 0, sizeof(int) + 1);
    
    if(!checkBBLKNum(uiBBLKNum, pBuf)){
        return BUF_BBNUM_ERROR;
    }

    if(!checkBIndex(uiIndex)){
        return BUF_INDEX_ERROR;
    }    

    itoa(record.attr1, temp, 10);
    for (size_t i = 0; i < sizeof(int); i++)
    {
        *(GET_BUF_DATA(pBuf, uiBBLKNum) + uiIndex * 8 + i) = temp[i];
    }

    itoa(record.attr2, temp, 10);
    for (size_t i = 0; i < sizeof(int); i++)
    {
        *(GET_BUF_DATA(pBuf, uiBBLKNum) + uiIndex * 8 + 4 + i) = temp[i];
    }
    return BUF_NO_ERROR;
}
/**
 * @brief ��Buffer BLK��д����һ���ֵ
 * 
 * @param uiBBLKNum Buffer BLK��
 * @return uINT ��һ���ֵ
 */
uINT bSetBLKNextBLK(uINT uiBBLKNum, uINT uiDBLKNextNum, pBuffer pBuf){
    char    temp[sizeof(int) + 1];

    itoa(uiDBLKNextNum, temp, 10);
    for (size_t i = 0; i < sizeof(int); i++)
    {
        *(GET_BUF_DATA(pBuf, uiBBLKNum) + BLK_NRECORD * 8 + i) = temp[i];
    }
    return uiDBLKNextNum;
}



/**
 * @brief ���Buffer BLK�е����ݣ�ȫ����Ϊ0��
 * 
 * @param uiBBLKNum
 * @param pBuf 
 */
void bClearBLK(uINT uiBBLKNum, pBuffer pBuf){

    if(!checkBBLKNum(uiBBLKNum, pBuf)){
        return;
    }

    memset(GET_BUF_DATA(pBuf, uiBBLKNum), 0, DISK_BLK_PER_SZ);
}


/**
 * @brief �鿴Buffer״̬
 * 
 * @param pBuf ������ 
 */
void checkBuffer(pBuffer pBuf){
    record_t    record;
    printf("buffer.blkSize: %d\n",pBuf->blkSize);
    printf("buffer.bufSize: %d\n",pBuf->bufSize);
    printf("buffer.numAllBlk: %d\n",pBuf->numAllBlk);
    printf("buffer.numFreeBlk: %d\n",pBuf->numFreeBlk);
    printf("buffer.numIO: %d\n",pBuf->numIO);

    for (uINT uiBBLKNum = 1; uiBBLKNum < pBuf->numAllBlk - pBuf->numFreeBlk + 1; uiBBLKNum++)
    {
        printf("[BBLK %d]: ", uiBBLKNum);
        for (uINT uiIndex = 0; uiIndex < BLK_NRECORD; uiIndex++)
        {
            record = bGetBLKRecord(uiBBLKNum, uiIndex, pBuf);
            printf("(%d, %d)\t", record.attr1, record.attr2);
        }
        printf("\n");
    }
    
}