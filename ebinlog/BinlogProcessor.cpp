#include "Python.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "BinlogTypes.h"
#include "BinlogTable.h"
#include "BinlogRow.h"

#define MAX_BUF_SIZE  (16*1024*1024)  //16M
char szBody[MAX_BUF_SIZE] = {0};

PyObject* wrap_Recv(PyObject* self, PyObject* args){
	int32 nSockFD = 0;
	char szHead[4] = {0};
	int32 nRecvHeadLen = 0;
	int32 nRecvLen = 0;
	int32 nBodyLen = 0;
	int32 nRecvBodyLen = 0;

	if(!PyArg_ParseTuple(args, "i", &nSockFD)){
		return Py_BuildValue("i", -1);
	}

	Py_BEGIN_ALLOW_THREADS;
	nRecvHeadLen = read(nSockFD, szHead, 4);
	Py_END_ALLOW_THREADS;

	if(nRecvHeadLen == -1) {
	    return Py_BuildValue("i",-2);
	}
	
	while (4 - nRecvHeadLen) {
		Py_BEGIN_ALLOW_THREADS;
		nRecvLen = read(nSockFD, szHead+nRecvHeadLen, 4 - nRecvHeadLen);
		Py_END_ALLOW_THREADS;
		if(nRecvLen == -1) {
		    return Py_BuildValue("i",-3);
		}
		if(nRecvLen == 0) {
			return Py_BuildValue("i",0);
		}
		nRecvHeadLen += nRecvLen;
	}
	
	nBodyLen = uint3korr(szHead);
	//memset(szBody,0,MAX_BUF_SIZE); //not must,and memset will affect speed
	while (nBodyLen - nRecvBodyLen) {
		Py_BEGIN_ALLOW_THREADS;
		nRecvLen = read(nSockFD, szBody+nRecvBodyLen, nBodyLen-nRecvBodyLen);
		Py_END_ALLOW_THREADS;
		if(nRecvLen == -1) {
		    return Py_BuildValue("i",-4);
		}
		if (nRecvLen == 0) {
			return Py_BuildValue("i", 0);
		}
		nRecvBodyLen += nRecvLen;
	}
	
	return Py_BuildValue("s#", szBody, nBodyLen);
}

static CMyRowLogParser *pParser = NULL;

PyObject* wrap_Process(PyObject* self, PyObject* args) {
	const char* pBuf = NULL;
	uint32  nBufLen = 0;
	if (pParser == NULL) {
		return Py_BuildValue("i", 0);
	}
	if(!PyArg_ParseTuple(args, "s#", &pBuf, &nBufLen)) {
		return Py_BuildValue("i",0);
	}
	return pParser->process(pBuf, nBufLen);
}

PyObject* wrap_Init(PyObject* self, PyObject* args) {
    const char* pLogName = NULL;
    uint32  nLogNameLen = 0;
    uint32  nPos = 0;
    uint32  nItemIndex = 0;

    if(!PyArg_ParseTuple(args,"s#ii",&pLogName,&nLogNameLen,&nPos,&nItemIndex)){
        return Py_BuildValue("i",0);
    }

    if (pParser == NULL) {
        pParser = new CMyRowLogParser();
    }
    
    pParser->Init(pLogName,nLogNameLen,nPos,nItemIndex);
    return Py_BuildValue("i",1);
}

PyObject* wrap_DeInit(PyObject* self, PyObject* args) {
    if (pParser != NULL){
        delete pParser;
    }
    return Py_BuildValue("i",1);
}

static PyMethodDef Methods[] = {
    {"Init",wrap_Init,METH_VARARGS,"create pParser object."},
    {"Process",wrap_Process,METH_VARARGS,"use the pParser to process the buffer."},
    {"DeInit",wrap_DeInit,METH_VARARGS,"delete the pParser object."},
    {"Recv",wrap_Recv,METH_VARARGS,"receive a complete event, write it to buffer."},
    {NULL,NULL}
};

#ifdef __cplusplus
extern "C"{
#endif 

void initBinlogProcessor(void) {
    PyObject* m;
    m = Py_InitModule("BinlogProcessor",Methods);
}

#ifdef __cplusplus
}
#endif 
