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

#ifndef __BUFFER_H__
#define __BUFFER_H__

/*__INCLUDES DEL SISTEMA______________________________________________________*/

/*__INCLUDES DE LA BD_________________________________________________________*/

/*__INCLUDES DE LA APLICACION_________________________________________________*/

#include "reflist.h"
#include "reftree.h"
#include "refmemo.h"

/*__CONSTANTES________________________________________________________________*/

//----------------

#define	BUFF_MAXLEN_STRING		4096
#define	BUFF_MAXLEN_NAME		256
#define	BUFF_MAXLEN_PATH		512
#define	BUFF_MAXLEN_FILE		1024
#define	BUFF_MAXLEN_LINE		1024

//----------------

#define	BUFF_FLAG_ON			1
#define	BUFF_FLAG_OFF			0

#define	BUFF_ON 				1
#define	BUFF_OFF				0

#define	BUFF_TRUE				1
#define	BUFF_FALSE				0

#define	BUFF_UP	        		1
#define	BUFF_DOWN				0

//----------------

#define BUFF_MAX(a,b) 		(a >= b ? a : b)
#define BUFF_MIN(a,b) 		(a <= b ? a : b)

#define BUFF_CMP(a,b)		(a < b ? -1 : a > b ? 1 : 0)

#define BUFF_CHR(c)			(c < 0 ? "<" : c > 0 ? ">" : "=")

#define BUFF_ALING(s)		((s) + (8 - ((s) % 8)) % 8);

//----------------

enum BUFF_RC_e
{
  BUFF_RC_TIMEOUT			= -6,
  BUFF_RC_INCOMPLETE		= -5,
  BUFF_RC_ALREADY_EXISTS	= -4,
  BUFF_RC_NOT_FOUND			= -3,
  BUFF_RC_MEMORY_FULL		= -2,
  BUFF_RC_ERROR				= -1,
  BUFF_RC_OK				=  0
};

extern const char*			BUFF_RC[];

#define BUFF_rc(e)			BUFF_RC[e * -1]

//----------------

/*__TIPOS_____________________________________________________________________*/

//----------------

typedef struct BUFF_data_tag		BUFF_data_t;
typedef struct BUFF_refs_tag		BUFF_refs_t;
typedef struct BUFF_memo_tag		BUFF_memo_t;

//----------------

struct BUFF_data_tag
{
  unsigned char*		data;
  int					refs;

  long					idx;
  long					len;

  BUFF_data_t*			next;
  BUFF_data_t*			prev;

  BUFF_memo_t*			type;
  MEMO_element_t		memo;
};

struct BUFF_refs_tag
{
  BUFF_data_t*			refs;
  BUFF_refs_t*			next;
  MEMO_element_t		memo;
};

struct BUFF_memo_tag
{
  long					size;
  long					used;
  long					mark;

  MEMO_refmemo_t*		refm;
  RFTR_element_t		tree;
};

/*__VARIABLES EXTERNAS________________________________________________________*/

/*__VARIABLES GLOBALES________________________________________________________*/

/*__VARIABLES LOCALES_________________________________________________________*/

/*__FUNCIONES EXTERNAS________________________________________________________*/

/*__FUNCIONES PUBLICAS________________________________________________________*/

//----------------

int BUFF_initialize(void);

//----------------

BUFF_memo_t* BUFF_memo_new(long inSize, long inBlen, long inBmin);
void BUFF_memo_delete(BUFF_memo_t* inBuffMemo);

int BUFF_memo_cmp(void* inVoidA, void* inVoidB);

//----------------

BUFF_data_t* BUFF_data_new(BUFF_memo_t* inBuffMemo);
void BUFF_data_delete(BUFF_data_t* inBuffData);

BUFF_data_t* BUFF_data_add(BUFF_data_t* inBuffData);

//----------------

BUFF_refs_t* BUFF_refs_new(void);

void BUFF_refs_delete(BUFF_refs_t* inBuffRefs);

void BUFF_refs_add
(
  BUFF_refs_t* 		inBuffRefs,
  BUFF_data_t* 		inBuffData
);

//----------------

/*__FUNCIONES PRIVADAS________________________________________________________*/

/*__FUNCIONES INLINE__________________________________________________________*/

static inline char* BUFF_trim(char* s)
{
  char*			pc;
  char*			pz;
  long			l;

  l = strlen(s); pc = s; pz = s + l - 1; 

  while(*pc == ' ' || *pc == '\t' || *pc == '\r' || *pc == '\n')
  {	
    pc++;
  }

  if(pz > pc) 
  {
    while(*pz == ' ' || *pz == '\t' || *pz == '\r' || *pz == '\n')
    {	
      *pz = 0; pz--;
    }
  }
  
  return pc;
}

/*----------------------------------------------------------------------------*/

static inline char* BUFF_ltrim(char* s)
{
  char*			pc = s;

  while(*pc == ' ' || *pc == '\t' || *pc == '\r' || *pc == '\n')
  {	
    pc++;
  }

  return pc;
}

/*----------------------------------------------------------------------------*/

static inline char* BUFF_rtrim(char* s)
{
  char*			pc = s;

  while(*pc == ' ' || *pc == '\t' || *pc == '\r' || *pc == '\n')
  {	
    *pc = 0; pc--;
  }

  return pc;
}

/*__DEFINES___________________________________________________________________*/

//----------------

void BUFF_fatal_error(char*, int, const char*, ...);

#define BUFF_FATAL0(cad) \
	BUFF_fatal_error((char *)__FILE__,(int)__LINE__, cad)

#define BUFF_FATAL1(cad,arg1) \
	BUFF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1))

#define BUFF_FATAL2(cad,arg1,arg2) \
	BUFF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2))

#define BUFF_FATAL3(cad,arg1,arg2,arg3) \
	BUFF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3))

#define BUFF_FATAL4(cad,arg1,arg2,arg3,arg4) \
	BUFF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3),(arg4))

#define BUFF_FATAL5(cad,arg1,arg2,arg3,arg4,arg5) \
	BUFF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3),(arg4),(arg5))

#define BUFF_FATAL6(cad,arg1,arg2,arg3,arg4,arg5,arg6) \
	BUFF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6))

#define BUFF_FATAL7(cad,arg1,arg2,arg3,arg4,arg5,arg6,arg7) \
	BUFF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6),(arg7))

#define BUFF_FATAL8(cad,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) \
	BUFF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6),(arg7),(arg8))

#define BUFF_FATAL9(cad,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9) \
	BUFF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6),(arg7),(arg8),(arg9))

//----------------

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

#endif
