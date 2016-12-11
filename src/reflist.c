/*____________________________________________________________________________

  MODULO:	Lista auto-referenciada

  CATEGORIA:	UTILIB

  AUTOR:	Ricardo Aguado Mart'n

  VERSION:	1.0

  FECHA:	2005 - 2013	

______________________________________________________________________________

  DESCRIPCION:

  OBSERVACIONES: 
______________________________________________________________________________*/

/*__INCLUDES DEL SISTEMA______________________________________________________*/

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>   
#include <sys/types.h>

#ifdef __linux__ 
#include <dirent.h>
#include <semaphore.h>
#else
#include <sys/dirent.h>
#include <sys/semaphore.h>
#endif

/*__INCLUDES DE LA BD_________________________________________________________*/

/*__INCLUDES DE LA APLICACION_________________________________________________*/

#include "trace_macros_refo.h"

#include "reflist.h"

/*__CONSTANTES________________________________________________________________*/
  
/*__TIPOS_____________________________________________________________________*/

/*__VARIABLES EXTERNAS________________________________________________________*/

/*__VARIABLES GLOBALES________________________________________________________*/

/*__VARIABLES LOCALES_________________________________________________________*/

/*__FUNCIONES EXTERNAS________________________________________________________*/

/*__FUNCIONES PUBLICAS________________________________________________________*/

/*__FUNCIONES PRIVADAS________________________________________________________*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

RLST_reflist_t* 
RLST_createRefList(void* inPtrOrig, RLST_element_t* inPtrOffs) 
{
  RLST_reflist_t*	ptrRefList;

  TRAZA0("Entering in RLST_createRefList()");

/*----------*/

  ptrRefList = (RLST_reflist_t*) malloc(sizeof(RLST_reflist_t));

  if(ptrRefList == NULL)
  {
    SUCESO2("FATAL: malloc() = %d (%s)", errno, strerror(errno)); return NULL;
  }
  
/*----------*/

  memset(ptrRefList, 0, sizeof(RLST_reflist_t));

/*----------*/

  ptrRefList->offs = (char*)(inPtrOffs) - (char*)(inPtrOrig);

/*----------*/

  TRAZA1("Returning from RLST_createRefList() = %p", ptrRefList);

  return ptrRefList;
}

/*----------------------------------------------------------------------------*/

int 
RLST_destroyRefList(RLST_reflist_t* inPtrRefList)
{
  int		ret = RLST_RC_OK;
  
  TRAZA1("Entering in RLST_destroyRefList(%p)", inPtrRefList);

/*----------*/

  if(inPtrRefList != NULL)
  {
    ret = RLST_finalizeRefList(inPtrRefList);
    
    free(inPtrRefList);
  }

/*----------*/

  TRAZA1("Returning from RLST_destroyRefList() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int 
RLST_initializeRefList
(
  RLST_reflist_t*	inPtrRefList,
  void*			inPtrOrig, 
  RLST_element_t*	inPtrOffs
) 
{
  RLST_reflist_t*	ptrRefList;

  int			ret = RLST_RC_OK;
  
  TRAZA1("Entering in RLST_initializeRefList(%p)", inPtrRefList);

/*----------*/

  ptrRefList = inPtrRefList;

/*----------*/

  memset(ptrRefList, 0, sizeof(RLST_reflist_t));

/*----------*/

  ptrRefList->offs = (char*)(inPtrOffs) - (char*)(inPtrOrig);

/*----------*/

  TRAZA1("Returning from RLST_initializeRefList() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int 
RLST_finalizeRefList(RLST_reflist_t* inPtrRefList)
{
  int		ret = RLST_RC_OK;
  
  TRAZA1("Entering in RLST_finalizeRefList(%p)", inPtrRefList);

/*----------*/

  if(inPtrRefList != NULL)
  {
    while(RLST_extractHead(inPtrRefList) != NULL);
    
    memset(inPtrRefList, 0, sizeof(RLST_reflist_t));
  }

/*----------*/

  TRAZA1("Returning from RLST_finalizeRefList() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int 
RLST_insertHead(RLST_reflist_t* inPtrRefList, void* inPtrElement)
{
  RLST_reflist_t*	ptrRefList;

  void*			ptrExtElem;
  
  void*			ptrAuxVoid;
  
  RLST_element_t*	ptrElement;

  int			ret = RLST_RC_OK;
  
  TRAZA2("Entering in RLST_insertHead(%p, %p)", inPtrRefList, inPtrElement);

/*----------*/

  ptrRefList = inPtrRefList;

  ptrExtElem = inPtrElement;

  ptrAuxVoid = (char*)(ptrExtElem) + ptrRefList->offs;

  ptrElement = (RLST_element_t*)(ptrAuxVoid);

/*----------*/

  if(ptrElement->list == NULL)
  {
    ptrElement->list = ptrRefList;
    ptrElement->elem = ptrExtElem;
    ptrElement->prev = NULL;
    ptrElement->next = ptrRefList->head;

    if(ptrRefList->head != NULL)
    {
      ptrRefList->head->prev = ptrElement;
    }

    ptrRefList->head = ptrElement;

    if(ptrRefList->tail == NULL)
    {
      ptrRefList->tail = ptrElement;
    }

    ptrRefList->nume++;
    
    ret = RLST_RC_OK;
  }

  else
  {
    SUCESO0("ERROR: This element belongs to another list!");
    
    ret = RLST_RC_ERROR;
  }

/*----------*/

  TRAZA1("Returning from RLST_insertHead() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int 
RLST_insertTail(RLST_reflist_t* inPtrRefList, void* inPtrElement)
{
  RLST_reflist_t*	ptrRefList;

  void*			ptrExtElem;
  
  void*			ptrAuxVoid;
  
  RLST_element_t*	ptrElement;

  int			ret = RLST_RC_OK;
  
  TRAZA2("Entering in RLST_insertTail(%p, %p)", inPtrRefList, inPtrElement);

/*----------*/

  ptrRefList = inPtrRefList;

  ptrExtElem = inPtrElement;

  ptrAuxVoid = (char*)(ptrExtElem) + ptrRefList->offs;

  ptrElement = (RLST_element_t*)(ptrAuxVoid);

/*----------*/

  if(ptrElement->list == NULL)
  {
    ptrElement->list = ptrRefList;
    ptrElement->elem = ptrExtElem;
    ptrElement->prev = ptrRefList->tail;
    ptrElement->next = NULL;

    if(ptrRefList->tail != NULL)
    {
      ptrRefList->tail->next = ptrElement;
    }

    ptrRefList->tail = ptrElement;

    if(ptrRefList->head == NULL)
    {
      ptrRefList->head = ptrElement;
    }

    ptrRefList->nume++;
    
    ret = RLST_RC_OK;
  }

  else
  {
    SUCESO0("ERROR: This element belongs to another list!");
    
    ret = RLST_RC_ERROR;
  }

/*----------*/

  TRAZA1("Returning from RLST_insertTail() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int 
RLST_insertNext
(
  RLST_reflist_t*	inPtrRefList,
  void*			inPtrPosElem,
  void*			inPtrElement
)
{
  RLST_reflist_t*	ptrRefList;

  void*			ptrExtElem;
  void*			ptrAuxVoid;
  
  RLST_element_t*	ptrElement;
  RLST_element_t*	ptrPosElem;

  int			ret = RLST_RC_OK;
  
  TRAZA3("Entering in RLST_insertNext(%p, %p, %p)", 
          inPtrRefList,
          inPtrPosElem,
	  inPtrElement);

/*----------*/

  ptrRefList = inPtrRefList;

  ptrExtElem = inPtrElement;

  ptrAuxVoid = (char*)(ptrExtElem) + ptrRefList->offs;

  ptrElement = (RLST_element_t*)(ptrAuxVoid);

  ptrExtElem = inPtrPosElem;

  ptrAuxVoid = (char*)(ptrExtElem) + ptrRefList->offs;

  ptrPosElem = (RLST_element_t*)(ptrAuxVoid);

/*----------*/

  if(ptrPosElem->list != ptrRefList)
  {
    SUCESO0("ERROR: The position element belongs to another list!");
    
    ret = RLST_RC_ERROR;
  }

  else if(ptrElement->list != NULL)
  {
    SUCESO0("ERROR: The insert element already belongs to a list!");
    
    ret = RLST_RC_ERROR;
  }

  else
  {
    ptrElement->list = ptrRefList;
    ptrElement->elem = ptrExtElem;
    ptrElement->prev = ptrPosElem;
    ptrElement->next = ptrPosElem->next;

    if(ptrElement->next != NULL)
    {
      ptrElement->next->prev = ptrElement;
    }

    if(ptrRefList->tail == ptrPosElem)
    {
      ptrRefList->tail = ptrElement;
    }

    ptrRefList->nume++;
    
    ret = RLST_RC_OK;
  }

/*----------*/

  TRAZA1("Returning from RLST_insertNext() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int 
RLST_insertPrev
(
  RLST_reflist_t*	inPtrRefList,
  void*			inPtrPosElem,
  void*			inPtrElement
)
{
  RLST_reflist_t*	ptrRefList;

  void*			ptrExtElem;
  void*			ptrAuxVoid;
  
  RLST_element_t*	ptrElement;
  RLST_element_t*	ptrPosElem;

  int			ret = RLST_RC_OK;
  
  TRAZA3("Entering in RLST_insertPrev(%p, %p, %p)", 
          inPtrRefList,
          inPtrPosElem,
	  inPtrElement);

/*----------*/

  ptrRefList = inPtrRefList;

  ptrExtElem = inPtrElement;

  ptrAuxVoid = (char*)(ptrExtElem) + ptrRefList->offs;

  ptrElement = (RLST_element_t*)(ptrAuxVoid);

  ptrExtElem = inPtrPosElem;

  ptrAuxVoid = (char*)(ptrExtElem) + ptrRefList->offs;

  ptrPosElem = (RLST_element_t*)(ptrAuxVoid);

/*----------*/

  if(ptrPosElem->list != ptrRefList)
  {
    SUCESO0("ERROR: The position element belongs to another list!");
    
    ret = RLST_RC_ERROR;
  }

  else if(ptrElement->list != NULL)
  {
    SUCESO0("ERROR: The insert element already belongs to a list!");
    
    ret = RLST_RC_ERROR;
  }

  else
  {
    ptrElement->list = ptrRefList;
    ptrElement->elem = ptrExtElem;
    ptrElement->prev = ptrPosElem->prev;
    ptrElement->next = ptrPosElem;

    if(ptrElement->prev != NULL)
    {
      ptrElement->prev->next = ptrElement;
    }

    if(ptrRefList->head == ptrPosElem)
    {
      ptrRefList->head = ptrElement;
    }

    ptrRefList->nume++;
    
    ret = RLST_RC_OK;
  }

/*----------*/

  TRAZA1("Returning from RLST_insertPrev() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void* 
RLST_extract(RLST_reflist_t* inPtrRefList, void* inPtrElement)
{
  void*			ptrExtElem;

  void*			ptrAuxVoid;
  
  RLST_element_t*	ptrElement;

  RLST_reflist_t*	ptrRefList;

  TRAZA2("Entering in RLST_extract(%p, %p)", inPtrRefList, inPtrElement);

/*----------*/

  ptrRefList = inPtrRefList;

  ptrAuxVoid = (char*)(inPtrElement) + inPtrRefList->offs;

  ptrElement = (RLST_element_t*)(ptrAuxVoid);

  ptrExtElem = NULL;
  
/*----------*/

  if(ptrElement->list == inPtrRefList)
  {
    ptrExtElem = ptrElement->elem;

    if(ptrElement->prev != NULL)
    {
      ptrElement->prev->next = ptrElement->next;
    }

    if(ptrElement->next != NULL)
    {
      ptrElement->next->prev = ptrElement->prev;
    }

    if(ptrRefList->head == ptrElement)
    {
      ptrRefList->head = ptrElement->next;
    }

    if(ptrRefList->tail == ptrElement)
    {
      ptrRefList->tail = ptrElement->prev;
    }

    if(ptrRefList->prev == ptrElement)
    {
      ptrRefList->prev = ptrElement->prev;
    }

    if(ptrRefList->next == ptrElement)
    {
      ptrRefList->next = ptrElement->next;
    }

    ptrRefList->nume--;
    
    ptrElement->list = NULL;
    ptrElement->elem = NULL;
    ptrElement->prev = NULL;
    ptrElement->next = NULL;
  }

  else if(ptrElement->list != NULL)
  {
    DEPURA0("AVISO: This element belongs to another list!");
    
    ptrExtElem = NULL;
  }

  else
  {
    DEPURA0("AVISO: This element does not belong to a list!");
    
    ptrExtElem = NULL;
  }

/*----------*/

  TRAZA1("Returning from RLST_extract() = %p", ptrExtElem);

  return ptrExtElem;
}

/*----------------------------------------------------------------------------*/

void* 
RLST_extractHead(RLST_reflist_t* inPtrRefList)
{
  void*			ptrExtElem;

  RLST_element_t*	ptrElement;

  RLST_reflist_t*	ptrRefList;

  TRAZA1("Entering in RLST_extractHead(%p)", inPtrRefList);

/*----------*/

  ptrRefList = inPtrRefList;

  ptrElement = inPtrRefList->head;

  ptrExtElem = NULL;
  
/*----------*/

  if(ptrElement != NULL)
  {
    ptrExtElem = RLST_extract(ptrRefList, ptrElement->elem);
  }

/*----------*/

  TRAZA1("Returning from RLST_extractHead() = %p", ptrExtElem);

  return ptrExtElem;
}

/*----------------------------------------------------------------------------*/

void* 
RLST_extractTail(RLST_reflist_t* inPtrRefList)
{
  void*			ptrExtElem;

  RLST_element_t*	ptrElement;

  RLST_reflist_t*	ptrRefList;

  TRAZA1("Entering in RLST_extractTail(%p)", inPtrRefList);

/*----------*/

  ptrRefList = inPtrRefList;

  ptrElement = inPtrRefList->tail;

  ptrExtElem = NULL;
  
/*----------*/

  if(ptrElement != NULL)
  {
    ptrExtElem = RLST_extract(ptrRefList, ptrElement->elem);
  }

/*----------*/

  TRAZA1("Returning from RLST_extractTail() = %p", ptrExtElem);

  return ptrExtElem;
}

/*----------------------------------------------------------------------------*/

void* 
RLST_getHead(RLST_reflist_t* inPtrRefList)
{
  void*			ptrExtElem;

  RLST_element_t*	ptrElement;

  RLST_reflist_t*	ptrRefList;

  TRAZA1("Entering in RLST_getHead(%p)", inPtrRefList);

/*----------*/

  ptrRefList = inPtrRefList;

  ptrElement = inPtrRefList->head;

  ptrExtElem = NULL;
  
/*----------*/

  if(ptrElement != NULL)
  {
    ptrExtElem = ptrElement->elem;
  }

/*----------*/

  TRAZA1("Returning from RLST_getHead() = %p", ptrExtElem);

  return ptrExtElem;
}

/*----------------------------------------------------------------------------*/

void* 
RLST_getTail(RLST_reflist_t* inPtrRefList)
{
  void*			ptrExtElem;

  RLST_element_t*	ptrElement;

  RLST_reflist_t*	ptrRefList;

  TRAZA1("Entering in RLST_getTail(%p)", inPtrRefList);

/*----------*/

  ptrRefList = inPtrRefList;

  ptrElement = inPtrRefList->tail;

  ptrExtElem = NULL;
  
/*----------*/

  if(ptrElement != NULL)
  {
    ptrExtElem = ptrElement->elem;
  }

/*----------*/

  TRAZA1("Returning from RLST_getTail() = %p", ptrExtElem);

  return ptrExtElem;
}

/*----------------------------------------------------------------------------*/

void 
RLST_resetGet(RLST_reflist_t* inPtrRefList, void* inPtrElement)
{
  RLST_reflist_t*	ptrRefList;

  void*			ptrAuxVoid;

  RLST_element_t*	ptrElement;

/*----------*/

  TRAZA2("Entering in RLST_resetGet(%p, %p)",
          inPtrRefList,
	  inPtrElement);

/*----------*/

  ptrRefList = inPtrRefList;

/*----------*/

  if(inPtrElement != NULL)
  {
    ptrAuxVoid = (char*)(inPtrElement) + ptrRefList->offs;

    ptrElement = (RLST_element_t*)(ptrAuxVoid);

    if(ptrElement->list == ptrRefList)
    {
      ptrRefList->next = ptrElement->next;
      ptrRefList->prev = ptrElement->prev;
    }

    else
    {
      SUCESO0("ERROR: This element does not belong to this list!");

      ptrRefList->next = ptrRefList->head;
      ptrRefList->prev = ptrRefList->tail;
    }
  }

/*----------*/

  else
  {
    ptrRefList->next = ptrRefList->head;
    ptrRefList->prev = ptrRefList->tail;
  }

/*----------*/

  TRAZA0("Returning from RLST_resetGet()");
}

/*----------------------------------------------------------------------------*/

void* 
RLST_getNext(RLST_reflist_t* inPtrRefList)
{
  void*			ptrExtElem;

  RLST_element_t*	ptrElement;

  RLST_reflist_t*	ptrRefList;

  TRAZA1("Entering in RLST_getNext(%p)", inPtrRefList);

/*----------*/

  ptrRefList = inPtrRefList;

  ptrElement = inPtrRefList->next;

  ptrExtElem = NULL;
  
/*----------*/

  if(ptrElement != NULL)
  {
    ptrExtElem = ptrElement->elem;

    ptrRefList->next = ptrElement->next;
  }

/*----------*/

  TRAZA1("Returning from RLST_getNext() = %p", ptrExtElem);

  return ptrExtElem;
}

/*----------------------------------------------------------------------------*/

void* 
RLST_getPrev(RLST_reflist_t* inPtrRefList)
{
  void*			ptrExtElem;

  RLST_element_t*	ptrElement;

  RLST_reflist_t*	ptrRefList;

  TRAZA1("Entering in RLST_getPrev(%p)", inPtrRefList);

/*----------*/

  ptrRefList = inPtrRefList;

  ptrElement = inPtrRefList->prev;

  ptrExtElem = NULL;
  
/*----------*/

  if(ptrElement != NULL)
  {
    ptrExtElem = ptrElement->elem;

    ptrRefList->prev = ptrElement->prev;
  }

/*----------*/

  TRAZA1("Returning from RLST_getPrev() = %p", ptrExtElem);

  return ptrExtElem;
}

/*----------------------------------------------------------------------------*/

long 
RLST_getNumElem(RLST_reflist_t* inPtrRefList)
{
  return inPtrRefList->nume;
}

/*----------------------------------------------------------------------------*/

int 
RLST_inList(RLST_reflist_t* inPtrRefList, void* inPtrElement)
{
  void*			ptrAuxVoid;
  
  RLST_element_t*	ptrElement;

  int			ret;
  
  TRAZA2("Entering in RLST_inList(%p, %p)", inPtrRefList, inPtrElement);

/*----------*/

  ptrAuxVoid = (char*)(inPtrElement) + inPtrRefList->offs;

  ptrElement = (RLST_element_t*)(ptrAuxVoid);

/*----------*/

  if(ptrElement->list == NULL)
  {
    ret = 0;
  }

  else if(ptrElement->list == inPtrRefList)
  {
    ret = 1;
  }

  else
  {
    ret = -1;
  }

/*----------*/

  TRAZA1("Returning from RLST_inList() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/


