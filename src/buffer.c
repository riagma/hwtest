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

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <pthread.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>   
#include <sys/types.h>
 
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

static int					BUFF_MemoInit_s = 0;

static MEMO_refmemo_t*		BUFF_RefsMemo_s = 0;
static long					BUFF_RefsUsed_s = 0;
static long					BUFF_RefsMark_s = 0;

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
  BUFF_refs_t			r;
  BUFF_memo_t			m;

  int					ret = BUFF_RC_OK;

  TRAZA0("Entering in BUFF_memo_initialize()");

//----------------

  if(BUFF_MemoInit_s == BUFF_FALSE)
  {
	BUFF_MemoInit_s = BUFF_TRUE;

	BUFF_RefsMemo_s = MEMO_createRefMemo(&r, &r.memo, sizeof(r), 128, 2);

    if(BUFF_RefsMemo_s == NULL)
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

  SUCESO4("BUFF-REFS(%p) = %ld(%ld)(%ld)", BUFF_RefsMemo_s,
	                                       BUFF_RefsUsed_s,
	                                       BUFF_RefsMark_s,
									       BUFF_RefsMemo_s->list.nume);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

BUFF_memo_t*
BUFF_memo_new(long inSize, long inBlen, long inBmin)
{
  BUFF_memo_t*		ptrBuffMemo = NULL;

  BUFF_data_t		d;

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

  size = sizeof(BUFF_data_t) + inSize + sizeof(void*) * 2;

  size = BUFF_ALING(size);

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

BUFF_data_t*
BUFF_data_new(BUFF_memo_t* inBuffMemo)
{
  BUFF_data_t*			ptrBuffData = NULL;

  static size_t			offs = sizeof(BUFF_data_t);

  TRAZA0("Entering in BUFF_data_new()");

//----------------

  ptrBuffData = (BUFF_data_t*) MEMO_new_no_ini(inBuffMemo->refm);

  if(ptrBuffData == NULL)
  {
    BUFF_FATAL0("MEMO_new()");
  }

  inBuffMemo->used++;

  if(inBuffMemo->mark < inBuffMemo->used)
  {
	inBuffMemo->mark = inBuffMemo->used;
  }

//----------------

  ptrBuffData->data = (unsigned char*)(ptrBuffData) + offs;
  ptrBuffData->refs = 1;

  ptrBuffData->idx = 0;
  ptrBuffData->len = 0;

  ptrBuffData->next = NULL;
  ptrBuffData->prev = NULL;
  ptrBuffData->type = inBuffMemo;

  ptrBuffData->data[0] = 0;

//----------------

  TRAZA1("Returning from BUFF_data_new() = %p", ptrBuffData);

  return ptrBuffData;
}

/*----------------------------------------------------------------------------*/

void
BUFF_data_delete(BUFF_data_t* inBuffData)
{
  TRAZA1("Entering in BUFF_data_delete(%p)", inBuffData);

//----------------

  inBuffData->refs--;

  if(inBuffData->refs <= 0)
  {
	if(inBuffData->prev != NULL)
	{
	  inBuffData->prev->next = inBuffData->next;
	}

	if(inBuffData->next != NULL)
	{
	  inBuffData->next->prev = inBuffData->prev;
	}

	inBuffData->type->used--;

    MEMO_delete(inBuffData->type->refm, inBuffData);
  }

//----------------

  TRAZA0("Returning from BUFF_data_delete()");
}

/*----------------------------------------------------------------------------*/

BUFF_data_t*
BUFF_data_add(BUFF_data_t* inBuffData)
{
  BUFF_data_t*			ptrBuffData = inBuffData;

  TRAZA0("Entering in BUFF_data_add()");

//----------------

  while(ptrBuffData->next != NULL) ptrBuffData = ptrBuffData->next;

  ptrBuffData->next = BUFF_data_new(ptrBuffData->type);

  ptrBuffData->next->prev = ptrBuffData;

//----------------

  TRAZA1("Returning from BUFF_data_add() = %p", ptrBuffData->next);

  return ptrBuffData->next;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

BUFF_refs_t*
BUFF_refs_new(void)
{
  BUFF_refs_t*		ptrBuffRefs = NULL;

  TRAZA0("Entering in BUFF_refs_new()");

//----------------

  ptrBuffRefs = (BUFF_refs_t*) MEMO_new(BUFF_RefsMemo_s);

  if(ptrBuffRefs == NULL)
  {
    BUFF_FATAL0("MEMO_new()");
  }

  BUFF_RefsUsed_s++;

  if(BUFF_RefsMark_s < BUFF_RefsUsed_s)
  {
	BUFF_RefsMark_s = BUFF_RefsUsed_s;
  }

 //----------------

  TRAZA1("Returning from BUFF_refs_new() = %p", ptrBuffRefs);

  return ptrBuffRefs;
}

/*----------------------------------------------------------------------------*/

void
BUFF_refs_delete(BUFF_refs_t* inBuffRefs)
{
  TRAZA1("Entering in BUFF_refs_delete(%p)", inBuffRefs);

//----------------

  MEMO_delete(BUFF_RefsMemo_s, inBuffRefs);

  BUFF_RefsUsed_s--;

//----------------

  TRAZA0("Returning from BUFF_refs_delete()");
}

/*----------------------------------------------------------------------------*/

void
BUFF_refs_add
(
  BUFF_refs_t* 			inBuffRefs,
  BUFF_data_t* 			inBuffData
)
{
  BUFF_refs_t* 			ptrBuffRefs;

  int					found = BUFF_FALSE;

  TRAZA2("Entering in BUFF_refs_add(%p, %p)", inBuffRefs, inBuffData);

//----------------

  ptrBuffRefs = inBuffRefs;

  if(ptrBuffRefs->refs == inBuffData)
  {
    found = BUFF_TRUE;
  }

  else while(ptrBuffRefs->next != NULL && found == BUFF_FALSE)
  {
	ptrBuffRefs = ptrBuffRefs->next;

    if(ptrBuffRefs->refs == inBuffData)
    {
      found = BUFF_TRUE;
    }
  }

//----------------

  if(found == BUFF_FALSE)
  {
    if(ptrBuffRefs->refs != NULL)
    {
      ptrBuffRefs->next = BUFF_refs_new();

      ptrBuffRefs = ptrBuffRefs->next;
    }
      
    ptrBuffRefs->refs = inBuffData;

    inBuffData->refs++;
  }

//----------------

  TRAZA0("Returning from BUFF_burefs_add()");
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
