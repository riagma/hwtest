/*____________________________________________________________________________

  MODULO:	Lista de memoria auto-referenciada

  CATEGORIA:	UTILIB

  AUTOR:	Ricardo Aguado Mart'n

  VERSION:	1.0

  FECHA:	2005	

______________________________________________________________________________

  DESCRIPCION:

  OBSERVACIONES: 
______________________________________________________________________________*/

#ifndef __REFMEMO_H__
#define __REFMEMO_H__

/*__INCLUDES DEL SISTEMA______________________________________________________*/

/*__INCLUDES DE LA BD_________________________________________________________*/

/*__INCLUDES DE LA APLICACION_________________________________________________*/

#include "reflist.h"

/*__CONSTANTES________________________________________________________________*/
  
//----------------

enum e_MEMO_RC
{
  MEMO_RC_NOT_FOUND	= -2,
  MEMO_RC_ERROR		= -1,
  MEMO_RC_OK		=  0
};

//----------------

/*__TIPOS_____________________________________________________________________*/

//----------------

typedef struct MEMO_element_tag		MEMO_element_t;
typedef struct MEMO_memelem_tag		MEMO_memelem_t;
typedef struct MEMO_refmemo_tag		MEMO_refmemo_t;

//----------------

struct MEMO_element_tag
{
  MEMO_memelem_t*		meme;
  long				offs;
};

//----------------

struct MEMO_memelem_tag
{
  MEMO_refmemo_t*		rmem;
  
  void*				memo;
  long*				newe;
  long				newp;
  long				mrkp;

  RLST_element_t		list;
  RLST_element_t		newl;
};

//----------------

struct MEMO_refmemo_tag
{
  long				offs;
  long				size;
  long				blen;
  long				bmin;
  long				free;
  long				fmin;
  
  RLST_reflist_t		list;
  RLST_reflist_t		newl;
};

//----------------

/*__VARIABLES EXTERNAS________________________________________________________*/

/*__VARIABLES GLOBALES________________________________________________________*/

/*__VARIABLES LOCALES_________________________________________________________*/

/*__FUNCIONES EXTERNAS________________________________________________________*/

/*__FUNCIONES PUBLICAS________________________________________________________*/

//----------------

MEMO_refmemo_t* MEMO_createRefMemo
(
  void*			inPtrOrig, 
  MEMO_element_t*	inPtrOffs,
  long			inElemSize,
  long			inBlockLen,
  long			inBlockMin
);

int MEMO_destroyRefMemo(MEMO_refmemo_t*);

int MEMO_initializeRefMemo
(
  MEMO_refmemo_t*	inPtrRefMemo,
  void*			inPtrOrig, 
  MEMO_element_t*	inPtrOffs,
  long			inElemSize,
  long			inBlockLen,
  long			inBlockMin
);

int MEMO_finalizeRefMemo(MEMO_refmemo_t*);

void* MEMO_new(MEMO_refmemo_t*);

void* MEMO_new_no_ini(MEMO_refmemo_t*);

int MEMO_delete(MEMO_refmemo_t*, void*);

//----------------

/*__FUNCIONES PRIVADAS________________________________________________________*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

#endif



