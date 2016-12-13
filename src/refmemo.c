/*____________________________________________________________________________

  MODULO:	Lista de memoria auto-referenciada

  CATEGORIA:	UTILIB

  AUTOR:	Ricardo Aguado Mart'n

  VERSION:	1.0

  FECHA:	2005 - 2013
______________________________________________________________________________

  DESCRIPCION:

  OBSERVACIONES: 
______________________________________________________________________________*/

/*__INCLUDES DEL SISTEMA______________________________________________________*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

/*__INCLUDES DE LA BD_________________________________________________________*/

/*__INCLUDES DE LA APLICACION_________________________________________________*/

#include "trace_macros_refo.h"

#include "reflist.h"
#include "refmemo.h"

/*__CONSTANTES________________________________________________________________*/
  
/*__TIPOS_____________________________________________________________________*/

/*__VARIABLES EXTERNAS________________________________________________________*/

/*__VARIABLES GLOBALES________________________________________________________*/

/*__VARIABLES LOCALES_________________________________________________________*/

/*__FUNCIONES EXTERNAS________________________________________________________*/

/*__FUNCIONES PUBLICAS________________________________________________________*/

/*__FUNCIONES PRIVADAS________________________________________________________*/

static MEMO_memelem_t* newMemElem(MEMO_refmemo_t*);
static int delMemElem(MEMO_refmemo_t*, MEMO_memelem_t*);

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

MEMO_refmemo_t* 
MEMO_createRefMemo
(
  void*			inPtrOrig, 
  MEMO_element_t*	inPtrOffs,
  long			inElemSize,
  long			inBlockLen,
  long			inBlockMin
) 
{
  MEMO_refmemo_t*	ptrRefMemo;

  TRAZA0("Entering in MEMO_createRefMemo()");

/*----------*/

  ptrRefMemo = (MEMO_refmemo_t*) malloc(sizeof(MEMO_refmemo_t));

  if(ptrRefMemo == NULL)
  {
    SUCESO2("ERROR: malloc() = %d (%s)", errno, strerror(errno));
  }

/*----------*/

  else
  {
    if((MEMO_initializeRefMemo(ptrRefMemo, 
                               inPtrOrig, 
			       inPtrOffs, 
			       inElemSize,
			       inBlockLen,
			       inBlockMin)) != MEMO_RC_OK)
    {
      SUCESO0("ERROR: MEMO_initializeRefMemo()");

      free(ptrRefMemo); ptrRefMemo = NULL;
    }
  }
  
/*----------*/

  TRAZA1("Returning from MEMO_createRefMemo() = %p", ptrRefMemo);

  return ptrRefMemo;
}

/*----------------------------------------------------------------------------*/

int 
MEMO_destroyRefMemo(MEMO_refmemo_t* inPtrRefMemo)
{
  int		ret = MEMO_RC_OK;
  
  TRAZA1("Entering in MEMO_destroyRefMemo(%p)", inPtrRefMemo);

/*----------*/

  if(inPtrRefMemo != NULL) 
  {
    ret = MEMO_finalizeRefMemo(inPtrRefMemo);

    free(inPtrRefMemo);
  }

/*----------*/

  TRAZA1("Returning from MEMO_destroyRefMemo() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int 
MEMO_initializeRefMemo
(
  MEMO_refmemo_t*	inPtrRefMemo,
  void*			inPtrOrig, 
  MEMO_element_t*	inPtrOffs,
  long			inElemSize,
  long			inBlockLen,
  long			inBlockMin
) 
{
  MEMO_memelem_t*	ptrMeme;

  MEMO_memelem_t	auxMeme;

  long			i;

  int			rc;

  int			ret = RLST_RC_OK;

  TRAZA6("Entering in MEMO_initializeRefMemo(%p, %p, %p, %ld, %ld, %ld)",
          inPtrRefMemo,
	  inPtrOrig,
	  inPtrOffs,
	  inElemSize,
	  inBlockLen,
	  inBlockMin);

/*----------*/

  memset(inPtrRefMemo, 0, sizeof(MEMO_refmemo_t));

/*----------*/

  inPtrRefMemo->offs = (char*)(inPtrOffs) - (char*)(inPtrOrig);
  inPtrRefMemo->size = inElemSize;
  inPtrRefMemo->blen = inBlockLen;
  inPtrRefMemo->bmin = inBlockMin;
  inPtrRefMemo->free = 0;
  inPtrRefMemo->fmin = inBlockLen / 2 + 1;

/*----------*/

  if(ret == RLST_RC_OK)
  {
    rc = RLST_initializeRefList(&inPtrRefMemo->list, &auxMeme, &(auxMeme.list));

    if(rc < 0)
    {
      SUCESO0("ERROR: RLST_initializeRefList()");

      ret = MEMO_RC_OK;
    }
  }

/*----------*/

  if(ret == RLST_RC_OK)
  {
    ret = RLST_initializeRefList(&inPtrRefMemo->newl, &auxMeme, &(auxMeme.newl));

    if(rc < 0)
    {
      SUCESO0("ERROR: RLST_initializeRefList()");

      ret = MEMO_RC_OK;
    }
  }

/*----------*/

  for(i = 0; (i < inPtrRefMemo->bmin) && (ret == MEMO_RC_OK); i++)
  {
    ptrMeme = newMemElem(inPtrRefMemo);

    if(ptrMeme == NULL)
    {
      SUCESO0("ERROR: newMemElem()");
      
      while((ptrMeme = (MEMO_memelem_t*) RLST_getHead(&(inPtrRefMemo->list))))
      {
	delMemElem(inPtrRefMemo, ptrMeme); ptrMeme = NULL;
      }
      
      ret = MEMO_RC_ERROR;
    }
  }

/*----------*/

  TRAZA1("Returning from MEMO_initializeRefMemo() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int 
MEMO_finalizeRefMemo(MEMO_refmemo_t* inPtrRefMemo)
{
  MEMO_memelem_t*	ptrMeme;

  int			ret = MEMO_RC_OK;
  
  TRAZA1("Entering in MEMO_finalizeRefMemo(%p)", inPtrRefMemo);

/*----------*/

  if(inPtrRefMemo != NULL) 
  {
    while((ptrMeme = (MEMO_memelem_t*) RLST_getHead(&(inPtrRefMemo->list))))
    {
      delMemElem(inPtrRefMemo, ptrMeme); ptrMeme = NULL;
    }

    memset(inPtrRefMemo, 0, sizeof(MEMO_refmemo_t));
  }

/*----------*/

  TRAZA1("Returning from MEMO_finalizeRefMemo() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static MEMO_memelem_t* 
newMemElem(MEMO_refmemo_t* inPtrRefMemo)
{
  MEMO_memelem_t*	ptrMeme;

  TRAZA1("Entering in newMemElem(%p)", inPtrRefMemo);

/*----------*/

  DEPURA9("newMemElem(%p) = %ld:%ld:%ld:%ld:%ld:%ld:%ld:%ld",
           inPtrRefMemo,
	   inPtrRefMemo->offs,
	   inPtrRefMemo->size,
	   inPtrRefMemo->blen,
	   inPtrRefMemo->bmin,
	   inPtrRefMemo->free,
	   inPtrRefMemo->fmin,
	   RLST_getNumElem(&inPtrRefMemo->list),
	   RLST_getNumElem(&inPtrRefMemo->newl));

/*----------*/

  ptrMeme = (MEMO_memelem_t*) malloc(sizeof(MEMO_memelem_t));

  if(ptrMeme == NULL)
  {
    SUCESO2("ERROR: malloc() = %d (%s)", errno, strerror(errno));
  }

  else
  {
    memset(ptrMeme, 0, sizeof(MEMO_memelem_t)); 

    ptrMeme->rmem = inPtrRefMemo;
  }

/*----------*/

  if(ptrMeme != NULL)
  {
    ptrMeme->memo = malloc(inPtrRefMemo->size * inPtrRefMemo->blen);

    if(ptrMeme->memo == NULL)
    {
      SUCESO2("ERROR: malloc() = %d (%s)", errno, strerror(errno));

      free(ptrMeme); ptrMeme = NULL;
    }
  }

/*----------*/

  if(ptrMeme != NULL)
  {
    ptrMeme->newe = malloc(sizeof(long) * inPtrRefMemo->blen);

    if(ptrMeme->newe == NULL)
    {
      SUCESO2("ERROR: malloc() = %d (%s)", errno, strerror(errno));

      free(ptrMeme->memo); ptrMeme->memo = NULL; 

      free(ptrMeme); ptrMeme = NULL;
    }
  }

/*----------*/

  if(ptrMeme != NULL)
  {
    inPtrRefMemo->free += inPtrRefMemo->blen;
    
    RLST_insertTail(&(inPtrRefMemo->list), ptrMeme);
    RLST_insertTail(&(inPtrRefMemo->newl), ptrMeme);
  }

/*----------*/

  DEPURA9("newMemElem(%p) = %ld:%ld:%ld:%ld:%ld:%ld:%ld:%ld",
           inPtrRefMemo,
	   inPtrRefMemo->offs,
	   inPtrRefMemo->size,
	   inPtrRefMemo->blen,
	   inPtrRefMemo->bmin,
	   inPtrRefMemo->free,
	   inPtrRefMemo->fmin,
	   RLST_getNumElem(&inPtrRefMemo->list),
	   RLST_getNumElem(&inPtrRefMemo->newl));

/*----------*/

  TRAZA1("Returning from newMemElem() = %p", ptrMeme);

  return ptrMeme;
}

/*----------------------------------------------------------------------------*/

static int 
delMemElem(MEMO_refmemo_t* inPtrRefMemo, MEMO_memelem_t* inPtrMeme)
{
  int		ret = MEMO_RC_OK;
  
  TRAZA2("Entering in delMemElem(%p, %p)", inPtrRefMemo, inPtrMeme);

/*----------*/

  DEPURA9("delMemElem(%p) = %ld:%ld:%ld:%ld:%ld:%ld:%ld:%ld",
           inPtrRefMemo,
	   inPtrRefMemo->offs,
	   inPtrRefMemo->size,
	   inPtrRefMemo->blen,
	   inPtrRefMemo->bmin,
	   inPtrRefMemo->free,
	   inPtrRefMemo->fmin,
	   RLST_getNumElem(&inPtrRefMemo->list),
	   RLST_getNumElem(&inPtrRefMemo->newl));

/*----------*/

  if(inPtrMeme != NULL)
  {
    RLST_extract(&(inPtrRefMemo->list), inPtrMeme);
    RLST_extract(&(inPtrRefMemo->newl), inPtrMeme);
  }

/*----------*/

  if(inPtrMeme != NULL)
  {
    free(inPtrMeme->newe); inPtrMeme->newe = NULL;
  }

/*----------*/

  if(inPtrMeme != NULL)
  {
    free(inPtrMeme->memo); inPtrMeme->memo = NULL;
  }

/*----------*/

  if(inPtrMeme != NULL)
  {
    free(inPtrMeme);

    inPtrRefMemo->free -= inPtrRefMemo->blen;
  }

/*----------*/

  DEPURA9("delMemElem(%p) = %ld:%ld:%ld:%ld:%ld:%ld:%ld:%ld",
           inPtrRefMemo,
	   inPtrRefMemo->offs,
	   inPtrRefMemo->size,
	   inPtrRefMemo->blen,
	   inPtrRefMemo->bmin,
	   inPtrRefMemo->free,
	   inPtrRefMemo->fmin,
	   RLST_getNumElem(&inPtrRefMemo->list),
	   RLST_getNumElem(&inPtrRefMemo->newl));

/*----------*/

  TRAZA1("Returning from delMemElem() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void* 
MEMO_new(MEMO_refmemo_t* inPtrRefMemo)
{
  MEMO_memelem_t*	ptrMeme;
  void*			ptrExte;
  MEMO_element_t*	ptrElem;
  void*			ptrVoid;

  TRAZA1("Entering in MEMO_new(%p)", inPtrRefMemo);

/*----------*/

  ptrExte = NULL;
  
/*----------*/

  ptrMeme = (MEMO_memelem_t*) RLST_getHead(&(inPtrRefMemo->newl));

  if(ptrMeme == NULL)
  {
    ptrMeme = newMemElem(inPtrRefMemo);
  }

/*----------*/

  if(ptrMeme != NULL)
  {
    if(ptrMeme->newp == ptrMeme->mrkp)
    {
      ptrMeme->newe[ptrMeme->newp] = ptrMeme->newp * inPtrRefMemo->size;
      
      ptrMeme->mrkp++;
    }

    ptrExte = (char*) ptrMeme->memo + ptrMeme->newe[ptrMeme->newp];

    memset(ptrExte, 0, inPtrRefMemo->size);

    ptrVoid = (char*) ptrExte + inPtrRefMemo->offs;

    ptrElem = (MEMO_element_t*) ptrVoid;

    ptrElem->meme = ptrMeme; ptrElem->offs = ptrMeme->newe[ptrMeme->newp];

    inPtrRefMemo->free--; ptrMeme->newp++;

    if(ptrMeme->newp == inPtrRefMemo->blen)
    {
      RLST_extract(&(inPtrRefMemo->newl), ptrMeme);
    }
  }

/*----------*/

  TRAZA1("Returning from MEMO_new() = %p", ptrExte);

  return ptrExte;
}

/*----------------------------------------------------------------------------*/

void* 
MEMO_new_no_ini(MEMO_refmemo_t* inPtrRefMemo)
{
  MEMO_memelem_t*	ptrMeme;
  void*			ptrExte;
  MEMO_element_t*	ptrElem;
  void*			ptrVoid;

  TRAZA1("Entering in MEMO_new_no_ini(%p)", inPtrRefMemo);

/*----------*/

  ptrExte = NULL;
  
/*----------*/

  ptrMeme = (MEMO_memelem_t*) RLST_getHead(&(inPtrRefMemo->newl));

  if(ptrMeme == NULL)
  {
    ptrMeme = newMemElem(inPtrRefMemo);
  }

/*----------*/

  if(ptrMeme != NULL)
  {
    if(ptrMeme->newp == ptrMeme->mrkp)
    {
      ptrMeme->newe[ptrMeme->newp] = ptrMeme->newp * inPtrRefMemo->size;
      
      ptrMeme->mrkp++;
    }

    ptrExte = (char*) ptrMeme->memo + ptrMeme->newe[ptrMeme->newp];

    ptrVoid = (char*) ptrExte + inPtrRefMemo->offs;

    ptrElem = (MEMO_element_t*) ptrVoid;

    ptrElem->meme = ptrMeme; ptrElem->offs = ptrMeme->newe[ptrMeme->newp];

    inPtrRefMemo->free--; ptrMeme->newp++;

    if(ptrMeme->newp == inPtrRefMemo->blen)
    {
      RLST_extract(&(inPtrRefMemo->newl), ptrMeme);
    }
  }

/*----------*/

  TRAZA1("Returning from MEMO_new_no_ini() = %p", ptrExte);

  return ptrExte;
}

/*----------------------------------------------------------------------------*/

int
MEMO_delete(MEMO_refmemo_t* inPtrRefMemo, void* inPtrExte)
{
  MEMO_memelem_t*	ptrMeme;
  void*			ptrExte;
  void*			ptrVoid;
  MEMO_element_t*	ptrElem;
  
  int			ret = MEMO_RC_OK;
  
  TRAZA2("Entering in MEMO_delete(%p, %p)", inPtrRefMemo, inPtrExte);

/*----------*/

  ptrExte = inPtrExte;

  ptrVoid = (char*)(ptrExte) + inPtrRefMemo->offs;

  ptrElem = (MEMO_element_t*)(ptrVoid);

  ptrMeme = ptrElem->meme;

/*----------*/

  if(ptrMeme->rmem == inPtrRefMemo)
  {
    if(ptrMeme->newp == inPtrRefMemo->blen)
    {
      RLST_insertTail(&(inPtrRefMemo->newl), ptrMeme);
    }

    inPtrRefMemo->free++; ptrMeme->newp--;

    ptrMeme->newe[ptrMeme->newp] = ptrElem->offs;
    
    ptrElem->meme = NULL; ptrElem->offs = 0;

    if(ptrMeme->newp == 0)
    {
      if(RLST_getNumElem(&(inPtrRefMemo->list)) > inPtrRefMemo->bmin)
      {
        if(inPtrRefMemo->fmin < (inPtrRefMemo->free - inPtrRefMemo->blen))
	{
          delMemElem(inPtrRefMemo, ptrMeme);
	}
      }
    }
  }
  
/*----------*/

  TRAZA1("Returning from MEMO_delete() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/



