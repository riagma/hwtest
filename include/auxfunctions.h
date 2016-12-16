/*__________________________________________________________________________

  MODULO:	Funciones Auxiliares

  CATEGORIA:	UTILIB

  AUTOR:	Ricardo Aguado Mart'n

  VERSION:	1.0

  FECHA:	2001 - 2013
____________________________________________________________________________

  DESCRIPCION:

  OBSERVACIONES: 
____________________________________________________________________________*/

#ifndef __AUXFUNCTIONS_H__ 
#define __AUXFUNCTIONS_H__ 

/*__INCLUDES DE LA APLICACION_______________________________________________*/

#include "reftree.h"

/*__INCLUDES DEL SISTEMA____________________________________________________*/

#include <time.h>
#include <sys/time.h>

/*__CONSTANTES______________________________________________________________*/

//----------------

#define AUXF_MAX(a,b) 		a >= b ? a : b
#define AUXF_MIN(a,b) 		a <= b ? a : b

#define	AUXF_MAXLEN_TIMESTAMP	14

#define	AUXF_MAXLEN_IP		15

#define	AUXF_MAXLEN_STRING	4096
#define	AUXF_MAXLEN_ADDRESS	1024

//----------------

#define	AUXF_FLAG_ON		1
#define	AUXF_FLAG_OFF		0

#define	AUXF_TRUE		1
#define	AUXF_FALSE		0

//----------------

enum e_AUXF_RC
{
  AUXF_RC_ERROR			= -1,
  AUXF_RC_OK			=  0,

  AUXF_RC_TOTAL
};

//----------------

/*__TIPOS___________________________________________________________________*/

//----------------

typedef struct T_tag
{
  time_t		 	sec;
  time_t			hth;
  struct tm			stm;
  char				tms[AUXF_MAXLEN_TIMESTAMP + 1];

} T_t;

//----------------

typedef struct AUXF_local_address_tag
{
  char				addr[AUXF_MAXLEN_ADDRESS + 1];

  RFTR_element_t		tree;

} AUXF_local_address_t;

//----------------

/*__CONSTANTES______________________________________________________________*/
  
/*__FUNCIONES PUBLICAS______________________________________________________*/

/*--------------------------------------------------------------------------*/

void AUXF_T(T_t* outPtrT);

void AUXF_T_r
(
  T_t*		inPtrT, 
  int 		inYY, 
  int 		inMM, 
  int		inDD,
  int 		inHH, 
  int 		inMI, 
  int		inSS,
  T_t*		outPtrT
);

long AUXF_T_substract_hth
(
  T_t*		inT1, 
  T_t*		inT2
);

/*--------------------------------------------------------------------------*/

long AUXF_sec(void);

long AUXF_sec_r
(
  long		inSec, 
  int 		inYY, 
  int 		inMM, 
  int		inDD,
  int 		inHH, 
  int 		inMI, 
  int		inSS
);

/*--------------------------------------------------------------------------*/

long AUXF_gmt_diff(void);

char* AUXF_tms(char* outTms);

long AUXF_tms_to_sec(char* inTms);

char* AUXF_sec_to_tms(long inSec, char* outTms);

char* AUXF_tms_r
(
  const char*	inTms, 
  int 		inYY, 
  int 		inMM,
  int		inDD,
  int 		inHH, 
  int 		inMI, 
  int		inSS,
  char*		outTms
);

char* AUXF_tms_f
(
  const char*	inTms,
  const char*	inFormat,
  long		inOutLen,
  char*		outTms
);

long AUXF_tms_substract
(
  const char*   inTms1,
  const char*   inTms2
);

int AUXF_tms_verify(const char* inTms);

/*--------------------------------------------------------------------------*/

long AUXF_least_common_multiple(long inA, long inB);

int AUXF_int_verify(const char* inStr);

int AUXF_long_verify(const char* inStr);

int AUXF_float_verify(const char* inStr);

/*--------------------------------------------------------------------------*/

int AUXF_ip_verify(const char* inIP);

char* AUXF_ip_normalize(const char* inIP, char* outIP);

unsigned int AUXF_ip_to_number(const char* inIP);

/*--------------------------------------------------------------------------*/

void AUXF_tval_substract
(
  struct timeval*	tval1, 
  struct timeval*	tval2,
  struct timeval*	tvalo
);

/*--------------------------------------------------------------------------*/

char* AUXF_ltrim(char* ioStr);
char* AUXF_rtrim(char* ioStr);
char* AUXF_btrim(char* ioStr);

char* AUXF_ltrim0(char* ioStr);
char* AUXF_rtrim0(char* ioStr);
char* AUXF_btrim0(char* ioStr);

char* AUXF_ltrimC(char* ioStr, char inC);
char* AUXF_rtrimC(char* ioStr, char inC);
char* AUXF_btrimC(char* ioStr, char inC);

/*--------------------------------------------------------------------------*/

char* AUXF_scape_del(char* ioStr);

char* AUXF_scape_add(char* inStr, char* inCs, long* ioPtrLen, char* outStr);

/*--------------------------------------------------------------------------*/

char* AUXF_memory_dump
(
  const void*		inPtrMem,
  long			inMemLen,
  long			inOutLen,
  char*			outStr
);

char* AUXF_mem_to_hex
(
  const void*		inPtrMem,
  long			inMemLen,
  long			inStrLen,
  char*			outStrHex
);

long AUXF_hex_to_mem
(
  const char*		inStrHex,
  long			inMemLen,
  void*			outPtrMem
);

/*--------------------------------------------------------------------------*/

int AUXF_file_check
(
  const char*		inFile, 
  long* 		outSize,
  long* 		outTime
);

int AUXF_file_copy(const char* inFileSrc, const char* inFileDst);

/*--------------------------------------------------------------------------*/

double AUXF_random_get(void);

/*--------------------------------------------------------------------------*/

char* AUXF_login_get(long inMaxLen, char* outLogin);

char* AUXF_machine_get(long inMaxLen, char* outMachine);

/*--------------------------------------------------------------------------*/

int AUXF_is_alfanum(const char*	inString);

int AUXF_is_alfanum_c(int inC);

/*--------------------------------------------------------------------------*/
/*
char* AUXF_local_ip_get(char* outAddr);

int AUXF_local_ip_check(char* inAddr);

int AUXF_local_address_check(char* inAddr);

void AUXF_local_address_load(void);
*/
/*--------------------------------------------------------------------------*/

void AUXF_set_fatal_error(void (*)(char*, int, const char*, ...));

void AUXF_fatal_error(char*, int, const char*, ...);

#define AUXF_FATAL0(cad) \
	AUXF_fatal_error((char *)__FILE__,(int)__LINE__, cad)

#define AUXF_FATAL1(cad,arg1) \
	AUXF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1))

#define AUXF_FATAL2(cad,arg1,arg2) \
	AUXF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2))

#define AUXF_FATAL3(cad,arg1,arg2,arg3) \
	AUXF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3))

#define AUXF_FATAL4(cad,arg1,arg2,arg3,arg4) \
	AUXF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3),(arg4))

#define AUXF_FATAL5(cad,arg1,arg2,arg3,arg4,arg5) \
	AUXF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3),(arg4),(arg5))

#define AUXF_FATAL6(cad,arg1,arg2,arg3,arg4,arg5,arg6) \
	AUXF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6))

#define AUXF_FATAL7(cad,arg1,arg2,arg3,arg4,arg5,arg6,arg7) \
	AUXF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6),(arg7))

#define AUXF_FATAL8(cad,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) \
	AUXF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6),(arg7),(arg8))

#define AUXF_FATAL9(cad,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9) \
	AUXF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6),(arg7),(arg8),(arg9))

//----------------

#define STRCPY(d, s, n) strncpy(d, s, n); d[n] = 0

/*--------------------------------------------------------------------------*/

#endif

