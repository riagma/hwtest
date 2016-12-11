/*______________________________________________________________________________

  MODULO:	Trazas

  CATEGORIA:	UTILIB

  AUTOR:	Ricardo Aguado Mart'n

  VERSION:	1.0

  FECHA:	2009 - 2013
________________________________________________________________________________

  DESCRIPCION:

  OBSERVACIONES: 
______________________________________________________________________________*/

#ifndef __TRACE_H__
#define __TRACE_H__

/*__INCLUDES DEL SISTEMA______________________________________________________*/

#include <pthread.h>
#include <stdio.h>

/*__INCLUDES DE LA BD_________________________________________________________*/

/*__INCLUDES DE LA APLICACION_________________________________________________*/

/*__CONSTANTES________________________________________________________________*/

enum e_TRACE_TYPE
{
  TRACE_TYPE_APPL,	// 0
  TRACE_TYPE_DDBB,	// 1
  TRACE_TYPE_LIBS,	// 2
  TRACE_TYPE_REFO,	// 3
  TRACE_TYPE_COMM,	// 4
  TRACE_TYPE_GNIF,	// 5
  TRACE_TYPE_PDAS,	// 6
  TRACE_TYPE_PFCS,	// 7
  TRACE_TYPE_SIPL,	// 8
  TRACE_TYPE_AMTA,	// 9
  TRACE_TYPE_RADV,	// 0

  TRACE_TYPE_TOTAL
};

extern const char* TRACE_type[];

//----------------

#define TRACE_TYPE_DEFAULT			TRACE_TYPE_APPL

//----------------

enum e_TRACE_RC
{
  TRACE_RC_ERROR				= -1,
  TRACE_RC_OK					=  0,

  TRACE_RC_TOTAL
};

//----------------

#define	TRACE_MAXLEN_TRACE			4096

#define	TRACE_MAXLEN_FILE			1024
#define	TRACE_MAXLEN_PATH			512
#define	TRACE_MAXLEN_NAME			256
#define	TRACE_MAXLEN_EXTN			16

#define	TRACE_MAXLEN_TIMESTAMP			16

#define	TRACE_TRUE				1
#define	TRACE_FALSE				0

//----------------

/*__TIPOS_____________________________________________________________________*/

//----------------

typedef struct TRACE_tag 
{
  FILE*		        fs;

  int			level[TRACE_TYPE_TOTAL];
  
  char			file[TRACE_MAXLEN_FILE + 1];
  char			path[TRACE_MAXLEN_PATH + 1];
  char			name[TRACE_MAXLEN_NAME + 1];
  char			extn[TRACE_MAXLEN_EXTN + 1];

  char			tms[TRACE_MAXLEN_TIMESTAMP + 1];

  pthread_mutex_t	mutex[1];
  pthread_key_t         ptkey;  

  int			thrid;

} TRACE_t;

//----------------

/*__VARIABLES EXTERNAS________________________________________________________*/

/*__VARIABLES GLOBALES________________________________________________________*/

/*__VARIABLES LOCALES_________________________________________________________*/

/*__FUNCIONES EXTERNAS________________________________________________________*/

/*__FUNCIONES PUBLICAS________________________________________________________*/

//----------------

int TRACE_init
(
  char*			inTraceBpath,
  char*			inTraceRpath,
  char*			inTraceFname,
  char*			inTraceFextn
);

int TRACE_final(void);

//----------------

int TRACE_thread_id_set(void);

//----------------

int TRACE_level_get(int inType);

int TRACE_level_change(int inType, int inLevel);

//----------------

int TRACE_write(int inType, int inLevel, const char* inTrace, ...);

//----------------

/*__FUNCIONES PRIVADAS________________________________________________________*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

#endif


