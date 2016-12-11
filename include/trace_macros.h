/*______________________________________________________________________________

  MODULO:	Trazas

  CATEGORIA:	UTILIB

  AUTOR:	Ricardo Aguado Mart'n

  VERSION:	1.0

  FECHA:	2009	

________________________________________________________________________________

  DESCRIPCION:

  OBSERVACIONES: 
______________________________________________________________________________*/

#ifndef __TRACE_MACROS_H__ 
#define __TRACE_MACROS_H__ 

#include "trace.h"

#define SUCESO0(cad) \
	TRACE_write(TRACE_TYPE_DEFAULT, 0,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__)

#define SUCESO1(cad,arg1) \
	TRACE_write(TRACE_TYPE_DEFAULT, 0,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1))

#define SUCESO2(cad,arg1,arg2) \
	TRACE_write(TRACE_TYPE_DEFAULT, 0,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2))

#define SUCESO3(cad,arg1,arg2,arg3) \
	TRACE_write(TRACE_TYPE_DEFAULT, 0,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3))

#define SUCESO4(cad,arg1,arg2,arg3,arg4) \
	TRACE_write(TRACE_TYPE_DEFAULT, 0,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3),(arg4))

#define SUCESO5(cad,arg1,arg2,arg3,arg4,arg5) \
	TRACE_write(TRACE_TYPE_DEFAULT, 0,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3),(arg4),(arg5))

#define SUCESO6(cad,arg1,arg2,arg3,arg4,arg5,arg6) \
	TRACE_write(TRACE_TYPE_DEFAULT, 0,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6))

#define SUCESO7(cad,arg1,arg2,arg3,arg4,arg5,arg6,arg7) \
	TRACE_write(TRACE_TYPE_DEFAULT, 0,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6),(arg7))

#define SUCESO8(cad,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) \
	TRACE_write(TRACE_TYPE_DEFAULT, 0,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6),(arg7),(arg8))

#define SUCESO9(cad,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9) \
	TRACE_write(TRACE_TYPE_DEFAULT, 0,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6),(arg7),(arg8),(arg9))

#define PRUEBA0(cad) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 1) \
	TRACE_write(TRACE_TYPE_DEFAULT, 1,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__)

#define PRUEBA1(cad,arg1) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 1) \
	TRACE_write(TRACE_TYPE_DEFAULT, 1,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1))

#define PRUEBA2(cad,arg1,arg2) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 1) \
	TRACE_write(TRACE_TYPE_DEFAULT, 1,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2))

#define PRUEBA3(cad,arg1,arg2,arg3) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 1) \
	TRACE_write(TRACE_TYPE_DEFAULT, 1,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3))

#define PRUEBA4(cad,arg1,arg2,arg3,arg4) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 1) \
	TRACE_write(TRACE_TYPE_DEFAULT, 1,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3),(arg4))

#define PRUEBA5(cad,arg1,arg2,arg3,arg4,arg5) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 1) \
	TRACE_write(TRACE_TYPE_DEFAULT, 1,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3),(arg4),(arg5))

#define PRUEBA6(cad,arg1,arg2,arg3,arg4,arg5,arg6) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 1) \
	TRACE_write(TRACE_TYPE_DEFAULT, 1,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6))

#define PRUEBA7(cad,arg1,arg2,arg3,arg4,arg5,arg6,arg7) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 1) \
	TRACE_write(TRACE_TYPE_DEFAULT, 1,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6),(arg7))

#define PRUEBA8(cad,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 1) \
	TRACE_write(TRACE_TYPE_DEFAULT, 1,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6),(arg7),(arg8))

#define PRUEBA9(cad,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 1) \
	TRACE_write(TRACE_TYPE_DEFAULT, 1,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6),(arg7),(arg8),(arg9))

#define TRAZA0(cad) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 2) \
	TRACE_write(TRACE_TYPE_DEFAULT, 2,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__)

#define TRAZA1(cad,arg1) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 2) \
	TRACE_write(TRACE_TYPE_DEFAULT, 2,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1))

#define TRAZA2(cad,arg1,arg2) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 2) \
	TRACE_write(TRACE_TYPE_DEFAULT, 2,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2))

#define TRAZA3(cad,arg1,arg2,arg3) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 2) \
	TRACE_write(TRACE_TYPE_DEFAULT, 2,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3))

#define TRAZA4(cad,arg1,arg2,arg3,arg4) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 2) \
	TRACE_write(TRACE_TYPE_DEFAULT, 2,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3),(arg4))

#define TRAZA5(cad,arg1,arg2,arg3,arg4,arg5) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 2) \
	TRACE_write(TRACE_TYPE_DEFAULT, 2,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3),(arg4),(arg5))

#define TRAZA6(cad,arg1,arg2,arg3,arg4,arg5,arg6) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 2) \
	TRACE_write(TRACE_TYPE_DEFAULT, 2,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6))

#define TRAZA7(cad,arg1,arg2,arg3,arg4,arg5,arg6,arg7) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 2) \
	TRACE_write(TRACE_TYPE_DEFAULT, 2,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6),(arg7))

#define TRAZA8(cad,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 2) \
	TRACE_write(TRACE_TYPE_DEFAULT, 2,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6),(arg7),(arg8))

#define TRAZA9(cad,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 2) \
	TRACE_write(TRACE_TYPE_DEFAULT, 2,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6),(arg7),(arg8),(arg9))

#define DEPURA0(cad) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 3) \
	TRACE_write(TRACE_TYPE_DEFAULT, 3,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__)

#define DEPURA1(cad,arg1) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 3) \
	TRACE_write(TRACE_TYPE_DEFAULT, 3,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1))

#define DEPURA2(cad,arg1,arg2) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 3) \
	TRACE_write(TRACE_TYPE_DEFAULT, 3,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2))

#define DEPURA3(cad,arg1,arg2,arg3) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 3) \
	TRACE_write(TRACE_TYPE_DEFAULT, 3,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3))

#define DEPURA4(cad,arg1,arg2,arg3,arg4) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 3) \
	TRACE_write(TRACE_TYPE_DEFAULT, 3,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3),(arg4))

#define DEPURA5(cad,arg1,arg2,arg3,arg4,arg5) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 3) \
	TRACE_write(TRACE_TYPE_DEFAULT, 3,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3),(arg4),(arg5))

#define DEPURA6(cad,arg1,arg2,arg3,arg4,arg5,arg6) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 3) \
	TRACE_write(TRACE_TYPE_DEFAULT, 3,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6))

#define DEPURA7(cad,arg1,arg2,arg3,arg4,arg5,arg6,arg7) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 3) \
	TRACE_write(TRACE_TYPE_DEFAULT, 3,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6),(arg7))

#define DEPURA8(cad,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 3) \
	TRACE_write(TRACE_TYPE_DEFAULT, 3,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6),(arg7),(arg8))

#define DEPURA9(cad,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9) if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 3) \
	TRACE_write(TRACE_TYPE_DEFAULT, 3,"<%s %04d> " cad, (char *)__FILE__,(int)__LINE__,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6),(arg7),(arg8),(arg9))

#endif


