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

static RFTR_reftree_t		BUFF_MemoTree_s[1];
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
      BUFF_FATAL0("FATAL: MEMO_createRefMemo()");
    }

	BUFF_BuffMemo_s = MEMO_createRefMemo(&b, &b.memo, sizeof(b), 16, 2);

    if(BUFF_BuffMemo_s == NULL)
    {
      BUFF_FATAL0("FATAL: MEMO_createRefMemo()");
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

//----------------

  memset(ptrBuffMemo, 0, sizeof(BUFF_memo_t));

  size = sizeof(BUFF_part_t) + inSize + 2; size = BUFF_ALING(size);

  ptrBuffMemo->refm = MEMO_createRefMemo(&d, &d.memo, size, inBlen, inBmin);

  if(ptrBuffMemo->refm == NULL)
  {
    BUFF_FATAL0("FATAL: MEMO_createRefMemo()");
  }

  if(RFTR_insert(BUFF_MemoTree_s, ptrBuffMemo) != RFTR_RC_OK)
  {
    BUFF_FATAL0("FATAL: RFTR_insert()");
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
BUFF_part_new(BUFF_buff_t* inBuff)
{
  BUFF_part_t*			ptrPart = NULL;
  BUFF_elem_t*			ptrElem = NULL;

  static size_t			offs = sizeof(BUFF_part_t);

  TRAZA0("Entering in BUFF_part_new()");

//----------------

  ptrPart = (BUFF_part_t*) MEMO_new_no_ini(inBuff->type->refm);

  if(ptrPart == NULL)
  {
    BUFF_FATAL0("MEMO_new()");
  }

  inBuff->type->used++;

  if(inBuff->type->mark < inBuff->type->used)
  {
	inBuff->type->mark = inBuff->type->used;
  }

//----------------

  ptrPart->data = (unsigned char*)(ptrPart) + offs;
  ptrPart->type = inBuff->type;
  ptrPart->refs = 1;

  ptrPart->idx = 0;
  ptrPart->len = 0;

  ptrPart->data[0] = 0;

//----------------

  ptrElem = (BUFF_elem_t*) MEMO_new(BUFF_ElemMemo_s);

  if(ptrElem == NULL)
  {
    BUFF_FATAL0("MEMO_new()");
  }

  BUFF_ElemUsed_s++;

  if(BUFF_ElemMark_s < BUFF_ElemUsed_s)
  {
    BUFF_ElemMark_s = BUFF_ElemUsed_s;
  }

//----------------

  ptrElem->part = ptrPart;

  if(inBuff->tail == NULL)
  {
	inBuff->head = ptrElem;
	inBuff->tail = ptrElem;
  }

  else // if(inBuff->tail == NULL)
  {
	inBuff->tail->next = ptrElem;
	ptrElem->prev = inBuff->tail;

	inBuff->tail = ptrElem;
  }

//----------------

  TRAZA1("Returning from BUFF_part_new() = %p", ptrPart);

  return ptrPart;
}

/*----------------------------------------------------------------------------*/

void
BUFF_part_delete(BUFF_buff_t* inBuff, BUFF_part_t* inPart)
{
  BUFF_elem_t*			ptrElem;

  int					found = BUFF_FALSE;

  TRAZA2("Entering in BUFF_part_delete(%p, %p)", inBuff, inPart);

//----------------

  ptrElem = inBuff->head;

  while(ptrElem != NULL && found == BUFF_FALSE)
  {
	if(ptrElem->part == inPart)
	{
	  found = BUFF_TRUE;
	}

	else { ptrElem = ptrElem->next; }
  }

//----------------

  if(found == BUFF_TRUE)
  {
    inPart->refs--;

    if(inPart->refs <= 0)
    {
      inPart->type->used--;

      MEMO_delete(inPart->type->refm, inPart);
    }

	if(ptrElem->prev != NULL)
	{
	  ptrElem->prev->next = ptrElem->next;
	}

	if(ptrElem->next != NULL)
	{
	  ptrElem->next->prev = ptrElem->prev;
	}

	if(inBuff->head != ptrElem)
	{
	  inBuff->head = ptrElem->next;
	}

	if(inBuff->tail != ptrElem)
	{
	  inBuff->tail = ptrElem->prev;
	}

    MEMO_delete(BUFF_ElemMemo_s, ptrElem);

    BUFF_ElemUsed_s--;
  }

//----------------

  TRAZA0("Returning from BUFF_part_delete()");
}

/*----------------------------------------------------------------------------*/

void
BUFF_part_add(BUFF_buff_t* inBuff, BUFF_part_t* inPart)
{
  BUFF_elem_t*			ptrElem;

  int					found = BUFF_FALSE;

  TRAZA2("Entering in BUFF_part_add(%p, %p)", inBuff, inPart);

//----------------

  ptrElem = inBuff->head;

  while(ptrElem != NULL && found == BUFF_FALSE)
  {
  	if(ptrElem->part == inPart)
  	{
  	  found = BUFF_TRUE;
  	}

  	else { ptrElem = ptrElem->next; }
  }

//----------------

  if(found == BUFF_FALSE)
  {
    ptrElem = MEMO_new(BUFF_ElemMemo_s);

    if(ptrElem == NULL)
    {
      BUFF_FATAL0("MEMO_new()");
    }

    BUFF_ElemUsed_s++;

    if(BUFF_ElemMark_s < BUFF_ElemUsed_s)
    {
      BUFF_ElemMark_s = BUFF_ElemUsed_s;
    }

//----------------

    ptrElem->part = inPart;

    if(inBuff->tail == NULL)
    {
      inBuff->head = ptrElem;
      inBuff->tail = ptrElem;
    }

    else // if(inBuff->tail == NULL)
    {
      inBuff->tail->next = ptrElem;
      ptrElem->prev = inBuff->tail;

      inBuff->tail = ptrElem;
    }
  }

//----------------

  TRAZA0("Returning from BUFF_part_add()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

BUFF_buff_t*
BUFF_buff_new(BUFF_memo_t* inBuffMemo)
{
  BUFF_buff_t*		ptrBuff = NULL;

  TRAZA1("Entering in BUFF_buff_new(%p)", inBuffMemo);

//----------------

  ptrBuff = MEMO_new(BUFF_ElemMemo_s);

  if(ptrBuff == NULL)
  {
    BUFF_FATAL0("MEMO_new()");
  }

  BUFF_BuffUsed_s++;

  if(BUFF_BuffMark_s < BUFF_BuffUsed_s)
  {
	BUFF_BuffMark_s = BUFF_BuffUsed_s;
  }

//----------------

  ptrBuff->type = inBuffMemo;

//----------------

  TRAZA1("Returning from BUFF_buff_new() = %p", ptrBuff);

  return ptrBuff;
}

/*----------------------------------------------------------------------------*/

void
BUFF_buff_delete(BUFF_buff_t* inBuff)
{
  TRAZA1("Entering in BUFF_buff_delete(%p)", inBuff);

//----------------

  MEMO_delete(BUFF_BuffMemo_s, inBuff);

  BUFF_BuffUsed_s--;

//----------------

  TRAZA0("Returning from BUFF_buff_delete()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

char*
BUFF_strchr(BUFF_buff_t* inBuffer, const char* inChars)
{
  BUFF_elem_t* 		elm;
  long				off;
  long				len;
  long				inc;

  char*				pc;
  char*				ps = NULL;

  int				end = BUFF_FALSE;

  TRAZA2("Entering in BUFF_strchr(%p, %s)", inBuffer, inChars);

//----------------

  elm = inBuffer->pc1Elm;
  off = inBuffer->pc1Off;
  len = 0;

  if(off >= elm->part->type->size)
  {
	inc = off - elm->part->type->size;

	if(elm->next == NULL)
	{
	  end = BUFF_TRUE;
	}

    else if(elm->next->part->len < inc)
    {
      end = BUFF_TRUE;
    }

    else
    {
      inBuffer->pc1Elm = inBuffer->pc1Elm->next;
      inBuffer->pc2Off = 0;
    }

    elm = inBuffer->pc1Elm;
    off = inBuffer->pc1Off;
  }

//----------------

  while(end == BUFF_FALSE)
  {
    pc = (char*)(elm->part->data + off);

    ps = strpbrk(pc, inChars);

    if(ps != NULL)
    {
      off = ps - (char*)(elm->part->data); len += ps - pc;

      inBuffer->pc2Elm = elm;
      inBuffer->pc2Off = off;
      inBuffer->pcsLen = len;

      end = BUFF_TRUE;
    }

    else if(elm->part->len < elm->part->type->size)
    {
      end = BUFF_TRUE;
    }

    else if(elm->next == NULL)
    {
      end = BUFF_TRUE;
    }

    else if(elm->next->part->len == 0)
    {
      end = BUFF_TRUE;
    }

    else // if(elm->next->part->len > 0)
    {
      if(inBuffer->pc1Off >= inBuffer->pc1Elm->part->type->size)
      {
        inBuffer->pc1Elm = inBuffer->pc1Elm->next;
        inBuffer->pc2Off = 0;
      }

      len += elm->part->type->size - off;

      elm = elm->next; off = 0;
    }
  }

//----------------

  TRAZA1("Returning from BUFF_strchr() = %p", ps);

  return ps;
}

/*----------------------------------------------------------------------------*/

char*
BUFF_strspn(BUFF_buff_t* inBuffer, const char* inChars)
{
  BUFF_elem_t* 		elm;
  long				off;
  long				len;
  long				spn;
  long				inc;

  char*				pc = NULL;

  int				end = BUFF_FALSE;

  TRAZA2("Entering in BUFF_strspn(%p, %s)", inBuffer, inChars);

//----------------

  elm = inBuffer->pc1Elm;
  off = inBuffer->pc1Off;
  len = 0;

  if(off >= elm->part->type->size)
  {
	inc = off - elm->part->type->size;

	if(elm->next == NULL)
	{
	  end = BUFF_TRUE;
	}

    else if(elm->next->part->len < inc)
    {
      end = BUFF_TRUE;
    }

    else
    {
      inBuffer->pc1Elm = inBuffer->pc1Elm->next;
      inBuffer->pc2Off = 0;

      elm = inBuffer->pc1Elm;
      off = inBuffer->pc1Off;
    }
  }

//----------------

  while(end == BUFF_FALSE)
  {
    pc = (char*)(elm->part->data + off);

    spn = strspn(pc, inChars);

    len += spn; off += spn; pc += spn;

    if(off < elm->part->type->size)
    {
      inBuffer->pc2Elm = elm;
      inBuffer->pc2Off = off;
      inBuffer->pcsLen = len;

      end = BUFF_TRUE;
    }

    else if(elm->next == NULL)
    {
      end = BUFF_TRUE;
    }

    else if(elm->next->part->len == 0)
    {
      end = BUFF_TRUE;
    }

    else // if(elm->next->part->len > 0)
    {
      if(inBuffer->pc1Off >= inBuffer->pc1Elm->part->type->size)
      {
        inBuffer->pc1Elm = inBuffer->pc1Elm->next;
        inBuffer->pc2Off = 0;
      }

      elm = elm->next; off = 0;
    }
  }

//----------------

  TRAZA1("Returning from BUFF_strspn() = %p", pc);

  return pc;
}

/*----------------------------------------------------------------------------*/

char*
BUFF_strfix(BUFF_buff_t* inBuffer, BUFF_buff_t* inAuxBuff)
{
  BUFF_elem_t* 		dst;
  long				dof;

  BUFF_elem_t* 		src;
  long				sof;

  long				len;
  long				inc;

  char*				pc;
  char*				pz;

  int				end = BUFF_FALSE;

  TRAZA2("Entering in BUFF_strfix(%p, %p, %p)", inBuffer, inAuxBuff);

//----------------

  if(inBuffer->pc1Elm == inBuffer->pc2Elm)
  {
    pc = (char*)(inBuffer->pc1Elm->part->data + inBuffer->pc1Off);
    pz = (char*)(inBuffer->pc2Elm->part->data + inBuffer->pc2Off); *pz = 0;
  }

  else // if(inBuffer->pc1Elm != inBuffer->pc2Elm)
  {
	if(inAuxBuff->tail == NULL)
	{
	  BUFF_part_new(inAuxBuff);
	}

	src = inBuffer->pc1Elm;
	sof = inBuffer->pc1Off;
	len = inBuffer->pcsLen + 1;

	dst = inAuxBuff->tail;
	dof = inAuxBuff->tail->part->len;

	if(len > (dst->part->type->size - dst->part->len))
	{
	  if(dst->part->len == 0)
	  {
		len = dst->part->type->size;
	  }

	  else // if(dst->part->len >= 0)
	  {
		BUFF_part_new(inAuxBuff);

     	dst = inAuxBuff->tail;
	    dof = inAuxBuff->tail->part->len;

		len = BUFF_MAX(len, dst->part->type->size);
	  }
	}

	pc = (char*)(dst->part->data + dof);

	while(end == BUFF_FALSE)
	{
      inc = BUFF_MIN(len, src->part->type->size - sof);

	  memcpy(dst->part->data + dof, src->part->data + sof, inc);

	  dof += inc; sof += inc; len -= inc;

	  if(len > 0)
	  {
		src = src->next; sof = 0;
	  }

	  else // if(len <= 0)
	  {
		dst->part->data[dof - 1] = 0;

		dst->part->len = dof;

		end = BUFF_TRUE;
	  }
	}
  }

//----------------

  TRAZA1("Returning from BUFF_strfix() = %p", pc);

  return pc;
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
