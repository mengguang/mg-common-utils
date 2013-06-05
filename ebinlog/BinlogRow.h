#ifndef __MY_ROWLOGPARSER_H
#define __MY_ROWLOGPARSER_H

#include "Python.h"
#include "BinlogTypes.h"
#include "BinlogTable.h"

#define NAME_MAX_LENGTH 128
#define MAX_COLUMNS     128  

typedef struct BinlogEvent{
    uint32 nTimeStamp;
    uint32 nTypeCode;
    uint32 nServerID;
    uint32 nEventLen;
    uint32 nNextPos;
    uint32 nFlag;
    const char* pBuf;
} BinlogEvent;

class  CMyRowLogParser {
private:
    uint32  _nTablePos;     //offset the pos from the file head.Modify By table map,rotate event
    uint32  _nBufOffSet;    //record the offset from the raw data buf
    uint32  _nStartPos;     //callers require start position
    char    _szCurLogName[NAME_MAX_LENGTH];
    uint8   _nCurStat;      // #0,means Null;1,mean,find table event;2,means find endflag;3,means,Idle stat
    PyObject* _pEventResults; //parser result
    uint32  _nItemIndex;    //record item index between table map and endflag #Modify By affect,table map 
    uint32  _nStartIndex;
    const char*   _pBuf;    //need to parser raw data
    uint32  _nCurPos;       //offset the tablemap  
    uint32  _nBufLen;       //raw data len
    BinlogEvent _nCurEvent;
    CMyTableMap _tableMap;
    
public:
    CMyRowLogParser() {}

    void Init(const char* pLogName,uint32 nLogNameLen,uint32 nStartPos,uint32 nStartIndex){
        memcpy(_szCurLogName,pLogName,nLogNameLen);
        _nTablePos = 0;
        _nCurPos = 0;
        _nStartPos = nStartPos;
        _nCurStat = 0;
        _nItemIndex = 0;
        _nStartIndex = nStartIndex;
        _nBufOffSet = 0;
    }

    PyObject* process(const char* pBuf,int nBufLen) {
        _pBuf = pBuf;
        _nBufOffSet = 0;
        _nBufLen = nBufLen;
        PyObject* pRows = PyList_New(0); 
        PyObject* pResult = PyList_New(0);
        while (_nBufLen) {
            if(_getEvent() == 0) {
                break;
            }
            processRowEvent(pRows);
        }

        PyObject* pItem = NULL;
        if ( PyList_Size(pRows) != 0) {
            pItem = Py_BuildValue("i",1);
            PyList_Append(pResult,pItem);
            Py_DECREF(pItem);

            pItem = Py_BuildValue("i",_nBufOffSet);
            PyList_Append(pResult,pItem);
            Py_DECREF(pItem);

            PyList_Append(pResult,pRows);
            Py_DECREF(pRows);
        }
        else {
            pItem = Py_BuildValue("i",0);
            PyList_Append(pResult,pItem);
            Py_DECREF(pItem);

            pItem = Py_BuildValue("i",_nBufOffSet);
            PyList_Append(pResult,pItem);
            Py_DECREF(pItem);

            pItem = Py_BuildValue("i",0);
            PyList_Append(pResult,pItem);
            Py_DECREF(pRows);
        }
        return pResult;
    }

    void processRowEvent(PyObject* pRows){
        uint32 nTypeCode = _nCurEvent.nTypeCode;
        if (nTypeCode == TABLE_MAP_EVENT){
            _tableMap.process(_nCurEvent.pBuf);
            if (_nCurStat != 0) {
                _nTablePos += (_nCurPos - _nCurEvent.nEventLen);
                _nStartIndex = 0;
            }
            else if (_nCurStat == 0) {
                if (_nStartPos > 4) {
                    _nTablePos = _nStartPos;
                }
                else if (_nStartPos == 4) {
                    _nTablePos = 4 + (_nCurPos - _nCurEvent.nEventLen);
                    _nStartPos = 0;
                }
            }
            _nCurStat = 1;
            _nItemIndex = 0;
            _nCurPos = _nCurEvent.nEventLen;
        }
        else if(nTypeCode == ROTATE_EVENT) {
            char szCurLogName[NAME_MAX_LENGTH] = {0};
            memcpy(szCurLogName,_nCurEvent.pBuf + 8,_nCurEvent.nEventLen - 27); //19 + 8 mean,header 19
            if( !strcmp(_szCurLogName,szCurLogName)) {
                _nTablePos = 0;
            }
            memcpy(_szCurLogName,szCurLogName,_nCurEvent.nEventLen - 27); 
            _nCurPos = 0;
            _nTablePos = 4;
        }
        else if (nTypeCode == WRITE_ROWS_EVENT ||
                nTypeCode == DELETE_ROWS_EVENT ||
                nTypeCode == UPDATE_ROWS_EVENT) {
            uint32 nOffSet = 0;
            //uint64 llTableID = uint6korr(_nCurEvent.pBuf + nOffSet); 
            nOffSet += 6;
            uint16 nEndFlag = uint2korr(_nCurEvent.pBuf + nOffSet); 
            nOffSet += 2;
            int nColumnsNumber = DecodePacketInt(_nCurEvent.pBuf + nOffSet,&nOffSet);
            int nBitmapSize = (nColumnsNumber + 7)/8;
            //const char* pBitmapData = _nCurEvent.pBuf + nOffSet + nColumnsNumber; 
            nOffSet += nBitmapSize;
            if (nEndFlag & 0x01) {
                _nCurStat = 2;
            }
            _decodeEvent(_nCurEvent.pBuf + nOffSet,_nCurEvent.nEventLen - 19 - nOffSet,pRows);
        }
    }

    int32 _getEvent() {
        if (_nBufLen < 19){
            return 0;
        }
        uint32 nPos = 0;
        _nCurEvent.nTimeStamp  = uint4korr(_pBuf + nPos); 
        nPos += 4;
        _nCurEvent.nTypeCode  = (uint32)(uint8)(*(_pBuf + nPos)) ; 
        nPos += 1;
        _nCurEvent.nServerID = uint4korr(_pBuf + nPos); 
        nPos += 4;
        _nCurEvent.nEventLen = uint4korr(_pBuf + nPos); 
        nPos += 4;
        _nCurEvent.nNextPos = uint4korr(_pBuf + nPos); 
        nPos += 4;
        _nCurEvent.nFlag = uint2korr(_pBuf + nPos); 
        nPos += 2;
        if (_nBufLen < _nCurEvent.nEventLen) {
            return 0;
        }
        _nCurEvent.pBuf = _pBuf + nPos;
        _nBufLen -= _nCurEvent.nEventLen;
        _nCurPos += _nCurEvent.nEventLen;
        _nBufOffSet += _nCurEvent.nEventLen;
        _pBuf = _pBuf + _nCurEvent.nEventLen;
        return 1;
    }

    void _decodeEvent(const char* pBuf,uint32 nBufLen,PyObject* pRows){
        uint32 nPos = 0;
        char szAct[128] = {0};
        char szPosition[512] = {0};
        uint32 nEventType = _nCurEvent.nTypeCode;
        if (nEventType == UPDATE_ROWS_EVENT) {
            nPos += _tableMap.getBitmapSize(); 
        }
        while (nPos < nBufLen) {
            PyObject* pUpdateRow = PyDict_New();
            PyObject* pRow = NULL;
            if(nEventType == UPDATE_ROWS_EVENT) {
                pRow = _decodeOneRow(&nPos,pBuf);
                PyDict_SetItemString(pUpdateRow,"old",pRow);
                Py_DECREF(pRow);

                PyDict_SetItemString(pUpdateRow,"old",pRow);
                pRow = _decodeOneRow(&nPos,pBuf);
                PyDict_SetItemString(pUpdateRow,"new",pRow);
                Py_DECREF(pRow);
                sprintf(szAct,"%s","update");
            }
            else {
                pRow = _decodeOneRow(&nPos,pBuf); 
                if (nEventType == DELETE_ROWS_EVENT) {
                    PyDict_SetItemString(pUpdateRow,"old",pRow);
                    Py_DECREF(pRow);
                    sprintf(szAct,"%s","delete");
                }
                else if(nEventType == WRITE_ROWS_EVENT) {
                    PyDict_SetItemString(pUpdateRow,"new",pRow);
                    Py_DECREF(pRow);
                    sprintf(szAct,"%s","insert");
                }
            }
            sprintf(szPosition,"%s/%d/%d",_szCurLogName,_nTablePos,_nItemIndex);
            PyObject* pContent = PyDict_New();
            PyObject* pItem = Py_BuildValue("s",szPosition);
            PyDict_SetItemString(pContent,"pos",pItem);
            Py_DECREF(pItem);

            pItem = Py_BuildValue("s",szAct);
            PyDict_SetItemString(pContent,"act",pItem);
            Py_DECREF(pItem);

            pItem = Py_BuildValue("s",_tableMap.getDBName()); 
            PyDict_SetItemString(pContent,"db",pItem);
            Py_DECREF(pItem);

            pItem = Py_BuildValue("s",_tableMap.getTableName());
            PyDict_SetItemString(pContent,"tbl",pItem);
            Py_DECREF(pItem);

            PyDict_SetItemString(pContent,"row",pUpdateRow);
            Py_DECREF(pUpdateRow);

            if (_nItemIndex >= _nStartIndex){
                PyList_Append(pRows,pContent);
                Py_DECREF(pContent);
            } 
            _nItemIndex++;
        }
    }

    PyObject* _decodeOneRow(uint32 *pPos,const char* pBuf) {
        const char* pBitmapData = pBuf + *pPos;
        int32 nNullBitIndex = 0;
        int16 nNullFlag = 0;

        int nColumnsNumber = _tableMap.getColumnsNumber();
        PyObject* pRow = PyList_New(0);

        *pPos += _tableMap.getBitmapSize();
        uint32 nType = 0;
        uint32 nMeta = 0;
        PyObject* pItem = NULL; 
        for(int i=0; i < nColumnsNumber; i++) {
            nNullFlag = (pBitmapData[nNullBitIndex/8] >> (nNullBitIndex%8)) & 0x01;
            if (nNullFlag == 0) {
                nType = _tableMap.getType(i);
                nMeta = _tableMap.getMeta(i);
                pItem= _decodeRowImageItem(pBuf+*pPos,nType,nMeta,pPos);
                PyList_Append(pRow,pItem);
                Py_DECREF(pItem);
            }
            else {
                pItem = Py_BuildValue("i",0);
                PyList_Append(pRow,pItem);
                Py_DECREF(pItem);
            }
            nNullBitIndex++;
        }
        return pRow;
    }

    PyObject* _decodeRowImageItem(const char*pBuf,uint32 type,uint32 meta,uint32 *pPos) {
        uint32 nLength= 0;
        if (type == MYSQL_TYPE_STRING) {
            if (meta >= 256) {
                uint byte0= meta >> 8;
                uint byte1= meta & 0xFF;
                if ((byte0 & 0x30) != 0x30) {
                    /* a long CHAR() field: see #37426 */
                    nLength= byte1 | (((byte0 & 0x30) ^ 0x30) << 4);
                    type= byte0 | 0x30;
                }
                else {
                    nLength = meta & 0xFF;
                }
            }
            else {
                nLength= meta;
            }
        }
        switch(type) {
            case MYSQL_TYPE_LONG:       // int
                {
                    int32 i = sint4korr(pBuf); 
                    *pPos += 4;
                    return Py_BuildValue("i",i);
                }
            case MYSQL_TYPE_TINY:       // tinyint
                {
                    int8 c = (int32)(int8)(*pBuf);
                    *pPos += 1;
                    return Py_BuildValue("i",c);
                }
            case MYSQL_TYPE_SHORT:      //smallint
                {
                    int16 s = sint2korr(pBuf);
                    *pPos += 2;
                    return Py_BuildValue("i",s); 
                }
            case MYSQL_TYPE_LONGLONG:   // bigint
                {
                    int64 ll = sint8korr(pBuf);
                    *pPos += 8;
                    return Py_BuildValue("L",ll); 
                }
            case MYSQL_TYPE_INT24:      // medium int
                {
                    int32 i = sint3korr(pBuf); 
                    *pPos += 3;
                    return Py_BuildValue("i",i);
                }
            case MYSQL_TYPE_TIMESTAMP:  // timestamp
                {
                    int32 i = sint4korr(pBuf);
                    *pPos += 4;
                    return Py_BuildValue("i",i);
                }
            case MYSQL_TYPE_DATETIME:   // datetime
                {
                    uint64 ll = uint8korr(pBuf);
                    char szDateTime[128] = {0};
                    int32 d = ll/1000000;
                    int32 t = ll%1000000;
                    sprintf(szDateTime,"%04d-%02d-%02d %02d:%02d:%02d",
                            d / 10000, (d % 10000) / 100, d % 100,
                            t / 10000, (t % 10000) / 100, t % 100);
                    *pPos += 8;
                    return Py_BuildValue("s#",szDateTime,strlen(szDateTime) + 1);
                }

            case MYSQL_TYPE_TIME:       // time
                {
                    uint32 i32= uint3korr(pBuf);
                    char szTime[128] = {0};
                    sprintf(szTime, "'%02d:%02d:%02d'",
                            i32 / 10000, (i32 % 10000) / 100, i32 % 100);
                    *pPos += 3;
                    return Py_BuildValue("s#",szTime,strlen(szTime)); 
                }

            case MYSQL_TYPE_NEWDATE:    // date ??
                {
                    uint32 tmp= uint3korr(pBuf);
                    int part;
                    char buf[11];
                    char *pos= &buf[10];  // start from '\0' to the beginning

                    /* Copied from field.cc */
                    *pos--=0;                 // End NULL
                    part=(int) (tmp & 31);
                    *pos--= (char) ('0'+part%10);
                    *pos--= (char) ('0'+part/10);
                    *pos--= ':';
                    part=(int) (tmp >> 5 & 15);
                    *pos--= (char) ('0'+part%10);
                    *pos--= (char) ('0'+part/10);
                    *pos--= ':';
                    part=(int) (tmp >> 9);
                    *pos--= (char) ('0'+part%10); part/=10;
                    *pos--= (char) ('0'+part%10); part/=10;
                    *pos--= (char) ('0'+part%10); part/=10;
                    *pos=   (char) ('0'+part);

                    char szNewDate[128] = {0};
                    sprintf(szNewDate,"%s",buf);

                    *pPos += 3;
                    return Py_BuildValue("s#",szNewDate,strlen(szNewDate));
                }
                
            case MYSQL_TYPE_DATE:       // date
                {
                    uint i32= uint3korr(pBuf);

                    char szDate[128] = {0};
                    sprintf(szDate,"'%04d-%02d-%02d'",
                            (int)(i32 / (16L * 32L)), (int)(i32 / 32L % 16L), (int)(i32 % 32L));
                    *pPos += 3;
                    return Py_BuildValue("s#",szDate,strlen(szDate)); 
                }

            case MYSQL_TYPE_YEAR:
                {
                    uint32 i32= (uint32)*pBuf;
                    char szYear[8];
                    sprintf(szYear, "%04d", i32+ 1900);
                    *pPos += 1;
                    return Py_BuildValue("s#",szYear,strlen(szYear)); 
                }

            case MYSQL_TYPE_ENUM:
                {
                    char szBuf[32] = {0};
                    switch (meta & 0xFF) {
                        case 1:
                            sprintf(szBuf,"%d",(int)*pBuf);
                            *pPos += 1;
                            return Py_BuildValue("s#",szBuf,strlen(szBuf));
                        case 2:
                            {
                                int32 i32= uint2korr(pBuf);
                                sprintf(szBuf,"%d",i32);
                                *pPos += 2;
                                return Py_BuildValue("s#",szBuf,strlen(szBuf));
                            }
                        default:
                            return Py_BuildValue("s","Error about MYSQL_TYPE_ENUM");
                    }
                }
                break;

            case MYSQL_TYPE_SET:
                {
                    char szBuf[32] = {0};
                    memcpy(szBuf,pBuf,meta & 0xFF);
                    *pPos += meta & 0xFF;
                    return Py_BuildValue("s#",pBuf,meta & 0xFF);
                }
            case MYSQL_TYPE_BLOB:
                switch (meta) {
                    case 1:     //TINYBLOB/TINYTEXT
                        {

                            nLength = (uint8)(*pBuf);
                            *pPos += nLength + 1;
                            return Py_BuildValue("s#",pBuf+1,nLength);
                        }
                    case 2:     //BLOB/TEXT
                        {

                            nLength= uint2korr(pBuf);
                            *pPos += nLength + 2;
                            return Py_BuildValue("s#",pBuf+2,nLength);
                        }
                    case 3:     //MEDIUMBLOB/MEDIUMTEXT
                        {
                            nLength= uint3korr(pBuf);
                            *pPos += nLength + 3;
                            return Py_BuildValue("s#",pBuf + 3,nLength);

                        }
                    case 4:     //LONGBLOB/LONGTEXT
                        {
                            nLength= uint4korr(pBuf);
                            *pPos += nLength + 4;
                            return Py_BuildValue("s#",pBuf + 4,nLength);
                        }
                    default:
                        return Py_BuildValue("s","type isn't support");
                }

            case MYSQL_TYPE_VARCHAR:
            case MYSQL_TYPE_VAR_STRING:
                {
                    nLength = meta;
                    if (nLength < 256){
                        nLength = (uint8)(*pBuf);
                        *pPos += 1 + nLength;
                        return Py_BuildValue("s#",pBuf+1,nLength);
                    }else{
                        nLength = uint2korr(pBuf);
                        *pPos += 2 + nLength;
                        return Py_BuildValue("s#",pBuf+2,nLength);
                    }
                }

            case MYSQL_TYPE_STRING:
                {
                    if (nLength < 256){
                        nLength = (uint8)(*pBuf);
                        *pPos += 1 + nLength;
                    //    memcpy(szBuf,pBuf+1,nLength);
                        return Py_BuildValue("s#",pBuf+1,nLength);
                    }else{
                        nLength = uint2korr(pBuf);
                        *pPos += 2 + nLength;
                        return Py_BuildValue("s#",pBuf+2,nLength);
                    }

                }

            case MYSQL_TYPE_BIT:
                {
                    uint32 nBits= ((meta >> 8) * 8) + (meta & 0xFF);
                    nLength= (nBits + 7) / 8;
                    char szBuf[16] = {0};
                    memcpy(szBuf,pBuf,nLength);
                    *pPos += nLength;

                    return Py_BuildValue("s#",szBuf,nLength);    
                }
            case MYSQL_TYPE_FLOAT:
                {
                    float fl;
                    float4get(fl, pBuf);
                    *pPos += 4;
                    return Py_BuildValue("f",fl);
                }
            case MYSQL_TYPE_DOUBLE:
                {
                    double dbl;
                    float8get(dbl,pBuf);
                    *pPos += 8;                        

                    return Py_BuildValue("d",dbl);
                }
            case MYSQL_TYPE_NEWDECIMAL:
                {
                    uint8 precision= meta >> 8;
                    uint8 decimals= meta & 0xFF;
                    //uint8 bin_size= my_decimal_get_binary_size(precision, decimals);
                    my_decimal dec;
                    binary2my_decimal(E_DEC_FATAL_ERROR, (uint8*) pBuf, &dec,precision, decimals);
                    int i, end;
                    char buff[512], *pos;
                    pos= buff;
                    pos+= sprintf(buff, "%s", dec.sign() ? "-" : "");
                    end= ROUND_UP(dec.frac) + ROUND_UP(dec.intg)-1;
                    for (i=0; i < end; i++)
                        pos+= sprintf(pos, "%09d.", dec.buf[i]);
                    pos+= sprintf(pos, "%09d", dec.buf[i]);
                    return Py_BuildValue("s#",buff,strlen(buff));
                }

            default:
                return Py_BuildValue("i",0);
        }
    }
};

#endif
