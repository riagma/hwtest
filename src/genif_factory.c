/*____________________________________________________________________________

  MODULO:	Factori'a de objetos genif

  CATEGORIA:	GENIF

  AUTOR:	Ricardo Aguado Mart'n

  VERSION:	1.0

  FECHA:	2013
______________________________________________________________________________

  DESCRIPCION:

  OBSERVACIONES: 
______________________________________________________________________________*/

/*__INCLUDES DEL SISTEMA______________________________________________________*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <stdarg.h>
#include <setjmp.h>

#ifndef __hpux
#include <sys/select.h>
#endif

#include <sys/times.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/utsname.h>

/*__INCLUDES DE LA BD_________________________________________________________*/

/*__INCLUDES DE LA APLICACION_________________________________________________*/

#include "trace_macros_gnif.h"
#include "auxfunctions.h"

#include "genif.h"

/*__CONSTANTES________________________________________________________________*/
  
/*----------*/

enum e_GENIF_OBJECT
{
  GENIF_OBJECT_NONE,

  GENIF_OBJECT_MESSAGE,
  GENIF_OBJECT_CHANNEL,

  GENIF_OBJECT_CLIENT,
  GENIF_OBJECT_SERVER,

  GENIF_OBJECT_TOTAL
};

/*----------*/

/*__TIPOS_____________________________________________________________________*/

/*----------*/

typedef struct GENIF_deleter_tag 
{
  int				type;
  void*				object;

  int*				notDelete;

  RLST_element_t		list;
  RFTR_element_t		tree;
  MEMO_element_t		memo;

} GENIF_deleter_t;

/*----------*/

/*__VARIABLES EXTERNAS________________________________________________________*/

/*__VARIABLES GLOBALES________________________________________________________*/

/*__VARIABLES LOCALES_________________________________________________________*/

/*----------*/

static int			a_FactoryInit = 0;

static MEMO_refmemo_t*		a_MessageMemo = NULL;
static RLST_reflist_t		a_MessageDelete[1];
static long			a_MessageCounter = 0;

static MEMO_refmemo_t*		a_ChannelMemo = NULL;
static RLST_reflist_t		a_ChannelDelete[1];
static long			a_ChannelCounter = 0;

static MEMO_refmemo_t*		a_ClientMemo = NULL;
static RLST_reflist_t		a_ClientDelete[1];
static long			a_ClientCounter = 0;

static MEMO_refmemo_t*		a_ServerMemo = NULL;
static RLST_reflist_t		a_ServerDelete[1];
static long			a_ServerCounter = 0;

/*----------*/

static struct Timer*		a_DeleterTimer = NULL;

/*----------*/

/*__FUNCIONES EXTERNAS________________________________________________________*/

/*__FUNCIONES PUBLICAS________________________________________________________*/

/*__FUNCIONES PRIVADAS________________________________________________________*/

/*----------*/

static void GENIF_deleter_timer_cb(const struct Timer* inPtrTimer);

/*----------*/

/*__FUNCIONES PUBLICAS________________________________________________________*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void
GENIF_factory_initialize(void) 
{
  if(a_FactoryInit == 0)
  {
    GENIF_message_t		m;
    GENIF_channel_t		c;

    GENIF_client_t		l;
    GENIF_server_t		s;
  
    TRAZA0("Entering in GENIF_factory_initialize()");
    
/*----------*/

    a_MessageMemo = MEMO_createRefMemo(&m, &m.memo, sizeof(m), 256, 4);
      
    if(a_MessageMemo == NULL)
    {
      GENIF_FATAL("ERROR: MEMO_createRefMemo()");
    }

    if(RLST_initializeRefList(a_MessageDelete, &m, &m.list) < 0)
    {
      GENIF_FATAL("ERROR: RLST_initializeRefList()");
    }

/*----------*/

    a_ChannelMemo = MEMO_createRefMemo(&c, &c.memo, sizeof(c), 128, 2);
      
    if(a_ChannelMemo == NULL)
    {
      GENIF_FATAL("ERROR: MEMO_createRefMemo()");
    }

    if(RLST_initializeRefList(a_ChannelDelete, &c, &c.list) < 0)
    {
      GENIF_FATAL("ERROR: RLST_initializeRefList()");
    }

/*----------*/

    a_ClientMemo = MEMO_createRefMemo(&l, &l.memo, sizeof(l), 64, 2);
      
    if(a_ClientMemo == NULL)
    {
      GENIF_FATAL("ERROR: MEMO_createRefMemo()");
    }

    if(RLST_initializeRefList(a_ClientDelete, &l, &l.list) < 0)
    {
      GENIF_FATAL("ERROR: RLST_initializeRefList()");
    }

    a_ServerMemo = MEMO_createRefMemo(&s, &s.memo, sizeof(s), 16, 2);
      
    if(a_ServerMemo == NULL)
    {
      GENIF_FATAL("ERROR: MEMO_createRefMemo()");
    }

    if(RLST_initializeRefList(a_ServerDelete, &s, &s.list) < 0)
    {
      GENIF_FATAL("ERROR: RLST_initializeRefList()");
    }

/*----------*/

    a_DeleterTimer = setTimer(1, GENIF_deleter_timer_cb, NULL);

    if(a_DeleterTimer == NULL)
    {
      GENIF_FATAL("ERROR: setTimer()");
    }

/*----------*/
    a_FactoryInit = 1;
/*----------*/

    TRAZA0("Returning from GENIF_factory_initialize()");
  }

/*----------*/
}

/*----------------------------------------------------------------------------*/

void
GENIF_factoty_memo_view()
{
  SUCESO4("GENIF MEMO: MSG(%ld) CHN(%ld) CLI(%ld) SRV(%ld)",
           a_MessageCounter,
           a_ChannelCounter,
           a_ClientCounter,
           a_ServerCounter);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static void
GENIF_deleter_timer_cb(const struct Timer* inPtrTimer)
{
//long				T = inPtrTimer->tv.tv_sec;

  GENIF_message_t*		ptrMessage;
  GENIF_channel_t*		ptrChannel;

  GENIF_client_t*		ptrClient;
  GENIF_server_t*		ptrServer;

  TRAZA1("Entering in GENIF_deleter_timer_cb(%ld)", inPtrTimer->tv.tv_sec);

/*----------*/

  a_DeleterTimer = setTimer(100, GENIF_deleter_timer_cb, NULL);

  if(a_DeleterTimer == NULL)
  {
    GENIF_FATAL("ERROR: setTimer()");
  }

/*----------*/

  RLST_resetGet(a_MessageDelete, NULL);

  while((ptrMessage = RLST_getNext(a_MessageDelete)))
  {
    if(ptrMessage->notDelete == GENIF_FALSE)
    {
      RLST_extract(a_MessageDelete, ptrMessage);

      GENIF_message_delete(ptrMessage);
    }
  }

/*----------*/

  RLST_resetGet(a_ChannelDelete, NULL);

  while((ptrChannel = RLST_getNext(a_ChannelDelete)))
  {
    if(ptrChannel->notDelete == GENIF_FALSE)
    {
      RLST_extract(a_ChannelDelete, ptrChannel);

      GENIF_channel_delete(ptrChannel);
    }
  }

/*----------*/

  RLST_resetGet(a_ClientDelete, NULL);

  while((ptrClient = RLST_getNext(a_ClientDelete)))
  {
    if(ptrClient->notDelete == GENIF_FALSE)
    {
      RLST_extract(a_ClientDelete, ptrClient);

      GENIF_client_delete(ptrClient);
    }
  }

/*----------*/

  RLST_resetGet(a_ServerDelete, NULL);

  while((ptrServer = RLST_getNext(a_ServerDelete)))
  {
    if(ptrServer->notDelete == GENIF_FALSE)
    {
      RLST_extract(a_ServerDelete, ptrServer);

      GENIF_server_delete(ptrServer);
    }
  }

/*----------*/

  TRAZA0("Returning from GENIF_deleter_timer_cb()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

GENIF_message_t* 
GENIF_message_new(GENIF_modifier_t* inPtrModifier)
{
  GENIF_message_t*		ptrMessage = NULL;

  TRAZA0("Entering in GENIF_message_new()");

/*----------*/

  ptrMessage = (GENIF_message_t*) MEMO_new(a_MessageMemo);

  if(ptrMessage == NULL)
  {
    GENIF_FATAL("ERROR: MEMO_new()");
  }

  a_MessageCounter++;

/*----------*/

  ptrMessage->modifier = inPtrModifier;

  ptrMessage->modMsg = ptrMessage->modifier->message_new(ptrMessage);

  if(ptrMessage->modMsg == NULL)
  {
    GENIF_FATAL("ERROR: modifier->message_new()");
  }

/*----------*/

  TRAZA1("Returning from GENIF_message_new() = %p", ptrMessage);

  return ptrMessage;
}

/*----------------------------------------------------------------------------*/

void
GENIF_message_delete(GENIF_message_t* inPtrMessage)
{
  TRAZA1("Entering in GENIF_message_delete(%p)", inPtrMessage);

/*----------*/

  if(inPtrMessage->notDelete == GENIF_TRUE)
  {
    if(RLST_insertTail(a_MessageDelete, inPtrMessage) < 0)
    {
      GENIF_FATAL("ERROR: RLST_insertTail()");
    }
  }

  else
  {
    inPtrMessage->modifier->message_delete(inPtrMessage->modMsg);

    MEMO_delete(a_MessageMemo, inPtrMessage);

    a_MessageCounter--;
  }

/*----------*/

  TRAZA0("Returning from GENIF_message_delete()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

GENIF_channel_t* 
GENIF_channel_new(void)
{
  GENIF_channel_t*		ptrChannel = NULL;

  TRAZA0("Entering in GENIF_channel_new()");

/*----------*/

  ptrChannel = (GENIF_channel_t*) MEMO_new(a_ChannelMemo);

  if(ptrChannel == NULL)
  {
    GENIF_FATAL("ERROR: MEMO_new()");
  }

  a_ChannelCounter++;

/*----------*/

  TRAZA1("Returning from GENIF_channel_new() = %p", ptrChannel);

  return ptrChannel;
}

/*----------------------------------------------------------------------------*/

void 
GENIF_channel_delete(GENIF_channel_t* inPtrChannel)
{
  TRAZA1("Entering in GENIF_channel_delete(%p)", inPtrChannel);

/*----------*/

  if(inPtrChannel->notDelete == GENIF_TRUE)
  {
    if(RLST_insertTail(a_ChannelDelete, inPtrChannel) < 0)
    {
      GENIF_FATAL("ERROR: RLST_insertTail()");
    }
  }

  else
  {
    MEMO_delete(a_ChannelMemo, inPtrChannel);

    a_ChannelCounter--;
  }

/*----------*/

  TRAZA0("Returning from GENIF_channel_delete()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

GENIF_client_t* 
GENIF_client_new(void)
{
  GENIF_client_t*		ptrClient = NULL;

  TRAZA0("Entering in GENIF_client_new()");

/*----------*/
  GENIF_factory_initialize();
/*----------*/

  ptrClient = (GENIF_client_t*) MEMO_new(a_ClientMemo);

  if(ptrClient == NULL)
  {
    GENIF_FATAL("ERROR: MEMO_new()");
  }

  a_ClientCounter++;

/*----------*/

  TRAZA1("Returning from GENIF_client_new() = %p", ptrClient);

  return ptrClient;
}

/*----------------------------------------------------------------------------*/

void 
GENIF_client_delete(GENIF_client_t* inPtrClient)
{
  TRAZA1("Entering in GENIF_client_delete(%p)", inPtrClient);

/*----------*/

  if(inPtrClient->notDelete == GENIF_TRUE)
  {
    if(RLST_insertTail(a_ClientDelete, inPtrClient) < 0)
    {
      GENIF_FATAL("ERROR: RLST_insertTail()");
    }
  }

  else
  {
    MEMO_delete(a_ClientMemo, inPtrClient);

    a_ClientCounter--;
  }

/*----------*/

  TRAZA0("Returning from GENIF_client_delete()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

GENIF_server_t* 
GENIF_server_new(void)
{
  GENIF_server_t*		ptrServer = NULL;

  TRAZA0("Entering in GENIF_server_new()");

/*----------*/
  GENIF_factory_initialize();
/*----------*/

  ptrServer = (GENIF_server_t*) MEMO_new(a_ServerMemo);

  if(ptrServer == NULL)
  {
    GENIF_FATAL("ERROR: MEMO_new()");
  }

  a_ServerCounter++;

/*----------*/

  TRAZA1("Returning from GENIF_server_new() = %p", ptrServer);

  return ptrServer;
}

/*----------------------------------------------------------------------------*/

void 
GENIF_server_delete(GENIF_server_t* inPtrServer)
{
  TRAZA1("Entering in GENIF_server_delete(%p)", inPtrServer);

/*----------*/

  if(inPtrServer->notDelete == GENIF_TRUE)
  {
    if(RLST_insertTail(a_ServerDelete, inPtrServer) < 0)
    {
      GENIF_FATAL("ERROR: RLST_insertTail()");
    }
  }

  else
  {
    MEMO_delete(a_ServerMemo, inPtrServer);

    a_ServerCounter--;
  }

/*----------*/

  TRAZA0("Returning from GENIF_server_delete()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

