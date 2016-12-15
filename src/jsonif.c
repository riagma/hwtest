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

#include <ctype.h>
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

int
JSON_object_decode
(
  JSON_object_t*		inObject,
  BUFF_buff_t*			inBuffer
)
{
  char*					pc;
  char*					pa;

  JSON_pair_t*			pair;

  int					state;

  int					brk = JSON_FALSE;
  int					end = JSON_FALSE;
  int					ret = JSON_RC_OK;

  TRAZA2("Entering in JSON_object_decode(%p, %p)", inObject, inBuffer);

//----------------

  if(inObject->decodeState == JSON_DECODE_STATE_INIT)
  {
    inBuffer->pc1Elm = inBuffer->idxElm;
    inBuffer->pc1Off = inBuffer->idxOff;

    pc = BUFF_strspn(inBuffer, " \t\r\n");

    if(pc == NULL)
    {
      inBuffer->idxElm = inBuffer->tail;
      inBuffer->idxOff = inBuffer->tail->part->len;

      ret = JSON_RC_INCOMPLETE;
    }

    else if(*pc == '{')
    {
      inBuffer->idxElm = inBuffer->pc2Elm;
      inBuffer->idxOff = inBuffer->pc2Off + 1;

      inObject->decodeState = JSON_DECODE_STATE_PAIR;
    }

    else // if(*pc != '{')
    {
      inBuffer->idxElm = inBuffer->pc2Elm;
      inBuffer->idxOff = inBuffer->pc2Off + 1;

      ret = JSON_RC_ERROR;
    }
  }

//----------------
  
  while((inObject->decodeState == JSON_DECODE_STATE_PAIR ||
	     inObject->decodeState == JSON_DECODE_STATE_VALUE) && ret == JSON_RC_OK)
  {
	if(inObject->decodeState == JSON_DECODE_STATE_PAIR)
	{
      inBuffer->pc1Elm = inBuffer->idxElm;
      inBuffer->pc1Off = inBuffer->idxOff;

      pc = BUFF_strspn(inBuffer, " \t\r\n");


      pc = BUFF_strchr(inBuffer, "\"");

	  for(state = 0, end = 0; end == 0 && ret == 0;)
	  {
        if(pc == NULL)
        {
          ret = JSON_RC_INCOMPLETE;
        }

        else if(state == 0)
	    {
          inBuffer->pc1Elm = inBuffer->pc2Elm;
          inBuffer->pc1Off = inBuffer->pc2Off + 1;

          pc = BUFF_strchr(inBuffer, "\\\"");

          state = 1;
	    }

	    else if(state == 1)
        {
	      if(*pc == '"')
	      {
            inBuffer->pc1Elm = inBuffer->pc2Elm;
            inBuffer->pc1Off = inBuffer->pc2Off + 1;

            pc = BUFF_strchr(inBuffer, ":");

            state = 2;
	      }

	      else // if(*pc == '\\')
	      {
            inBuffer->pc1Elm = inBuffer->pc2Elm;
            inBuffer->pc1Off = inBuffer->pc2Off + 2;

            pc = BUFF_strchr(inBuffer, "\\\"");

            state = 1;
	      }
        }

        else if(state == 2)
	    {
          pair = JSON_pair_new(); pair->value = JSON_value_new();

          if(RFTR_insert(inObject->pairTree, pair) != RFTR_RC_OK)
          {
            JSON_FATAL0("FATAL: RFTR_insert()");
          }

          if(RLST_insertTail(inObject->pairList, pair) != RFTR_RC_OK)
          {
            JSON_FATAL0("FATAL: RLST_insertTail()");
          }

          inBuffer->idxElm = inBuffer->pc2Elm;
          inBuffer->idxOff = inBuffer->pc2Off + 1;

          inObject->decodeState = JSON_DECODE_STATE_PAIR;

          end = JSON_TRUE;
	    }
	  }
	}

//----------------

	if(inObject->decodeState == JSON_DECODE_STATE_VALUE)
	{
	  pair = RLST_getTail(inObject->pairList);

	  if(pair == NULL)
	  {
		JSON_FATAL0("FATAL: ASSERTION!!");
	  }

	  if(pair->value->type == JSON_VALUE_NONE)
	  {
        inBuffer->pc1Elm = inBuffer->idxElm;
        inBuffer->pc1Off = inBuffer->idxOff;

		pc = BUFF_strspn(inBuffer, " \t\r\n");

		if(pc == NULL)
		{
		  ret = JSON_RC_INCOMPLETE;
		}

		else if(*pc == '{')
		{
		  pair->value->type = JSON_VALUE_OBJECT;
		}

		else if(*pc == '[')
		{
		  pair->value->type = JSON_VALUE_ARRAY;
		}

		else if(*pc == '"')
		{
		  pair->value->type = JSON_VALUE_STRING;
		}

		else if(*pc == '-' || isdigit(*pc))
		{
		  pair->value->type = JSON_VALUE_OBJECT;
		}

		else if(*pc == 't' || *pc == 'T')
		{
		  pair->value->type = JSON_VALUE_TRUE;
		}

		else if(*pc == 'f' || *pc == 'F')
		{
		  pair->value->type = JSON_VALUE_FALSE;
		}

		else if(*pc == 'n' || *pc == 'N')
		{
		  pair->value->type = JSON_VALUE_NULL;
		}
	  }

	  if(pair->value->type == JSON_VALUE_OBJECT)
	  {
		ret = JSON_object_decode(inObject, inBuffer);
	  }

	  else if(pair->value->type == JSON_VALUE_ARRAY)
	  {
		ret = JSON_array_decode(inObject, inBuffer);
	  }
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
  BUFF_buff_t*			inBuffer,
  char*					outSep
)
{
  JSON_pair_t*			pair;

  char*					pc;

  int					state;

  int					end = JSON_FALSE;
  int					ret = JSON_RC_OK;

  TRAZA3("Entering in JSON_string_decode(%p, %p, %p)",
	      inObject,
		  inBuffer,
		  outSep);

//----------------

  inBuffer->pc1Elm = inBuffer->idxElm;
  inBuffer->pc1Off = inBuffer->idxOff;

  pc = BUFF_strspn(inBuffer, " \t\r\n");

  if(pc == NULL)
  {
    inBuffer->idxElm = inBuffer->tail;
    inBuffer->idxOff = inBuffer->tail->part->len;

    ret = JSON_RC_INCOMPLETE;
  }

  else if(*pc == '"')
  {
    inBuffer->pc1Elm = inBuffer->pc2Elm;
    inBuffer->pc1Off = inBuffer->pc2Off + 1;

    inObject->decodeState = JSON_DECODE_STATE_PAIR;
  }

  else // if(*pc != '"')
  {
    inBuffer->idxElm = inBuffer->pc2Elm;
    inBuffer->idxOff = inBuffer->pc2Off + 1;

    ret = JSON_RC_ERROR;
  }

//----------------

  for(state = 0, end = 0; end == 0 && ret == 0;)
  {
    pc = BUFF_strchr(inBuffer, "\\\"");

    if(pc == NULL)
    {
      ret = JSON_RC_INCOMPLETE;
    }

    else if(state == 0)
    {
      if(*pc == '"')
      {
        inBuffer->pc1Elm = inBuffer->pc2Elm;
        inBuffer->pc1Off = inBuffer->pc2Off + 1;

        pc = BUFF_strspn(inBuffer, " \t\r\n");

        state = 1;
      }

      else // if(*pc == '\\')
      {
        inBuffer->pc1Elm = inBuffer->pc2Elm;
        inBuffer->pc1Off = inBuffer->pc2Off + 2;

        pc = BUFF_strchr(inBuffer, "\\\"");

        state = 0;
      }
    }

    else if(state == 1)
    {
      if(pc == NULL)
      {
        inBuffer->idxElm = inBuffer->tail;
        inBuffer->idxOff = inBuffer->tail->part->len;

        ret = JSON_RC_INCOMPLETE;
      }

      else if(*pc == ':')
      {
 	    pair = RLST_getTail(inObject->pairList);

 	    pair->value

        inBuffer->idxElm = inBuffer->pc2Elm;
        inBuffer->idxOff = inBuffer->pc2Off + 1;

     	end = JSON_TRUE; *outSep = *pc;
      }

      else if(*pc == ',' || *pc == ']' || *pc == '}')
      {
 	    pair = RLST_getTail(inObject->pairList);

 	    pair->value = inBuffer->pc1Elm

        inBuffer->idxElm = inBuffer->pc2Elm;
        inBuffer->idxOff = inBuffer->pc2Off + 1;

     	end = JSON_TRUE; *outSep = *pc;
      }

      else // if(*pc != '"')
      {
        inBuffer->idxElm = inBuffer->pc2Elm;
        inBuffer->idxOff = inBuffer->pc2Off + 1;

        ret = JSON_RC_ERROR;
      }

      end = JSON_TRUE;
    }
  }

//----------------

  TRAZA1("Returning from JSON_string_decode() = %d", value);

  return value;
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


