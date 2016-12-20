/*____________________________________________________________________________

  MODULO:	Mensaje protocolo peticio'n respuesta generico

  CATEGORIA:	GENIF

  AUTOR:	Ricardo Aguado Mart'n

  VERSION:	1.0

  FECHA:	2007 - 2013
______________________________________________________________________________

  DESCRIPCION:

  OBSERVACIONES: 
______________________________________________________________________________*/

/*__INCLUDES DEL SISTEMA______________________________________________________*/

#include <stdio.h>
#include <stdlib.h>

/*__INCLUDES DE LA BD_________________________________________________________*/

/*__INCLUDES DE LA APLICACION_________________________________________________*/

#include "trace_macros_gnif.h"
#include "auxfunctions.h"
#include "genif.h"

/*__CONSTANTES________________________________________________________________*/
  
//----------------

const char* a_GENIF_CHANNEL_TYPE[] =
{
  "GENIF_CHANNEL_TYPE_NONE",
  "GENIF_CHANNEL_TYPE_CLIENT",
  "GENIF_CHANNEL_TYPE_SERVER",

  ""
};

//----------------

const char* a_GENIF_MESSAGE_TYPE[] =
{
  "GENIF_MESSAGE_TYPE_UNKNOWN",
  "GENIF_MESSAGE_TYPE_MESSAGE",
  "GENIF_MESSAGE_TYPE_REQUEST",
  "GENIF_MESSAGE_TYPE_RESPONSE",

  ""
};

//----------------

const char* GENIF_notify_name[] =
{
  "GENIF_NOTIFY_NONE",

  "GENIF_NOTIFY_REQUEST_ERROR",
  "GENIF_NOTIFY_REQUEST_CLOSED",
  "GENIF_NOTIFY_REQUEST_TIMEOUT",
  "GENIF_NOTIFY_REQUEST_SENT",
  "GENIF_NOTIFY_REQUEST_SENT_CLOSED",
  "GENIF_NOTIFY_REQUEST_SENT_TIMEOUT",

  "GENIF_NOTIFY_REQUEST_RECEIVED",
  "GENIF_NOTIFY_REQUEST_RECEIVED_CLOSED",
  "GENIF_NOTIFY_REQUEST_RECEIVED_TIMEOUT",

  "GENIF_NOTIFY_RESPONSE_ERROR",
  "GENIF_NOTIFY_RESPONSE_CLOSED",
  "GENIF_NOTIFY_RESPONSE_TIMEOUT",
  "GENIF_NOTIFY_RESPONSE_SENT",

  "GENIF_NOTIFY_RESPONSE_RECEIVED",

  "GENIF_NOTIFY_MESSAGE_ERROR",
  "GENIF_NOTIFY_MESSAGE_CLOSED",
  "GENIF_NOTIFY_MESSAGE_TIMEOUT",
  "GENIF_NOTIFY_MESSAGE_SENT",
		
  "GENIF_NOTIFY_MESSAGE_RECEIVED",
  "GENIF_NOTIFY_MESSAGE_RECEIVED_CLOSED",
  "GENIF_NOTIFY_MESSAGE_RECEIVED_TIMEOUT",

  "GENIF_NOTIFY_UNKNOWN_RECEIVED",
  "GENIF_NOTIFY_UNKNOWN_RECEIVED_CLOSED",
  "GENIF_NOTIFY_UNKNOWN_RECEIVED_TIMEOUT",

  "GENIF_NOTIFY_CHANNEL_ACCEPTED",
  "GENIF_NOTIFY_CHANNEL_CONNECTED",
  
  "GENIF_NOTIFY_CHANNEL_HEARTBEAT",
  "GENIF_NOTIFY_CHANNEL_INACTIVITY",
  "GENIF_NOTIFY_CHANNEL_MALFUNCTION",

  "GENIF_NOTIFY_CHANNEL_ERROR",
  "GENIF_NOTIFY_CHANNEL_CLOSED",
  
  "GENIF_NOTIFY_CLIENT_CONNECTED",
  "GENIF_NOTIFY_CLIENT_DISCONNECTED",

  "GENIF_NOTIFY_CLIENT_EMPTY",

  "GENIF_NOTIFY_SERVER_CONNECTED",
  "GENIF_NOTIFY_SERVER_DISCONNECTED",

  "GENIF_NOTIFY_SERVER_EMPTY",

  ""
};

//----------------

/*__TIPOS_____________________________________________________________________*/

/*__VARIABLES EXTERNAS________________________________________________________*/

/*__VARIABLES GLOBALES________________________________________________________*/

/*__VARIABLES LOCALES_________________________________________________________*/

//----------------

static void (*a_fatal_error_function)(const char*) = NULL;

//----------------

/*__FUNCIONES EXTERNAS________________________________________________________*/

/*__FUNCIONES PUBLICAS________________________________________________________*/

/*__FUNCIONES PRIVADAS________________________________________________________*/

/*__FUNCIONES PUBLICAS________________________________________________________*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int 
GENIF_message_cmp(void* inPtrMsgA, void* inPtrMsgB)
{
  GENIF_message_t*		ptrMsgA = (GENIF_message_t*) inPtrMsgA;
  GENIF_message_t*		ptrMsgB = (GENIF_message_t*) inPtrMsgB;

  int				cmp = 0;

//TRAZA2("Entering in GENIF_message_cmp(%p, %p)", inPtrMsgA, inPtrMsgB);

//----------------

  cmp = ptrMsgA->modifier->message_cmp(ptrMsgA->modMsg, ptrMsgB->modMsg);

//----------------

//TRAZA1("Returning from GENIF_message_cmp() = %d", cmp);

  return cmp;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int 
GENIF_message_encode
(
  GENIF_message_t*		inPtrMsg,
  long				inMaxLen,
  unsigned char*		outPtrBuff,
  long*				outPtrLen
)
{
  char				buff[GENIF_MAXLEN_DUMP];

  int				ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_message_encode(%p)", inPtrMsg);

//----------------

  GENIF_message_view(inPtrMsg, 3);

//----------------

  ret = inPtrMsg->modifier->message_encode(inPtrMsg->modMsg,
                                           inPtrMsg->type, 
                                           inMaxLen, 
					   outPtrBuff, 
					   outPtrLen);

//----------------

  if(ret == GENIF_RC_ERROR)
  {
    SUCESO0("ERROR: GENIF_message_encode()");

    GENIF_message_view(inPtrMsg, 0);
  }

  else if(TRACE_level_get(TRACE_TYPE_DEFAULT) == 3)
  {
    AUXF_memory_dump(outPtrBuff, *outPtrLen, GENIF_MAXLEN_DUMP, buff);

    DEPURA1("ENCODE MESSAGE:\n%s", buff);
  }
  
//----------------

  TRAZA1("Returning from GENIF_message_encode() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int 
GENIF_message_decode
(
  GENIF_message_t*		inPtrMsg,
  unsigned char*		inPtrBuff,
  long				inBuffLen,
  long*				outPtrLen
)
{
  char				buff[GENIF_MAXLEN_DUMP];

  int				ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_message_decode(%p)", inPtrMsg);

//----------------
    
  if(TRACE_level_get(TRACE_TYPE_DEFAULT) == 3)
  {
    AUXF_memory_dump(inPtrBuff, inBuffLen, GENIF_MAXLEN_DUMP, buff);

    DEPURA1("DECODE MESSAGE:\n%s", buff);
  }

//----------------

  ret = inPtrMsg->modifier->message_decode(inPtrMsg->modMsg, 
                                           inPtrBuff, 
					   inBuffLen, 
					   outPtrLen, &inPtrMsg->type);

//----------------

  if(ret == GENIF_RC_ERROR)
  {
    AUXF_memory_dump(inPtrBuff, inBuffLen, GENIF_MAXLEN_DUMP, buff);

    SUCESO1("ERROR: GENIF_message_decode():\n%s", buff);
  }

  else
  {
    GENIF_message_view(inPtrMsg, 3);
  }

//----------------

  TRAZA1("Returning from GENIF_message_decode() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

void 
GENIF_message_resp_copy(GENIF_message_t* inPtrMsg, GENIF_message_t* inPtrResp)
{
//TRAZA2("Entering in GENIF_message_resp_copy(%p, %p)", 
//        inPtrMsg,
//        inPtrResp);

  inPtrMsg->modifier->message_resp_copy(inPtrMsg->modMsg, inPtrResp->modMsg);

//TRAZA0("Returning from GENIF_message_resp_copy()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void 
GENIF_message_view(GENIF_message_t* inPtrMsg, int inTraceLevel)
{
  inPtrMsg->modifier->message_view(inPtrMsg->modMsg, inTraceLevel);
}

/*----------------------------------------------------------------------------*/

char* 
GENIF_message_dump
(
  GENIF_message_t*		inPtrMsg,
  long				inStrLen,
  char*				outStr
)
{
  return inPtrMsg->modifier->message_dump(inPtrMsg->modMsg, inStrLen, outStr);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void
GENIF_set_fatal_error(void (*inFatalError)(const char*))
{
  a_fatal_error_function = inFatalError;
}

/*----------------------------------------------------------------------------*/

void
GENIF_fatal_error(char* inFile, int inLine, const char* inErrorStr, ...)
{
  va_list		list;

  char			errorStr[GENIF_MAXLEN_STRING + 1];

//----------------

  va_start(list, inErrorStr);

  vsnprintf(errorStr, GENIF_MAXLEN_STRING + 1, inErrorStr, list);

  va_end(list);

  errorStr[GENIF_MAXLEN_STRING] = 0;

//----------------

  TRACE_write(TRACE_TYPE_DEFAULT, 0, "<%s %04d> %s", inFile, inLine, errorStr);

//----------------

  exit(-1);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

