/*____________________________________________________________________________

  MODULO:	Basic SIP library

  CATEGORIA:	JSON

  AUTOR:	Ricardo Aguado Mart'n

  VERSION:	1.0

  FECHA:	2015
______________________________________________________________________________

  DESCRIPCION:

  OBSERVACIONES: 
______________________________________________________________________________*/

#ifndef __JSONIF_H__
#define __JSONIF_H__

/*__INCLUDES DEL SISTEMA______________________________________________________*/

/*__INCLUDES DE RADVISION_____________________________________________________*/

/*__INCLUDES DE LA BD_________________________________________________________*/

/*__INCLUDES DE LA APLICACION_________________________________________________*/

#include "reflist.h"
#include "reftree.h"
#include "refmemo.h"

#include "buffer.h"

/*__CONSTANTES________________________________________________________________*/

//----------------

#define	JSON_MAXLEN_STRING		4096
#define	JSON_MAXLEN_NAME		256
#define	JSON_MAXLEN_PATH		512
#define	JSON_MAXLEN_FILE		1024
#define	JSON_MAXLEN_LINE		1024

//----------------

#define	JSON_FLAG_ON			1
#define	JSON_FLAG_OFF			0

#define	JSON_ON 				1
#define	JSON_OFF				0

#define	JSON_TRUE				1
#define	JSON_FALSE				0

#define	JSON_UP	        		1
#define	JSON_DOWN				0

//----------------

#define JSON_MAX(a,b) 		(a >= b ? a : b)
#define JSON_MIN(a,b) 		(a <= b ? a : b)

#define JSON_CMP(a,b)		(a < b ? -1 : a > b ? 1 : 0)

#define JSON_CHR(c)			(c < 0 ? "<" : c > 0 ? ">" : "=")

//----------------

enum JSON_VALUE_e
{
  JSON_VALUE_NONE,
  JSON_VALUE_STRING,
  JSON_VALUE_NUMBER,
  JSON_VALUE_OBJECT,
  JSON_VALUE_ARRAY,
  JSON_VALUE_TRUE,
  JSON_VALUE_FALSE,
  JSON_VALUE_NULL,

  JSON_VALUE_TOTAL
};

extern const char*			JSON_value[];

//----------------

enum JSON_DECODE_STATE_e
{
  JSON_DECODE_STATE_INIT,		// 00
  JSON_DECODE_STATE_PAIR,		// 01
  JSON_DECODE_STATE_VALUE,		// 02
  JSON_DECODE_STATE_SUCCESS,	// 03
  JSON_DECODE_STATE_ERROR,		// 04

  JSON_DECODE_STATE_TOTAL
};

// extern const char* JSON_decode_state[];

//----------------

enum JSON_RC_e
{
  JSON_RC_TIMEOUT			= -6,
  JSON_RC_INCOMPLETE		= -5,
  JSON_RC_ALREADY_EXISTS	= -4,
  JSON_RC_NOT_FOUND			= -3,
  JSON_RC_MEMORY_FULL		= -2,
  JSON_RC_ERROR				= -1,
  JSON_RC_OK				=  0
};

extern const char*			JSON_RC[];

#define JSON_rc(e)			JSON_RC[e * -1]

//----------------

/*__TIPOS_____________________________________________________________________*/

//----------------

typedef struct JSON_map_tag			JSON_map_t;
typedef struct JSON_value_tag		JSON_value_t;
typedef struct JSON_array_tag		JSON_array_t;
typedef struct JSON_pair_tag		JSON_pair_t;
typedef struct JSON_object_tag		JSON_object_t;

//---------------- Map

struct JSON_map_tag
{
  char*					key;
  int					idx;

  RFTR_element_t		tree;
  MEMO_element_t		memo;
};

//---------------- Value

struct JSON_value_tag
{
  int					type;
  void*					data;

  RLST_element_t		list;
  MEMO_element_t		memo;
};

//---------------- Array

struct JSON_array_tag
{
  RLST_reflist_t		list[1];

  int					type;
  JSON_value_t*			curr;

  MEMO_element_t		memo;
};

//---------------- Pair

struct JSON_pair_tag
{
  char*					name;
  JSON_value_t*			value;

  RLST_element_t		list;
  RFTR_element_t		tree;
  MEMO_element_t		memo;
};

//---------------- Object

struct JSON_object_tag
{
  BUFF_buff_t			buffRefs[1];

  RLST_reflist_t		pairList[1];
  RFTR_reftree_t		pairTree[1];

  int					decodeState;

  MEMO_element_t		memo;
};

//----------------

/*__VARIABLES EXTERNAS________________________________________________________*/

/*__VARIABLES GLOBALES________________________________________________________*/

/*__VARIABLES LOCALES_________________________________________________________*/

/*__FUNCIONES EXTERNAS________________________________________________________*/

/*__FUNCIONES PUBLICAS________________________________________________________*/

//----------------

int JSON_initialize(void);

//----------------

JSON_map_t* JSON_map_new(void);
void JSON_map_delete(JSON_map_t* inMap);

int JSON_map_cmp(void* inVoidA, void* inVoidB);

JSON_value_t* JSON_value_new(void);
void JSON_value_delete(JSON_value_t* inValue);

int JSON_value_cmp(void* inVoidA, void* inVoidB);

JSON_array_t* JSON_array_new(void);
void JSON_array_delete(JSON_array_t* inArray);

JSON_pair_t* JSON_pair_new(void);
void JSON_pair_delete(JSON_pair_t* inPair);

int JSON_pair_cmp(void* inVoidA, void* inVoidB);

JSON_object_t* JSON_object_new(void);
void JSON_object_delete(JSON_object_t* inObject);

//----------------

int JSON_object_decode
(
  JSON_object_t*		inObject,
  BUFF_buff_t*			inBuffer
);

int JSON_object_encode(JSON_object_t* inObject);

void JSON_object_view
(
  JSON_object_t*		inObject,
  int					inLevel
);

char* JSON_object_dump
(
  JSON_object_t*		inObject,
  long			        inStrLen,
  char*			        outStr
);

//----------------

int JSON_name_decode
(
  JSON_object_t*		inObject,
  BUFF_buff_t*			inBuffer,
  JSON_pair_t*			inPair
);

int JSON_value_decode
(
  JSON_object_t*		inObject,
  BUFF_buff_t*			inBuffer,
  JSON_value_t*			inValue,
  char*					outSep
);

//----------------

int JSON_array_decode
(
  JSON_object_t*		inObject,
  BUFF_buff_t*			inBuffer,
  JSON_value_t*			inValue,
  char*					outSep
);

int JSON_string_decode
(
  JSON_object_t*		inObject,
  BUFF_buff_t*			inBuffer,
  JSON_value_t*			inValue,
  char*					outSep
);

int JSON_number_decode
(
  JSON_object_t*		inObject,
  BUFF_buff_t*			inBuffer,
  JSON_value_t*			inValue,
  char*					outSep
);

int JSON_literal_decode
(
  JSON_object_t*		inObject,
  BUFF_buff_t*			inBuffer,
  JSON_value_t*			inValue,
  char*					outSep
);

//----------------

/*__DEFINES___________________________________________________________________*/

void JSON_fatal_error(char*, int, const char*, ...);

#define JSON_FATAL0(cad) \
	JSON_fatal_error((char *)__FILE__,(int)__LINE__, cad)

#define JSON_FATAL1(cad,arg1) \
	JSON_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1))

#define JSON_FATAL2(cad,arg1,arg2) \
	JSON_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2))

#define JSON_FATAL3(cad,arg1,arg2,arg3) \
	JSON_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3))

#define JSON_FATAL4(cad,arg1,arg2,arg3,arg4) \
	JSON_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3),(arg4))

#define JSON_FATAL5(cad,arg1,arg2,arg3,arg4,arg5) \
	JSON_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3),(arg4),(arg5))

#define JSON_FATAL6(cad,arg1,arg2,arg3,arg4,arg5,arg6) \
	JSON_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6))

#define JSON_FATAL7(cad,arg1,arg2,arg3,arg4,arg5,arg6,arg7) \
	JSON_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6),(arg7))

#define JSON_FATAL8(cad,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) \
	JSON_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6),(arg7),(arg8))

#define JSON_FATAL9(cad,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9) \
	JSON_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6),(arg7),(arg8),(arg9))

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

#endif


