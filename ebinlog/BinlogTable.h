#ifndef __MY_TABLEMAP_H
#define __MY_TABLEMAP_H

#include "Python.h"
#include "BinlogTypes.h"

#define NAME_MAX_LENGTH 128
#define MAX_COLUMNS     128  

class  CMyTableMap{

private:
    uint64  _llTableID;
    uint16  _nColumnsNumber;
    uint16  _nBitmapSize;
    uint32  _szMeta[MAX_COLUMNS];    
    uint8   _szType[MAX_COLUMNS];
    uint32  _szNullAble[MAX_COLUMNS];
    char    _szTableName[NAME_MAX_LENGTH];
    char    _szDBName[NAME_MAX_LENGTH];

public:
    CMyTableMap(){
        _llTableID = 0;
        _nColumnsNumber = 0;
        _nBitmapSize = 0;
    }
    
    int process(const char* pBuf){
        memset(_szTableName,0,NAME_MAX_LENGTH);
        memset(_szDBName,0,NAME_MAX_LENGTH);
        uint32 nPos = 0;
        _llTableID = uint6korr(pBuf);
        nPos += 6;
        nPos += 2;  //skip tablemap start flags 
        uint32 nLen = 0;
        
        //DBName
        nLen = (int16)(uint8)(*(pBuf+nPos));
        nPos += 1;
        memcpy(_szDBName,pBuf+nPos,nLen);
        nPos += nLen+1;

        //TableName
        nLen = (int16)(uint8)(*(pBuf+nPos));
        nPos += 1;
        memcpy(_szTableName,pBuf+nPos,nLen); 
        nPos += nLen+1;

        //columns info
        _nColumnsNumber = DecodePacketInt(pBuf+nPos,&nPos);
    
        //column data type
        const char* pTypeBuf = pBuf + nPos ;
        nPos += _nColumnsNumber; 

        //meta info
        int nMetaLen = DecodePacketInt(pBuf+nPos,&nPos);
        const char* pMetaBuf = pBuf + nPos ; 
        nPos += nMetaLen; 
        const char* pNullAble = pBuf + nPos ;  

        _decodeMetaData(_nColumnsNumber,pTypeBuf,pMetaBuf);
        _decodeDataType(pTypeBuf);
        _decodeNullAble(pNullAble);
        return 1;
    }

    void _decodeMetaData(int nColumnsNumber,const char* pTypeBuf,const char* pMetaBuf) {
        int j = 0 ;
        uint8 nType = 0;
        int nMeta = 0;
        for (int i = 0; i < nColumnsNumber; i++){
            nType = (uint8)pTypeBuf[i]; 
            if (nType == MYSQL_TYPE_TINY_BLOB || nType == MYSQL_TYPE_BLOB ||
                nType == MYSQL_TYPE_MEDIUM_BLOB || nType == MYSQL_TYPE_LONG_BLOB ||
                nType == MYSQL_TYPE_DOUBLE || nType == MYSQL_TYPE_FLOAT ||
                nType == MYSQL_TYPE_GEOMETRY) {
                nMeta = (int16)(uint8)(*(pMetaBuf+j)); 
                _szMeta[i] = nMeta;
                j += 1;
            }
            else if (nType == MYSQL_TYPE_SET || nType == MYSQL_TYPE_ENUM ||
                nType == MYSQL_TYPE_STRING) {
                int meta_j = (int)(uint8)(*(pMetaBuf+j));
                j += 1;
                int x = meta_j << 8;
                meta_j = (int)(uint8)(*(pMetaBuf+j)); 
                j += 1;
                _szMeta[i] = x + meta_j;
            }
            else if(nType ==MYSQL_TYPE_BIT) {
                int meta_j = (int16)(uint8)(*(pMetaBuf+j));
                j += 1;
                int x = meta_j;
                meta_j = (int16)(uint8)(*(pMetaBuf+j));
                j += 1;
                x = x + meta_j << 8;
                nMeta = x;
                _szMeta[i] = nMeta;
            }
            else if(nType == MYSQL_TYPE_VARCHAR) {
                int meta_j = sint2korr(pMetaBuf+j);
                j += 2;
                _szMeta[i] = meta_j;
    
            }
            else if(nType == MYSQL_TYPE_NEWDECIMAL) {
                int meta_j = (int16)(uint8)(*(pMetaBuf+j));
                j += 1;
                int x = meta_j << 8;
                meta_j = (int16)(uint8)(*(pMetaBuf+j));
                j += 1;
                x += meta_j;
                _szMeta[i] = x;
            }
            else {
                _szMeta[i] = 0;
            }
        }
    }

    void _decodeDataType(const char* pTypeBuf) {
        uint8 nType = 0;
        uint32 nMeta = 0;
        uint8 nRealType = 0;
        for(int i = 0; i < _nColumnsNumber; i++){
            nType = (uint8)(uint8)(pTypeBuf[i]);
            if(nType == MYSQL_TYPE_STRING) {
                nMeta = _szMeta[i];
                nRealType = nMeta >> 8;
                if (nRealType == MYSQL_TYPE_ENUM || nRealType == MYSQL_TYPE_SET) {
                    _szType[i] = nRealType;
                }
                else if (nType == MYSQL_TYPE_DATE) {
                    _szType[i] = MYSQL_TYPE_NEWDATE;
                }
                else {
                    _szType[i] = nType;
                }
            }
            else {
                _szType[i] = nType; 
            } 
        }
    }

    void _decodeNullAble(const char* pNullAble){
        int16 nBit = 0;
        uint32 tmp = 0;
        for (int i = 0; i < _nColumnsNumber; i++) {      
            nBit = (int16)(uint8)(pNullAble[i/8]);
            tmp = ((nBit & (1 << (i%8))) == (1 << (i%8)));
            _szNullAble[i] = tmp;
        }
    }

    char* getTableName(){
        return _szTableName;
    }

    uint64 getTableID(){
        return _llTableID;
    }

    char* getDBName(){
        return _szDBName;
    }

    uint32 getMeta(int i){
        return _szMeta[i]; 
    }

    uint32 getNullAble(int i){
        return _szNullAble[i]; 
    }

    uint32 getType(int i){
        return _szType[i]; 
    }

    uint32 getColumnsNumber(){
        return _nColumnsNumber;
    }
    
    uint32 getBitmapSize(){
        return (_nColumnsNumber + 7)/8;
    }
};

#endif
