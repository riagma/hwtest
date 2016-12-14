/*____________________________________________________________________________

  MODULO:		Basic JSON interface

  CATEGORIA:	JSON

  AUTOR:		Ricardo Aguado Mart'n

  VERSION:		0.1

  FECHA:		2016
______________________________________________________________________________

  DESCRIPCION:

  OBSERVACIONES: 
______________________________________________________________________________*/

/*__INCLUDES DEL SISTEMA______________________________________________________*/

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
 
/*__INCLUDES DE LA BD_________________________________________________________*/

/*__INCLUDES DE LA APLICACION_________________________________________________*/

#include "trace_macros.h"
#include "auxfunctions.h"

#include "buffer.h"

#include "jsonif.h"

/*__CONSTANTES________________________________________________________________*/
  
//----------------

const char* JSON_value[] =
{
  "NONE",
  "STRING",
  "NUMBER",
  "OBJECT",
  "ARRAY",
  "TRUE",
  "FALSE",
  "NULL",

  ""
};

//----------------

const char* JSON_RC[] =
{
  "OK",
  "ERROR",
  "MEMORY FULL",
  "NOT FOUND",
  "ALREADY EXISTS",
  "INCOMPLETE",
  "TIMEOUT",

  ""
};

//----------------

/*__TIPOS_____________________________________________________________________*/

/*__VARIABLES EXTERNAS________________________________________________________*/

/*__VARIABLES GLOBALES________________________________________________________*/

/*__VARIABLES LOCALES_________________________________________________________*/

//----------------

static int					JSON_Init_s = 0;

static MEMO_refmemo_t*		JSON_MapMemo_s = 0;
static long					JSON_MapUsed_s = 0;
static long					JSON_MapMark_s = 0;

static MEMO_refmemo_t*		JSON_ValueMemo_s = 0;
static long					JSON_ValueUsed_s = 0;
static long					JSON_ValueMark_s = 0;

static MEMO_refmemo_t*		JSON_PairMemo_s = 0;
static long					JSON_PairUsed_s = 0;
static long					JSON_PairMark_s = 0;

static MEMO_refmemo_t*		JSON_ObjectMemo_s = 0;
static long					JSON_ObjectUsed_s = 0;
static long					JSON_ObjectMark_s = 0;

//----------------

/*__FUNCIONES EXTERNAS________________________________________________________*/

/*__FUNCIONES PUBLICAS________________________________________________________*/

/*__FUNCIONES PRIVADAS________________________________________________________*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int 
JSON_initialize(void)
{
  JSON_map_t			m;
  JSON_value_t			v;
  JSON_pair_t			p;
  JSON_object_t			o;

  int					ret = JSON_RC_OK;

  TRAZA0("Entering in JSON_memo_initialize()");

//----------------

  if(JSON_Init_s == JSON_FALSE)
  {
	JSON_Init_s = JSON_TRUE;

	JSON_MapMemo_s = MEMO_createRefMemo(&m, &m.memo, sizeof(m), 128, 2);

    if(JSON_MapMemo_s == NULL)
    {
      JSON_FATAL0("ERROR: MEMO_createRefMemo()");
    }

	JSON_ValueMemo_s = MEMO_createRefMemo(&v, &v.memo, sizeof(v), 1024, 4);

    if(JSON_ValueMemo_s == NULL)
    {
      JSON_FATAL0("ERROR: MEMO_createRefMemo()");
    }

	JSON_PairMemo_s = MEMO_createRefMemo(&p, &p.memo, sizeof(p), 1024, 4);

    if(JSON_PairMemo_s == NULL)
    {
      JSON_FATAL0("ERROR: MEMO_createRefMemo()");
    }

	JSON_ObjectMemo_s = MEMO_createRefMemo(&o, &o.memo, sizeof(o), 256, 4);

    if(JSON_ObjectMemo_s == NULL)
    {
      JSON_FATAL0("ERROR: MEMO_createRefMemo()");
    }
  }

//----------------

  TRAZA1("Returning from JSON_memo_initialize() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

void
JSON_memo_view(void)
{

  SUCESO3("JSON-MAP    = %ld(%ld)(%ld)", JSON_MapUsed_s,
	                                     JSON_MapMark_s,
					                     JSON_MapMemo_s->list.nume);

  SUCESO3("JSON-VALUE  = %ld(%ld)(%ld)", JSON_ValueUsed_s,
	                                     JSON_ValueMark_s,
					                     JSON_ValueMemo_s->list.nume);

  SUCESO3("JSON-PAIR   = %ld(%ld)(%ld)", JSON_PairUsed_s,
	                                     JSON_PairMark_s,
					                     JSON_PairMemo_s->list.nume);

  SUCESO3("JSON-OBJECT = %ld(%ld)(%ld)", JSON_ObjectUsed_s,
	                                     JSON_ObjectMark_s,
					                     JSON_ObjectMemo_s->list.nume);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

JSON_map_t*
JSON_map_new(void)
{
  JSON_map_t*		ptrMap = NULL;

//TRAZA0("Entering in JSON_map_new()");

//----------------

  ptrMap = (JSON_map_t*) MEMO_new(JSON_MapMemo_s);

  if(ptrMap == NULL)
  {
    JSON_FATAL0("MEMO_new()");
  }

  JSON_MapUsed_s++;

  if(JSON_MapMark_s < JSON_MapUsed_s)
  {
	JSON_MapMark_s = JSON_MapUsed_s;
  }

//----------------

//TRAZA1("Returning from JSON_map_new() = %p", ptrMap);

  return ptrMap;
}

/*----------------------------------------------------------------------------*/

void
JSON_map_delete(JSON_map_t* inMap)
{
//TRAZA1("Entering in JSON_map_delete(%p)", inMap);

//----------------

  MEMO_delete(JSON_MapMemo_s, inMap);

  JSON_MapUsed_s--;

//----------------

//TRAZA0("Returning from JSON_map_delete()");
}

/*----------------------------------------------------------------------------*/

int
JSON_map_cmp(void* inVoidA, void* inVoidB)
{
  JSON_map_t*		inMapA = inVoidA;
  JSON_map_t*		inMapB = inVoidB;

  int				cmp = 0;

//TRAZA2("Entering in JSON_map_cmp(%p, %p)", inMapA, inMapB);

//----------------

  cmp = strcasecmp(inMapA->key, inMapB->key);

//----------------

//TRAZA1("Returning from JSON_map_cmp() = %d", cmp);

  return cmp;
}

/*----------------------------------------------------------------------------*/

int
JSON_map_find(RFTR_reftree_t* inTree, char* inKey)
{
  JSON_map_t		map[1];
  JSON_map_t*		ptrMap;

  int				outIdx;

//TRAZA2("Entering in JSON_map_find(%p, %s)", inTree, inKey);

//----------------

  map->key = inKey;

  ptrMap = RFTR_find(inTree, map, NULL, NULL);

  if(ptrMap != NULL)
  {
	outIdx = ptrMap->idx;
  }

  else { map->idx = -1; }

//----------------

//TRAZA1("Returning from JSON_map_find() = %d", outIdx);

  return outIdx;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

JSON_value_t*
JSON_value_new(void)
{
  JSON_value_t*		ptrValue = NULL;

//TRAZA0("Entering in JSON_value_new()");

//----------------

  ptrValue = (JSON_value_t*) MEMO_new(JSON_ValueMemo_s);

  if(ptrValue == NULL)
  {
    JSON_FATAL0("MEMO_new()");
  }

  JSON_ValueUsed_s++;

  if(JSON_ValueMark_s < JSON_ValueUsed_s)
  {
	JSON_ValueMark_s = JSON_ValueUsed_s;
  }

//----------------

//TRAZA1("Returning from JSON_value_new() = %p", ptrValue);

  return ptrValue;
}

/*----------------------------------------------------------------------------*/

void
JSON_value_delete(JSON_value_t* inValue)
{
  JSON_value_t*			ptrValue;

//TRAZA1("Entering in JSON_value_delete(%p)", inValue);

//----------------

  if(inValue->type == JSON_VALUE_OBJECT)
  {
	JSON_object_delete(inValue->value);
  }

  else if(inValue->type == JSON_VALUE_ARRAY)
  {
	while((ptrValue = RLST_extractHead(inValue->value)))
	{
	  JSON_value_delete(ptrValue);
	}
  }

  MEMO_delete(JSON_ValueMemo_s, inValue);

  JSON_ValueUsed_s--;

//----------------

//TRAZA0("Returning from JSON_value_delete()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

JSON_pair_t*
JSON_pair_new(void)
{
  JSON_pair_t*		ptrPair = NULL;

//TRAZA0("Entering in JSON_pair_new()");

//----------------

  ptrPair = (JSON_pair_t*) MEMO_new(JSON_PairMemo_s);

  if(ptrPair == NULL)
  {
    JSON_FATAL0("MEMO_new()");
  }

  JSON_PairUsed_s++;

  if(JSON_PairMark_s < JSON_PairUsed_s)
  {
	JSON_PairMark_s = JSON_PairUsed_s;
  }

//----------------

//TRAZA1("Returning from JSON_pair_new() = %p", ptrPair);

  return ptrPair;
}

/*----------------------------------------------------------------------------*/

void
JSON_pair_delete(JSON_pair_t* inPair)
{
//TRAZA1("Entering in JSON_pair_delete(%p)", inPair);

//----------------

  if(inPair->value != NULL)
  {
	JSON_value_delete(inPair->value);
  }

  MEMO_delete(JSON_PairMemo_s, inPair);

  JSON_PairUsed_s--;

//----------------

//TRAZA0("Returning from JSON_pair_delete()");
}

/*----------------------------------------------------------------------------*/

int
JSON_pair_cmp(void* inVoidA, void* inVoidB)
{
  JSON_pair_t*		inPairA = inVoidA;
  JSON_pair_t*		inPairB = inVoidB;

  int				cmp = 0;

//TRAZA2("Entering in JSON_pair_cmp(%p, %p)", inPairA, inPairB);

//----------------

  cmp = strcasecmp(inPairA->name, inPairB->name);

//----------------

//TRAZA1("Returning from JSON_pair_cmp() = %d", cmp);

  return cmp;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

JSON_object_t*
JSON_object_new(void)
{
  JSON_object_t*		ptrObject = NULL;
  JSON_pair_t			p;

//TRAZA0("Entering in JSON_object_new()");

//----------------

  ptrObject = (JSON_object_t*) MEMO_new(JSON_ObjectMemo_s);

  if(ptrObject == NULL)
  {
    JSON_FATAL0("MEMO_new()");
  }

  JSON_ObjectUsed_s++;

  if(JSON_ObjectMark_s < JSON_ObjectUsed_s)
  {
	JSON_ObjectMark_s = JSON_ObjectUsed_s;
  }

//----------------

  if(RLST_initializeRefList(ptrObject->pairList, &p, &p.list) < 0)
  {
    JSON_FATAL0("RLST_initializeRefList()");
  }

  if(RFTR_initializeRefTree(ptrObject->pairTree, &p, &p.tree, JSON_pair_cmp) < 0)
  {
    JSON_FATAL0("RFTR_initializeRefTree()");
  }

//----------------

//TRAZA1("Returning from JSON_object_new() = %p", ptrObject);

  return ptrObject;
}

/*----------------------------------------------------------------------------*/

void
JSON_object_delete(JSON_object_t* inObject)
{
  JSON_pair_t*			ptrPair;

//TRAZA1("Entering in JSON_object_delete(%p)", inObject);

//----------------

  while((ptrPair = RFTR_extractMini(inObject->pairTree)))
  {
	JSON_pair_delete(ptrPair);
  }

  MEMO_delete(JSON_ObjectMemo_s, inObject);

  JSON_ObjectUsed_s--;

//----------------

//TRAZA0("Returning from JSON_object_delete()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void
JSON_fatal_error(char* inFile, int inLine, const char* inErrorStr, ...)
{
  va_list		list;

  char			errorStr[JSON_MAXLEN_STRING + 1];

//----------------

  va_start(list, inErrorStr);

  vsnprintf(errorStr, JSON_MAXLEN_STRING + 1, inErrorStr, list);

  va_end(list);

  errorStr[JSON_MAXLEN_STRING] = 0;

//----------------

  TRACE_write(TRACE_TYPE_DEFAULT, 0, "<%s %04d> %s", inFile, inLine, errorStr);

//----------------

  exit(-1);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
JSON_object_decode
(
  JSON_object_t*		inObject,
  BUFF_buff_t*			inBuffer
)
{
  BUFF_part_t*			part;

  char*					ps;
  char*					pc;

  char*					pi;

  char*					pair;

  int					state;

  int					end = JSON_FALSE;
  int					ret = JSON_RC_OK;

  TRAZA2("Entering in JSON_object_decode(%p, %p)", inObject, inBuffer);

//----------------
/*
  BUFF_part_add(inObject->buffRefs, part);

  pi = (char*)(buff->data + buff->idx);

  pc = pi; pc[buff->len] = 0;
*/
//----------------

  if(inObject->decodeState == JSON_DECODE_STATE_INIT && ret == JSON_RC_OK)
  {
    inBuffer->pc1Elm = inBuffer->idxElm;
    inBuffer->pc1Off = inBuffer->idxOff;

    ps = BUFF_strchr(inBuffer, "{");

    if(ps != NULL)
    {
      inBuffer->idxElm = inBuffer->pc2Elm;
      inBuffer->idxOff = inBuffer->pc2Off + 1;

      inBuffer->pc1Elm = inBuffer->idxElm;
      inBuffer->pc1Off = inBuffer->idxOff;

      inObject->decodeState = JSON_DECODE_STATE_PAIR;
    }

    else // if(ps == NULL)
    {
      inBuffer->idxElm = inBuffer->tail;
      inBuffer->idxOff = inBuffer->tail->part->len;

      ret = JSON_RC_ERROR;
    }
  }

//----------------
  
  while((inObject->decodeState == JSON_DECODE_STATE_PAIR ||
	     inObject->decodeState == JSON_DECODE_STATE_VALUE) && ret == JSON_RC_OK)
  {
	if(inObject->decodeState == JSON_DECODE_STATE_PAIR && ret == JSON_RC_OK)
	{
	  ps = BUFF_strchr(inBuffer, "\"");

	  for(state = 0, end = 0; end == 0 && ret == 0;)
	  {
	    if(state == 0)
	    {
	      if(ps != NULL)
	      {
		    if(BUFF_strspn(inBuffer, " \t\r\n") == BUFF_TRUE)
		    {
              inBuffer->pc1Elm = inBuffer->pc2Elm;
              inBuffer->pc1Off = inBuffer->pc2Off + 1;

              ps = BUFF_strchr(inBuffer, "\"");

              state = 1;
		    }

		    else // if(BUFF_strspn(inBuffer, " \t\r\n") == BUFF_FALSE)
	    	{
		      pc = inBuffer->idxElm->part->data + inBuffer->idxOff;

              SUCESO1("ERROR: Decode object (%s)", pc);

		      inBuffer->idxElm = inBuffer->pc2Elm;
		      inBuffer->idxOff = inBuffer->pc2Off + 1;

              ret = JSON_RC_ERROR;
	    	}
	      }

	      else { ret = JSON_RC_INCOMPLETE; }
	    }

	    else if(state == 1)
        {
	      if(ps != NULL)
	      {
	      }

	      else { ret = JSON_RC_INCOMPLETE; }
       }
	  }
	}

	if(inObject->decodeState == JSON_DECODE_STATE_VALUE && ret == JSON_RC_OK)
	{

	}
  }

//----------------

  TRAZA1("Returning from JSON_object_decode() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
JSON_string_decode
(
  JSON_object_t*		inObject,
  BUFF_buff_t**			ioBuffer
)
{
  BUFF_buff_t*			buff = *ioBuffer;

  char*					ps;
  char*					pc;
  char*					pi;

  long					len = 0;

  int					state;

  int					end = JSON_FALSE;
  int					ret = JSON_RC_OK;

  TRAZA4("Entering in JSON_string_decode(%p, %p:%ld:%ld)",
          inObject,
          inBuffer,
	      inBuffer->idx,
	      inBuffer->len);

//----------------

  pi = (char*)(buff->data + buff->idx);

  pc = pi; pc[buff->len] = 0;

//----------------

  ps = strchr(pc, '"');

  for(state = 0, end = 0; end == 0 && ret == 0;)
  {
    if(ps == NULL)
    {
      buff->idx = buff->len;

      ps = (char*)(buff->data + buff->idx);

      while(*pc==' ' || *pc=='\t' || *pc=='\r' || *pc=='\n') pc++;

      if(pc != ps)
      {
        SUCESO2("ERROR: Decode string (%s%s)", pi, pi ? pi : "");

        ret = JSON_RC_ERROR;
      }

      else if(buff->len < buff->type->size)
      {
    	ret = JSON_RC_INCOMPLETE;
      }

      else if(buff->next == NULL)
      {
    	ret = JSON_RC_INCOMPLETE;
      }

      else if(buff->next->len == 0)
      {
    	ret = JSON_RC_INCOMPLETE;
      }

      else // if(buff->next->len > 0)
      {
    	buff = buff->next; BUFF_refs_add(inObject->buffRefs, buff);

    	pi = (char*)(buff->data + 0); // buff->idx);

    	pc = pi; pc[buff->len] = 0;

    	ps = strchr(pc, '{');
      }
    }

    else // if(ps != NULL)
    {
      buff->idx = ps - pc + 1; pc = ps + 1; *ps = 0;

      inObject->decodeState = JSON_DECODE_STATE_PAIR;
    }
  }

//----------------

  while((inObject->decodeState == JSON_DECODE_STATE_PAIR ||
	     inObject->decodeState == JSON_DECODE_STATE_VALUE) && ret == JSON_RC_OK)
  {
	if(inObject->decodeState == JSON_DECODE_STATE_PAIR && ret == JSON_RC_OK)
	{
	  ps = strchr(pc, '"');

	  for(state = 0, end = 0; end == 0 && ret == 0;)
	  {
	    if(state == 0)
	    {
	      if(ps != NULL)
	      {
	    	while(*pc==' ' || *pc=='\t' || *pc=='\r' || *pc=='\n') pc++;

	    	if(pc == ps)
	    	{
	    	  pc = ps + 1; state = 1;

	    	  ps = strpbrk(pc, "\\\"");
	    	}

	    	else // if(pc != ps)
	    	{
              SUCESO2("ERROR: Decode error (%s%s)", pi, pi ? pi : "");

              ret = JSON_RC_ERROR;
	    	}
	      }

	      else // if(*ps == '/')
	      {
            while(*pc==' ' || *pc=='\t' || *pc=='\r' || *pc=='\n') pc++;
	      }
	    }

	  }
	}

	if(inObject->decodeState == JSON_DECODE_STATE_VALUE && ret == JSON_RC_OK)
	{

	}
  }
//----------------

  TRAZA1("Returning from JSON_string_decode() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static inline long
JSON_message_line_encode(JSON_message_t* inMessage, ...)
{
  JSON_buffer_t*		buff = inMessage->buffer;

  va_list		        list;

  char*				str;
  char*				enc;

  long 				max;
  long 				len;
  long 				tot = 0;

//----------------

  while(buff->next) buff = buff->next;

  max = g_BufferSize[buff->type] - buff->len;

  enc = buff->data + buff->len; va_start(list, inMessage);

  while((str = va_arg(list, char*)) != NULL)
  {
    len = strlen(str);

    if(max > len)
    {
      memcpy(enc + tot, str, len); max -= len; tot += len;
    }

    else if(tot + len < g_BufferSize[buff->type])
    {
      buff = JSON_buffer_next_new(buff);

      memcpy(buff->data, enc, tot); enc = buff->data; 
      
      memcpy(enc + tot, str, len); tot += len; 
      
      max = g_BufferSize[buff->type] - tot;

      inMessage->encodeIov[inMessage->encodeNum].iov_base = buff->data;
      inMessage->encodeIov[inMessage->encodeNum].iov_len = 0;

      inMessage->encodeNum++;
    }
  }

  va_end(list); enc[tot] = 0; buff->len += tot; 

  inMessage->encodeIov[inMessage->encodeNum - 1].iov_len += tot;

//----------------

  return tot;
}

#define LINE_ENCODE(...) JSON_message_line_encode(inMessage,__VA_ARGS__)

/*-----------------------------------------------------------------------------*/

int
JSON_message_encode(JSON_message_t* inMessage)
{
  JSON_header_t*		ptrHeader;
  JSON_value_t*		ptrValue;

  JSON_buffer_t*		buff = inMessage->buffer;

  const char*			method;

  char				status[16 + 1];
  const char*			reason;

  char*				to;
  char*				from;
  char*				callId;
  char				cSeq[128 + 1];
  char				maxForwards[16 + 1];
  char				contentLength[16 + 1];

  long				idx = 0;

  int				ret = JSON_RC_OK;

  TRAZA1("Entering in JSON_message_encode(%p)", inMessage);

//----------------

  while(buff->next) buff = buff->next;

  memset(inMessage->encodeIov, 0, sizeof(inMessage->encodeIov));

  inMessage->encodeIov[0].iov_base = buff->data + buff->len;
  inMessage->encodeIov[0].iov_len = 0;
  inMessage->encodeNum = 1;

//----------------

  if(inMessage->type == JSON_MESSAGE_TYPE_REQUEST)
  {
    if(inMessage->method == JSON_METHOD_OTHER)
    {
      method = inMessage->metstr;
    }

    else 
    { 
      method = JSON_method[inMessage->method];
    }

    LINE_ENCODE(method, " ", inMessage->requestUri, " SIP/2.0\r\n", NULL);
  }

  else // if(inMessage->type == JSON_MESSAGE_TYPE_RESPONSE)
  {
    sprintf(status, "%d ", inMessage->status);

    if(inMessage->reason != NULL)
    {
      reason = inMessage->reason;
    }

    else if(inMessage->status < 700)
    {
      reason = JSON_reason_phrase[inMessage->status];
    }

    else // if(inMessage->status >= 700)
    {
      reason = " ";
    }

    LINE_ENCODE("SIP/2.0 ", status, reason, "\r\n", NULL);
  }

//----------------

  if(ret == JSON_RC_OK)
  {
    to = inMessage->to;
    from = inMessage->from;
    callId = inMessage->callId;

    if(inMessage->method == JSON_METHOD_OTHER)
    {
      snprintf(cSeq, 128, "%u %s", inMessage->cSeqStep, inMessage->metstr);
    }

    else // if(inMessage->method != JSON_METHOD_OTHER)
    {
      snprintf(cSeq, 128, "%u %s", inMessage->cSeqStep, JSON_method[inMessage->method]);
    }

    if(inMessage->type == JSON_MESSAGE_TYPE_REQUEST)
    {
      sprintf(maxForwards, "%d", inMessage->maxForwards);

      LINE_ENCODE("To: ", to, "\r\n", 
	          "From: ", from, "\r\n",
	          "Call-ID: ", callId, "\r\n",
	          "CSeq: ", cSeq, "\r\n", 
	          "Max-Forwards: ", maxForwards, "\r\n", NULL);
    }

    else // if(inMessage->type == JSON_MESSAGE_TYPE_RESPONSE)
    {
      LINE_ENCODE("To: ", to, "\r\n", 
	          "From: ", from, "\r\n",
	          "Call-ID: ", callId, "\r\n",
	          "CSeq: ", cSeq, "\r\n", NULL);
    }
  }

//----------------

  if(ret == JSON_RC_OK)
  {
    RLST_resetGet(inMessage->headerList, NULL);

    while((ptrHeader = RLST_getNext(inMessage->headerList)))
    {
      RLST_resetGet(ptrHeader->valueList, NULL);

      while((ptrValue = RLST_getNext(ptrHeader->valueList)))
      {
	LINE_ENCODE(ptrHeader->name, ": ", ptrValue->data, "\r\n", NULL);
      }
    }
  }

//----------------

  if(ret == JSON_RC_OK)
  {
    if(inMessage->contentType != NULL)
    {
      LINE_ENCODE("Content-Type: ", inMessage->contentType, "\r\n", NULL);
    }

    sprintf(contentLength, "%d", inMessage->contentLength);

    LINE_ENCODE("Content-Length: ", contentLength, "\r\n", NULL);
    LINE_ENCODE("\r\n", NULL);

    if(inMessage->bodyRaw[0] != NULL)
    {
      if(inMessage->encodeNum < 8)
      {
	inMessage->encodeIov[inMessage->encodeNum].iov_base = inMessage->bodyRaw[0];
	inMessage->encodeIov[inMessage->encodeNum].iov_len = inMessage->bodyLen[0];

	inMessage->encodeNum++;
      }
    }

    if(inMessage->bodyRaw[1] != NULL)
    {
      if(inMessage->encodeNum < 8)
      {
	inMessage->encodeIov[inMessage->encodeNum].iov_base = inMessage->bodyRaw[1];
	inMessage->encodeIov[inMessage->encodeNum].iov_len = inMessage->bodyLen[1];

	inMessage->encodeNum++;
      }
    }
  }

//----------------

  if(ret == JSON_RC_OK)
  {
    inMessage->encodeLen = 0;

    for(idx = 0; idx < inMessage->encodeNum; idx++)
    {
      inMessage->encodeLen += inMessage->encodeIov[idx].iov_len;
    }

    if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 3)
    {
      DEPURA2("ENCODE LENGTH: (%d:%05ld)", 
	       inMessage->encodeNum, 
	       inMessage->encodeLen);

      for(idx = 0; idx < 8; idx++)
      {
	if(inMessage->encodeIov[idx].iov_len > 0)
	{
	  DEPURA4("JSONER ENCODE: (%ld:%05ld)(%p)\n\n%s\n",
		   idx,
		   inMessage->encodeIov[idx].iov_len,
		   inMessage->encodeIov[idx].iov_base,
		   inMessage->encodeIov[idx].iov_base);
	}
      }
    }
  }

//----------------

  TRAZA1("Returning from JSON_message_encode() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void
JSON_message_view
(
  JSON_message_t*		inMessage,
  int				inLevel
)
{
  char				str[JSON_MAXLEN_JSONER + 1];

  if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= inLevel)
  {
    JSON_message_dump(inMessage, JSON_MAXLEN_JSONER + 1, str);

    if(inLevel == 0) { SUCESO1("\n%s", str); } else 
    if(inLevel == 1) { PRUEBA1("\n%s", str); } else 
    if(inLevel == 2) { TRAZA1 ("\n%s", str); } else 
    if(inLevel == 3) { DEPURA1("\n%s", str); }
  }
}

/*----------------------------------------------------------------------------*/

char* 
JSON_message_dump
(
  JSON_message_t*		inMessage,
  long			        inStrLen,
  char*			        outStr
)
{
  outStr[0] = 0; return outStr;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

JSON_header_t*
JSON_message_header_add
(
  JSON_message_t*		inMessage,
  char*				inHeaderName,
  char*				inValueData
)
{
  JSON_header_t*		ptrHeader = NULL;
  JSON_header_t		header[1];

  JSON_value_t*		ptrValue = NULL;

  TRAZA3("Entering in JSON_message_header_add(%p, %s, %s)",
          inMessage,
          inHeaderName,
	  inValueData);

//----------------

  header->name = inHeaderName;

  ptrHeader = RFTR_find(inMessage->headerTree, header, NULL, NULL);

  if(ptrHeader == NULL)
  {
    ptrHeader = JSON_header_new(NULL);

    ptrHeader->name = JSON_strcpy(inMessage->buffer, inHeaderName);

    if(RFTR_insert(inMessage->headerTree, ptrHeader) != RFTR_RC_OK)
    {
      JSON_FATAL0("ERROR: RFTR_insert()");
    }

    if(RLST_insertTail(inMessage->headerList, ptrHeader) != RFTR_RC_OK)
    {
      JSON_FATAL0("ERROR: RLST_insertTail()");
    }
  }

//----------------

  ptrValue = JSON_value_new(NULL);

  ptrValue->data = JSON_strcpy(inMessage->buffer, inValueData);

  if(RLST_insertHead(ptrHeader->valueList, ptrValue) < 0)
  {
    JSON_FATAL0("ERROR: RLST_insertHead()");
  }

  ptrHeader->data = ptrValue->data;

//----------------

  TRAZA1("Returning from JSON_message_header_add() = %p", ptrHeader);

  return ptrHeader;
}

/*----------------------------------------------------------------------------*/

JSON_header_t*
JSON_message_header_sbl_add
(
  JSON_message_t*		inMessage,
  char*				inHeaderName,
  char*				inValueData
)
{
  JSON_header_t*		ptrHeader = NULL;
  JSON_header_t		header[1];

  JSON_value_t*		ptrValue = NULL;

  TRAZA3("Entering in JSON_message_header_sbl_add(%p, %s, %s)",
          inMessage,
          inHeaderName,
	  inValueData);

//----------------

  header->name = inHeaderName;

  ptrHeader = RFTR_find(inMessage->headerTree, header, NULL, NULL);

  if(ptrHeader == NULL)
  {
    ptrHeader = JSON_header_new(NULL);

    ptrHeader->name = JSON_strcpy(inMessage->buffer, inHeaderName);

    if(RFTR_insert(inMessage->headerTree, ptrHeader) != RFTR_RC_OK)
    {
      JSON_FATAL0("ERROR: RFTR_insert()");
    }

    if(RLST_insertTail(inMessage->headerList, ptrHeader) != RFTR_RC_OK)
    {
      JSON_FATAL0("ERROR: RLST_insertTail()");
    }
  }

//----------------

  ptrValue = JSON_value_new(inValueData);

  if(RLST_insertHead(ptrHeader->valueList, ptrValue) < 0)
  {
    JSON_FATAL0("ERROR: RLST_insertHead()");
  }

  ptrHeader->data = ptrValue->data;

//----------------

  TRAZA1("Returning from JSON_message_header_add() = %p", ptrHeader);

  return ptrHeader;
}

/*----------------------------------------------------------------------------*/

JSON_header_t*
JSON_message_header_print
(
  JSON_message_t*		inMessage,
  char*				inHeaderName,
  char*				inValueData, ...
)
{
  JSON_buffer_t*		buff = inMessage->buffer;

  JSON_header_t*		ptrHeader = NULL;
  JSON_header_t		header[1];

  JSON_value_t*		ptrValue = NULL;

  va_list		        list;

  long				size;
  long				plen;

  TRAZA3("Entering in JSON_message_header_print(%p, %s, %s)",
          inMessage,
          inHeaderName,
	  inValueData);

//----------------

  header->name = inHeaderName;

  ptrHeader = RFTR_find(inMessage->headerTree, header, NULL, NULL);

  if(ptrHeader == NULL)
  {
    ptrHeader = JSON_header_new(NULL);

    ptrHeader->name = JSON_strcpy(inMessage->buffer, inHeaderName);

    if(RFTR_insert(inMessage->headerTree, ptrHeader) != RFTR_RC_OK)
    {
      JSON_FATAL0("ERROR: RFTR_insert()");
    }

    if(RLST_insertTail(inMessage->headerList, ptrHeader) != RFTR_RC_OK)
    {
      JSON_FATAL0("ERROR: RLST_insertTail()");
    }
  }

//----------------

  ptrValue = JSON_value_new(NULL);

  while(buff->next) buff = buff->next;

  ptrValue->data = buff->data + buff->len;

  size = g_BufferSize[buff->type] - buff->len;

  va_start(list, inValueData);
    
  plen = vsnprintf(ptrValue->data, size, inValueData, list); 

  if(plen >= size)
  {
    buff->next = JSON_buffer_next_new(buff);

    ptrValue->data = buff->data; size = g_BufferSize[buff->type];

    plen = vsnprintf(ptrValue->data, size, inValueData, list);

    plen = JSON_MIN(plen, size - 1);
  }

  va_end(list); ptrValue->data[plen] = 0; buff->len += plen + 1;

  if(RLST_insertHead(ptrHeader->valueList, ptrValue) < 0)
  {
    JSON_FATAL0("ERROR: RLST_insertHead()");
  }

  ptrHeader->data = ptrValue->data;

//----------------

  TRAZA1("Returning from JSON_message_header_print() = %p", ptrHeader);

  return ptrHeader;
}

/*----------------------------------------------------------------------------*/

int
JSON_message_header_del
(
  JSON_message_t*		inMessage,
  char*				inHeaderName
)
{
  JSON_header_t*		ptrHeader = NULL;
  JSON_header_t		header[1];

  int				ret = JSON_RC_OK;

  TRAZA2("Entering in JSON_message_header_del(%p, %s)",
          inMessage,
          inHeaderName);

//----------------

  header->name = inHeaderName;

  ptrHeader = RFTR_find(inMessage->headerTree, header, NULL, NULL);

  if(ptrHeader != NULL)
  {
    RFTR_extract(inMessage->headerTree, ptrHeader);   
    RLST_extract(inMessage->headerList, ptrHeader);

    JSON_header_delete(ptrHeader);
  }

  else { ret = JSON_RC_NOT_FOUND; }

//----------------

  TRAZA1("Returning from JSON_message_header_del() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

JSON_header_t*
JSON_message_header_value_del
(
  JSON_message_t*		inMessage,
  char*				inHeaderName
)
{
  JSON_header_t*		ptrHeader = NULL;
  JSON_header_t		header[1];

  JSON_value_t*		ptrValue = NULL;

  TRAZA2("Entering in JSON_message_header_value_del(%p, %s)",
          inMessage,
          inHeaderName);

//----------------

  header->name = inHeaderName;

  ptrHeader = RFTR_find(inMessage->headerTree, header, NULL, NULL);

  if(ptrHeader != NULL)
  {
    ptrValue = RLST_extractHead(ptrHeader->valueList);

    if(ptrValue != NULL)
    {
      JSON_value_delete(ptrValue);
    }

    ptrValue = RLST_getHead(ptrHeader->valueList);

    if(ptrValue != NULL)
    {
      ptrHeader->data = ptrValue->data;
    }

    else
    {
      RFTR_extract(inMessage->headerTree, ptrHeader);   
      RLST_extract(inMessage->headerList, ptrHeader);

      JSON_header_delete(ptrHeader); ptrHeader = NULL;
    }
  }

//----------------

  TRAZA1("Returning from JSON_message_header_value_del() = %p", ptrHeader);

  return ptrHeader;
}

/*----------------------------------------------------------------------------*/

JSON_header_t*
JSON_message_header_find
(
  JSON_message_t*		inMessage,
  char*				inHeaderName
)
{
  JSON_header_t*		ptrHeader = NULL;
  JSON_header_t		header[1];

  TRAZA2("Entering in JSON_message_header_find(%p, %s)",
          inMessage,
          inHeaderName);

//----------------

  header->name = inHeaderName;

  ptrHeader = RFTR_find(inMessage->headerTree, header, NULL, NULL);

  if(ptrHeader != NULL)
  {
    RLST_resetGet(ptrHeader->valueList, NULL);
  }

//----------------

  TRAZA1("Returning from JSON_message_header_find() = %p", ptrHeader);

  return ptrHeader;
}

/*----------------------------------------------------------------------------*/

JSON_header_t*
JSON_message_header_copy
(
  JSON_message_t*		inMessage,
  JSON_header_t*		inHeader
)
{
  JSON_header_t*		ptrHeader = NULL;

  JSON_value_t*		ptrSrcVal;
  JSON_value_t*		ptrDstVal;

  TRAZA2("Entering in JSON_message_header_copy(%p, %p)",
          inMessage,
          inHeader);

//----------------

  ptrHeader = RFTR_find(inMessage->headerTree, inHeader, NULL, NULL);

  if(ptrHeader == NULL)
  {
    ptrHeader = JSON_header_new(inHeader->name);

    if(RFTR_insert(inMessage->headerTree, ptrHeader) != RFTR_RC_OK)
    {
      JSON_FATAL0("RFTR_insert()");
    }

    if(RLST_insertTail(inMessage->headerList, ptrHeader) != RFTR_RC_OK)
    {
      JSON_FATAL0("RLST_insertTail()");
    }
  }

//----------------

  RLST_resetGet(inHeader->valueList, NULL);

  while((ptrSrcVal = RLST_getPrev(inHeader->valueList)))
  {
    ptrDstVal = JSON_value_new(ptrSrcVal->data);

    if(RLST_insertHead(ptrHeader->valueList, ptrDstVal) < 0)
    {
      JSON_FATAL0("ERROR: RLST_insertHead()");
    }
  }

//----------------

  ptrDstVal = RLST_getHead(ptrHeader->valueList);

  if(ptrDstVal == NULL)
  {
    RFTR_extract(inMessage->headerTree, ptrHeader);   
    RLST_extract(inMessage->headerList, ptrHeader);

    JSON_header_delete(ptrHeader); ptrHeader = NULL;
  }

  else { ptrHeader->data = ptrDstVal->data; }

//----------------

  TRAZA1("Returning from JSON_message_header_copy() = %p", ptrHeader);

  return ptrHeader;
}

/*----------------------------------------------------------------------------*/

JSON_header_t*
JSON_message_header_r_copy
(
  JSON_message_t*		inMessage,
  JSON_header_t*		inHeader
)
{
  JSON_header_t*		ptrHeader = NULL;

  JSON_value_t*		ptrSrcVal;
  JSON_value_t*		ptrDstVal;

  TRAZA2("Entering in JSON_message_header_r_copy(%p, %p)",
          inMessage,
          inHeader);

//----------------

  ptrHeader = RFTR_find(inMessage->headerTree, inHeader, NULL, NULL);

  if(ptrHeader == NULL)
  {
    ptrHeader = JSON_header_new(inHeader->name);

    if(RFTR_insert(inMessage->headerTree, ptrHeader) != RFTR_RC_OK)
    {
      JSON_FATAL0("RFTR_insert()");
    }

    if(RLST_insertTail(inMessage->headerList, ptrHeader) != RFTR_RC_OK)
    {
      JSON_FATAL0("RLST_insertTail()");
    }
  }

//----------------

  RLST_resetGet(inHeader->valueList, NULL);

  while((ptrSrcVal = RLST_getNext(inHeader->valueList)))
  {
    ptrDstVal = JSON_value_new(ptrSrcVal->data);

    if(RLST_insertHead(ptrHeader->valueList, ptrDstVal) < 0)
    {
      JSON_FATAL0("ERROR: RLST_insertHead()");
    }
  }

//----------------

  ptrHeader->data = RLST_getHead(ptrHeader->valueList);

  if(ptrHeader->data == NULL)
  {
    RFTR_extract(inMessage->headerTree, ptrHeader);   
    RLST_extract(inMessage->headerList, ptrHeader);

    JSON_header_delete(ptrHeader); ptrHeader = NULL;
  }

//----------------

  TRAZA1("Returning from JSON_message_header_r_copy() = %p", ptrHeader);

  return ptrHeader;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
JSON_message_via_add
(
  JSON_message_t*		inMessage,
  char*				inViaHost,
  int				inViaPort,
  int				inViaTrans,
  char*				inViaBranch
)
{
  char*				via;

  char				port[128 + 1];

  char*				str[128];

  long				idx = 0;
  long				len;

  int				ret = JSON_RC_OK;

  TRAZA5("Entering in JSON_message_via_add(%p, %s, %d, %d, %s)",
          inMessage,
          inViaHost,
          inViaPort,
          inViaTrans,
	  inViaBranch ? inViaBranch : "NULL");

//----------------

  str[idx++] = "SIP/2.0/";
  str[idx++] = (char*)(JSON_transport[inViaTrans]);

  str[idx++] = " ";
  str[idx++] = inViaHost;

  if(inViaPort != 0)
  {
    sprintf(port, "%d", inViaPort);

    str[idx++] = ":";
    str[idx++] = port;
  }

  if(inViaBranch != NULL) if(inViaBranch[0] != 0)
  {
    str[idx++] = ";branch=";
    str[idx++] = inViaBranch;
  }

//----------------

  str[idx++] = NULL;

  via = JSON_strenc(inMessage->buffer, str, &len);

  JSON_message_header_sbl_add(inMessage, "Via", via);

  DEPURA1("Via = (%s)", via);

//----------------

  TRAZA1("Returning from JSON_message_via_add() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
JSON_message_route_add
(
  JSON_message_t*		inMessage,
  char*				inRouteUser,
  char*				inRouteHost,
  int				inRoutePort,
  int				inRouteTrans,
  char*				inRouteParams
)
{
  char*				route;

  char				port[128 + 1];

  char*				str[128];

  long				idx = 0;
  long				len;

  int				ret = JSON_RC_OK;

  TRAZA6("Entering in JSON_message_route_add(%p, %s, %s, %d, %d, %s)",
          inMessage,
          inRouteUser ? inRouteParams : "NULL",
          inRouteHost,
          inRoutePort,
          inRouteTrans,
	  inRouteParams ? inRouteParams : "NULL");

//----------------

  str[idx++] = "<sip:";

  if(inRouteUser != NULL) if(inRouteUser[0] != 0)
  {
    str[idx++] = inRouteUser;

    str[idx++] = "@";
  }

  str[idx++] = inRouteHost;

  if(inRoutePort != 0)
  {
    sprintf(port, "%d", inRoutePort);

    str[idx++] = ":"; 
    str[idx++] = port;
  }

  str[idx++] = ";lr";

  if(inRouteTrans == JSON_TRANSPORT_TCP)
  {
    str[idx++] = ";transport=tcp";
  }

  if(inRouteParams != NULL) if(inRouteParams != 0)
  {
    if(inRouteParams[0] != ';') str[idx++] = ";"; 

    str[idx++] = inRouteParams;
  }

  str[idx++] = ">"; 

//----------------

  str[idx++] = NULL;

  route = JSON_strenc(inMessage->buffer, str, &len);

  JSON_message_header_sbl_add(inMessage, "Route", route);

  DEPURA1("Route = (%s)", route);

//----------------

  TRAZA1("Returning from JSON_message_route_add() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
JSON_message_record_route_add
(
  JSON_message_t*		inMessage,
  char*				inRouteUser,
  char*				inRouteHost,
  int				inRoutePort,
  int				inRouteTrans,
  char*				inRouteParams
)
{
  char*				route;

  char				port[128 + 1];

  char*				str[128];

  long				idx = 0;
  long				len;

  int				ret = JSON_RC_OK;

  TRAZA6("Entering in JSON_message_record_route_add(%p, %s, %s, %d, %d, %s)",
          inMessage,
          inRouteUser ? inRouteParams : "-",
          inRouteHost,
          inRoutePort,
          inRouteTrans,
	  inRouteParams ? inRouteParams : "-");

//----------------

  str[idx++] = "<sip:";

  if(inRouteUser != NULL) if(inRouteUser[0] != 0)
  {
    str[idx++] = inRouteUser;

    str[idx++] = "@";
  }

  str[idx++] = inRouteHost;

  if(inRoutePort != 0)
  {
    sprintf(port, "%d", inRoutePort);

    str[idx++] = ":"; 
    str[idx++] = port;
  }

  str[idx++] = ";lr";

  if(inRouteTrans == JSON_TRANSPORT_TCP)
  {
    str[idx++] = ";transport=tcp";
  }

  if(inRouteParams != NULL) if(inRouteParams != 0)
  {
    if(inRouteParams[0] != ';') str[idx++] = ";"; 

    str[idx++] = inRouteParams;
  }

  str[idx++] = ">"; 

//----------------

  str[idx++] = NULL;

  route = JSON_strenc(inMessage->buffer, str, &len);

  JSON_message_header_sbl_add(inMessage, "Record-Route", route);

  DEPURA1("Route = (%s)", route);

//----------------

  TRAZA1("Returning from JSON_message_record_route_add() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
JSON_message_body_copy
(
  JSON_message_t*		inDstMsg,
  JSON_message_t*		inSrcMsg
)
{
  int                           ret = JSON_RC_OK;

  TRAZA2("Entering in JSON_message_body_copy(%p, %p)",
          inDstMsg,
          inSrcMsg);

//----------------

  inDstMsg->contentLength = inSrcMsg->contentLength;
  inDstMsg->contentType = inSrcMsg->contentType;

  inDstMsg->mediaType = inSrcMsg->mediaType;
  inDstMsg->mediaSubType = inSrcMsg->mediaSubType;

  inDstMsg->bodyRaw[0] = inSrcMsg->bodyRaw[0];
  inDstMsg->bodyRaw[1] = inSrcMsg->bodyRaw[1];
  inDstMsg->bodyLen[0] = inSrcMsg->bodyLen[0];
  inDstMsg->bodyLen[1] = inSrcMsg->bodyLen[1];

  inDstMsg->bodyDec = inSrcMsg->bodyDec;

  if(inSrcMsg->body == NULL)
  {
    if(inDstMsg->body != NULL)
    {
      JSON_body_delete(inDstMsg->body);

      inDstMsg->body = NULL;
    }
  }

  else // if(inSrcMsg->body == NULL)
  {
    if(inDstMsg->body == NULL)
    {
      inDstMsg->body = JSON_body_new();
    }
     
    inDstMsg->body->body = inSrcMsg->body->body;
    inDstMsg->body->blen = inSrcMsg->body->blen;
  }

//----------------

  TRAZA1("Returning from JSON_message_body_copy() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
JSON_message_body_del(JSON_message_t*	inMessage)
{
  int                           ret = JSON_RC_OK;

  TRAZA1("Entering in JSON_message_body_del(%p, %p)", inMessage);

//----------------

  inMessage->contentType = ZEROLEN;

  inMessage->contentLength = 0;
  inMessage->mediaSubType = 0;
  inMessage->mediaType = 0;

  inMessage->bodyRaw[0] = ZEROLEN;
  inMessage->bodyRaw[1] = ZEROLEN;
  inMessage->bodyLen[0] = 0;
  inMessage->bodyLen[1] = 0;

  inMessage->bodyDec = JSON_FALSE;

  if(inMessage->body != NULL)
  {
    JSON_body_delete(inMessage->body);

    inMessage->body = NULL;
  }

//----------------

  TRAZA1("Returning from JSON_message_body_del() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
JSON_via_decode
(
  JSON_via_t*			inVia,
  JSON_buffer_t* 		inBuffer,
  char*				inString
)
{
  char*				pi;
  char*				ps;
  char*				pt;

  int				state;
  int				end;

  char*				port;

  int				ret = JSON_RC_OK;

  TRAZA3("Entering in JSON_via_decode(%p, %p, %p)",
          inVia, 
          inBuffer, 
	  inString);

//----------------

  inVia->trns = 0;
  inVia->host = ZEROLEN;
  inVia->port = 0;
  inVia->prms = ZEROLEN;

  inVia->branch = ZEROLEN;

  DEPURA1("Via (%s)", inString);

//----------------

  pi = inString; pt = pi; ps = strchr(pt, '/');

  for(state = 0, end = 0; end == 0 && ret == 0;)
  {
    if(state == 0) // protocol-name
    {
      if(ps == NULL)
      {
	SUCESO2("ERROR: Via decode error (%s)(%ld)", pi, strlen(pi));

	ret = JSON_RC_ERROR;
      }

      else // if(*ps == '/')
      {
	while(*pt==' ' || *pt=='\t' || *pt=='\r' || *pt=='\n') pt++;

	if(memcmp(pt, "SIP", 3) != 0)
	{
	  SUCESO2("ERROR: Via protocol not supported (%s)(%ld)", pi, pt-pi);

	  ret = JSON_RC_ERROR;
	}

	else { pt = ps + 1; ps = strchr(pt, '/'); state = 1; }
      }
    }

    else if(state == 1) // protocol-version
    {
      if(ps == NULL)
      {
	SUCESO2("ERROR: Via decode error (%s)(%ld)", pi, strlen(pi));

	ret = JSON_RC_ERROR;
      }

      else // if(*ps == '/')
      {
	while(*pt==' ' || *pt=='\t' || *pt=='\r' || *pt=='\n') pt++;

	if(memcmp(pt, "2.0", 3) != 0)
	{
	  SUCESO2("ERROR: Via version not supported (%s)(%ld)", pi, pt-pi);

	  ret = JSON_RC_ERROR;
	}

	else { pt = ps + 1; ps = strpbrk(pt, " \t"); state = 2; }
      }
    }

    else if(state == 2) // transport
    {
      if(ps == NULL)
      {
	SUCESO2("ERROR: Via decode error (%s)(%ld)", pi, strlen(pi));

	ret = JSON_RC_ERROR;
      }

      else // if(*ps == ' ' || *ps == '\t') 
      {
	while(*pt==' ' || *pt=='\t' || *pt=='\r' || *pt=='\n') pt++;

	if(memcmp(pt, "UDP", 3) == 0)
	{
	  inVia->trns = JSON_TRANSPORT_UDP;
	}

	else if(memcmp(pt, "TCP", 3) == 0)
	{
	  inVia->trns = JSON_TRANSPORT_TCP;
	}

	else if(memcmp(pt, "SCTP", 4) == 0)
	{
	  inVia->trns = JSON_TRANSPORT_SCTP;
	}

	else { inVia->trns = JSON_TRANSPORT_OTHER; }

        DEPURA1("Via tstr (%s)", JSON_transport[inVia->trns]);

	pt = ps + 1; ps = strpbrk(pt, ":;"); state = 3;
      }
    }

//----------------

    else if(state == 3) // sent-by - host
    {
      if(ps == NULL)
      {
	inVia->host = JSON_strcpy(inBuffer, pt);

	inVia->host = JSON_trim(inVia->host);

	DEPURA1("Via host (%s)", inVia->host);

	inVia->hostType = JSON_host_type_get(inVia->host);

	DEPURA1("Via type (%s)", JSON_host_type[inVia->hostType]);

        end = JSON_TRUE;
      }

      else if(*ps == ':')
      {
	inVia->host = JSON_memcpy(inBuffer, pt, ps - pt);

	inVia->host = JSON_trim(inVia->host);

	DEPURA1("Via host (%s)", inVia->host);

	inVia->hostType = JSON_host_type_get(inVia->host);

	DEPURA1("Via type (%s)", JSON_host_type[inVia->hostType]);

	pt = ps + 1; ps = strpbrk(pt, ";"); state = 4;
      }

      else // if(*ps == ';')
      {
	inVia->host = JSON_memcpy(inBuffer, pt, ps - pt);

	inVia->host = JSON_trim(inVia->host);

	DEPURA1("Via host (%s)", inVia->host);

	inVia->hostType = JSON_host_type_get(inVia->host);

	DEPURA1("Via type (%s)", JSON_host_type[inVia->hostType]);

	pt = ps; state = 5;
      }
    }

    else if(state == 4) // sent-by - port
    {
      if(ps == NULL)
      {
	port = JSON_strcpy(inBuffer, pt);
	
	port = JSON_trim(port);

	inVia->port = atoi(port);

	DEPURA1("Via port (%d)", inVia->port);

	end = JSON_TRUE;
      }

      else // if(*ps == ';')
      {
	port = JSON_memcpy(inBuffer, pt, ps - pt);
	
	port = JSON_trim(port);

	inVia->port = atoi(port);

	DEPURA1("Via port (%d)", inVia->port);

	pt = ps; state = 5;
      }
    }

//----------------

    else if(state == 5) // via-params 
    {
      inVia->prms = JSON_strcpy(inBuffer, pt);

      DEPURA1("Via prms (%s)", inVia->prms);

      inVia->branch = JSON_parameter_get(inBuffer, inVia->prms, "branch");
        
      DEPURA1("Via bnch (%s)", inVia->branch);
	
      inVia->received = JSON_parameter_get(inBuffer, inVia->prms, "received");
        
      DEPURA1("Via recv (%s)", inVia->branch);
	
      end = JSON_TRUE;
    }
  }

//----------------

  TRAZA1("Returning from JSON_via_decode() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

char*
JSON_via_encode
(
  JSON_via_t*			inVia,
  JSON_buffer_t* 		inBuffer,
  long*				outStrLen
)
{
  char				port[128 + 1];

  char*				str[128];

  long				idx = 0;
  long				len = 0;

  char*				ret;

  TRAZA3("Entering in JSON_via_encode(%p, %p, %p)",
          inVia,
          inBuffer,
	  outStrLen);

//----------------

  str[idx++] = "SIP/2.0/";
  str[idx++] = (char*)(JSON_transport[inVia->trns]);

  if(inVia->host != NULL)
  {
    str[idx++] = " ";
    str[idx++] = inVia->host;
  }

  if(inVia->port != 0)
  {
    sprintf(port, "%d", inVia->port);

    str[idx++] = ":";
    str[idx++] = port;
  }
/*
  if(inVia->branch != NULL) if(inVia->branch[0] != 0)
  {
    str[idx++] = ";branch=";
    str[idx++] = inVia->branch;
  }
*/
  if(inVia->prms != NULL) if(inVia->prms[0] != 0)
  {
    str[idx++] = ";";
    str[idx++] = inVia->prms;
  }

//----------------

  str[idx++] = NULL;

  ret = JSON_strenc(inBuffer, str, &len);

  if(outStrLen != NULL) *outStrLen = len;

  DEPURA1("Via (%s)", ret ? ret : "NULL");

//----------------

  TRAZA2("Returning from JSON_via_encode() = %p (%ld)", ret, len);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
JSON_address_decode
(
  JSON_address_t*		inAddress,
  JSON_buffer_t* 		inBuffer,
  char*				inString,
  long*				outStrOff
)
{
  char				str[JSON_MAXLEN_STRING + 1];

  char*				pi;
  char*				pt;
  char*				ps;

  long				len;

  char*				param;

  int				state;
  int				end;

  int				ret = JSON_RC_OK;

  TRAZA4("Entering in JSON_address_decode(%p, %p, %p, %p)",
          inAddress, 
          inBuffer, 
	  inString,
	  outStrOff);

//----------------

  inAddress->schm = JSON_URI_SCHEME_NONE;
  inAddress->user = ZEROLEN;
  inAddress->info = ZEROLEN;
  inAddress->pass = ZEROLEN;
  inAddress->host = ZEROLEN;
  inAddress->port = 0;
  inAddress->prms = ZEROLEN;
  inAddress->hdrs = ZEROLEN;

  inAddress->hostType = JSON_HOST_TYPE_NONE;
  inAddress->transport = JSON_TRANSPORT_NONE;

  DEPURA1("Address (%s)", inString);

//----------------

  pi = inString; pt = pi; ps = strpbrk(pt, ":<");

  for(state = 0, end = 0; end == 0 && ret == 0;)
  {

//---------------- SCHEME

    if(state == 0)
    {
      if(ps == NULL)
      {
	SUCESO2("ERROR: Address scheme not supported (%s)(%ld)", pi, pt - pi);

	len = pt - pi + strlen(pt); ret = JSON_RC_ERROR;
      }

      else if(*ps == ':')
      {
	while(*pt==' ' || *pt=='\t' || *pt=='\r' || *pt=='\n') pt++;

	len = JSON_MIN(ps - pt, JSON_MAXLEN_STRING);

	memcpy(str, pt, len); str[len] = 0;

	DEPURA1("Address schm (%s)", str);

	if(strcmp(str, "sip") == 0)
	{
	  inAddress->schm = JSON_URI_SCHEME_SIP;

	  if(strchr(ps + 1, '@') == 0)
	  {
            pt = ps + 1; ps = strpbrk(pt, ":;>"); state = 4;
	  }

	  else // if(strchr((char*)(ps), '@') != 0)
	  { 
	    pt = ps + 1; ps = strpbrk(pt, ";:@>"); state = 1;
	  }
	}

	else if(strcmp(str, "tel") == 0)
	{
	  inAddress->schm = JSON_URI_SCHEME_TEL;

	  pt = ps + 1; ps = strpbrk(pt, ";:@>"); state = 1;
	}

	else
 	{
	  SUCESO2("ERROR: Address scheme not supported (%s)(%ld)", pi, pt - pi);

	  len = ps - pi + 1; ret = JSON_RC_ERROR;
	}
      }

      else // if(*ps == '<')
      {
	pt = ps + 1; ps = strpbrk(pt, ":");
      }
    }

//---------------- USER

    else if(state == 1)
    {
      if(ps == NULL)
      {
	SUCESO2("ERROR: Address bad sytanx (%s)(%ld)", pi, pt - pi);

	len = pt - pi + strlen(pt); ret = JSON_RC_ERROR;
      }

      else // if(*ps == ';' || *ps == ':' || *ps == '@' || *ps == '>')
      {
	inAddress->user = JSON_memcpy(inBuffer, pt, ps - pt);
	
	DEPURA1("Address user (%s)", inAddress->user);

        if(*ps == '@')
	{
	  pt = ps + 1; ps = strpbrk(pt, ":;>"); state = 4;
	}

	else if(*ps == ';')
	{
	  pt = ps; ps = strpbrk(pt, ":@>"); state = 2;
	}
	
	else if(*ps == ':')
	{
	  pt = ps + 1; ps = strpbrk(pt, "@>"); state = 3;
	}
	
	else // if(*ps == '>')
	{
	  len = ps - pi + 1; end = JSON_TRUE;
	}
      }
    }

//---------------- USER INFO

    else if(state == 2)
    {
      if(ps == NULL)
      {
	SUCESO2("ERROR: Address bad sytanx (%s)(%ld)", pi, pt - pi);

	len = pt - pi + strlen(pt); ret = JSON_RC_ERROR;
      }

      else // if(*ps == ':' || *ps == '@' || *ps == '>')
      {
	inAddress->info = JSON_memcpy(inBuffer, pt, ps - pt);
	
	DEPURA1("Address info (%s)", inAddress->info);

        if(*ps == '@')
	{
	  pt = ps + 1; ps = strpbrk(pt, ":;>"); state = 4;
	}

	else if(*ps == ':')
	{
	  pt = ps + 1; ps = strpbrk(pt, "@>"); state = 3;
	}
	
	else // if(*ps == '>')
	{
	  len = ps - pi + 1; end = JSON_TRUE;
	}
      }
    }

//---------------- PASSWORD

    else if(state == 3)
    {
      if(ps == NULL)
      {
	SUCESO2("ERROR: Address bad sytanx (%s)(%ld)", pi, pt - pi);

	len = pt - pi + strlen(pt); ret = JSON_RC_ERROR;
      }

      else // if(*ps == '@' || *ps == '>')
      {
	inAddress->pass = JSON_memcpy(inBuffer, pt, ps - pt);
	
	DEPURA1("Address pass (%s)", inAddress->pass);

        if(*ps == '@')
	{
	  pt = ps + 1; ps = strpbrk(pt, ":;?>"); state = 4;
	}

	else // if(*ps == '>')
	{
	  len = ps - pi + 1; end = JSON_TRUE;
	}
      }
    }

//---------------- HOST

    else if(state == 4)
    {
      if(ps == NULL)
      {
	inAddress->host = JSON_strcpy(inBuffer, pt);
	
	DEPURA1("Address host (%s)", inAddress->host);

	len = pt - pi + strlen(pt); end = JSON_TRUE;
      }

      else // if(*ps == ':' || *ps == ';' || *ps == '?' || *ps == '>')
      {
	inAddress->host = JSON_memcpy(inBuffer, pt, ps - pt);
	
	DEPURA1("Address host (%s)", inAddress->host);

	inAddress->hostType = JSON_host_type_get(inAddress->host);

        if(*ps == ':')
	{
	  pt = ps + 1; ps = strpbrk(pt, ";?>"); state = 5;
	}
	
	else if(*ps == ';')
	{
	  pt = ps; ps = strpbrk(pt, "?>"); state = 6;
	}

	else // if(*ps == '>')
	{
	  len = ps - pi + 1; end = JSON_TRUE;
	}
      }
    }

//---------------- PORT

    else if(state == 5)
    {
      if(ps == NULL)
      {
	len = JSON_MIN(strlen(pt), JSON_MAXLEN_STRING);

	memcpy(str, pt, len); str[len] = 0;

	inAddress->port = atoi(str);
	
	DEPURA2("Address port (%d)(%s)", inAddress->port, str);

	len = pt - pi + strlen(pt); end = JSON_TRUE;
      }

      else // if(*ps == ';' || *ps == '?' || *ps == '>')
      {
	len = JSON_MIN(ps - pt, JSON_MAXLEN_STRING);

	memcpy(str, pt, len); str[len] = 0;

	inAddress->port = atoi(str);
	
	DEPURA2("Address port (%d)(%s)", inAddress->port, str);

	if(*ps == ';')
	{
	  pt = ps; ps = strpbrk(pt, "?>"); state = 6;
	}

	else if(*ps == '?')
	{
	  pt = ps + 1; ps = strpbrk(pt, ">"); state = 7;
	}
	
	else // if(*ps == '>')
	{
	  len = ps - pi + 1; end = JSON_TRUE;
	}
      }
    }

//---------------- PARAMETERS

    else if(state == 6)
    {
      if(ps == NULL)
      {
	inAddress->prms = JSON_strcpy(inBuffer, pt);
	
	DEPURA1("Address prms (%s)", inAddress->prms);

	len = pt - pi + strlen(pt); end = JSON_TRUE;
      }

      else // if(*ps == '?' || *ps == '>')
      {
	inAddress->prms = JSON_memcpy(inBuffer, pt, ps - pt);
	
	DEPURA1("Address prms (%s)", inAddress->prms);

        param = JSON_parameter_get(inBuffer, inAddress->prms, "transport");

        if(param[0] == 0)
	{
	  inAddress->transport = JSON_TRANSPORT_NONE;
	}

	else if(strcasecmp(param, "UDP") == 0)
	{
	  inAddress->transport = JSON_TRANSPORT_UDP;

	  DEPURA1("Address tstr (%s)", param);
	}

	else if(strcasecmp(param, "TCP") == 0)
	{
	  inAddress->transport = JSON_TRANSPORT_TCP;

	  DEPURA1("Address tstr (%s)", param);
	}

	else if(strcasecmp(param, "TLS") == 0)
	{
	  inAddress->transport = JSON_TRANSPORT_TLS;

	  DEPURA1("Address tstr (%s)", param);
	}

	else if(strcasecmp(param, "SCTP") == 0)
	{
	  inAddress->transport = JSON_TRANSPORT_SCTP;

	  DEPURA1("Address tstr (%s)", param);
	}

	else
	{
	  inAddress->transport = JSON_TRANSPORT_OTHER;

	  DEPURA1("Address tstr (%s)", param);
	}

	if(*ps == '?')
	{
	  pt = ps + 1; ps = strpbrk(pt, ">"); state = 7;
	}
	
	else // if(*ps == '>')
	{
	  len = ps - pi + 1; end = JSON_TRUE;
	}
      }
    }

//---------------- HEADERS

    else if(state == 8)
    {
      if(ps == NULL)
      {
	inAddress->hdrs = JSON_strcpy(inBuffer, pt);
	
	DEPURA1("Address hdrs (%s)", inAddress->hdrs);

	len = pt - pi + strlen(pt); end = JSON_TRUE;
      }

      else // if(*ps == '>')
      {
	inAddress->hdrs = JSON_memcpy(inBuffer, pt, ps - pt);
	
	DEPURA1("Address hdrs (%s)", inAddress->hdrs);

	len = ps - pi + 1; end = JSON_TRUE;
      }
    }
  }

//----------------

  if(outStrOff) *outStrOff = len;

  TRAZA2("Returning from JSON_address_decode() = %d (%ld)", ret, len);

  return ret;
}

/*----------------------------------------------------------------------------*/

char*
JSON_address_encode
(
  JSON_address_t*		inAddress,
  JSON_buffer_t* 		inBuffer,
  long*				outStrLen
)
{
  char				port[128 + 1];

  char*				str[128];

  long				idx = 0;
  long				len = 0;

  char*				ret;

  TRAZA3("Entering in JSON_address_encode(%p, %p, %p)",
          inAddress,
          inBuffer,
	  outStrLen);

//----------------

  if(inAddress->schm == JSON_URI_SCHEME_SIP)
  {
    str[idx++] = "sip:";

    if(inAddress->user != NULL) if(inAddress->user[0] != 0)
    {
      str[idx++] = inAddress->user;

      if(inAddress->info != NULL) if(inAddress->info[0] != 0)
      {
	str[idx++] = inAddress->info;
      }

      str[idx++] = "@";
    }

    if(inAddress->host != NULL) if(inAddress->host[0] != 0)
    {
      str[idx++] = inAddress->host;
    }

    if(inAddress->port != 0)
    {
      sprintf(port, "%d", inAddress->port);

      str[idx++] = ":"; 
      str[idx++] = port;
    }

    if(inAddress->prms != NULL) if(inAddress->prms[0] != 0)
    {
    //str[idx++] = ";"; 
      str[idx++] = inAddress->prms;
    }

    if(inAddress->hdrs != NULL) if(inAddress->hdrs[0] != 0)
    {
      str[idx++] = "?"; 
      str[idx++] = inAddress->hdrs;
    }
  }

  else if(inAddress->schm == JSON_URI_SCHEME_TEL)
  {
    str[idx++] = "tel";

    if(inAddress->user != NULL) if(inAddress->user[0] != 0)
    {
      str[idx++] = inAddress->user;

      if(inAddress->info != NULL)
      {
	str[idx++] = inAddress->info;
      }
    }
  }

//----------------

  str[idx++] = NULL;

  ret = JSON_strenc(inBuffer, str, &len);

  if(outStrLen != NULL) *outStrLen = len;

  DEPURA1("Address (%s)", ret ? ret : "NULL");

//----------------

  TRAZA2("Returning from JSON_address_encode() = %p (%ld)", ret, len);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
JSON_party_decode
(
  JSON_party_t*		inParty,
  JSON_buffer_t* 		inBuffer,
  char*				inString
)
{
  char*				pi;
  char*				pt;
  char*				ps;

  long				inc;

  int				state;
  int				end;

  int				addrDec = JSON_FALSE;

  int				ret = JSON_RC_OK;

  TRAZA3("Entering in JSON_party_decode(%p, %p, %p)",
          inParty, 
          inBuffer, 
	  inString);

//----------------

  inParty->name = ZEROLEN;
  inParty->prms = ZEROLEN;

  inParty->tag = ZEROLEN;

  DEPURA1("Party (%s)", inString);

//----------------

  pi = inString; pt = pi; ps = strpbrk(pt, "<:\"");

  for(state = 0, end = 0; end == 0 && ret == 0;)
  {
    if(state == 0)
    {
      if(ps == NULL)
      {
    	inParty->name = JSON_memcpy(inBuffer, pt, strlen(pt));

	inParty->name = JSON_trim(inParty->name);

	DEPURA1("Party name (%s)", inParty->name);
        
	end = JSON_TRUE;
      }

      else if(*ps == '"')
      {
	inParty->dqot = JSON_TRUE;

	pt = ps + 1; ps = strpbrk(pt, "\\\""); state = 1;
      }

      else if(*ps == '<')
      {
    	inParty->name = JSON_memcpy(inBuffer, pt, ps - pt);

	inParty->name = JSON_trim(inParty->name);

	pt = ps + 1; state = 2;
      }

      else // if(*ps == ':')
      {
	state = 2;
      }
    }

    else if(state == 1)
    {
      if(ps == NULL)
      {
	SUCESO2("ERROR: Party decode error (%s)(%ld)", pi, pt - pi);

	ret = JSON_RC_ERROR;
      }

      else if(*ps == '\\') 
      { 
	ps = strpbrk(ps + 2, "\\\""); 
      }

      else // if(*ps == '"') 
      { 
    	inParty->name = JSON_memcpy(inBuffer, pt, ps - pt);

	DEPURA1("Party name (%s)", inParty->name);
        
	ps++; while(*ps==' ' || *ps=='\t' || *ps=='\r' || *ps=='\n') ps++;
        
	if(*ps == '<')
	{
	  pt = ps + 1; state = 2;
	}

	else // if(*ps != '<')
	{
	  SUCESO2("ERROR: Party decode error (%s)(%ld)", pi, ps - pi);

	  ret = JSON_RC_ERROR;
	}
      }
    }

    else if(state == 2)
    {
      addrDec = JSON_TRUE;

      ret = JSON_address_decode(inParty->addr, inBuffer, pt, &inc);

      if(ret == JSON_RC_OK)
      {
	pt += inc; ps = strpbrk(pt, ">;"); state = 3;
      }
    }

    else if(state == 3)
    {
      if(ps == NULL)
      {
	end = JSON_TRUE;
      }

      else if(*ps == '>') 
      { 
	ps = strpbrk(ps + 1, ";"); 
      }

      else // if(*ps == ';') 
      { 
    	inParty->prms = JSON_strcpy(inBuffer, ps);

	DEPURA1("Party prms (%s)", inParty->prms);

	end = JSON_TRUE;
      }
    }
  }

//----------------

  if(addrDec == JSON_FALSE)
  {
    inParty->addr->schm = JSON_URI_SCHEME_NONE;
    inParty->addr->user = ZEROLEN;
    inParty->addr->info = ZEROLEN;
    inParty->addr->pass = ZEROLEN;
    inParty->addr->host = ZEROLEN;
    inParty->addr->port = 0;
    inParty->addr->prms = ZEROLEN;
    inParty->addr->hdrs = ZEROLEN;

    inParty->addr->hostType = JSON_HOST_TYPE_NONE;
    inParty->addr->transport = JSON_TRANSPORT_NONE;
  }

//----------------

  TRAZA1("Returning from JSON_party_decode() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

char*
JSON_party_encode
(
  JSON_party_t*		inParty,
  JSON_buffer_t* 		inBuffer,
  long*				outStrLen
)
{
  char				port[128 + 1];

  char*				str[128];

  long				idx = 0;
  long				len = 0;

  char*				ret;

  TRAZA3("Entering in JSON_party_encode(%p, %p, %p)",
          inParty,
          inBuffer,
	  outStrLen);

//----------------

  if(inParty->name != NULL) if(inParty->name[0] != 0)
  {
    if(inParty->dqot) str[idx++] = "\"";

    str[idx++] = inParty->name;

    if(inParty->dqot) str[idx++] = "\"";

    str[idx++] = " ";
  }

  str[idx++] = "<";

//----------------

  if(inParty->addr->schm == JSON_URI_SCHEME_SIP)
  {
    str[idx++] = "sip:";

    if(inParty->addr->user != NULL) if(inParty->addr->user[0] != 0)
    {
      str[idx++] = inParty->addr->user;

      if(inParty->addr->info != NULL) if(inParty->addr->info[0] != 0)
      {
	str[idx++] = inParty->addr->info;
      }

      str[idx++] = "@";
    }

    if(inParty->addr->host != NULL) if(inParty->addr->host[0] != 0)
    {
      str[idx++] = inParty->addr->host;
    }

    if(inParty->addr->port != 0)
    {
      sprintf(port, "%d", inParty->addr->port);

      str[idx++] = ":"; 
      str[idx++] = port;
    }

    if(inParty->addr->prms != NULL) if(inParty->addr->prms[0] != 0)
    {
    //str[idx++] = ";"; 
      str[idx++] = inParty->addr->prms;
    }

    if(inParty->addr->hdrs != NULL) if(inParty->addr->hdrs[0] != 0)
    {
      str[idx++] = "?"; 
      str[idx++] = inParty->addr->hdrs;
    }
  }

  else if(inParty->addr->schm == JSON_URI_SCHEME_TEL)
  {
    str[idx++] = "tel";

    if(inParty->addr->user != NULL) if(inParty->addr->user[0] != 0)
    {
      str[idx++] = inParty->addr->user;

      if(inParty->addr->info != NULL)
      {
	str[idx++] = inParty->addr->info;
      }
    }
  }

//----------------

  str[idx++] = ">"; 

  if(inParty->prms != NULL) if(inParty->prms[0] != 0)
  {
  //str[idx++] = ";"; 
    str[idx++] = inParty->prms;
  }

//----------------

  str[idx++] = NULL;

  ret = JSON_strenc(inBuffer, str, &len);

  if(outStrLen != NULL) *outStrLen = len;

//----------------

  TRAZA2("Returning from JSON_party_encode() = (%s) (%ld)", ret, len);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

char*
JSON_parameter_get
(
  JSON_buffer_t* 		inBuffer,
  char*				inParamList,
  char*				inParamName
)
{
  char				sep;

  char*				ps;
  char*				pz;

  char*				pl;
  char*				pn;
  char*				pv = NULL;

  char*				value = ZEROLEN;

  int				state =  0;
  int				end;

  TRAZA3("Entering in JSON_parameter_get(%p, %s, %s)",
          inBuffer,
          inParamList,
          inParamName);

//---------------- 
 
  pl = inParamList; pn = pl; ps = strpbrk(pl, "=;");

  for(state = 0, end = 0; end == 0;)
  {
    if(state == 0) // param-name 
    {
      if(ps == NULL)
      {
	end = JSON_TRUE;
      }

      else if(*ps == '=')
      {
	while(*pn==';' || *pn==' ' || *pn=='\t' || *pn=='\r' || *pn=='\n') 
	{
	  pn++;
	}

	pz = ps - 1;

	while((pz>pn) && (*pz==' ' || *pz=='\t' || *pz=='\r' || *pz=='\n')) 
	{
	  pz--;
	}

	pz++; sep = *pz; *pz = 0;

	DEPURA1("Param-Name (%s)", pn);

	if(strcasecmp(pn, inParamName) == 0)
	{
	  *pz = sep; pv = ps + 1; ps = strpbrk(pv, ";?>\""); state = 3;
	}

	else
	{
	  *pz = sep; pv = ps + 1; ps = strpbrk(pv, ";\""); state = 1;
	}
      }

      else // if(*ps == ';')
      {
	pn = ps + 1; ps = strpbrk(pn, "=;");
      }
    }
    
//---------------- 
 
    else if(state == 1)
    {
      if(ps == NULL)
      {
	end = JSON_TRUE;
      }

      else if(*ps == ';')
      {
	pn = ps + 1; ps = strpbrk(pn, "=;"); state = 0;
      }

      else // if(*ps == '"')
      {
	ps = strpbrk(ps + 2, "\\\""); state = 2;
      }
    }

    else if(state == 2)
    {
      if(ps == NULL) 
      { 
	end = JSON_TRUE;
      }

      else if(*ps == '"') 
      { 
	ps = strpbrk(ps + 1, ";\""); state = 1; 
      }

      else // if(*ps == '\\') 
      { 
	ps = strpbrk(ps + 2, "\\\""); 
      }
    }

//----------------

    else if(state == 3) // param-value
    {
      if(ps == NULL)
      {
	value = JSON_memcpy(inBuffer, pv, strlen(pv));

	value = JSON_trim(value); end = JSON_TRUE;
      }

      else if(*ps == ';' || *ps == '?' || *ps == '>')
      {
	value = JSON_memcpy(inBuffer, pv, ps - pv);

	value = JSON_trim(value); end = JSON_TRUE;
      }

      else // if(*ps == '"')
      {
	ps = strpbrk(ps + 2, "\\\""); state = 4;
      }
    }

    else if(state == 4)
    {
      if(ps == NULL) 
      { 
	end = JSON_TRUE;
      }

      else if(*ps == '"') 
      { 
	ps = strpbrk(ps + 1, ";\""); state = 3; 
      }

      else // if(*ps == '\\') 
      { 
	ps = strpbrk(ps + 2, "\\\""); 
      }
    }
  }

//----------------

  TRAZA1("Returning from JSON_parameter_get() = (%s)", value);

  return value;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
JSON_parameter_check
(
  char*				inParamList,
  char*				inParamName
)
{
  char*				pl;
  char*				pn;
  char*				ps;
  char*				pz;

  int				sep;

  int				state;
  int				end;

  int				check = JSON_FALSE;

  TRAZA2("Entering in JSON_parameter_del(%s, %s)", inParamList, inParamName);

//---------------- 
 
  pl = inParamList; pn = pl; ps = strpbrk(pl, "=;");

  for(state = 0, end = 0; end == 0 && check == JSON_FALSE;)
  {
    if(state == 0)
    {
      if(ps == NULL)
      {
	while(*pn==';' || *pn==' ' || *pn=='\t' || *pn=='\r' || *pn=='\n') 
	{
	  pn++;
	}
	
	pz = pn + strlen(pn) - 1;

	while((pz>pn) && (*pz==' ' || *pz=='\t' || *pz=='\r' || *pz=='\n')) 
	{
	  pz--;
	}

	pz++; sep = *pz; *pz = 0;

	DEPURA1("Param-Name (%s)", pn);

	if(strcasecmp(pn, inParamName) == 0) check = JSON_TRUE;
	  
	*pz = sep; end = JSON_TRUE;
      }

      else if(*ps == '=')
      {
	while(*pn==';' || *pn==' ' || *pn=='\t' || *pn=='\r' || *pn=='\n') 
	{
	  pn++;
	}
	
	pz = ps - 1;

	while((pz>pn) && (*pz==' ' || *pz=='\t' || *pz=='\r' || *pz=='\n')) 
	{
	  pz--;
	}

	pz++; sep = *pz; *pz = 0;

	DEPURA1("Param-Name (%s)", pn);

	if(strcasecmp(pn, inParamName) == 0)
	{
	  *pz = sep; check = JSON_RC_OK;
	}

	else { *pz = sep; ps = strpbrk(ps + 1, ";\""); state = 1; }
      }

      else // if(*ps == ';')
      {
	while(*pn==';' || *pn==' ' || *pn=='\t' || *pn=='\r' || *pn=='\n') 
	{
	  pn++;
	}
	
	pz = ps - 1;

	while((pz>pn) && (*pz==' ' || *pz=='\t' || *pz=='\r' || *pz=='\n')) 
	{
	  pz--;
	}

	pz++; sep = *pz; *pz = 0;

	DEPURA1("Param-Name (%s)", pn);

	if(strcasecmp(pn, inParamName) == 0)
	{
	  *pz = sep; check = JSON_RC_OK;
	}

	else { *pz = sep; pn = ps + 1; ps = strpbrk(pn, "=;"); }
      }
    }
    
//---------------- 
 
    else if(state == 1)
    {
      if(ps == NULL)
      {
	end = JSON_TRUE;
      }

      else if(*ps == ';')
      {
	pn = ps + 1; ps = strpbrk(pn, "=;"); state = 0;
      }

      else // if(*ps == '"')
      {
	ps = strpbrk(ps + 2, "\\\""); state = 2;
      }
    }

    else if(state == 2)
    {
      if(ps == NULL) 
      { 
	end = JSON_TRUE;
      }

      else if(*ps == '"') 
      { 
	ps = strpbrk(ps + 1, ";\""); state = 1; 
      }

      else // if(*ps == '\\') 
      { 
	ps = strpbrk(ps + 2, "\\\""); 
      }
    }
  }

//----------------

  TRAZA1("Returning from JSON_parameter_check() = %d", check);

  return check;
}

/*----------------------------------------------------------------------------*/

char*
JSON_parameter_set
(
  JSON_buffer_t* 		inBuffer,
  char*				inParamList,
  char*				inParamName,
  char*				inParamValue
)
{
  char*				plist;

  TRAZA4("Entering in JSON_parameter_set(%p, %s, %s, %s)",
          inBuffer,
          inParamList,
          inParamName,
	  inParamValue ? inParamValue : "");

//----------------

  JSON_parameter_del(inParamList, inParamName);

  if(inParamValue == NULL)
  {
    plist = JSON_strcat(inBuffer, inParamList, ";", inParamName, NULL);
  }

  else if(inParamValue[0] == 0)
  {
    plist = JSON_strcat(inBuffer, inParamList, ";", inParamName, NULL);
  }

  else // if(inParamValue[0] != 0)
  {
    plist = JSON_strcat(inBuffer,
	                 inParamList, ";", 
			 inParamName, "=", inParamValue, 
			 NULL);
  }

//----------------

  TRAZA1("Returning from JSON_parameter_set() = %s", plist);

  return plist;
}

/*----------------------------------------------------------------------------*/

int
JSON_parameter_del
(
  char*				ioParamList,
  char*				inParamName
)
{
  char*				pl;
  char*				pn;
  char*				ps;
  char*				pz;
  char*				pi = NULL;

  int				sep;

  int				state = 0;

  int				end = JSON_FALSE;
  int				ret = JSON_RC_NOT_FOUND;

  TRAZA2("Entering in JSON_parameter_del(%s, %s)", ioParamList, inParamName);

//---------------- 
 
  pl = ioParamList; pn = pl; pi = pn; ps = strpbrk(pl, "=;");

  while(end == JSON_FALSE && ret == JSON_RC_NOT_FOUND)
  {
    if(state == 0)
    {
      if(ps == NULL)
      {
	end = JSON_TRUE;

	while(*pn==';' || *pn==' ' || *pn=='\t' || *pn=='\r' || *pn=='\n') 
	{
	  pn++;
	}
	
	pz = pn + strlen(pn) - 1;

	while((pz>pn) && (*pz==' ' || *pz=='\t' || *pz=='\r' || *pz=='\n')) 
	{
	  pz--;
	}

	pz++; sep = *pz; *pz = 0;

	DEPURA1("Param-Name (%s)", pn);

	if(strcasecmp(pn, inParamName) == 0) 
	{ 
	  *pi = 0; ret = JSON_RC_OK;
	} 
	  
	else { *pz = sep; }
      }

      else if(*ps == '=')
      {
	while(*pn==';' || *pn==' ' || *pn=='\t' || *pn=='\r' || *pn=='\n') 
	{
	  pn++;
	}

	pz = ps - 1;

	while((pz>pn) && (*pz==' ' || *pz=='\t' || *pz=='\r' || *pz=='\n')) 
	{
	  pz--;
	}

	pz++; sep = *pz; *pz = 0;

	DEPURA1("Param-Name (%s)", pn);

	if(strcasecmp(pn, inParamName) == 0)
	{
	  *pz = sep; ps = strpbrk(ps + 1, ";?>\""); state = 3;
	}

	else { *pz = sep; ps = strpbrk(ps + 1, ";\""); state = 1; }
      }

      else // if(*ps == ';')
      {
	while(*pn==';' || *pn==' ' || *pn=='\t' || *pn=='\r' || *pn=='\n') 
	{
	  pn++;
	}

	pz = ps - 1;

	while((pz>pn) && (*pz==' ' || *pz=='\t' || *pz=='\r' || *pz=='\n')) 
	{
	  pz--;
	}

	pz++; sep = *pz; *pz = 0;

	DEPURA1("Param-Name (%s)", pn);

	if(strcasecmp(pn, inParamName) == 0)
	{
	  strcpy(pi, ps); ret = JSON_RC_OK;
	}

	else { *pz = sep; pn = ps + 1; pi = ps; ps = strpbrk(pn, "=;"); }
      }
    }
    
//---------------- 
 
    else if(state == 1)
    {
      if(ps == NULL)
      {
	end = JSON_TRUE;
      }

      else if(*ps == ';')
      {
	pn = ps + 1; pi = ps; ps = strpbrk(pn, "=;"); state = 0;
      }

      else // if(*ps == '"')
      {
	ps = strpbrk(ps + 2, "\\\""); state = 2;
      }
    }

    else if(state == 2)
    {
      if(ps == NULL) 
      { 
	end = JSON_TRUE;
      }

      else if(*ps == '"') 
      { 
	ps = strpbrk(ps + 1, ";\""); state = 1; 
      }

      else // if(*ps == '\\') 
      { 
	ps = strpbrk(ps + 2, "\\\""); 
      }
    }

//----------------

    else if(state == 3)
    {
      if(ps == NULL)
      {
	*pi = 0; ret = JSON_RC_OK;
      }

      else if(*ps == ';' || *ps == '?' || *ps == '>')
      {
	strcpy(pi, ps); ret = JSON_RC_OK;
      }

      else // if(*ps == '"')
      {
	ps = strpbrk(ps + 2, "\\\""); state = 4;
      }
    }

    else if(state == 4)
    {
      if(ps == NULL) 
      { 
	end = JSON_TRUE;
      }

      else if(*ps == '"') 
      { 
	ps = strpbrk(ps + 1, ";\""); state = 3; 
      }

      else // if(*ps == '\\') 
      { 
	ps = strpbrk(ps + 2, "\\\""); 
      }
    }
  }

//----------------

  TRAZA2("Returning from JSON_parameter_del() = %d (%s)", ret, ioParamList);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
JSON_tag_check
(
  char*				inTagList,
  char*				inTagName
)
{
  char*				pn;
  char*				ps;
  char*				pz;

  int				sep;

  int				end = JSON_FALSE;
  int				check = JSON_FALSE;

  TRAZA2("Entering in JSON_tag_check(%s, %s)", inTagList, inTagName);

//---------------- 
 
  pn = inTagList; ps = strpbrk(pn, ",:");

  while(end == JSON_FALSE && check == JSON_FALSE)
  {
    if(ps == NULL)
    {
      end = JSON_TRUE;

      while(*pn==',' || *pn==' ' || *pn=='\t' || *pn=='\r' || *pn=='\n') 
      {
	pn++;
      }
      
      pz = pn + strlen(pn) - 1;

      while((pz>pn) && (*pz==' ' || *pz=='\t' || *pz=='\r' || *pz=='\n')) 
      {
	pz--;
      }

      pz++; sep = *pz; *pz = 0;

      DEPURA1("Tag-Name (%s)", pn);

      if(strcasecmp(pn, inTagName) == 0) check = JSON_TRUE;

      *pz = sep;
    }

    else if(*ps == ',')
    {
      while(*pn==',' || *pn==' ' || *pn=='\t' || *pn=='\r' || *pn=='\n') 
      {
	pn++;
      }
      
      pz = ps - 1;

      while((pz>pn) && (*pz==' ' || *pz=='\t' || *pz=='\r' || *pz=='\n')) 
      {
	pz--;
      }

      pz++; sep = *pz; *pz = 0;

      DEPURA1("Tag-Name (%s)", pn);

      if(strcasecmp(pn, inTagName) == 0)
      {
	*pz = sep; check = JSON_TRUE;
      }

      else { *pz = sep; pn = ps + 1; ps = strpbrk(ps + 1, ",:"); }
    }

    else // if(*ps == ':')
    {
      pn = ps + 1; ps = strpbrk(pn, ",");
    }
  }
    
//----------------

  TRAZA1("Returning from JSON_tag_check() = %d", check);

  return check;
}

/*----------------------------------------------------------------------------*/

char*
JSON_tag_set
(
  JSON_buffer_t* 		inBuffer,
  char*				inTagList,
  char*				inTagName
)
{
  char*				tlist;

  TRAZA3("Entering in JSON_tag_set(%p, %p, %p)",
          inBuffer,
          inTagList,
          inTagName);

  DEPURA1("Tag-List (%s)", inTagList);
  DEPURA1("Tag-Name (%s)", inTagList);

//----------------

  JSON_tag_del(inTagList, inTagName);

  tlist = JSON_strcat(inBuffer, inTagList, ";", inTagName, NULL);

//----------------

  DEPURA1("Tag-List (%s)", tlist);

  TRAZA1("Returning from JSON_tag_set() = %p", tlist);

  return tlist;
}

/*----------------------------------------------------------------------------*/

int
JSON_tag_del
(
  char*				ioTagList,
  char*				inTagName
)
{
  char*				pi;
  char*				pn;
  char*				ps;
  char*				pz;

  int				sep;

  int				end = JSON_FALSE;
  int				ret = JSON_RC_NOT_FOUND;

  TRAZA2("Entering in JSON_tag_del(%p, %p)", ioTagList, inTagName);

  DEPURA1("Tag-List (%s)", ioTagList);
  DEPURA1("Tag-Name (%s)", inTagName);

//---------------- 
 
  pn = ioTagList; pi = pn; ps = strpbrk(pn, ",:");

  while(end == JSON_FALSE && ret == JSON_RC_NOT_FOUND)
  {
    if(ps == NULL)
    {
      end = JSON_TRUE;

      while(*pn==',' || *pn==' ' || *pn=='\t' || *pn=='\r' || *pn=='\n') 
      {
	pn++;
      }
      
      pz = pn + strlen(pn) - 1;

      while((pz>pn) && (*pz==' ' || *pz=='\t' || *pz=='\r' || *pz=='\n')) 
      {
	pz--;
      }

      pz++; sep = *pz; *pz = 0;

      DEPURA1("Tag-Name (%s)", pn);

      if(strcasecmp(pn, inTagName) == 0) 
      { 
	*pi = 0; ret = JSON_RC_OK;
      }

      else { *pz = sep; }
    }

    else if(*ps == ',')
    {
      while(*pn==',' || *pn==' ' || *pn=='\t' || *pn=='\r' || *pn=='\n') 
      {
	pn++;
      }

      pz = ps - 1;

      while((pz>pn) && (*pz==' ' || *pz=='\t' || *pz=='\r' || *pz=='\n')) 
      {
	pz--;
      }

      pz++; sep = *pz; *pz = 0;

      DEPURA1("Tag-Name (%s)", pn);
      
      if(strcasecmp(pn, inTagName) == 0)
      {
	strcpy(pi, ps); ret = JSON_RC_OK;
      }

      else // if(strcasecmp(pn, inTagName) != 0)
      { 
	*pz = sep; pn = ps + 1; pi = ps; ps = strpbrk(pn, ",:"); 
      }
    }

    else // if(*ps == ':')
    {
      pn = ps + 1; pi = ps; ps = strpbrk(pn, ",");
    }
  }
    
//----------------

  DEPURA1("Tag-List (%s)", ioTagList);

  TRAZA2("Returning from JSON_tag_del() = %d (%p)", ret, ioTagList);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

char*
JSON_string_replace
(
  JSON_buffer_t* 		inBuffer,
  char* 		        inString,
  char*                 	inOldSub,
  char*                 	inNewSub
)
{
  char*				ps;

  char*				str[128];
  char*				off[128];
  char				sep[128];

  long				idx;
  long				num = 0;
  long				len;

  char*				ret = ZEROLEN;
  
  TRAZA4("Entering in JSON_string_replace(%p, %s, %s, %s)",
          inBuffer,
          inString,
          inOldSub,
	  inNewSub);

//----------------

  str[num] = inString; sep[num++] = 0;

  len = strlen(inOldSub);

  if(len > 0)
  {
    ps = strstr(inString, inOldSub);

    while(ps != NULL && num < 127)
    {
      off[num] = ps; sep[num] = *ps; *ps = 0; ps += len;

      str[num++] = inNewSub; str[num++] = ps;
      
      ps = strstr(ps, inOldSub);
    }
  }

  str[num] = NULL;

  ret = JSON_strenc(inBuffer, str, &len);

  for(idx = 1; idx < num; idx += 2)
  {
    ps = off[idx]; *ps = sep[num];
  }

//----------------

  TRAZA1("Returning from JSON_string_replace() = %s", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
JSON_host_type_get(char* inHost)
{
  int			c;
  int			s = 0;
  int			b = 0;

  long			i = 0;
  long			j = 0;

  int			type = JSON_HOST_TYPE_NONE;

  int			end = JSON_FALSE;

  TRAZA1("Entering in JSON_host_type_get(%s)", inHost);

//----------------

  while(end == JSON_FALSE)
  {
    c = inHost[i++];

    if(s == 0)
    {
      if(c >= '0' && c <= '9')
      {
	j++; s = 1;
      }

      else if(c == '[')
      {
	type = JSON_HOST_TYPE_IP6; end = JSON_TRUE;
      }

      else if(JSON_is_alphanum(c))
      {
	type = JSON_HOST_TYPE_FQDN; end = JSON_TRUE;
      }

      else { end = JSON_TRUE; }
    }

    else if(s == 1)
    {
      if(c >= '0' && c <= '9')
      {
	j++;
      }

      else if(c >= '.')
      {
	if(j >= 1 && j <= 3)
	{
	  j = 0; b++;
	}

	else 
	{ 
	  type = JSON_HOST_TYPE_FQDN; end = JSON_TRUE;
	}
      }

      else if(c == 0)
      {
	if(++b == 4)
	{
	  type = JSON_HOST_TYPE_IP; end = JSON_TRUE;
	}

	else
	{
	  type = JSON_HOST_TYPE_FQDN; end = JSON_TRUE;
	}
      }

      else if(JSON_is_alphanum(c))
      {
	type = JSON_HOST_TYPE_FQDN; end = JSON_TRUE;
      }

      else if(c == '-')
      {
	type = JSON_HOST_TYPE_FQDN; end = JSON_TRUE;
      }

      else { end = JSON_TRUE; }
    }
  }

//----------------

  TRAZA1("Returning from JSON_host_type_get() = %d", type);

  return type;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

long
JSON_uri_escape(char* ioSipStr, long inMaxLen)
{
  char			c;
  char			h[16 + 1];

  long			i = 0;
  long			j = 0;

  long			max = inMaxLen - 1;

  int			end = JSON_FALSE;

  char			aux[JSON_MAXLEN_STRING + 1];

  TRAZA2("JSON_uri_escape(%s, %ld)", ioSipStr, inMaxLen);

//----------------

  STRCPY(aux, ioSipStr, JSON_MAXLEN_STRING);

  while(end == JSON_FALSE)
  {
    c = aux[i++];

    if(c >= '0' && c <= '9')
    {
      if(j < max) ioSipStr[j++] = c;
    }

    else if(c >= 'a' && c <= 'z')
    {
      if(j < max) ioSipStr[j++] = c;
    }

    else if(c >= 'A' && c <= 'Z')
    {
      if(j < max) ioSipStr[j++] = c;
    }

    else if(c == '-' || c == '_' || c == '.' || c == '\'' ||
	    c == '!' || c == '?' || c == '(' || c == ')'  ||
	    c == '~' || c == '*' || c == '&' || c == '='  ||
	    c == '+' || c == '$' || c == ',' || c == ';'  || c == '/')
    {
      if(j < max) ioSipStr[j++] = c;
    }

    else if(c == 0x0a || c == 0x0d)
    {
      if(j < max) ioSipStr[j++] = c;
    }

    else if(c == 0)
    {
      ioSipStr[j] = 0; end = JSON_TRUE;
    }

    else // escape
    {
      sprintf(h, "%02x", c);

      if(j < max) ioSipStr[j++] = '%';
      if(j < max) ioSipStr[j++] = h[0];
      if(j < max) ioSipStr[j++] = h[1];

      // DEPURA4("escape %c -> %c (%ld, %ld)", c, ioSipStr[j - 3], i, j - 2);
      // DEPURA4("escape %c -> %c (%ld, %ld)", c, ioSipStr[j - 2], i, j - 1);
    }
/*
    if(end == JSON_FALSE)
    {
      DEPURA4("escape %c -> %c (%ld, %ld)", c, ioSipStr[j - 1], i, j);
    }

    else
    {
      DEPURA4("escape %c -> %c (%ld, %ld)", c, ioSipStr[j], i - 1, j);
    }
*/
  }

//----------------

  TRAZA2("Returning from JSON_uri_escape() = %ld (%s)", j, ioSipStr);

  return j;
}

/*----------------------------------------------------------------------------*/

long
JSON_uri_unescape(char* ioSipStr)
{
  char			c;
  char			h[2 + 1];

  long			i = 0;
  long			j = 0;

  int			end = JSON_FALSE;

  TRAZA1("JSON_uri_unescape(%s)", ioSipStr);

//----------------

  while(end == JSON_FALSE)
  {
    c = ioSipStr[i++];

    if(c == '%')
    {
      h[0] = ioSipStr[i++];

      if(h[0] < 0x20)
      {
	ioSipStr[j++] = c; c = h[0];
      }

      else
      {
	h[1] = ioSipStr[i++];

	if(h[1] < 0x20)
        {
	  ioSipStr[j++] = c; ioSipStr[j++] = h[0]; c = h[1];
        }

        else
        {
          h[2] = 0; sscanf(h, "%hhx", &c);
        }
      }
    }

    if(c >= 0x20)
    {
      ioSipStr[j++] = c;
    }

    else if(c == 0x0a || c == 0x0d)
    {
      ioSipStr[j++] = c;
    }

    else if(c == 0)
    {
      ioSipStr[j] = 0; end = JSON_TRUE;
    }

    else
    {
      ioSipStr[j++] = '.';
    }
  }

//----------------

  TRAZA2("Returning from JSON_uri_unescape() = %ld (%s)", j, ioSipStr);

  return j;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static inline void
JSON_message_refs_add
(
  JSON_message_t* 		inMessage,
  JSON_message_t* 		inMsgRefs
)
{
  JSON_buffer_t* 		ptrBuffer;
  JSON_burefs_t* 		ptrBuRefs;

  TRAZA2("Entering in JSON_message_refs_add(%p, %p)", inMessage, inMsgRefs);

//----------------

  ptrBuRefs = inMsgRefs->burefs;

  while(ptrBuRefs != NULL)
  {
    if(ptrBuRefs->buff != NULL)
    {
      JSON_burefs_add(inMessage->burefs, ptrBuRefs->buff);
    }

    ptrBuRefs = ptrBuRefs->next;
  }

//----------------

  ptrBuffer = inMsgRefs->buffer;

  while(ptrBuffer != NULL)
  {
    JSON_burefs_add(inMessage->burefs, ptrBuffer);

    ptrBuffer = ptrBuffer->next;
  }

//----------------

  TRAZA0("Returning from JSON_message_refs_add()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

JSON_message_t*
JSON_message_copy_make(JSON_message_t* inMessage)
{
  JSON_message_t*		ptrMessage = NULL;

  JSON_header_t*		ptrHeader;

  TRAZA1("Entering in JSON_message_copy_make(%p)", inMessage);

//----------------

  ptrMessage = JSON_message_new();

  JSON_message_refs_add(ptrMessage, inMessage);

  memcpy(ptrMessage->m0, inMessage->m0, inMessage->m1 - inMessage->m0);

  RLST_resetGet(inMessage->headerList, NULL);

  while((ptrHeader = RLST_getNext(inMessage->headerList)))
  {
    JSON_message_header_copy(ptrMessage, ptrHeader);
  }

  if(JSON_message_body_copy(ptrMessage, inMessage) < 0)
  {
    JSON_message_delete(ptrMessage); ptrMessage = NULL;
  }

//----------------

  TRAZA1("Returning from JSON_message_copy_make() = %p", ptrMessage);

  return ptrMessage;
}

/*----------------------------------------------------------------------------*/

int
JSON_message_complete
(
  JSON_message_t*		inDstMsg,
  JSON_message_t*		inSrcMsg
)
{
  JSON_header_t*		ptrSrcHdr;
  JSON_header_t*		ptrDstHdr;

  int				ret = JSON_RC_OK;

  TRAZA2("Entering in JSON_message_complete( %p, %p)",
	  inDstMsg,
	  inSrcMsg);

//----------------

  JSON_message_refs_add(inDstMsg, inSrcMsg);

  RLST_resetGet(inSrcMsg->headerList, NULL);

  while((ptrSrcHdr = RLST_getNext(inSrcMsg->headerList)))
  {
    ptrDstHdr = RFTR_find(inDstMsg->headerTree, ptrSrcHdr, NULL, NULL);

    if(ptrDstHdr == NULL)
    {
      JSON_message_header_copy(inDstMsg, ptrSrcHdr);
    }
  }

  if(inSrcMsg->body != NULL && inDstMsg->body == NULL)
  {
    ret = JSON_message_body_copy(inDstMsg, inSrcMsg);
  }

//----------------

  TRAZA1("Returning from JSON_message_complete() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

JSON_message_t*
JSON_request_response_make
(
  JSON_message_t*		inRequest,
  int				inStatus,
  char*				inReason
)
{
  JSON_message_t*		ptrResponse = NULL;

  JSON_header_t*		ptrHeader;

  TRAZA3("Entering in JSON_request_response_make(%p, %d, %s)",
	  inRequest,
	  inStatus,
	  inReason != NULL ? inReason : "NULL");

//----------------

  ptrResponse = JSON_message_new();
  ptrResponse->manager = inRequest->manager;

  JSON_message_refs_add(ptrResponse, inRequest);

  ptrResponse->localIp   = inRequest->localIp;
  ptrResponse->localPort = inRequest->localPort;
  ptrResponse->localTrns = inRequest->localTrns;

  ptrResponse->remoteIp   = inRequest->remoteIp;
  ptrResponse->remoteHost = inRequest->remoteHost;
  ptrResponse->remotePort = inRequest->remotePort;
  ptrResponse->remoteTrns = inRequest->remoteTrns;

//----------------

  ptrResponse->type = JSON_MESSAGE_TYPE_RESPONSE;
  ptrResponse->method = inRequest->method;
  ptrResponse->metstr = inRequest->metstr;

  ptrResponse->status = inStatus;

  if(inReason != NULL)
  {
    ptrResponse->reason = JSON_strcpy(ptrResponse->buffer, inReason);
  }

  ptrResponse->to = inRequest->to;
  ptrResponse->from = inRequest->from;

  ptrResponse->callId = inRequest->callId;

  ptrResponse->cSeqStep = inRequest->cSeqStep;

  ptrHeader = JSON_message_header_find(inRequest, "Via");

  if(ptrHeader != NULL)
  {
    JSON_message_header_copy(ptrResponse, ptrHeader);
  }

//----------------

  TRAZA1("Returning from JSON_request_response_make() = %p", ptrResponse);

  return ptrResponse;
}

/*----------------------------------------------------------------------------*/

JSON_message_t*
JSON_message_err_ack_make
(
  JSON_message_t*		inRequest,
  JSON_message_t*		inResponse
)
{
  JSON_message_t*		ptrAck = NULL;

  JSON_header_t*		ptrHeader;

  TRAZA2("Entering in JSON_message_err_ack_make(%p, %p)",
	  inRequest,
	  inResponse);

//----------------

  ptrAck = JSON_message_new();
  ptrAck->manager = inRequest->manager;

  JSON_message_refs_add(ptrAck, inRequest);

  ptrAck->type = JSON_MESSAGE_TYPE_REQUEST;
  ptrAck->method = JSON_METHOD_ACK;

  ptrAck->requestUri = inRequest->requestUri;

  ptrAck->to = inResponse->to;
  ptrAck->from = inRequest->from;

  ptrAck->callId = inRequest->callId;

  ptrAck->cSeqStep = inRequest->cSeqStep;

  ptrAck->maxForwards = 70;

  ptrHeader = JSON_message_header_find(inRequest, "Via");

  if(ptrHeader != NULL)
  {
    JSON_message_header_copy(ptrAck, ptrHeader);
  }

  ptrHeader = JSON_message_header_find(inRequest, "Route");

  if(ptrHeader != NULL)
  {
    JSON_message_header_copy(ptrAck, ptrHeader);
  }

//----------------

  TRAZA1("Returning from JSON_message_err_ack_make() = %p", ptrAck);

  return ptrAck;
}

/*----------------------------------------------------------------------------*/

JSON_message_t*
JSON_request_cancel_make(JSON_message_t* inRequest)
{
  JSON_message_t*		ptrCancel = NULL;

  JSON_header_t*		ptrHeader;

  TRAZA1("Entering in JSON_request_cancel_make(%p)", inRequest);

//----------------

  ptrCancel = JSON_message_new();
  ptrCancel->manager = inRequest->manager;

  JSON_message_refs_add(ptrCancel, inRequest);

  ptrCancel->type = JSON_MESSAGE_TYPE_REQUEST;
  ptrCancel->method = JSON_METHOD_CANCEL;

  ptrCancel->requestUri = inRequest->requestUri;

  ptrCancel->to = inRequest->to;
  ptrCancel->from = inRequest->from;

  ptrCancel->callId = inRequest->callId;

  ptrCancel->cSeqStep = inRequest->cSeqStep;

  ptrCancel->maxForwards = 70;

  ptrHeader = JSON_message_header_find(inRequest, "Via");

  if(ptrHeader != NULL)
  {
    JSON_message_header_copy(ptrCancel, ptrHeader);
  }

  ptrHeader = JSON_message_header_find(inRequest, "Route");

  if(ptrHeader != NULL)
  {
    JSON_message_header_copy(ptrCancel, ptrHeader);
  }

//----------------

  TRAZA1("Returning from JSON_request_cancel_make() = %p", ptrCancel);

  return ptrCancel;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

