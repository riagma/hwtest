/*____________________________________________________________________________

  MODULO:	Arbol AVL auto-referenciado

  CATEGORIA:	UTILIB

  AUTOR:	Ricardo Aguado Mart'n

  VERSION:	1.0

  FECHA:	2005	

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

#include "reftree.h"

/*__CONSTANTES________________________________________________________________*/
  
/*__TIPOS_____________________________________________________________________*/

/*__VARIABLES EXTERNAS________________________________________________________*/

/*__VARIABLES GLOBALES________________________________________________________*/

/*__VARIABLES LOCALES_________________________________________________________*/

/*__FUNCIONES EXTERNAS________________________________________________________*/

/*__FUNCIONES PUBLICAS________________________________________________________*/

/*__FUNCIONES PRIVADAS________________________________________________________*/

static inline void balanceTree(RFTR_reftree_t*, RFTR_element_t*, int, int);

static RFTR_element_t* rotateDoubleRight(RFTR_reftree_t*, RFTR_element_t*);
static RFTR_element_t* rotateSimpleRight(RFTR_reftree_t*, RFTR_element_t*);

static RFTR_element_t* rotateDoubleLeft(RFTR_reftree_t*, RFTR_element_t*);
static RFTR_element_t* rotateSimpleLeft(RFTR_reftree_t*, RFTR_element_t*);

static void iterateInOrden(RFTR_element_t*, void (*)(void *));
static void iteratePreOrden(RFTR_element_t*, void (*)(void *));
static void iteratePostOrden(RFTR_element_t*, void (*)(void *));

static inline void exchangeNodes(RFTR_reftree_t*, RFTR_element_t*, RFTR_element_t*);

static inline void RFTR_printNode
(
  RFTR_reftree_t*	inPtrRefTree,
  RFTR_element_t*	inPtrElement
);

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

RFTR_reftree_t* 
RFTR_createRefTree
(
  void*			inPtrOrig, 
  RFTR_element_t*	inPtrOffs,
  int 			(*inCmpFunc)(void *, void*)
) 
{
  RFTR_reftree_t*	ptrRefTree;

  TRAZA0("Entering in RFTR_createRefTree()");

/*----------*/

  ptrRefTree = (RFTR_reftree_t*) malloc(sizeof(RFTR_reftree_t));

  if(ptrRefTree == NULL)
  {
    SUCESO2("ERROR: malloc() = %d (%s)", errno, strerror(errno)); return NULL;
  }

/*----------*/

  else
  {
    if((RFTR_initializeRefTree(ptrRefTree, inPtrOrig, inPtrOffs, inCmpFunc)) != RFTR_RC_OK)
    {
      SUCESO0("ERROR: RFTR_initializeRefTree()");
      
      free(ptrRefTree); ptrRefTree = NULL;
    }
  }
  
/*----------*/

  TRAZA0("Returning from RFTR_createRefTree()");

  return ptrRefTree;
}

/*----------------------------------------------------------------------------*/

int 
RFTR_destroyRefTree(RFTR_reftree_t* inPtrRefTree)
{
  int		ret = RFTR_RC_OK;
  
  TRAZA1("Entering in RFTR_destroyRefTree(%p)", inPtrRefTree);

/*----------*/

  if(inPtrRefTree != NULL) 
  {
    ret = RFTR_finalizeRefTree(inPtrRefTree);

    free(inPtrRefTree);
  }

  else
  {
    SUCESO0("ERROR: inPtrRefTree == NULL");
  }

/*----------*/

  TRAZA0("Returning from RFTR_destroyRefTree()");

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int 
RFTR_initializeRefTree
(
  RFTR_reftree_t*	inPtrRefTree,
  void*			inPtrOrig, 
  RFTR_element_t*	inPtrOffs,
  int 			(*inCmpFunc)(void *, void*)
) 
{
  RFTR_reftree_t*	ptrRefTree;

  TRAZA1("Entering in RFTR_initializeRefTree(%p)", inPtrRefTree);

/*----------*/

  ptrRefTree = inPtrRefTree;

/*----------*/

  memset(ptrRefTree, 0, sizeof(RFTR_reftree_t));

/*----------*/

  ptrRefTree->offs = (char*)(inPtrOffs) - (char*)(inPtrOrig);
  ptrRefTree->cmpe = inCmpFunc;

/*----------*/

  TRAZA0("Returning from RFTR_initializeRefTree()");

  return RFTR_RC_OK;
}

/*----------------------------------------------------------------------------*/

int 
RFTR_finalizeRefTree(RFTR_reftree_t* inPtrRefTree)
{
  int		ret = RFTR_RC_OK;
  
  TRAZA1("Entering in RFTR_finalizeRefTree(%p)", inPtrRefTree);

/*----------*/

  if(inPtrRefTree != NULL) 
  {
    while(RFTR_extractMini(inPtrRefTree) != NULL);
    
    memset(inPtrRefTree, 0, sizeof(RFTR_reftree_t));
  }

/*----------*/

  TRAZA0("Returning from RFTR_finalizeRefTree()");

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int 
RFTR_insert(RFTR_reftree_t* inPtrRefTree, void* inPtrElement)
{
  RFTR_reftree_t*	ptrRefTree;

  void*			ptrExtElem;
  
  void*			ptrAuxVoid;
  
  RFTR_element_t*	ptrElement;

  RFTR_element_t*	ptrCurrent;
  RFTR_element_t*	ptrFatherc = NULL;

  int			end = 0;

  int			cmp = 0;

  int			ret = RFTR_RC_OK;
  
  TRAZA2("Entering in RFTR_insert(%p, %p)", inPtrRefTree, inPtrElement);

/*----------*/

  ptrRefTree = inPtrRefTree;

  ptrExtElem = inPtrElement;

  ptrAuxVoid = (char*)(ptrExtElem) + ptrRefTree->offs;

  ptrElement = (RFTR_element_t*)(ptrAuxVoid);

/*----------*/

  if(ptrElement->tree == NULL)
  {
    ptrElement->tree = ptrRefTree;
    ptrElement->elem = ptrExtElem;
    
    ptrElement->nodl = NULL;
    ptrElement->nodr = NULL;
    ptrElement->next = NULL;
    ptrElement->prev = NULL;
    ptrElement->bfac = 0;

    ptrCurrent = ptrRefTree->root;

    if(ptrCurrent == NULL)
    {
      ptrRefTree->root = ptrElement;
      ptrElement->nodf = NULL;

      ptrRefTree->nume++;

      ptrRefTree->maxi = ptrElement;
      ptrRefTree->mini = ptrElement;

      ret = RFTR_RC_OK;
    }

/*----------*/

    else
    {
      while((ptrCurrent != NULL) && (!end))
      {
        ptrFatherc = ptrCurrent;

        cmp = ptrRefTree->cmpe(ptrExtElem, ptrCurrent->elem);

	if(cmp <  0) ptrCurrent = ptrCurrent->nodl;
        if(cmp == 0) end = 1;
	if(cmp >  0) ptrCurrent = ptrCurrent->nodr;
      }

      if(cmp == 0)
      {
        DEPURA0("ERROR: Element already exists");

	memset(ptrElement, 0, sizeof(RFTR_element_t)); 

	ret = RFTR_RC_ALREADY_EXISTS;
      }

/*----------*/

      else if(cmp < 0)
      {
	ptrFatherc->nodl = ptrElement;
        ptrElement->nodf = ptrFatherc;

	ptrRefTree->nume++;

	ptrElement->prev = ptrFatherc->prev;
	ptrElement->next = ptrFatherc;
	ptrFatherc->prev = ptrElement;

	if(ptrElement->prev != NULL)
	{
	  ptrElement->prev->next = ptrElement;
	}

	else
	{
	  ptrRefTree->mini = ptrElement;
	}

	balanceTree(ptrRefTree, ptrFatherc, 1, 1);

	ret = RFTR_RC_OK;
      }

/*----------*/

      else if(cmp > 0)
      {
	ptrFatherc->nodr = ptrElement;
        ptrElement->nodf = ptrFatherc;

	ptrRefTree->nume++;

	ptrElement->next = ptrFatherc->next;
	ptrElement->prev = ptrFatherc;
	ptrFatherc->next = ptrElement;

	if(ptrElement->next != NULL)
	{
	  ptrElement->next->prev = ptrElement;
	}

	else
	{
	  ptrRefTree->maxi = ptrElement;
	}

	balanceTree(ptrRefTree, ptrFatherc, 0, 1);

	ret = RFTR_RC_OK;
      }
    }
  }

/*----------*/

  else
  {
    SUCESO0("ERROR: This element belongs to another tree!");
    
    ret = RFTR_RC_ERROR;
  }

/*----------*/

  TRAZA1("Returning from RFTR_insert() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

void* 
RFTR_extract(RFTR_reftree_t* inPtrRefTree, void* inPtrElement)
{
  RFTR_reftree_t*	ptrRefTree;

  void*			ptrExtElem;
  
  void*			ptrAuxVoid;

  RFTR_element_t*	ptrElement;

  RFTR_element_t*	ptrCurrent;
  RFTR_element_t*	ptrFatherc;

  int			left = 0;

  TRAZA2("Entering in RFTR_extract(%p, %p)", inPtrRefTree, inPtrElement);

/*----------*/

  ptrRefTree = inPtrRefTree;

  ptrAuxVoid = (char*)(inPtrElement) + ptrRefTree->offs;

  ptrElement = (RFTR_element_t*)(ptrAuxVoid);

  ptrExtElem = NULL;

/*----------*/

  if(ptrElement->tree == ptrRefTree)
  {
    RFTR_printNode(ptrRefTree, ptrElement->elem);

    ptrExtElem = ptrElement->elem;
    
    ptrCurrent = ptrElement;

    if((ptrCurrent->nodl != NULL) || (ptrCurrent->nodr != NULL))
    {
      if(ptrCurrent->nodr != NULL)
      {
        ptrCurrent = ptrCurrent->nodr;

	while(ptrCurrent->nodl != NULL)
	{
	  ptrCurrent = ptrCurrent->nodl;
	}
      }

      else
      {
        ptrCurrent = ptrCurrent->nodl;

	while(ptrCurrent->nodr != NULL)
	{
	  ptrCurrent = ptrCurrent->nodr;
	}
      }

      exchangeNodes(ptrRefTree, ptrElement, ptrCurrent);
    }

/*----------*/

    ptrFatherc = ptrElement->nodf;
    ptrCurrent = ptrElement;

/*----------*/

    if((ptrCurrent->nodl == NULL) && (ptrCurrent->nodr == NULL))
    {
      if(ptrFatherc == NULL)
      {
        ptrRefTree->root = NULL;
      }

      else if(ptrFatherc->nodl == ptrCurrent)
      {
        ptrFatherc->nodl = NULL; left = 1;
      }

      else if(ptrFatherc->nodr == ptrCurrent)
      {
        ptrFatherc->nodr = NULL; left = 0;
      }

      else
      {
        SUCESO0("ERROR: Assertion failure"); exit(-1);
      }
    }

/*----------*/

    else if(ptrCurrent->nodl != NULL)
    {
      if(ptrFatherc == NULL)
      {
        ptrRefTree->root = ptrCurrent->nodl;
	ptrCurrent->nodl->nodf = NULL;
      }

      else if(ptrFatherc->nodl == ptrCurrent)
      {
        ptrFatherc->nodl = ptrCurrent->nodl;
	ptrCurrent->nodl->nodf = ptrFatherc; left = 1;
      }

      else if(ptrFatherc->nodr == ptrCurrent)
      {
        ptrFatherc->nodr = ptrCurrent->nodl;
	ptrCurrent->nodl->nodf = ptrFatherc; left = 0;
      }

      else
      {
        SUCESO0("ERROR: Assertion failure"); exit(-1);
      }
    }

/*----------*/

    else if(ptrCurrent->nodr != NULL)
    {
      if(ptrFatherc == NULL)
      {
        ptrRefTree->root = ptrCurrent->nodr;
	ptrCurrent->nodr->nodf = NULL;
      }

      else if(ptrFatherc->nodl == ptrCurrent)
      {
        ptrFatherc->nodl = ptrCurrent->nodr;
	ptrCurrent->nodr->nodf = ptrFatherc; left = 1;
      }

      else if(ptrFatherc->nodr == ptrCurrent)
      {
        ptrFatherc->nodr = ptrCurrent->nodr;
	ptrCurrent->nodr->nodf = ptrFatherc; left = 0;
      }

      else
      {
        SUCESO0("ERROR: Assertion failure"); exit(-1);
      }
    }

/*----------*/

    else
    {
      SUCESO0("ERROR: Assertion failure"); exit(-1);
    }

/*----------*/

    if(ptrFatherc != NULL) 
    {
      if((ptrFatherc->nodl == NULL) && (ptrFatherc->nodr == NULL))
      {
        ptrFatherc->bfac = 0;
      
        ptrCurrent = ptrFatherc;
        ptrFatherc = ptrCurrent->nodf;

        if(ptrFatherc != NULL) 
        {
  	  if(ptrFatherc->nodl == ptrCurrent) { left = 1; } else { left = 0; }
	}
      }
    }

    if(ptrFatherc != NULL) balanceTree(ptrRefTree, ptrFatherc, left, 0);

/*----------*/

    if((ptrElement->next != NULL) && (ptrElement->prev != NULL))
    {
      ptrElement->next->prev = ptrElement->prev;
      ptrElement->prev->next = ptrElement->next;
    }

    else if(ptrElement->next != NULL)
    {
      ptrElement->next->prev = NULL;
      ptrRefTree->mini = ptrElement->next;
    }

    else if(ptrElement->prev != NULL)
    {
      ptrElement->prev->next = NULL;
      ptrRefTree->maxi = ptrElement->prev;
    }

    else
    {
      ptrRefTree->maxi = NULL;
      ptrRefTree->mini = NULL;
      ptrRefTree->next = NULL;
      ptrRefTree->prev = NULL;
    }

    if(ptrRefTree->next == ptrElement) ptrRefTree->next = ptrElement->next;
    if(ptrRefTree->prev == ptrElement) ptrRefTree->prev = ptrElement->prev;
    
/*----------*/

    ptrElement->tree = NULL;
    ptrElement->elem = NULL;
    ptrElement->nodf = NULL;
    ptrElement->nodl = NULL;
    ptrElement->nodr = NULL;
    ptrElement->next = NULL;
    ptrElement->prev = NULL;
    ptrElement->bfac = 0;

    ptrRefTree->nume--;
  }

/*----------*/

  else if(ptrElement->tree != NULL)
  {
    DEPURA0("AVISO: This element belongs to another tree!");
    
    ptrExtElem = NULL;
  }

  else
  {
    DEPURA0("AVISO: This element does not belong to a tree!");
    
    ptrExtElem = NULL;
  }

/*----------*/

  TRAZA1("Returning from RFTR_extract() = %X", ptrExtElem);

  return ptrExtElem;
}

/*----------------------------------------------------------------------------*/

void* 
RFTR_extractMini(RFTR_reftree_t* inPtrRefTree)
{
  RFTR_reftree_t*	ptrRefTree;

  void*			ptrExtElem;
  
  RFTR_element_t*	ptrElement;

  TRAZA1("Entering in RFTR_extractMini(%p)", inPtrRefTree);

/*----------*/

  ptrRefTree = inPtrRefTree;

  ptrExtElem = NULL;

  ptrElement = ptrRefTree->mini;

/*----------*/

  if(ptrElement != NULL)
  {
    ptrExtElem = RFTR_extract(ptrRefTree, ptrElement->elem);
  }

/*----------*/

  TRAZA1("Returning from RFTR_extractMini() = %p", ptrExtElem);

  return ptrExtElem;
}

/*----------------------------------------------------------------------------*/

void* 
RFTR_extractMaxi(RFTR_reftree_t* inPtrRefTree)
{
  RFTR_reftree_t*	ptrRefTree;

  void*			ptrExtElem;
  
  RFTR_element_t*	ptrElement;

  TRAZA1("Entering in RFTR_extractMaxi(%p)", inPtrRefTree);

/*----------*/

  ptrRefTree = inPtrRefTree;

  ptrExtElem = NULL;

  ptrElement = ptrRefTree->maxi;

/*----------*/

  if(ptrElement != NULL)
  {
    ptrExtElem = RFTR_extract(ptrRefTree, ptrElement->elem);
  }

/*----------*/

  TRAZA1("Returning from RFTR_extractMaxi() = %p", ptrExtElem);

  return ptrExtElem;
}

/*----------------------------------------------------------------------------*/

long 
RFTR_getNumElem(RFTR_reftree_t* inPtrRefTree)
{
  return inPtrRefTree->nume;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void* 
RFTR_find
(
  RFTR_reftree_t*	inPtrRefTree,
  void*			inPtrElement,
  void**		outPtrLtElem,
  void**		outPtrGtElem
)
{
  RFTR_reftree_t*	ptrRefTree;

  void*			ptrExtElem;
  
  RFTR_element_t*	ptrCurrent;

  RFTR_element_t*	ptrLtElem = NULL;
  RFTR_element_t*	ptrGtElem = NULL;

  int			end = 0;

  int			cmp = 0;

  TRAZA1("Entering in RFTR_find(%p)", inPtrRefTree);

/*----------*/

  ptrRefTree = inPtrRefTree;

  ptrExtElem = NULL;

  ptrCurrent = ptrRefTree->root;

/*----------*/

  while((ptrCurrent != NULL) && (!end))
  {
    cmp = ptrRefTree->cmpe(inPtrElement, ptrCurrent->elem);

    if(cmp < 0) 
    {
      ptrGtElem = ptrCurrent; 

      ptrCurrent = ptrCurrent->nodl;
    }

    else if(cmp > 0) 
    {
      ptrLtElem = ptrCurrent; 

      ptrCurrent = ptrCurrent->nodr; 
    }

    else 
    {
      ptrGtElem = ptrCurrent->next; 
      ptrLtElem = ptrCurrent->prev; 

      ptrExtElem = ptrCurrent->elem; end = 1;
    }
  }

/*----------*/

  ptrRefTree->next = ptrGtElem;
  ptrRefTree->prev = ptrLtElem;

/*----------*/

  if(outPtrLtElem != NULL)
  {
    *outPtrLtElem = ptrLtElem != NULL ? ptrLtElem->elem : NULL;
  }

  if(outPtrGtElem != NULL)
  {
    *outPtrGtElem = ptrGtElem != NULL ? ptrGtElem->elem : NULL;
  }

/*----------*/

  TRAZA1("Returning from RFTR_find() = %p", ptrExtElem);

  return ptrExtElem;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void* 
RFTR_getMaxi(RFTR_reftree_t* inPtrRefTree)
{
  RFTR_reftree_t*	ptrRefTree;

  void*			ptrExtElem;
  
  RFTR_element_t*	ptrElement;

  TRAZA1("Entering in RFTR_getMaxi(%p)", inPtrRefTree);

/*----------*/

  ptrRefTree = inPtrRefTree;

  ptrExtElem = NULL;

  ptrElement = ptrRefTree->maxi;

/*----------*/

  if(ptrElement != NULL)
  {
    ptrExtElem = ptrElement->elem;
  }

/*----------*/

  TRAZA1("Returning from RFTR_getMaxi() = %p", ptrExtElem);

  return ptrExtElem;
}

/*----------------------------------------------------------------------------*/

void* 
RFTR_getMini(RFTR_reftree_t* inPtrRefTree)
{
  RFTR_reftree_t*	ptrRefTree;

  void*			ptrExtElem;
  
  RFTR_element_t*	ptrElement;

  TRAZA1("Entering in RFTR_getMini(%p)", inPtrRefTree);

/*----------*/

  ptrRefTree = inPtrRefTree;

  ptrExtElem = NULL;

  ptrElement = ptrRefTree->mini;

/*----------*/

  if(ptrElement != NULL)
  {
    ptrExtElem = ptrElement->elem;
  }

/*----------*/

  TRAZA1("Returning from RFTR_getMini() = %p", ptrExtElem);

  return ptrExtElem;
}

/*----------------------------------------------------------------------------*/

void 
RFTR_resetGet(RFTR_reftree_t* inPtrRefTree, void* inPtrElement)
{
  RFTR_reftree_t*	ptrRefTree;

  void*			ptrAuxVoid;

  RFTR_element_t*	ptrElement;

/*----------*/

  TRAZA2("Entering in RFTR_resetGet(%p, %p)",
          inPtrRefTree,
	  inPtrElement);

/*----------*/

  ptrRefTree = inPtrRefTree;

  if(inPtrElement != NULL)
  {
    ptrAuxVoid = (char*)(inPtrElement) + ptrRefTree->offs;

    ptrElement = (RFTR_element_t*)(ptrAuxVoid);

    if(ptrElement->tree == ptrRefTree)
    {
      ptrRefTree->next = ptrElement->next;
      ptrRefTree->prev = ptrElement->prev;
    }

    else
    {
      SUCESO0("ERROR: This element does not belong to this tree!");

      ptrRefTree->next = ptrRefTree->mini;
      ptrRefTree->prev = ptrRefTree->maxi;
    }
  }

  else
  {
    ptrRefTree->next = ptrRefTree->mini;
    ptrRefTree->prev = ptrRefTree->maxi;
  }

/*----------*/

  TRAZA0("Returning from RFTR_resetGet()");
}

/*----------------------------------------------------------------------------*/

void* 
RFTR_getNext(RFTR_reftree_t* inPtrRefTree)
{
  RFTR_reftree_t*	ptrRefTree;

  void*			ptrExtElem;
  
  RFTR_element_t*	ptrElement;

  TRAZA1("Entering in RFTR_getNext(%p)", inPtrRefTree);

/*----------*/

  ptrRefTree = inPtrRefTree;

  ptrExtElem = NULL;

  ptrElement = ptrRefTree->next;

/*----------*/

  if(ptrElement != NULL)
  {
    ptrExtElem = ptrElement->elem;

    ptrRefTree->next = ptrElement->next;
  }

/*----------*/

  TRAZA1("Returning from RFTR_getNext() = %p", ptrExtElem);

  return ptrExtElem;
}

/*----------------------------------------------------------------------------*/

void* 
RFTR_getPrev(RFTR_reftree_t* inPtrRefTree)
{
  RFTR_reftree_t*	ptrRefTree;

  void*			ptrExtElem;
  
  RFTR_element_t*	ptrElement;

  TRAZA1("Entering in RFTR_getPrev(%p)", inPtrRefTree);

/*----------*/

  ptrRefTree = inPtrRefTree;

  ptrExtElem = NULL;

  ptrElement = ptrRefTree->prev;

/*----------*/

  if(ptrElement != NULL)
  {
    ptrExtElem = ptrElement->elem;

    ptrRefTree->prev = ptrElement->prev;
  }

/*----------*/

  TRAZA1("Returning from RFTR_getPrev() = %p", ptrExtElem);

  return ptrExtElem;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void 
RFTR_iterateInOrden(RFTR_reftree_t* inPtrRefTree, void (*cbFunction)(void *))
{
  if(inPtrRefTree->root != NULL) iterateInOrden(inPtrRefTree->root, cbFunction);
}

/*----------------------------------------------------------------------------*/

void 
RFTR_iteratePreOrden(RFTR_reftree_t* inPtrRefTree, void (*cbFunction)(void *))
{
  if(inPtrRefTree->root != NULL) iteratePreOrden(inPtrRefTree->root, cbFunction);
}

/*----------------------------------------------------------------------------*/

void 
RFTR_iteratePostOrden(RFTR_reftree_t* inPtrRefTree, void (*cbFunction)(void *))
{
  if(inPtrRefTree->root != NULL) iteratePostOrden(inPtrRefTree->root, cbFunction);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void 
RFTR_printNodeFunction
(
  RFTR_reftree_t*	inPtrRefTree,
  void			(*cbFunction)(void *, void*, void*, void*, long)
)
{

/*----------*/

  inPtrRefTree->prnt = cbFunction;

/*----------*/

}

/*----------------------------------------------------------------------------*/

static inline void 
RFTR_printNode
(
  RFTR_reftree_t*	inPtrRefTree,
  RFTR_element_t*	inPtrElement
)
{
  RFTR_reftree_t*	ptrRefTree;

  void*			ptrExtElem;
  
  void*			ptrAuxVoid;

  RFTR_element_t*	ptrElement;
  
/*----------*/

  ptrRefTree = inPtrRefTree;

  ptrExtElem = inPtrElement;

  ptrAuxVoid = (char*)(ptrExtElem) + ptrRefTree->offs;

  ptrElement = (RFTR_element_t*)(ptrAuxVoid);

/*----------*/

  if(ptrElement->tree == ptrRefTree) if(ptrRefTree->prnt)
  {
    ptrRefTree->prnt(ptrElement->elem,
	             ptrElement->nodf ? ptrElement->nodf->elem : NULL,
	             ptrElement->nodl ? ptrElement->nodl->elem : NULL,
	             ptrElement->nodr ? ptrElement->nodr->elem : NULL,
	             ptrElement->bfac);
  }
  
/*----------*/

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static inline void 
balanceTree
(
  RFTR_reftree_t*	inPtrRefTree, 
  RFTR_element_t*	inPtrElement,
  int			inLeft,
  int			inNew
)
{
  RFTR_reftree_t*	ptrRefTree;

  RFTR_element_t*	ptrElement;

  long			bfc1;
  long			bfc2;

  int			left = inLeft;

  int			end = 0;

//TRAZA1("Entering in balanceTree(%p)", inPtrRefTree);

/*----------*/

  ptrRefTree = inPtrRefTree;

  ptrElement = inPtrElement;

/*----------*/

  while((ptrElement != NULL) && (!end))
  {
    RFTR_printNode(ptrRefTree, ptrElement->elem);

    bfc1 = ptrElement->bfac;

    if(inNew)
    {
      if(left) { ptrElement->bfac--; } else { ptrElement->bfac++; }
    }

    else
    {
      if(left) { ptrElement->bfac++; } else { ptrElement->bfac--; }
    }

    bfc2 = ptrElement->bfac;

    RFTR_printNode(ptrRefTree, ptrElement->elem);

/*----------*/

    if((bfc1 ==  0) && (bfc2 == -1))
    {
      if(inNew) {end = 0;} else {end = 1;}
    }

    else 

/*----------*/

    if((bfc1 ==  0) && (bfc2 ==  1))
    {
      if(inNew) {end = 0;} else {end = 1;}
    }

    else 

/*----------*/

    if((bfc1 == -1) && (bfc2 ==  0))
    {
      if(inNew) {end = 1;} else {end = 0;}
    }

    else 

/*----------*/

    if((bfc1 ==  1) && (bfc2 ==  0))
    {
      if(inNew) {end = 1;} else {end = 0;}
    }

    else 

/*----------*/

    if((bfc1 == -1) && (bfc2 == -2))
    {
      if(ptrElement->nodl->bfac == -1)
      {
        ptrElement = rotateSimpleRight(ptrRefTree, ptrElement);

	if(inNew) {end = 1;} else {end = 0;}
      }

      else

      if(ptrElement->nodl->bfac ==  0)
      {
        ptrElement = rotateSimpleRight(ptrRefTree, ptrElement);

	end = 1;
      }

      else

      if(ptrElement->nodl->bfac ==  1)
      {
        ptrElement = rotateDoubleRight(ptrRefTree, ptrElement);

	if(inNew) {end = 1;} else {end = 0;}
      }
    }

    else 

/*----------*/

    if((bfc1 ==  1) && (bfc2 ==  2))
    {
      if(ptrElement->nodr->bfac == -1)
      {
        ptrElement = rotateDoubleLeft(ptrRefTree, ptrElement);

	if(inNew) {end = 1;} else {end = 0;}
      }

      else

      if(ptrElement->nodr->bfac ==  0)
      {
        ptrElement = rotateSimpleLeft(ptrRefTree, ptrElement);

	end = 1;
      }

      else

      if(ptrElement->nodr->bfac ==  1)
      {
        ptrElement = rotateSimpleLeft(ptrRefTree, ptrElement);

	if(inNew) {end = 1;} else {end = 0;}
      }
    }

/*----------*/

    if(!end)
    {
      if(ptrElement->nodf != NULL)
      {
        left = ptrElement->nodf->nodl == ptrElement ? 1 : 0;
      }

      ptrElement = ptrElement->nodf;
    }
  }

/*----------*/

//TRAZA0("Returning from balanceTree()");
}

/*----------------------------------------------------------------------------*/

static RFTR_element_t* 
rotateSimpleLeft
(
  RFTR_reftree_t*	inPtrRefTree, 
  RFTR_element_t*	inPtrElement
)
{
  RFTR_element_t*	ptrF = inPtrElement->nodf;

  RFTR_element_t*	ptrP = inPtrElement;
  
  RFTR_element_t*	ptrQ = ptrP->nodr;
  RFTR_element_t*	ptrB = ptrQ->nodl;
  
  TRAZA1("Entering in rotateSimpleLeft(%p)", inPtrRefTree);

/*----------*/

  if(ptrF != NULL)
  {
    if(ptrF->nodl == ptrP) { ptrF->nodl = ptrQ; } else { ptrF->nodr = ptrQ; }
  }

  else
  {
    inPtrRefTree->root = ptrQ;
  }

/*----------*/

  ptrP->nodf = ptrQ;
  ptrP->nodr = ptrB;

  ptrQ->nodf = ptrF;
  ptrQ->nodl = ptrP;

  if(ptrB) ptrB->nodf = ptrP;

/*----------*/

  ptrP->bfac = ptrP->bfac - 1 - (ptrQ->bfac > 0 ? ptrQ->bfac : 0);
  ptrQ->bfac = ptrQ->bfac - 1 + (ptrP->bfac < 0 ? ptrP->bfac : 0);

/*----------*/

  TRAZA1("Returning from rotateSimpleLeft() = %p", ptrQ);

  return ptrQ;
}

/*----------------------------------------------------------------------------*/

static RFTR_element_t* 
rotateSimpleRight
(
  RFTR_reftree_t*	inPtrRefTree, 
  RFTR_element_t*	inPtrElement
)
{
  RFTR_element_t*	ptrF = inPtrElement->nodf;

  RFTR_element_t*	ptrP = inPtrElement;
  
  RFTR_element_t*	ptrQ = ptrP->nodl;
  RFTR_element_t*	ptrB = ptrQ->nodr;
  
  TRAZA1("Entering in rotateSimpleRight(%p)", inPtrRefTree);

/*----------*/

  if(ptrF != NULL)
  {
    if(ptrF->nodl == ptrP) { ptrF->nodl = ptrQ; } else { ptrF->nodr = ptrQ; }
  }

  else
  {
    inPtrRefTree->root = ptrQ;
  }

/*----------*/

  ptrP->nodf = ptrQ;
  ptrP->nodl = ptrB;

  ptrQ->nodf = ptrF;
  ptrQ->nodr = ptrP;

  if(ptrB) ptrB->nodf = ptrP;

/*----------*/

  ptrP->bfac = ptrP->bfac + 1 - (ptrQ->bfac < 0 ? ptrQ->bfac : 0);
  ptrQ->bfac = ptrQ->bfac + 1 + (ptrP->bfac > 0 ? ptrP->bfac : 0);;

/*----------*/

  TRAZA1("Returning from rotateSimpleRight() = %p", ptrQ);

  return ptrQ;
}

/*----------------------------------------------------------------------------*/

static RFTR_element_t* 
rotateDoubleLeft
(
  RFTR_reftree_t*	inPtrRefTree, 
  RFTR_element_t*	inPtrElement
)
{
  RFTR_element_t*	ptrElement;

  TRAZA1("Entering in rotateDoubleLeft(%p)", inPtrRefTree);

/*----------*/

  ptrElement = rotateSimpleRight(inPtrRefTree, inPtrElement->nodr);
  
  ptrElement = rotateSimpleLeft(inPtrRefTree, inPtrElement);

/*----------*/

  TRAZA1("Returning from rotateDoubleLeft() = %p", ptrElement);

  return ptrElement;
}

/*----------------------------------------------------------------------------*/

static RFTR_element_t* 
rotateDoubleRight
(
  RFTR_reftree_t*	inPtrRefTree, 
  RFTR_element_t*	inPtrElement
)
{
  RFTR_element_t*	ptrElement;

  TRAZA1("Entering in rotateDoubleRight(%p)", inPtrRefTree);

/*----------*/

  ptrElement = rotateSimpleLeft(inPtrRefTree, inPtrElement->nodl);
  
  ptrElement = rotateSimpleRight(inPtrRefTree, inPtrElement);

/*----------*/

  TRAZA1("Returning from rotateDoubleRight() = %p", ptrElement);

  return ptrElement;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static inline void 
exchangeNodes
(
  RFTR_reftree_t*	inPtrRefTree,
  RFTR_element_t*	inPtrElemen1,
  RFTR_element_t*	inPtrElemen2
)
{
  RFTR_reftree_t*	ptrRefTree = inPtrRefTree;

  RFTR_element_t*	ptrElemen1 = inPtrElemen1;
  RFTR_element_t*	ptrElemen2 = inPtrElemen2;
  
  RFTR_element_t	auxEle1;
  RFTR_element_t	auxEle2;
  
  RFTR_element_t*	ptrNewEle1 = &auxEle1;
  RFTR_element_t*	ptrNewEle2 = &auxEle2;

/*----------*/
/*
  TRAZA3("Entering in exchangeNodes(%p, %p, %p)", 
          inPtrRefTree,
	  inPtrElemen1,
	  inPtrElemen2);
*/
/*----------*/

  RFTR_printNode(ptrRefTree, ptrElemen1->elem);
  RFTR_printNode(ptrRefTree, ptrElemen2->elem);

/*----------*/

  ptrNewEle1->nodf = ptrElemen2->nodf;
  ptrNewEle1->nodl = ptrElemen2->nodl;
  ptrNewEle1->nodr = ptrElemen2->nodr;
  ptrNewEle1->bfac = ptrElemen2->bfac;

  ptrNewEle2->nodf = ptrElemen1->nodf;
  ptrNewEle2->nodl = ptrElemen1->nodl;
  ptrNewEle2->nodr = ptrElemen1->nodr;
  ptrNewEle2->bfac = ptrElemen1->bfac;

/*----------*/

  if(ptrNewEle1->nodf == NULL)
  {
    ptrRefTree->root = ptrElemen1;
  }

  else if(ptrNewEle1->nodf == ptrElemen1)
  {
    ptrNewEle1->nodf = ptrElemen2;
  }

  else
  {
    if(ptrNewEle1->nodf->nodl == ptrElemen2)
    {
      ptrNewEle1->nodf->nodl = ptrElemen1;
    }

    else
    {
      ptrNewEle1->nodf->nodr = ptrElemen1;
    }
  }

  if(ptrNewEle1->nodl != NULL)
  {
    if(ptrNewEle1->nodl == ptrElemen1)
    {
      ptrNewEle1->nodl = ptrElemen2;
    }

    else
    {
      ptrNewEle1->nodl->nodf = ptrElemen1;
    }
  }

  if(ptrNewEle1->nodr != NULL)
  {
    if(ptrNewEle1->nodr == ptrElemen1)
    {
      ptrNewEle1->nodr = ptrElemen2;
    }

    else
    {
      ptrNewEle1->nodr->nodf = ptrElemen1;
    }
  }

/*----------*/

  if(ptrNewEle2->nodf == NULL)
  {
    ptrRefTree->root = ptrElemen2;
  }

  else if(ptrNewEle2->nodf == ptrElemen2)
  {
    ptrNewEle2->nodf = ptrElemen1;
  }
  
  else
  {
    if(ptrNewEle2->nodf->nodl == ptrElemen1)
    {
      ptrNewEle2->nodf->nodl = ptrElemen2;
    }

    else
    {
      ptrNewEle2->nodf->nodr = ptrElemen2;
    }
  }

  if(ptrNewEle2->nodl != NULL)
  {
    if(ptrNewEle2->nodl == ptrElemen2)
    {
      ptrNewEle2->nodl = ptrElemen1;
    }

    else
    {
      ptrNewEle2->nodl->nodf = ptrElemen2;
    }
  }

  if(ptrNewEle2->nodr != NULL)
  {
    if(ptrNewEle2->nodr == ptrElemen2)
    {
      ptrNewEle2->nodr = ptrElemen1;
    }

    else
    {
      ptrNewEle2->nodr->nodf = ptrElemen2;
    }
  }

/*----------*/

  ptrElemen1->nodf = ptrNewEle1->nodf;
  ptrElemen1->nodl = ptrNewEle1->nodl;
  ptrElemen1->nodr = ptrNewEle1->nodr;
  ptrElemen1->bfac = ptrNewEle1->bfac;

  ptrElemen2->nodf = ptrNewEle2->nodf;
  ptrElemen2->nodl = ptrNewEle2->nodl;
  ptrElemen2->nodr = ptrNewEle2->nodr;
  ptrElemen2->bfac = ptrNewEle2->bfac;

/*----------*/

  RFTR_printNode(ptrRefTree, ptrElemen1->elem);
  RFTR_printNode(ptrRefTree, ptrElemen2->elem);
  
/*----------*/

//TRAZA0("Returning from exchangeNodes()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static void 
iterateInOrden(RFTR_element_t* inPtrElement, void (*cbFunction)(void *))
{
  if(inPtrElement->nodl != NULL) iterateInOrden(inPtrElement->nodl, cbFunction);
  
  cbFunction(inPtrElement->elem);

  if(inPtrElement->nodr != NULL) iterateInOrden(inPtrElement->nodr, cbFunction);
}

/*----------------------------------------------------------------------------*/

static void 
iteratePreOrden(RFTR_element_t* inPtrElement, void (*cbFunction)(void *))
{
  cbFunction(inPtrElement->elem);

  if(inPtrElement->nodl != NULL) iteratePreOrden(inPtrElement->nodl, cbFunction);
  
  if(inPtrElement->nodr != NULL) iteratePreOrden(inPtrElement->nodr, cbFunction);
}

/*----------------------------------------------------------------------------*/

static void 
iteratePostOrden(RFTR_element_t* inPtrElement, void (*cbFunction)(void *))
{
  if(inPtrElement->nodl != NULL) iteratePostOrden(inPtrElement->nodl, cbFunction);

  if(inPtrElement->nodr != NULL) iteratePostOrden(inPtrElement->nodr, cbFunction);
  
  cbFunction(inPtrElement->elem);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/


