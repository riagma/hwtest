/*____________________________________________________________________________

  MODULO:	Cliente protocolo peticio'n respuesta generico

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
  
/*__TIPOS_____________________________________________________________________*/

/*__VARIABLES EXTERNAS________________________________________________________*/

/*__VARIABLES GLOBALES________________________________________________________*/

/*__VARIABLES LOCALES_________________________________________________________*/

/*__FUNCIONES EXTERNAS________________________________________________________*/

/*__FUNCIONES PUBLICAS________________________________________________________*/

/*__FUNCIONES PRIVADAS________________________________________________________*/

/*----------*/

static int GENIF_client_connect(GENIF_client_t* inPtrClient);

static void GENIF_client_connect_cb
(
  int				inFd,
  int				inError,
  void*				inPtrVoidClient
);

/*----------*/

static void GENIF_client_timer_cb(const struct Timer* inPtrTimer);

static void GENIF_client_mask(GENIF_client_t* inPtrClient);

static void GENIF_client_channel_close_cb
( 
  GENIF_client_t*		inPtrClient,
  GENIF_channel_t*		inPtrChannel
);

static GENIF_message_t* GENIF_client_channel_write_cb
(
  GENIF_channel_t*		inPtrChannel
);

static void GENIF_client_channel_notify_cb
(
  GENIF_channel_t*		inPtrChannel,
  int				inNotify,
  GENIF_message_t*		inPtrMessage
);

static void GENIF_client_notify
(
  GENIF_client_t*		inPtrCli,
  int				inNotify,
  GENIF_channel_t*		inPtrChn,
  GENIF_message_t*		inPtrMsg
);

/*----------*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
GENIF_client_initialize
(
  GENIF_client_t*		inPtrClient,
  char*				inHost,
  int				inPort,
  void*				inPtrParent,
  void				(*inCbNotify)(GENIF_client_t*, int,
                                              GENIF_channel_t*,
					      GENIF_message_t*),
  GENIF_client_config_t*	inPtrConfig,
  GENIF_modifier_t*		inPtrModifier
)
{
  GENIF_channel_t		channel;

  GENIF_message_t		msg;

  int				rc;
  
  int				ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_client_initialize(%p)", inPtrClient);

/*----------*/
  GENIF_factory_initialize();
/*----------*/

  if(ret == GENIF_RC_OK)
  {
    strncpy(inPtrClient->host, inHost, GENIF_MAXLEN_HOST);
    inPtrClient->host[GENIF_MAXLEN_HOST] = 0;
    inPtrClient->port = inPort;

    inPtrClient->localHP = GENIF_FLAG_OFF;
    
    inPtrClient->localHost[0] = 0;
    inPtrClient->localPort    = 0;

    inPtrClient->parent   = inPtrParent;
    inPtrClient->cbNotify = inCbNotify;

    inPtrClient->modifier = inPtrModifier;

    inPtrClient->extRef   = NULL;

    inPtrClient->timer    = NULL;

    inPtrClient->connecting = 0;

    inPtrClient->disconnFlag = GENIF_FLAG_OFF;
    
    inPtrClient->reconnectT    = 0;
    inPtrClient->reconnectFlag = GENIF_FLAG_OFF;

//  inPtrClient->channel_;
    inPtrClient->channel = NULL;

    inPtrClient->extQueueFunc = NULL;
    inPtrClient->extQueueFlag = GENIF_FLAG_OFF;

//  inPtrClient->outQueue_;
    inPtrClient->outQueue  = NULL;

    inPtrClient->emptyFlag = GENIF_FLAG_OFF;
    
    inPtrClient->outRefCount  = 0;
    inPtrClient->outFlowCount = 0;

    inPtrClient->disabledInput  = GENIF_FLAG_OFF;
    inPtrClient->disabledOutput = GENIF_FLAG_OFF;

    inPtrClient->inputMask  = GENIF_FLAG_ON;
    inPtrClient->outputMask = GENIF_FLAG_ON;

    memcpy(&inPtrClient->config, inPtrConfig, sizeof(GENIF_client_config_t));
    memset(&inPtrClient->counter_, 0, sizeof(GENIF_client_counter_t));

    inPtrClient->counter = &inPtrClient->counter_;
  
    inPtrClient->notDelete = GENIF_FALSE;
  }

/*----------*/

  if(ret == GENIF_RC_OK)
  {
    inPtrClient->timer = setTimer(1, GENIF_client_timer_cb, inPtrClient);

    if(inPtrClient->timer == NULL)
    {
      GENIF_FATAL("ERROR: setTimer()");
    }
  }

/*----------*/

  if(ret == GENIF_RC_OK)
  {
    inPtrClient->channel = &inPtrClient->channel_;

    rc = RLST_initializeRefList(inPtrClient->channel, &channel, &channel.list);

    if(rc < 0)
    {
      GENIF_FATAL("ERROR: RLST_initializeRefList()");
    }
  }
  
/*----------*/

  if(ret == GENIF_RC_OK)
  {
    inPtrClient->outQueue = &inPtrClient->outQueue_;

    rc = RLST_initializeRefList(inPtrClient->outQueue, &msg, &msg.list);

    if(rc < 0)
    {
      GENIF_FATAL("ERROR: RLST_initializeRefList()");
    }
  }
  
/*----------*/

  if(ret == GENIF_RC_OK)
  {
    rc = GENIF_client_connect(inPtrClient);

    if(rc < 0)
    {
      SUCESO0("ERROR: GENIF_client_connect()");

      GENIF_client_finalize(inPtrClient); 

      ret = GENIF_RC_ERROR;
    }
  }
  
/*----------*/

  TRAZA1("Returning from GENIF_client_initialize() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int 
GENIF_client_finalize(GENIF_client_t* inPtrClient)
{
  GENIF_channel_t*		ptrChannel;

  GENIF_message_t*		ptrMessage;

  char				strMessage[GENIF_MAXLEN_STRING + 1];

  int				ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_client_finalize(%p)", inPtrClient);

/*----------*/

  if(inPtrClient->outQueue != NULL)
  {
    while((ptrMessage = RLST_extractHead(inPtrClient->outQueue)) != NULL)
    {
      GENIF_message_dump(ptrMessage, GENIF_MAXLEN_STRING + 1, strMessage);
      
      SUCESO1("WARNING: CANCELED QUEUED MESSAGE %s", strMessage);
    }
  }

/*----------*/

  if(inPtrClient->channel != NULL)
  {
    if(RLST_getNumElem(inPtrClient->channel) < inPtrClient->config.channelMax)
    {
      inPtrClient->reconnectT = inPtrClient->config.connectTout;
    }

    else
    {
      inPtrClient->reconnectT = 0;
    }

    while((ptrChannel = RLST_extractHead(inPtrClient->channel)))
    {
      if(GENIF_channel_finalize(ptrChannel) != GENIF_RC_OK)
      {
        SUCESO0("ERROR: GENIF_channel_finalize()");
      }

      GENIF_channel_delete(ptrChannel);
    }
  }
  
/*----------*/

  if(inPtrClient->timer != NULL)
  {
    if(cancelTimer(inPtrClient->timer))
    {
      SUCESO0("ERROR: cancelTimer()");
    }

    inPtrClient->timer = NULL;
  }

/*----------*/

  TRAZA1("Returning from GENIF_client_finalize() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
GENIF_client_initialize_hp
(
  GENIF_client_t*		inPtrClient,
  char*				inHost,
  int				inPort,
  char*				inLocalHost,
  int				inLocalPort,
  void*				inPtrParent,
  void				(*inCbNotify)(GENIF_client_t*, int,
                                              GENIF_channel_t*,
					      GENIF_message_t*),
  GENIF_client_config_t*	inPtrConfig,
  GENIF_modifier_t*		inPtrModifier
)
{
  GENIF_channel_t		channel;

  GENIF_message_t		msg;

  int				rc;
  
  int				ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_client_initialize(%p)", inPtrClient);

/*----------*/
  GENIF_factory_initialize();
/*----------*/

  if(ret == GENIF_RC_OK)
  {
    strncpy(inPtrClient->host, inHost, GENIF_MAXLEN_HOST);
    inPtrClient->host[GENIF_MAXLEN_HOST] = 0;
    inPtrClient->port = inPort;

    inPtrClient->localHP = GENIF_FLAG_ON;
    
    strncpy(inPtrClient->localHost, inLocalHost, GENIF_MAXLEN_HOST);
    inPtrClient->localHost[GENIF_MAXLEN_HOST] = 0;
    inPtrClient->localPort = inLocalPort;

    inPtrClient->parent   = inPtrParent;
    inPtrClient->cbNotify = inCbNotify;

    inPtrClient->modifier = inPtrModifier;

    inPtrClient->extRef   = NULL;

    inPtrClient->timer    = NULL;

    inPtrClient->connecting = 0;

    inPtrClient->disconnFlag = GENIF_FLAG_OFF;
    
    inPtrClient->reconnectT    = 0;
    inPtrClient->reconnectFlag = GENIF_FLAG_OFF;

//  inPtrClient->channel_;
    inPtrClient->channel = NULL;

    inPtrClient->extQueueFunc = NULL;
    inPtrClient->extQueueFlag = GENIF_FLAG_OFF;

//  inPtrClient->outQueue_;
    inPtrClient->outQueue  = NULL;

    inPtrClient->emptyFlag = GENIF_FLAG_OFF;
    
    inPtrClient->outRefCount  = 0;
    inPtrClient->outFlowCount = 0;

    inPtrClient->disabledInput  = GENIF_FLAG_OFF;
    inPtrClient->disabledOutput = GENIF_FLAG_OFF;

    inPtrClient->inputMask  = GENIF_FLAG_ON;
    inPtrClient->outputMask = GENIF_FLAG_ON;

    memcpy(&inPtrClient->config, inPtrConfig, sizeof(GENIF_client_config_t));
    memset(&inPtrClient->counter_, 0, sizeof(GENIF_client_counter_t));

    inPtrClient->counter = &inPtrClient->counter_;

    inPtrClient->notDelete = GENIF_FALSE;
  }

/*----------*/

  if(ret == GENIF_RC_OK)
  {
    inPtrClient->timer = setTimer(1, GENIF_client_timer_cb, inPtrClient);

    if(inPtrClient->timer == NULL)
    {
      GENIF_FATAL("ERROR: setTimer()");
    }
  }

/*----------*/

  if(ret == GENIF_RC_OK)
  {
    inPtrClient->channel = &inPtrClient->channel_;

    rc = RLST_initializeRefList(inPtrClient->channel, &channel, &channel.list);

    if(rc < 0)
    {
      GENIF_FATAL("ERROR: RLST_initializeRefList()");
    }
  }
  
/*----------*/

  if(ret == GENIF_RC_OK)
  {
    inPtrClient->outQueue = &inPtrClient->outQueue_;

    rc = RLST_initializeRefList(inPtrClient->outQueue, &msg, &msg.list);

    if(rc < 0)
    {
      GENIF_FATAL("ERROR: RLST_initializeRefList()");
    }
  }
  
/*----------*/

  if(ret == GENIF_RC_OK)
  {
    rc = GENIF_client_connect(inPtrClient);

    if(rc < 0)
    {
      SUCESO0("ERROR: GENIF_client_connect()");

      GENIF_client_finalize(inPtrClient); 

      ret = GENIF_RC_ERROR;
    }
  }
  
/*----------*/

  TRAZA1("Returning from GENIF_client_initialize() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
GENIF_client_reconfig
(
  GENIF_client_t*		inPtrClient,
  GENIF_client_config_t*	inPtrConfig
)
{
  int				ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_client_reconfig(%p)", inPtrClient);

/*----------*/

  memcpy(&inPtrClient->config, inPtrConfig, sizeof(GENIF_client_config_t));

  GENIF_client_connect(inPtrClient);

/*----------*/

  TRAZA1("Returning from GENIF_client_reconfig() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
GENIF_client_count
(
  GENIF_client_t*		inPtrClient,
  GENIF_client_counter_t*	outPtrCounter
)
{
  static long			size = sizeof(GENIF_client_counter_t);
  
  int				ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_client_count(%p)", inPtrClient);

/*----------*/

  memcpy(outPtrCounter, inPtrClient->counter, size);

  memset(inPtrClient->counter, 0, size);

/*----------*/

  TRAZA1("Returning from GENIF_client_count() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static void 
GENIF_client_timer_cb(const struct Timer* inPtrTimer)
{
  GENIF_client_t*		inPtrClient = inPtrTimer->data;

  long				T = inPtrTimer->tv.tv_sec;
  
  GENIF_channel_t*		ptrChannel;

  GENIF_message_t*		ptrMessage;

  long				Tout = 0;

  int                           notify;

  int				endTout;

  long				Tm, Om, Wm, Im, Fi, Fo;

  TRAZA1("Entering in GENIF_client_timer_cb(%p)", inPtrClient);

/*----------*/

  inPtrClient->timer = setTimer(100, GENIF_client_timer_cb, inPtrClient);

  if(inPtrClient->timer == NULL)
  {
    GENIF_FATAL("ERROR: setTimer()");
  }

/*----------*/

  Fi = 0; Fo = 0; 

  RLST_resetGet(inPtrClient->channel, NULL);

  while((ptrChannel = RLST_getNext(inPtrClient->channel)))
  {
    Fi += ptrChannel->inFlowCount; Fo += ptrChannel->outFlowCount;
    
    GENIF_channel_timer_cb(ptrChannel, T);
  }

/*----------*/

  if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 1)
  {
    Tm = GENIF_client_message_num(inPtrClient, &Om, &Wm, &Im);

    PRUEBA9("%s:%d (%d) -> T=%ld, O=%ld, W=%ld, I=%ld, Fi=%ld, Fo=%ld",
	    inPtrClient->host,
	    inPtrClient->port,
	    inPtrClient->disabledOutput,
	    Tm, Om, Wm, Im, Fi, Fo); 
  }

/*----------*/

  if(inPtrClient->config.reconnectTout > 0)
  {
    if(inPtrClient->reconnectFlag == GENIF_FLAG_ON)
    {
      Tout = T - inPtrClient->config.reconnectTout;

      if(Tout >= inPtrClient->reconnectT)
      {
	GENIF_client_connect(inPtrClient);
      }
    }
  }

/*----------*/

  if(inPtrClient->config.outQueueTout > 0)
  {
    if(RLST_getNumElem(inPtrClient->outQueue))
    {
      Tout = T - inPtrClient->config.outQueueTout;

      RLST_resetGet(inPtrClient->outQueue, NULL); endTout = 0;

      while((ptrMessage = RLST_getHead(inPtrClient->outQueue)) && (!endTout))
      {
        if(Tout >= ptrMessage->T)
        {
	  RLST_extractHead(inPtrClient->outQueue);
  
	  if(ptrMessage->type == GENIF_MESSAGE_TYPE_MESSAGE)
	  {
	    inPtrClient->counter->channel.outMessageTimeout++;

	    notify = GENIF_NOTIFY_MESSAGE_TIMEOUT;

	    GENIF_client_notify(inPtrClient, notify, NULL, ptrMessage);
	  }

	  else
	  {
	    inPtrClient->counter->channel.outRequestTimeout++;

	    notify = GENIF_NOTIFY_REQUEST_TIMEOUT;
	  }

	  GENIF_client_notify(inPtrClient, notify, NULL, ptrMessage);
        }

        else {endTout = 1;}
      }
    }
  }

/*----------*/

  inPtrClient->outFlowCount = 0;

  GENIF_client_mask(inPtrClient);

/*----------*/

  if(inPtrClient->emptyFlag == GENIF_FLAG_ON)
  {
    if(GENIF_client_message_num(inPtrClient, NULL, NULL, NULL) == 0)
    {
      inPtrClient->emptyFlag = GENIF_FLAG_OFF;

      GENIF_client_notify(inPtrClient, GENIF_NOTIFY_CLIENT_EMPTY, NULL, NULL);
    }
  }
  
/*----------*/

  TRAZA0("Returning from GENIF_client_timer_cb()");
}

/*----------------------------------------------------------------------------*/

static int
GENIF_client_connect(GENIF_client_t* inPtrClient)
{
  long				num;

  int				rc;
  
  int				ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_client_connect(%p)", inPtrClient);

/*----------*/

  inPtrClient->reconnectFlag = GENIF_FLAG_OFF;

  inPtrClient->reconnectT = 0;

/*----------*/

  num = RLST_getNumElem(inPtrClient->channel);
  
  for( ; num < inPtrClient->config.channelMax; num++)
  {
    if(inPtrClient->localHP == GENIF_FLAG_ON)
    {
      rc = openSockChannelHP(NULL,
			     inPtrClient->config.connectTout * 100, 
                             inPtrClient->localPort,
			     NULL, NULL, GENIF_client_connect_cb, 
			     inPtrClient,
			     inPtrClient->host,
			     inPtrClient->localHost,
			     inPtrClient->port);

    }

    else
    {
      rc = openSockChannel(NULL, 
			   inPtrClient->config.connectTout * 100, 
			   NULL, NULL, GENIF_client_connect_cb, 
			   inPtrClient,
			   inPtrClient->host,
			   inPtrClient->port);
    }
  
    if(rc < 0)
    {
      SUCESO3("ERROR: OPENING CONNECTION (%s, %d) ERROR (%d)", 
	       inPtrClient->host, 
	       inPtrClient->port, rc);

      inPtrClient->reconnectFlag = GENIF_FLAG_ON;

      inPtrClient->reconnectT = time(NULL);

      ret = GENIF_RC_ERROR;
    }

    else
    {
      DEPURA2("OPENING CONNECTION (%s, %d)", 
	       inPtrClient->host, 
	       inPtrClient->port);

      inPtrClient->connecting++;

      inPtrClient->notDelete = GENIF_TRUE;
    }
  }

/*----------*/

  TRAZA1("Returning from GENIF_client_connect() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

static void
GENIF_client_connect_cb(int inFd, int inError, void* inPtrVoidClient)
{
  GENIF_client_t*		inPtrClient = inPtrVoidClient;

  GENIF_channel_t*		ptrChannel = NULL;
  
  int				rc;
  
  TRAZA2("Entering in GENIF_client_connect_cb(%p, %d)", inPtrClient, inFd);

/*----------*/

  inPtrClient->connecting--;

  if(inPtrClient->connecting <= 0)
  {
    inPtrClient->notDelete = GENIF_FALSE;
  }

/*----------*/

  if(inPtrClient->timer == NULL)
  {
    SUCESO1("WARNING: CONNECT CALLBACK FOR A FINALIZED CLIENT (%d)", inFd);

    forceEndChannel(inFd); close(inFd);
  }

  else if(inError == 0)
  {
    DEPURA3("CONNECTION (%s, %d) ESTABLISHED CHANNEL (%d)", 
	     inPtrClient->host, 
	     inPtrClient->port, inFd);

    inPtrClient->counter->channelOpen++;
    
    ptrChannel = GENIF_channel_new();

    rc = GENIF_channel_initialize(ptrChannel, inFd, inPtrClient,
                                  GENIF_CHANNEL_TYPE_CLIENT,
				  GENIF_client_channel_write_cb, 
				  GENIF_client_channel_notify_cb, 
		                 &inPtrClient->config.channel,
		                 &inPtrClient->counter->channel,
				  inPtrClient->modifier);

    if(rc != GENIF_RC_OK)
    {
      GENIF_FATAL("ERROR: GENIF_channel_initialize()");
    }
  
    rc = RLST_insertTail(inPtrClient->channel, ptrChannel);

    if(rc != RLST_RC_OK)
    {
      GENIF_FATAL("ERROR: RLST_insertTail()");
    }

    if(RLST_getNumElem(inPtrClient->outQueue) > 0)
    {
      GENIF_channel_external_queue(ptrChannel, GENIF_FLAG_ON);
    }

    GENIF_channel_read_disable(ptrChannel, inPtrClient->disabledInput);
    
    GENIF_channel_write_disable(ptrChannel, inPtrClient->disabledOutput);

    GENIF_client_notify(inPtrClient, GENIF_NOTIFY_CHANNEL_CONNECTED, 
                        ptrChannel, 
			NULL);
    
    if(RLST_getNumElem(inPtrClient->channel) == 1)
    {
      inPtrClient->disconnFlag = GENIF_FLAG_OFF;

      GENIF_client_notify(inPtrClient, GENIF_NOTIFY_CLIENT_CONNECTED, 
                          NULL,
			  NULL);
    }
  }
  
/*----------*/
  
  else
  {
    SUCESO3("ERROR: OPENING CONNECTION (%s, %d) ERROR (%d)", 
	     inPtrClient->host, 
	     inPtrClient->port, inError);

    forceEndChannel(inFd); close(inFd);

    inPtrClient->reconnectFlag = GENIF_FLAG_ON;
    
    inPtrClient->reconnectT = time(NULL); 

    if(RLST_getNumElem(inPtrClient->channel) == 0)
    {
      if(inPtrClient->disconnFlag == GENIF_FLAG_OFF)
      {
        inPtrClient->disconnFlag = GENIF_FLAG_ON;

        GENIF_client_notify(inPtrClient, GENIF_NOTIFY_CLIENT_DISCONNECTED, 
                            NULL,
			    NULL);
      }
    }
  }

/*----------*/

  TRAZA0("Returning from GENIF_client_connect_cb()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void
GENIF_client_channel_close
( 
  GENIF_client_t*		inPtrClient,
  GENIF_channel_t*		inPtrChannel
)
{
  TRAZA2("Entering in GENIF_client_channel_close(%p, %p)", 
          inPtrClient,
	  inPtrChannel);

/*----------*/

  if(RLST_extract(inPtrClient->channel, inPtrChannel))
  {
    GENIF_channel_finalize(inPtrChannel);

    GENIF_channel_delete(inPtrChannel);
    
    inPtrClient->reconnectFlag = GENIF_FLAG_ON;
    
    inPtrClient->reconnectT = time(NULL); 
  }

/*----------*/

  if(RLST_getNumElem(inPtrClient->channel) == 0)
  {
    inPtrClient->disconnFlag = GENIF_FLAG_ON;
  }

/*----------*/

  TRAZA0("Returning from GENIF_client_channel_close()");
}

/*----------------------------------------------------------------------------*/

static void
GENIF_client_channel_close_cb
( 
  GENIF_client_t*		inPtrClient,
  GENIF_channel_t*		inPtrChannel
)
{
  TRAZA2("Entering in GENIF_client_channel_close_cb(%p, %p)", 
          inPtrClient,
	  inPtrChannel);

/*----------*/

  if(RLST_extract(inPtrClient->channel, inPtrChannel))
  {
    GENIF_channel_finalize(inPtrChannel);

    GENIF_channel_delete(inPtrChannel);

    inPtrClient->reconnectFlag = GENIF_FLAG_ON;
    
    inPtrClient->reconnectT = time(NULL); 
  }

/*----------*/

  if(RLST_getNumElem(inPtrClient->channel) == 0)
  {
    if(inPtrClient->disconnFlag == GENIF_FLAG_OFF)
    {
      inPtrClient->disconnFlag = GENIF_FLAG_ON;

      GENIF_client_notify(inPtrClient, GENIF_NOTIFY_CLIENT_DISCONNECTED,
                          NULL,
			  NULL);
    }
  }

/*----------*/

  TRAZA0("Returning from GENIF_client_channel_close_cb()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void
GENIF_client_empty_notify(GENIF_client_t* inPtrClient)
{
  TRAZA1("Entering in GENIF_client_empty_notify(%p)", inPtrClient);

/*----------*/

  inPtrClient->emptyFlag = GENIF_FLAG_ON;

/*----------*/

  TRAZA0("Returning from GENIF_client_empty_notify()");
}

/*----------------------------------------------------------------------------*/

void 
GENIF_client_input_disable(GENIF_client_t* inPtrClient, int inFlag)
{
  TRAZA2("Entering in GENIF_client_input_disable(%p, %d)",
          inPtrClient,
	  inFlag);

/*----------*/

  if(inPtrClient->disabledInput != inFlag)
  {
    inPtrClient->disabledInput = inFlag;

    GENIF_client_mask(inPtrClient);
  }

/*----------*/

  TRAZA0("Returning from GENIF_client_input_disable()");
}

/*----------------------------------------------------------------------------*/

void 
GENIF_client_output_disable(GENIF_client_t* inPtrClient, int inFlag)
{
  TRAZA2("Entering in GENIF_client_output_disable(%p, %d)",
          inPtrClient,
	  inFlag);

/*----------*/

  if(inPtrClient->disabledOutput != inFlag)
  {
    inPtrClient->disabledOutput = inFlag;

    GENIF_client_mask(inPtrClient);
  }

/*----------*/

  TRAZA0("Returning from GENIF_client_output_disable()");
}

/*----------------------------------------------------------------------------*/

static void
GENIF_client_mask(GENIF_client_t* inPtrClient)
{
  int				inputMask;
  int				outputMask;
  
  GENIF_channel_t*		ptrChannel;

  TRAZA1("Entering in GENIF_client_mask(%p)", inPtrClient);

/*----------*/

  inputMask = GENIF_FLAG_ON;

  if(inPtrClient->disabledInput == GENIF_FLAG_ON)
  {
    inputMask = GENIF_FLAG_OFF;
  }

  if(inPtrClient->inputMask != inputMask)
  {
    inPtrClient->inputMask = inputMask;

    RLST_resetGet(inPtrClient->channel, NULL);
    
    if(inputMask == GENIF_FLAG_ON)
    {
      while((ptrChannel = RLST_getNext(inPtrClient->channel)))
      {
	GENIF_channel_read_disable(ptrChannel, GENIF_FLAG_OFF);
      }
    }

    else
    {
      while((ptrChannel = RLST_getNext(inPtrClient->channel)))
      {
	GENIF_channel_read_disable(ptrChannel, GENIF_FLAG_ON);
      }
    }
  }
  
/*----------*/

  outputMask = GENIF_FLAG_ON;

  if(inPtrClient->disabledOutput == GENIF_FLAG_ON)
  {
    outputMask = GENIF_FLAG_OFF;
  }

  if(inPtrClient->config.outRefMax > 0)
  {
    if(inPtrClient->outRefCount >= inPtrClient->config.outRefMax)
    {
      outputMask = GENIF_FLAG_OFF;
    }
  }

  if(inPtrClient->config.outFlowMax > 0)
  {
    if(inPtrClient->outFlowCount >= inPtrClient->config.outFlowMax)
    {
      outputMask = GENIF_FLAG_OFF;
    }
  }

  if(inPtrClient->outputMask != outputMask)
  {
    inPtrClient->outputMask = outputMask;
    
    RLST_resetGet(inPtrClient->channel, NULL);
    
    if(outputMask == GENIF_FLAG_ON)
    {
      while((ptrChannel = RLST_getNext(inPtrClient->channel)))
      {
	GENIF_channel_write_disable(ptrChannel, GENIF_FLAG_OFF);
      }
    }

    else
    {
      while((ptrChannel = RLST_getNext(inPtrClient->channel)))
      {
	GENIF_channel_write_disable(ptrChannel, GENIF_FLAG_ON);
      }
    }
  }
  
/*----------*/

  TRAZA0("Returning from GENIF_client_mask()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
GENIF_client_external_queue_set
(
  GENIF_client_t*	inPtrClient,
  GENIF_message_t*	(*inExtQueueFunc)(GENIF_client_t*, GENIF_channel_t*)
)
{
  int			ret = GENIF_RC_OK;

/*----------*/

  TRAZA2("Entering in GENIF_client_external_queue_set(%p, %p)", 
          inPtrClient, 
	  inExtQueueFunc);

/*----------*/

  inPtrClient->extQueueFunc = inExtQueueFunc;

/*----------*/

  TRAZA1("Returning from GENIF_client_external_queue_set() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
GENIF_client_external_queue(GENIF_client_t* inPtrClient, int inFlag)
{
  GENIF_channel_t*		ptrChannel;

  int				ret = GENIF_RC_OK;

/*----------*/

  TRAZA2("Entering in GENIF_client_external_queue(%p, %d)", 
          inPtrClient,
	  inFlag);

/*----------*/

  if(inPtrClient->extQueueFunc != NULL)
  {
    inPtrClient->extQueueFlag = inFlag;

    RLST_resetGet(inPtrClient->channel, NULL);

    while((ptrChannel = RLST_getNext(inPtrClient->channel)))
    {
      GENIF_channel_external_queue(ptrChannel, inFlag);
    }
  }

/*----------*/

  TRAZA1("Returning from GENIF_client_external_queue() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
GENIF_client_send
(
  GENIF_client_t*		inPtrClient,
  GENIF_message_t*		inPtrMessage
)
{
  long				T = time(NULL);
  
  int				rc;

  GENIF_channel_t*		ptrChannel;

  int				ret = GENIF_RC_OK;

/*----------*/

  TRAZA2("Entering in GENIF_client_send(%p, %p)", 
          inPtrClient, 
	  inPtrMessage);

/*----------*/

  inPtrClient->counter->channel.outMessage++;

/*----------*/

  if(inPtrClient->config.outQueueMax > 0)
  {
    if(RLST_getNumElem(inPtrClient->outQueue) >= inPtrClient->config.outQueueMax)
    {
      inPtrClient->counter->channel.outMessageError++;

      ret = GENIF_RC_QUEUE_FULL;
    }
  }

/*----------*/

  if(ret == GENIF_RC_OK)
  {
    inPtrMessage->type    = GENIF_MESSAGE_TYPE_MESSAGE;
    inPtrMessage->channel = NULL;
    inPtrMessage->T       = T;

    rc = RLST_insertTail(inPtrClient->outQueue, inPtrMessage);

    if(rc != RLST_RC_OK)
    {
      GENIF_FATAL("ERROR: RLST_insertTail()");
    }

    RLST_resetGet(inPtrClient->channel, NULL);
    
    while((ptrChannel = RLST_getNext(inPtrClient->channel)))
    {
      GENIF_channel_external_queue(ptrChannel, GENIF_FLAG_ON);
    }

    GENIF_client_mask(inPtrClient);
  }

/*----------*/

  TRAZA1("Returning from GENIF_client_send() = %d", ret);

  return ret;
}
    
/*----------------------------------------------------------------------------*/

int
GENIF_client_request
(
  GENIF_client_t*		inPtrClient,
  GENIF_message_t*		inPtrMessage
)
{
  long				T = time(NULL);
  
  int				rc;

  GENIF_channel_t*		ptrChannel;

  int				ret = GENIF_RC_OK;

/*----------*/

  TRAZA2("Entering in GENIF_client_request(%p, %p)", 
          inPtrClient, 
	  inPtrMessage);

/*----------*/

  inPtrClient->counter->channel.outRequest++;

/*----------*/

  if(inPtrClient->config.outQueueMax > 0)
  {
    if(RLST_getNumElem(inPtrClient->outQueue) >= inPtrClient->config.outQueueMax)
    {
      inPtrClient->counter->channel.outRequestError++;

      ret = GENIF_RC_QUEUE_FULL;
    }
  }

/*----------*/

  if(ret == GENIF_RC_OK)
  {
    inPtrMessage->type    = GENIF_MESSAGE_TYPE_REQUEST;
    inPtrMessage->channel = NULL;
    inPtrMessage->T       = T;

    rc = RLST_insertTail(inPtrClient->outQueue, inPtrMessage);

    if(rc != RLST_RC_OK)
    {
      GENIF_FATAL("ERROR: RLST_insertTail()");
    }

    RLST_resetGet(inPtrClient->channel, NULL);
    
    while((ptrChannel = RLST_getNext(inPtrClient->channel)))
    {
      GENIF_channel_external_queue(ptrChannel, GENIF_FLAG_ON);
    }

    GENIF_client_mask(inPtrClient);
  }

/*----------*/

  TRAZA1("Returning from GENIF_client_request() = %d", ret);

  return ret;
}
    
/*----------------------------------------------------------------------------*/

int
GENIF_client_prior_request
(
  GENIF_client_t*		inPtrClient,
  GENIF_message_t*		inPtrMessage
)
{
  long				T = time(NULL);
  
  int				rc;

  GENIF_channel_t*		ptrChannel;

  int				ret = GENIF_RC_OK;

/*----------*/

  TRAZA2("Entering in GENIF_client_prior_request(%p, %p)", 
          inPtrClient, 
	  inPtrMessage);

/*----------*/

  inPtrClient->counter->channel.outRequest++;

/*----------*/

  if(inPtrClient->config.outQueueMax > 0)
  {
    if(RLST_getNumElem(inPtrClient->outQueue) >= inPtrClient->config.outQueueMax)
    {
      inPtrClient->counter->channel.outRequestError++;

      ret = GENIF_RC_QUEUE_FULL;
    }
  }

/*----------*/

  if(ret == GENIF_RC_OK)
  {
    inPtrMessage->type    = GENIF_MESSAGE_TYPE_REQUEST;
    inPtrMessage->channel = NULL;
    inPtrMessage->T       = T;

    rc = RLST_insertHead(inPtrClient->outQueue, inPtrMessage);

    if(rc != RLST_RC_OK)
    {
      GENIF_FATAL("ERROR: RLST_insertHead()");
    }

    RLST_resetGet(inPtrClient->channel, NULL);
    
    while((ptrChannel = RLST_getNext(inPtrClient->channel)))
    {
      GENIF_channel_external_queue(ptrChannel, GENIF_FLAG_ON);
    }

    GENIF_client_mask(inPtrClient);
  }

/*----------*/

  TRAZA1("Returning from GENIF_client_prior_request() = %d", ret);

  return ret;
}
    
/*----------------------------------------------------------------------------*/

int
GENIF_client_channel_send
(
  GENIF_client_t*		inPtrClient,
  GENIF_channel_t* 		inPtrChannel,
  GENIF_message_t*		inPtrMessage
)
{
  int				ret = GENIF_RC_OK;

/*----------*/

  TRAZA3("Entering in GENIF_client_channel_send(%p, %p, %p)", 
          inPtrClient, 
          inPtrChannel, 
	  inPtrMessage);

/*----------*/

  if(inPtrChannel->parent == inPtrClient)
  {
    ret = GENIF_channel_send(inPtrChannel, inPtrMessage);
  }

  else
  {
    GENIF_FATAL("ASSERTION!!");
  }

/*----------*/

  TRAZA1("Returning from GENIF_client_channel_send() = %d", ret);

  return ret;
}
    
/*----------------------------------------------------------------------------*/

int
GENIF_client_channel_request
(
  GENIF_client_t*		inPtrClient,
  GENIF_channel_t* 		inPtrChannel,
  GENIF_message_t*		inPtrMessage
)
{
  int				ret = GENIF_RC_OK;

/*----------*/

  TRAZA3("Entering in GENIF_client_channel_request(%p, %p, %p)", 
          inPtrClient, 
          inPtrChannel, 
	  inPtrMessage);

/*----------*/

  if(inPtrChannel->parent == inPtrClient)
  {
    ret = GENIF_channel_request(inPtrChannel, inPtrMessage);
  }

  else
  {
    GENIF_FATAL("ASSERTION!!");
  }

/*----------*/

  TRAZA1("Returning from GENIF_client_channel_request() = %d", ret);

  return ret;
}
    
/*----------------------------------------------------------------------------*/

int
GENIF_client_reply
(
  GENIF_client_t*		inPtrClient,
  GENIF_message_t*		inPtrMessage
)
{
  int				ret = GENIF_RC_OK;

/*----------*/

  TRAZA2("Entering in GENIF_client_reply(%p, %p)", 
          inPtrClient, 
	  inPtrMessage);

/*----------*/

  if(inPtrMessage->channel != NULL)
  {
    if(inPtrMessage->channel->parent == inPtrClient)
    {
      ret = GENIF_channel_reply(inPtrMessage->channel, inPtrMessage);
    }

    else
    {
      GENIF_FATAL("ASSERTION!!");
    }
  }

  else
  {
    GENIF_FATAL("ASSERTION!!");
  }

/*----------*/

  TRAZA1("Returning from GENIF_client_reply() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
GENIF_client_acquire_msg
(
  GENIF_client_t*		inPtrClient,
  GENIF_message_t*		inPtrMessage
)
{
  int				ret = GENIF_RC_OK;

/*----------*/

  TRAZA2("Entering in GENIF_client_acquire_msg(%p, %p)", 
          inPtrClient, 
	  inPtrMessage);

/*----------*/

  if(inPtrMessage->channel != NULL)
  {
    if(inPtrMessage->channel->parent == inPtrClient)
    {
      ret = GENIF_channel_acquire_msg(inPtrMessage->channel, inPtrMessage);
    }

    else
    {
      ret = GENIF_RC_NOT_FOUND;
    }
  }

  else
  {
    ret = GENIF_RC_NOT_FOUND;
  }

/*----------*/

  TRAZA1("Returning from GENIF_client_acquire_msg() = %d", ret);

  return ret;
}
    
/*----------------------------------------------------------------------------*/

int
GENIF_client_cancel_msg
(
  GENIF_client_t*		inPtrClient,
  GENIF_message_t*		inPtrMessage,
  int*				outStore
)
{
  GENIF_message_t*		ptrMessage;

  int				ret = GENIF_RC_OK;

/*----------*/

  TRAZA2("Entering in GENIF_client_cancel_msg(%p, %p)", 
          inPtrClient, 
	  inPtrMessage);

/*----------*/

  ptrMessage = RLST_extract(inPtrClient->outQueue, inPtrMessage);

  if(ptrMessage != NULL)
  {
    inPtrClient->counter->channel.outRequestCanceled++;

    *outStore = GENIF_O_STORE;
  }

/*----------*/

  else if(inPtrMessage->channel != NULL)
  {
    if(inPtrMessage->channel->parent == inPtrClient)
    {
      ret = GENIF_channel_cancel_msg(inPtrMessage->channel, 
				     inPtrMessage, 
				     outStore);

      if((ret == GENIF_RC_OK) && (*outStore == GENIF_W_STORE))
      {
	inPtrClient->outRefCount--; 

	GENIF_client_mask(inPtrClient);
      }
    }

    else
    {
      ret = GENIF_RC_NOT_FOUND;
    }
  }

  else
  {
    ret = GENIF_RC_NOT_FOUND;
  }

/*----------*/

  TRAZA1("Returning from GENIF_client_cancel_msg() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

long
GENIF_client_message_num
(
  GENIF_client_t*		inPtrClient,
  long*				outOstore,
  long*				outWstore,
  long*				outIstore
)
{
  GENIF_channel_t*		ptrChannel;
 
  long				msgT = 0;
  
  long				msgO = 0;
  long				msgW = 0;
  long				msgI = 0;
  
  TRAZA1("Entering in GENIF_client_message_num(%p)", inPtrClient);

/*----------*/

  if(outOstore != NULL) *outOstore = 0; 
  if(outWstore != NULL) *outWstore = 0; 
  if(outIstore != NULL) *outIstore = 0; 

/*----------*/

  msgT = RLST_getNumElem(inPtrClient->outQueue);

  if(outOstore != NULL) *outOstore += msgT;

/*----------*/

  RLST_resetGet(inPtrClient->channel, NULL);

  while((ptrChannel = RLST_getNext(inPtrClient->channel)))
  {
    msgT += GENIF_channel_message_num(ptrChannel, &msgO, &msgW, &msgI);

    if(outOstore != NULL) *outOstore += msgO;
    if(outWstore != NULL) *outWstore += msgW;
    if(outIstore != NULL) *outIstore += msgI;
  }

/*----------*/

  TRAZA1("Returning from GENIF_client_message_num() = %ld", msgT);

  return msgT;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static GENIF_message_t* 
GENIF_client_channel_write_cb(GENIF_channel_t* inPtrChannel)
{
  GENIF_client_t*		inPtrClient = inPtrChannel->parent;

  GENIF_message_t*		ptrMsgChn = NULL;
  GENIF_message_t*		ptrMsgCli = NULL;

  GENIF_message_t*		ptrMessage = NULL;

/*----------*/

  TRAZA2("Entering in GENIF_client_channel_write_cb(%p, %p)", 
          inPtrChannel, 
	  inPtrClient);

/*----------*/

  ptrMsgChn = RLST_getHead(inPtrChannel->outQueue);
  ptrMsgCli = RLST_getHead(inPtrClient->outQueue);

  if(ptrMsgCli == NULL)
  {
    if(inPtrClient->extQueueFlag == GENIF_FLAG_ON)
    {
      if(inPtrClient->extQueueFunc != NULL)
      {
        ptrMessage = inPtrClient->extQueueFunc(inPtrClient, inPtrChannel);
      }

      if(ptrMessage == NULL)
      {
        inPtrClient->extQueueFlag = GENIF_FLAG_OFF;
      }

      else { inPtrChannel->counter->outRequest++; }
    }
  }

  else if(ptrMsgChn == NULL)
  {
    ptrMessage = RLST_extractHead(inPtrClient->outQueue);
  }

  else if(ptrMsgCli->T < ptrMsgChn->T)
  {
    ptrMessage = RLST_extractHead(inPtrClient->outQueue);
  }

  else if(ptrMsgCli->O < ptrMsgChn->O)
  {
    ptrMessage = RLST_extractHead(inPtrClient->outQueue);
  }

  else { ptrMessage = RLST_extractHead(inPtrChannel->outQueue); }

/*----------*/

  TRAZA1("Returning from GENIF_client_channel_write_cb() = %p", ptrMessage);

  return ptrMessage;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static void 
GENIF_client_channel_notify_cb
(
  GENIF_channel_t*		inPtrChannel,
  int				inNotify,
  GENIF_message_t*		inPtrMessage
)
{
  GENIF_client_t*		inPtrClient = inPtrChannel->parent;

/*----------*/

  TRAZA3("Entering in GENIF_client_channel_notify_cb(%p, %p, %d)", 
          inPtrChannel, 
	  inPtrMessage, 
	  inNotify);

/*----------*/

  if(inNotify < GENIF_NOTIFY_TOTAL)
  {
    DEPURA1("%s", GENIF_notify_name[inNotify]);

    switch(inNotify)
    {

/*----------*/

      case GENIF_NOTIFY_REQUEST_ERROR:
      case GENIF_NOTIFY_REQUEST_CLOSED:
      case GENIF_NOTIFY_REQUEST_TIMEOUT:
      {
        GENIF_client_notify(inPtrClient, inNotify, inPtrChannel, inPtrMessage);
	
        break;
      }
      
      case GENIF_NOTIFY_REQUEST_SENT:
      {
        inPtrClient->outRefCount++; inPtrClient->outFlowCount++;

	GENIF_client_mask(inPtrClient);

        GENIF_client_notify(inPtrClient, inNotify, inPtrChannel, inPtrMessage);
	
        break;
      }
      
      case GENIF_NOTIFY_REQUEST_SENT_CLOSED:
      case GENIF_NOTIFY_REQUEST_SENT_TIMEOUT:
      {
        inPtrClient->outRefCount--;

	GENIF_client_mask(inPtrClient);
	
        GENIF_client_notify(inPtrClient, inNotify, inPtrChannel, inPtrMessage);
	
        break;
      }
      
      case GENIF_NOTIFY_RESPONSE_RECEIVED:
      {
        inPtrClient->outRefCount--;

	GENIF_client_mask(inPtrClient);
	
        GENIF_client_notify(inPtrClient, inNotify, inPtrChannel, inPtrMessage);
	
        break;
      }
      
/*----------*/

      case GENIF_NOTIFY_REQUEST_RECEIVED:
      case GENIF_NOTIFY_REQUEST_RECEIVED_CLOSED:
      case GENIF_NOTIFY_REQUEST_RECEIVED_TIMEOUT:
      case GENIF_NOTIFY_RESPONSE_ERROR:
      case GENIF_NOTIFY_RESPONSE_CLOSED:
      case GENIF_NOTIFY_RESPONSE_TIMEOUT:
      case GENIF_NOTIFY_RESPONSE_SENT:
      {
        GENIF_client_notify(inPtrClient, inNotify, inPtrChannel, inPtrMessage);
	
        break;
      }

/*----------*/

      case GENIF_NOTIFY_MESSAGE_ERROR:
      case GENIF_NOTIFY_MESSAGE_CLOSED:
      case GENIF_NOTIFY_MESSAGE_TIMEOUT:
      {
        GENIF_client_notify(inPtrClient, inNotify, inPtrChannel, inPtrMessage);
	
        break;
      }
      
      case GENIF_NOTIFY_MESSAGE_SENT:
      {
        inPtrClient->outFlowCount++;

	GENIF_client_mask(inPtrClient);

        GENIF_client_notify(inPtrClient, inNotify, inPtrChannel, inPtrMessage);
	
        break;
      }
      

      case GENIF_NOTIFY_MESSAGE_RECEIVED:
      case GENIF_NOTIFY_MESSAGE_RECEIVED_CLOSED:
      case GENIF_NOTIFY_MESSAGE_RECEIVED_TIMEOUT:
      {
        GENIF_client_notify(inPtrClient, inNotify, inPtrChannel, inPtrMessage);
	
        break;
      }

/*----------*/

      case GENIF_NOTIFY_UNKNOWN_RECEIVED:
      case GENIF_NOTIFY_UNKNOWN_RECEIVED_CLOSED:
      case GENIF_NOTIFY_UNKNOWN_RECEIVED_TIMEOUT:
      {
        GENIF_client_notify(inPtrClient, inNotify, inPtrChannel, inPtrMessage);
	
        break;
      }
      
/*----------*/

      case GENIF_NOTIFY_CHANNEL_HEARTBEAT:
      {
        GENIF_client_notify(inPtrClient, inNotify, inPtrChannel, inPtrMessage);
	
	break;
      }

      case GENIF_NOTIFY_CHANNEL_INACTIVITY:
      {
	SUCESO3("WARNING: CONNECTION (%s, %d) CHANNEL (%d) INACTIVITY", 
		 inPtrClient->host,
		 inPtrClient->port,
		 inPtrChannel->fd);

        GENIF_channel_empty(inPtrChannel, GENIF_END_CLOSE);

        GENIF_client_notify(inPtrClient, inNotify, inPtrChannel, inPtrMessage);

        GENIF_client_channel_close_cb(inPtrClient, inPtrChannel);

	break;
      }

      case GENIF_NOTIFY_CHANNEL_MALFUNCTION:
      {
	SUCESO3("WARNING: CONNECTION (%s, %d) CHANNEL (%d) MALFUNCTION", 
		 inPtrClient->host,
		 inPtrClient->port,
		 inPtrChannel->fd);

        GENIF_channel_empty(inPtrChannel, GENIF_END_CLOSE);

        GENIF_client_notify(inPtrClient, inNotify, inPtrChannel, inPtrMessage);

        GENIF_client_channel_close_cb(inPtrClient, inPtrChannel);

	break;
      }

      case GENIF_NOTIFY_CHANNEL_ERROR:
      case GENIF_NOTIFY_CHANNEL_CLOSED:
      {
        GENIF_client_notify(inPtrClient, inNotify, inPtrChannel, inPtrMessage);

        GENIF_client_channel_close_cb(inPtrClient, inPtrChannel);
	
	break;
      }

/*----------*/

      default:
      {
        SUCESO1("ERROR: UNEXPECTED %s", GENIF_notify_name[inNotify]);

        GENIF_FATAL("ASSERTION!!");

	break;
      }
    }
  }

/*----------*/

  else
  {
    SUCESO1("ERROR: UNKNOWN (%d)", inNotify);

    GENIF_FATAL("ASSERTION!!");
  }

/*----------*/

  TRAZA0("Returning from GENIF_client_channel_notify_cb()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static void 
GENIF_client_notify
(
  GENIF_client_t*		inPtrCli,
  int				inNotify,
  GENIF_channel_t*		inPtrChn,
  GENIF_message_t*		inPtrMsg
)
{
  inPtrCli->notDelete = GENIF_TRUE;

  inPtrCli->cbNotify(inPtrCli, inNotify, inPtrChn, inPtrMsg);

  if(inPtrCli->connecting <= 0)
  {
    inPtrCli->notDelete = GENIF_FALSE;
  }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

