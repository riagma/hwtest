/*____________________________________________________________________________

  MODULO:	Basic SIP library

  CATEGORIA:	BUFF

  AUTOR:	Ricardo Aguado Mart'n

  VERSION:	1.0

  FECHA:	2015
______________________________________________________________________________

  DESCRIPCION:

  OBSERVACIONES: 
______________________________________________________________________________*/

/*__INCLUDES DEL SISTEMA______________________________________________________*/

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
/*__INCLUDES DE LA BD_________________________________________________________*/

/*__INCLUDES DE LA APLICACION_________________________________________________*/

#include "trace_macros.h"
#include "auxfunctions.h"

#include "buffer.h"

/*__CONSTANTES________________________________________________________________*/
  
//----------------

const char* BUFF_RC[] =
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

static int					BUFF_Init_s = 0;

static MEMO_refmemo_t*		BUFF_ElemMemo_s = 0;
static long					BUFF_ElemUsed_s = 0;
static long					BUFF_ElemMark_s = 0;

static MEMO_refmemo_t*		BUFF_BuffMemo_s = 0;
static long					BUFF_BuffUsed_s = 0;
static long					BUFF_BuffMark_s = 0;

static RFTR_reftree_t*		BUFF_MemoTree_s = 0;
static long					BUFF_MemoUsed_s = 0;
static long					BUFF_MemoMark_s = 0;

//----------------

/*__FUNCIONES EXTERNAS________________________________________________________*/

/*__FUNCIONES PUBLICAS________________________________________________________*/

/*__FUNCIONES PRIVADAS________________________________________________________*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int 
BUFF_initialize(void)
{
  BUFF_elem_t			e;
  BUFF_buff_t			b;
  BUFF_memo_t			m;

  int					ret = BUFF_RC_OK;

  TRAZA0("Entering in BUFF_memo_initialize()");

//----------------

  if(BUFF_Init_s == BUFF_FALSE)
  {
	BUFF_Init_s = BUFF_TRUE;

	BUFF_ElemMemo_s = MEMO_createRefMemo(&e, &e.memo, sizeof(e), 256, 4);

    if(BUFF_ElemMemo_s == NULL)
    {
      BUFF_FATAL0("ERROR: MEMO_createRefMemo()");
    }

	BUFF_BuffMemo_s = MEMO_createRefMemo(&b, &b.memo, sizeof(b), 16, 2);

    if(BUFF_BuffMemo_s == NULL)
    {
      BUFF_FATAL0("ERROR: MEMO_createRefMemo()");
    }

    if(RFTR_initializeRefTree(BUFF_MemoTree_s, &m, &m.tree, BUFF_memo_cmp) < 0)
    {
      BUFF_FATAL0("RLST_initializeRefList()");
    }
  }

//----------------

  TRAZA1("Returning from BUFF_memo_initialize() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

void 
BUFF_memo_view(void)
{	   
  BUFF_memo_t*		ptrBuffMemo = NULL;

  RFTR_resetGet(BUFF_MemoTree_s, NULL);

  while((ptrBuffMemo = RFTR_getNext(BUFF_MemoTree_s)))
  {
	SUCESO3("BUFF-DATA(%p) = %ld(%ld)", ptrBuffMemo,
		                                ptrBuffMemo->used,
										ptrBuffMemo->mark);
  }

  SUCESO4("BUFF-ELEM(%p) = %ld(%ld)(%ld)", BUFF_ElemMemo_s,
	                                       BUFF_ElemUsed_s,
	                                       BUFF_ElemMark_s,
									       BUFF_ElemMemo_s->list.nume);

  SUCESO4("BUFF-BUFF(%p) = %ld(%ld)(%ld)", BUFF_BuffMemo_s,
	                                       BUFF_BuffUsed_s,
	                                       BUFF_BuffMark_s,
									       BUFF_ElemMemo_s->list.nume);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

BUFF_memo_t*
BUFF_memo_new(long inSize, long inBlen, long inBmin)
{
  BUFF_memo_t*		ptrBuffMemo = NULL;

  BUFF_part_t		d;

  long				size;

  TRAZA3("Entering in BUFF_memo_new(%ld, %ld, %ld)", inSize, inBlen, inBmin);

//----------------

  ptrBuffMemo = (BUFF_memo_t*) malloc(sizeof(BUFF_memo_t));

  if(ptrBuffMemo == NULL)
  {
    BUFF_FATAL0("MEMO_new()");
  }

  BUFF_MemoUsed_s++;

  if(BUFF_MemoMark_s < BUFF_MemoUsed_s)
  {
	BUFF_MemoMark_s = BUFF_MemoUsed_s;
  }

  if(RFTR_insert(BUFF_MemoTree_s, BUFF_MemoUsed_s) != RFTR_RC_OK)
  {
    BUFF_FATAL0("ERROR: RFTR_insert()");
  }

//----------------

  size = sizeof(BUFF_part_t) + inSize + 1; size = BUFF_ALING(size);

  ptrBuffMemo->refm = MEMO_createRefMemo(&d, &d.memo, size, inBlen, inBmin);

  if(ptrBuffMemo->refm == NULL)
  {
    BUFF_FATAL0("ERROR: MEMO_createRefMemo()");
  }

//----------------

  TRAZA1("Returning from BUFF_memo_new() = %p", ptrBuffMemo);

  return ptrBuffMemo;
}

/*----------------------------------------------------------------------------*/

void
BUFF_memo_delete(BUFF_memo_t* inBuffMemo)
{
  TRAZA1("Entering in BUFF_memo_delete(%p)", inBuffMemo);

//----------------

  RFTR_extract(BUFF_MemoTree_s, inBuffMemo);

  free(inBuffMemo); BUFF_MemoUsed_s--;

//----------------

  TRAZA0("Returning from BUFF_memo_delete()");
}

/*----------------------------------------------------------------------------*/

int
BUFF_memo_cmp(void* inVoidA, void* inVoidB)
{
  BUFF_memo_t*		inMemoA = inVoidA;
  BUFF_memo_t*		inMemoB = inVoidB;

  int				cmp = 0;

//TRAZA2("Entering in BUFF_memo_cmp(%p, %p)", inMemoA, inMemoB);

//----------------

  cmp = BUFF_CMP(inMemoA, inMemoB);

//----------------

//TRAZA1("Returning from BUFF_memo_cmp() = %d", cmp);

  return cmp;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

BUFF_part_t*
BUFF_part_new(BUFF_buff_t* inBuffBuff)
{
  BUFF_part_t*			ptrBuffData = NULL;
  BUFF_elem_t*			ptrBuffElem = NULL;

  static size_t			offs = sizeof(BUFF_part_t);

  TRAZA0("Entering in BUFF_part_new()");

//----------------

  ptrBuffData = (BUFF_part_t*) MEMO_new_no_ini(inBuffBuff->type->refm);

  if(ptrBuffData == NULL)
  {
    BUFF_FATAL0("MEMO_new()");
  }

  inBuffBuff->type->used++;

  if(inBuffBuff->type->mark < inBuffBuff->type->used)
  {
	inBuffBuff->type->mark = inBuffBuff->type->used;
  }

//----------------

  ptrBuffData->data = (unsigned char*)(ptrBuffData) + offs;
  ptrBuffData->size = inBuffBuff->type->size;
  ptrBuffData->refs = 1;

  ptrBuffData->idx = 0;
  ptrBuffData->len = 0;

  ptrBuffData->data[0] = 0;

//----------------

  ptrBuffElem = (BUFF_elem_t*) MEMO_new(BUFF_ElemMemo_s);

  if(ptrBuffElem == NULL)
  {
    BUFF_FATAL0("MEMO_new()");
  }

  BUFF_ElemUsed_s++;

  if(BUFF_ElemMark_s < BUFF_ElemUsed_s)
  {
    BUFF_ElemMark_s = BUFF_ElemUsed_s;
  }

//----------------

  ptrBuffElem->part = ptrBuffData;

  if(inBuffBuff->tail == NULL)
  {
	inBuffBuff->head = ptrBuffElem;
	inBuffBuff->tail = ptrBuffElem;
  }

  else // if(inBuffBuff->tail == NULL)
  {
	inBuffBuff->tail->next = ptrBuffElem;
	ptrBuffElem->prev = inBuffBuff->tail;

	inBuffBuff->tail = ptrBuffElem;
  }

//----------------

  TRAZA1("Returning from BUFF_part_new() = %p", ptrBuffData);

  return ptrBuffData;
}

/*----------------------------------------------------------------------------*/

void
BUFF_part_delete(BUFF_buff_t* inBuffBuff, BUFF_part_t* inBuffData)
{
  BUFF_elem_t*			ptrBuffElem;

  int					found = BUFF_FALSE;

  TRAZA2("Entering in BUFF_part_delete(%p, %p)", inBuffBuff, inBuffData);

//----------------

  ptrBuffElem = inBuffBuff->head;

  while(ptrBuffElem != NULL && found == BUFF_FALSE)
  {
	if(ptrBuffElem->part == inBuffData)
	{
	  found = BUFF_TRUE;
	}

	else { ptrBuffElem = ptrBuffElem->next; }
  }

//----------------

  if(found == BUFF_TRUE)
  {
    inBuffData->refs--;

    if(inBuffData->refs <= 0)
    {
      inBuffBuff->type->used--;

      MEMO_delete(inBuffBuff->type->refm, inBuffData);
    }

	if(ptrBuffElem->prev != NULL)
	{
	  ptrBuffElem->prev->next = ptrBuffElem->next;
	}

	if(ptrBuffElem->next != NULL)
	{
	  ptrBuffElem->next->prev = ptrBuffElem->prev;
	}

	if(inBuffBuff->head != ptrBuffElem)
	{
	  inBuffBuff->head = ptrBuffElem->next;
	}

	if(inBuffBuff->tail != ptrBuffElem)
	{
	  inBuffBuff->tail = ptrBuffElem->prev;
	}

    MEMO_delete(BUFF_ElemMemo_s, ptrBuffElem);

    BUFF_ElemUsed_s--;
  }

//----------------

  TRAZA0("Returning from BUFF_part_delete()");
}

/*----------------------------------------------------------------------------*/

void
BUFF_part_add(BUFF_buff_t* inBuffBuff, BUFF_part_t* inBuffData)
{
  BUFF_elem_t*			ptrBuffElem;

  int					found = BUFF_FALSE;

  TRAZA2("Entering in BUFF_part_add(%p, %p)", inBuffBuff, inBuffData);

//----------------

  ptrBuffElem = inBuffBuff->head;

  while(ptrBuffElem != NULL && found == BUFF_FALSE)
  {
  	if(ptrBuffElem->part == inBuffData)
  	{
  	  found = BUFF_TRUE;
  	}

  	else { ptrBuffElem = ptrBuffElem->next; }
  }

//----------------

  if(found == BUFF_FALSE)
  {
    ptrBuffElem = (BUFF_elem_t*) MEMO_new(BUFF_ElemMemo_s);

    if(ptrBuffElem == NULL)
    {
      BUFF_FATAL0("MEMO_new()");
    }

    BUFF_ElemUsed_s++;

    if(BUFF_ElemMark_s < BUFF_ElemUsed_s)
    {
      BUFF_ElemMark_s = BUFF_ElemUsed_s;
    }

//----------------

    ptrBuffElem->part = inBuffData;

    if(inBuffBuff->tail == NULL)
    {
      inBuffBuff->head = ptrBuffElem;
      inBuffBuff->tail = ptrBuffElem;
    }

    else // if(inBuffBuff->tail == NULL)
    {
      inBuffBuff->tail->next = ptrBuffElem;
      ptrBuffElem->prev = inBuffBuff->tail;

      inBuffBuff->tail = ptrBuffElem;
    }
  }

//----------------

  TRAZA0("Returning from BUFF_part_add()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

char*
BUFF_strchr(BUFF_buff_t* inBuffer, const char* inChars)
{
  BUFF_part_t 		part;

  char*				pc;
  char*				ps;

  long				off = 0;
  int				end = BUFF_FALSE;

  TRAZA2("Entering in BUFF_strchr(%p, %s)", inBuffer, inChars);

//----------------

  part = inBuffer->org->part;

  pc = (char*)(part->data + part->idx);

  while(end == BUFF_FALSE)
  {
    ps = strpbrk(pc, inChars);

	if(ps != NULL)
	{
      if(inBuffer->idx == inBuffer->org)
	  {

	  }

	  else // if(inBuffer->idx != inBuffer->org)
	  {

	  }
	}

	else
	{
	}
  }

//----------------

  TRAZA1("Returning from BUFF_strchr() = %p", ps);

  return ps;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void
BUFF_fatal_error(char* inFile, int inLine, const char* inErrorStr, ...)
{
  va_list		list;

  char			errorStr[BUFF_MAXLEN_STRING + 1];

//----------------

  va_start(list, inErrorStr);

  vsnprintf(errorStr, BUFF_MAXLEN_STRING + 1, inErrorStr, list);

  va_end(list);

  errorStr[BUFF_MAXLEN_STRING] = 0;

//----------------

  TRACE_write(TRACE_TYPE_DEFAULT, 0, "<%s %04d> %s", inFile, inLine, errorStr);

//----------------

  exit(-1);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
