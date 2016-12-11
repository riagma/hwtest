/*____________________________________________________________________________

  MODULO:	Arbol AVL auto-referenciado

  CATEGORIA:	UTILIB

  AUTOR:	Ricardo Aguado Mart¡n

  VERSION:	1.0

  FECHA:	2005	

______________________________________________________________________________

  DESCRIPCION:

  OBSERVACIONES: 
______________________________________________________________________________*/

#ifndef __REFTREE_H__
#define __REFTREE_H__

/*__INCLUDES DEL SISTEMA______________________________________________________*/

/*__INCLUDES DE LA BD_________________________________________________________*/

/*__INCLUDES DE LA APLICACION_________________________________________________*/

/*__CONSTANTES________________________________________________________________*/
  
//----------------

enum e_RFTR_RC
{
  RFTR_RC_ALREADY_EXISTS	= -3,
  RFTR_RC_NOT_FOUND		= -2,
  RFTR_RC_ERROR			= -1,
  RFTR_RC_OK			=  0
};

//----------------

/*__TIPOS_____________________________________________________________________*/

//----------------

typedef struct RFTR_element_tag		RFTR_element_t;
typedef struct RFTR_reftree_tag		RFTR_reftree_t;

//----------------

struct RFTR_element_tag
{
  RFTR_reftree_t*		tree;
  void*				elem;

  RFTR_element_t*		nodf;
  RFTR_element_t*		nodl;
  RFTR_element_t*		nodr;

  RFTR_element_t*		next;
  RFTR_element_t*		prev;

  long				bfac;

};

//----------------

struct RFTR_reftree_tag
{
  long				offs;

  RFTR_element_t*		root;

  RFTR_element_t*		maxi;
  RFTR_element_t*		mini;

  RFTR_element_t*		next;
  RFTR_element_t*		prev;

  int				(*cmpe)(void*, void*);
  void			        (*prnt)(void*, void*, void*, void*, long);

  long				nume;

};

//----------------

/*__VARIABLES EXTERNAS________________________________________________________*/

/*__VARIABLES GLOBALES________________________________________________________*/

/*__VARIABLES LOCALES_________________________________________________________*/

/*__FUNCIONES EXTERNAS________________________________________________________*/

/*__FUNCIONES PUBLICAS________________________________________________________*/

//----------------

RFTR_reftree_t* RFTR_createRefTree
(
  void*, 
  RFTR_element_t*,
  int (*)(void*, void*)
);

int RFTR_destroyRefTree(RFTR_reftree_t*);

int RFTR_initializeRefTree
(
  RFTR_reftree_t*, 
  void*, 
  RFTR_element_t*, 
  int (*)(void*, void*)
);

int RFTR_finalizeRefTree(RFTR_reftree_t*);

//----------------

int RFTR_insert(RFTR_reftree_t*, void*);

void* RFTR_extract(RFTR_reftree_t*, void*);

void* RFTR_extractMaxi(RFTR_reftree_t*);
void* RFTR_extractMini(RFTR_reftree_t*);
				 
void* RFTR_find
(
  RFTR_reftree_t*	inPtrRefTree,
  void*			inPtrElement,
  void**		outPtrLtElem,
  void**		outPtrGtElem
);

//----------------

void* RFTR_getMaxi(RFTR_reftree_t*);
void* RFTR_getMini(RFTR_reftree_t*);

void RFTR_resetGet(RFTR_reftree_t*, void*);

void* RFTR_getNext(RFTR_reftree_t*);
void* RFTR_getPrev(RFTR_reftree_t*);

long  RFTR_getNumElem(RFTR_reftree_t*);

//----------------

void RFTR_iterateInOrden(RFTR_reftree_t* , void (*)(void *));
void RFTR_iteratePreOrden(RFTR_reftree_t* , void (*)(void *));
void RFTR_iteratePostOrden(RFTR_reftree_t* , void (*)(void *));

void RFTR_printNodeFunction
(
  RFTR_reftree_t*	inPtrRefTree,
  void			(*cbFunction)(void *, void*, void*, void*, long)
);

//----------------

/*__FUNCIONES PRIVADAS________________________________________________________*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

#endif



