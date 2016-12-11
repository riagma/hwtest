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

#ifndef __REFLIST_H__
#define __REFLIST_H__

/*__INCLUDES DEL SISTEMA______________________________________________________*/

/*__INCLUDES DE LA BD_________________________________________________________*/

/*__INCLUDES DE LA APLICACION_________________________________________________*/

/*__CONSTANTES________________________________________________________________*/
  
//----------------

enum e_RLST_RC
{
  RLST_RC_NOT_FOUND		= -2,
  RLST_RC_ERROR			= -1,
  RLST_RC_OK			=  0
};

//----------------

/*__TIPOS_____________________________________________________________________*/

//----------------

typedef struct RLST_element_tag		RLST_element_t;
typedef struct RLST_reflist_tag		RLST_reflist_t;

//----------------

struct RLST_element_tag
{
  RLST_reflist_t*		list;
  void*				elem;

  RLST_element_t*		next;
  RLST_element_t*		prev;

};

//----------------

struct RLST_reflist_tag
{
  long				offs;

  RLST_element_t*		head;
  RLST_element_t*		tail;
  RLST_element_t*		next;
  RLST_element_t*		prev;

  long				nume;

};

//----------------

/*__VARIABLES EXTERNAS________________________________________________________*/

/*__VARIABLES GLOBALES________________________________________________________*/

/*__VARIABLES LOCALES_________________________________________________________*/

/*__FUNCIONES EXTERNAS________________________________________________________*/

/*__FUNCIONES PUBLICAS________________________________________________________*/

//----------------

RLST_reflist_t* RLST_createRefList(void*, RLST_element_t*);
int RLST_destroyRefList(RLST_reflist_t*);

int RLST_initializeRefList(RLST_reflist_t*, void*, RLST_element_t*);
int RLST_finalizeRefList(RLST_reflist_t*);

int RLST_insertHead(RLST_reflist_t*, void*);
int RLST_insertTail(RLST_reflist_t*, void*);
				 
int RLST_insertNext(RLST_reflist_t*, void*, void*);
int RLST_insertPrev(RLST_reflist_t*, void*, void*);
				 
void* RLST_extract(RLST_reflist_t*, void*);

void* RLST_extractHead(RLST_reflist_t*);
void* RLST_extractTail(RLST_reflist_t*);
				 
void* RLST_getHead(RLST_reflist_t*);
void* RLST_getTail(RLST_reflist_t*);
				 
void RLST_resetGet(RLST_reflist_t*, void*);

void* RLST_getNext(RLST_reflist_t*);
void* RLST_getPrev(RLST_reflist_t*);

long  RLST_getNumElem(RLST_reflist_t*);

int RLST_inList(RLST_reflist_t*, void*);

//----------------

/*__FUNCIONES PRIVADAS________________________________________________________*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

#endif



