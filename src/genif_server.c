/*____________________________________________________________________________

  MODULO:	Servidor protocolo peticio'n respuesta generico

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

static void GENIF_server_accept_cb
( 
  int				inFd,
  struct sockaddr*		inSockAddr,
  void*				inPtrVoidServer
);

static void GENIF_server_timer_cb(const struct Timer* inPtrTimer);

static void GENIF_server_channel_close_cb
( 
  GENIF_server_t*		inPtrServer,
  GENIF_channel_t*		inPtrChannel
);

static void GENIF_server_channel_notify_cb
(
  GENIF_channel_t*		inPtrChannel,
  int				inNotify,
  GENIF_message_t*		inPtrMessage
);

static void GENIF_server_notify
(
  GENIF_server_t*		inPtrSrv,
  int				inNotify,
  GENIF_channel_t*		inPtrChn,
  GENIF_message_t*		inPtrMsg
);

/*----------*/

/*__FUNCIONES PUBLICAS________________________________________________________*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
GENIF_server_initialize
(
  GENIF_server_t*		inPtrServer,
  int				inPort,
  void*				inPtrParent,
  void				(*inCbNotify)(GENIF_server_t*, int,
                                              GENIF_channel_t*,
				              GENIF_message_t*),
  GENIF_server_config_t*	inPtrConfig,
  GENIF_modifier_t*		inPtrModifier
)
{
  GENIF_channel_t		channel;

  int				rc;

  int				ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_server_initialize(%p)", inPtrServer);

/*----------*/
  GENIF_factory_initialize();
/*----------*/

  if(ret == GENIF_RC_OK)
  {
    inPtrServer->port = inPort;

    inPtrServer->listenFd = -1;

    inPtrServer->parent   = inPtrParent;
    inPtrServer->cbNotify = inCbNotify;

    inPtrServer->modifier = inPtrModifier;

    inPtrServer->extRef = NULL;

    inPtrServer->timer = NULL;

    inPtrServer->disconnFlag = GENIF_FLAG_OFF;
    
//  inPtrServer->channel_;
    inPtrServer->channel = NULL;

    inPtrServer->extQueueFunc = NULL;
    inPtrServer->extQueueFlag = GENIF_FLAG_OFF;

    inPtrServer->emptyFlag = GENIF_FLAG_OFF;
    
    inPtrServer->disabledListen = GENIF_FLAG_OFF;

    inPtrServer->disabledInput  = GENIF_FLAG_OFF;
    inPtrServer->disabledOutput = GENIF_FLAG_OFF;

    memcpy(&inPtrServer->config, inPtrConfig, sizeof(GENIF_server_config_t));
    memset(&inPtrServer->counter_, 0, sizeof(GENIF_server_counter_t));

    inPtrServer->counter = &inPtrServer->counter_;

    inPtrServer->notDelete = 0;
  }

/*----------*/

  if(ret == GENIF_RC_OK)
  {
    inPtrServer->timer = setTimer(1, GENIF_server_timer_cb, inPtrServer);

    if(inPtrServer->timer == NULL)
    {
      GENIF_FATAL("ERROR: setTimer()");
    }
  }

/*----------*/

  if(ret == GENIF_RC_OK)
  {
    inPtrServer->channel = &inPtrServer->channel_;

    rc = RLST_initializeRefList(inPtrServer->channel, &channel, &channel.list);

    if(rc < 0)
    {
      GENIF_FATAL("ERROR: RLST_initializeRefList()");
    }
  }
  
/*----------*/

  if(ret == GENIF_RC_OK)
  {
    inPtrServer->listenFd = listenToPort(inPort, NULL, NULL, 
                                         GENIF_server_accept_cb, 
					 inPtrServer);

    if(inPtrServer->listenFd < 0)
    {
      SUCESO1("ERROR: listenToPort() = %d", inPtrServer->listenFd);

      GENIF_server_finalize(inPtrServer); 

      ret = GENIF_RC_ERROR;
    }
  }
  
/*----------*/

  TRAZA1("Returning from GENIF_server_initialize() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int 
GENIF_server_finalize(GENIF_server_t* inPtrServer)
{
  GENIF_channel_t*		ptrChannel;
  
  int				ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_server_final(%p)", inPtrServer);

/*----------*/

  if(inPtrServer->listenFd != -1)
  {
    endListenToPort(inPtrServer->listenFd); 

    inPtrServer->listenFd = -1;
  }

/*----------*/

  if(inPtrServer->channel != NULL)
  {
    while((ptrChannel = RLST_extractHead(inPtrServer->channel)))
    {
      if(GENIF_channel_finalize(ptrChannel) != GENIF_RC_OK)
      {
        SUCESO0("ERROR: GENIF_channel_finalize()");
      }

      GENIF_channel_delete(ptrChannel);
    }
  }
  
/*----------*/

  if(inPtrServer->timer != NULL)
  {
    if(cancelTimer(inPtrServer->timer))
    {
      SUCESO0("ERROR: cancelTimer()");
    }

    inPtrServer->timer = NULL;
  }

/*----------*/

  TRAZA0("Returning from GENIF_server_final()");

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
GENIF_server_reconfig
(
  GENIF_server_t*		inPtrServer,
  GENIF_server_config_t*	inPtrConfig
)
{
  int				ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_server_reconfig(%p)", inPtrServer);

/*----------*/

  memcpy(&inPtrServer->config, inPtrConfig, sizeof(GENIF_server_config_t));

/*----------*/

  TRAZA1("Returning from GENIF_server_reconfig() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
GENIF_server_count
(
  GENIF_server_t*		inPtrServer,
  GENIF_server_counter_t*	outPtrCounter
)
{
  static long			size = sizeof(GENIF_server_counter_t);
  
  int				ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_server_count(%p)", inPtrServer);

/*----------*/

  memcpy(outPtrCounter, inPtrServer->counter, size);

  memset(inPtrServer->counter, 0, size);

/*----------*/

  TRAZA1("Returning from GENIF_server_count() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static void 
GENIF_server_timer_cb(const struct Timer* inPtrTimer)
{
  GENIF_server_t*		inPtrServer = inPtrTimer->data;

  long				T = inPtrTimer->tv.tv_sec;
  
  GENIF_channel_t*		ptrChannel;
  
  long				Tm, Om, Wm, Im, Fi, Fo;

  TRAZA1("Entering in GENIF_server_timer_cb(%p)", inPtrServer);

/*----------*/

  inPtrServer->timer = setTimer(100, GENIF_server_timer_cb, inPtrServer);

  if(inPtrServer->timer == NULL)
  {
    GENIF_FATAL("ERROR: setTimer()");
  }

/*----------*/

  Fi = 0; Fo = 0; RLST_resetGet(inPtrServer->channel, NULL);

  while((ptrChannel = RLST_getNext(inPtrServer->channel)))
  {
    Fi += ptrChannel->inFlowCount; Fo += ptrChannel->outFlowCount;
    
    GENIF_channel_timer_cb(ptrChannel, T);
  }

/*----------*/

  if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 1)
  {
    Tm = GENIF_server_message_num(inPtrServer, &Om, &Wm, &Im);

    PRUEBA7("localhost:%d -> T=%ld, O=%ld, W=%ld, I=%ld, Fi=%ld, Fo=%ld",
	     inPtrServer->port, Tm, Om, Wm, Im, Fi, Fo); 
  }

/*----------*/

  if(inPtrServer->emptyFlag == GENIF_FLAG_ON)
  {
    if(GENIF_server_message_num(inPtrServer, NULL, NULL, NULL) == 0)
    {
      inPtrServer->emptyFlag = GENIF_FLAG_OFF;

      GENIF_server_notify(inPtrServer, GENIF_NOTIFY_SERVER_EMPTY, NULL, NULL);
    }
  }
  
/*----------*/

  TRAZA0("Returning from GENIF_server_timer_cb()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static void
GENIF_server_accept_cb
( 
  int				inFd,
  struct sockaddr*		inSockAddr,
  void*				inPtrVoidServer
)
{
  GENIF_server_t*		inPtrServer = inPtrVoidServer;

  GENIF_channel_t*		ptrChannel = NULL;

  struct sockaddr_in		addr;
  int				port;

  int				rc;

  TRAZA1("Entering in GENIF_server_accept_cb(%p)", inPtrServer);

/*----------*/

  memcpy(&addr, inSockAddr, sizeof(struct sockaddr_in));

  port = ntohs(addr.sin_port);

/*----------*/

  if((inPtrServer->config.channelMax >  0) &&
     (inPtrServer->config.channelMax <= RLST_getNumElem(inPtrServer->channel)))
  {
    SUCESO1("WARNING: REACHED MAXNUM OF CHANNELS %ld", 
             inPtrServer->config.channelMax);

    SUCESO3("WARNING: REFUSED CONNECTION FROM (%s, %d) CHANNEL (%d)", 
	     inet_ntoa(addr.sin_addr), port, inFd);

    forceEndChannel(inFd); close(inFd);
  }

/*----------*/

  else if(inPtrServer->disabledListen)
  {
    SUCESO0("WARNING: SERVER LISTEN DISABLED");

    SUCESO3("WARNING: REFUSED CONNECTION FROM (%s, %d) CHANNEL (%d)", 
	     inet_ntoa(addr.sin_addr), port, inFd);

    forceEndChannel(inFd); close(inFd); 
  }

/*----------*/

  else
  {
    inPtrServer->counter->channelOpen++;
    
    ptrChannel = GENIF_channel_new();

    rc = GENIF_channel_initialize(ptrChannel, inFd, inPtrServer,
                                  GENIF_CHANNEL_TYPE_SERVER,
				  NULL,
		                  GENIF_server_channel_notify_cb, 
		                 &inPtrServer->config.channel,
		                 &inPtrServer->counter->channel,
				  inPtrServer->modifier);


    if(rc != GENIF_RC_OK)
    {
      GENIF_FATAL("ERROR: GENIF_channel_initialize()");
    }
    
    rc = RLST_insertTail(inPtrServer->channel, ptrChannel);

    if(rc != RLST_RC_OK)
    {
      GENIF_FATAL("ERROR: RLST_insertTail()");
    }

    strncpy(ptrChannel->remHost, inet_ntoa(addr.sin_addr), GENIF_MAXLEN_HOST);

    ptrChannel->remHost[GENIF_MAXLEN_HOST] = 0;

    ptrChannel->remPort = port;

    GENIF_channel_read_disable(ptrChannel, inPtrServer->disabledInput);

    GENIF_channel_write_disable(ptrChannel, inPtrServer->disabledOutput);

    DEPURA3("ACCEPTED CONNECTION FROM (%s, %d) CHANNEL (%d)",
             ptrChannel->remHost,
             ptrChannel->remPort,
	     ptrChannel->fd);
	     
    GENIF_server_notify(inPtrServer, GENIF_NOTIFY_CHANNEL_ACCEPTED,
                        ptrChannel,
			NULL);

    if(RLST_getNumElem(inPtrServer->channel) == 1)
    {
      inPtrServer->disconnFlag = GENIF_FLAG_OFF;

      GENIF_server_notify(inPtrServer, GENIF_NOTIFY_SERVER_CONNECTED, 
                          NULL,
			  NULL);
    }
  }

/*----------*/

  TRAZA0("Returning from GENIF_server_accept_cb()");
}

/*----------------------------------------------------------------------------*/

void
GENIF_server_channel_close
( 
  GENIF_server_t*		inPtrServer,
  GENIF_channel_t*		inPtrChannel
)
{
  TRAZA2("Entering in GENIF_server_channel_close(%p, %p)", 
          inPtrServer,
	  inPtrChannel);

/*----------*/

  if(RLST_extract(inPtrServer->channel, inPtrChannel))
  {
    GENIF_channel_finalize(inPtrChannel);
    
    GENIF_channel_delete(inPtrChannel);
  }

/*----------*/

  if(RLST_getNumElem(inPtrServer->channel) == 0)
  {
    inPtrServer->disconnFlag = GENIF_FLAG_ON;
  }

/*----------*/

  TRAZA0("Returning from GENIF_server_channel_close()");
}

/*----------------------------------------------------------------------------*/

static void
GENIF_server_channel_close_cb
( 
  GENIF_server_t*		inPtrServer,
  GENIF_channel_t*		inPtrChannel
)
{
  TRAZA2("Entering in GENIF_server_channel_close_cb(%p, %p)", 
          inPtrServer,
	  inPtrChannel);

/*----------*/

  if(RLST_extract(inPtrServer->channel, inPtrChannel))
  {
    GENIF_channel_finalize(inPtrChannel);
    
    GENIF_channel_delete(inPtrChannel);
  }

/*----------*/

  if(RLST_getNumElem(inPtrServer->channel) == 0)
  {
    if(inPtrServer->disconnFlag == GENIF_FLAG_OFF)
    {
      inPtrServer->disconnFlag = GENIF_FLAG_ON;

      GENIF_server_notify(inPtrServer, GENIF_NOTIFY_SERVER_DISCONNECTED, 
                          NULL,
			  NULL);
    }
  }

/*----------*/

  TRAZA0("Returning from GENIF_server_channel_close_cb()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void
GENIF_server_empty_notify(GENIF_server_t* inPtrServer)
{
  TRAZA1("Entering in GENIF_server_empty_notify(%p)", inPtrServer);

/*----------*/

  inPtrServer->emptyFlag = GENIF_FLAG_ON;

/*----------*/

  TRAZA0("Returning from GENIF_server_empty_notify()");
}

/*----------------------------------------------------------------------------*/

void 
GENIF_server_listen_disable(GENIF_server_t* inPtrServer, int inFlag)
{
  TRAZA2("Entering in GENIF_server_listen_disable(%p, %d)",
          inPtrServer,
	  inFlag);

/*----------*/

  inPtrServer->disabledListen = inFlag;

/*----------*/

  TRAZA0("Returning from GENIF_server_listen_disable()");
}

/*----------------------------------------------------------------------------*/

void 
GENIF_server_input_disable(GENIF_server_t* inPtrServer, int inFlag)
{
  GENIF_channel_t*		ptrChannel = NULL;

  TRAZA2("Entering in GENIF_server_input_disable(%p, %d)",
          inPtrServer,
	  inFlag);

/*----------*/

  if(inPtrServer->disabledInput != inFlag)
  {
    inPtrServer->disabledInput = inFlag;

    RLST_resetGet(inPtrServer->channel, NULL);
    
    while((ptrChannel = RLST_getNext(inPtrServer->channel)))
    {
      GENIF_channel_read_disable(ptrChannel, inFlag);
    }
  }

/*----------*/

  TRAZA0("Returning from GENIF_server_input_disable()");
}

/*----------------------------------------------------------------------------*/

void 
GENIF_server_output_disable(GENIF_server_t* inPtrServer, int inFlag)
{
  GENIF_channel_t*		ptrChannel = NULL;

  TRAZA2("Entering in GENIF_server_output_disable(%p, %d)",
          inPtrServer,
	  inFlag);

/*----------*/

  if(inPtrServer->disabledOutput != inFlag)
  {
    inPtrServer->disabledOutput = inFlag;

    RLST_resetGet(inPtrServer->channel, NULL);
    
    while((ptrChannel = RLST_getNext(inPtrServer->channel)))
    {
      GENIF_channel_write_disable(ptrChannel, inFlag);
    }
  }

/*----------*/

  TRAZA0("Returning from GENIF_server_output_disable()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
GENIF_server_external_queue_set
(
  GENIF_server_t*	inPtrServer,
  GENIF_message_t*	(*inExtQueueFunc)(GENIF_server_t*, GENIF_channel_t*)
)
{
  int			ret = GENIF_RC_OK;

/*----------*/

  TRAZA2("Entering in GENIF_server_external_queue_set(%p, %p)", 
          inPtrServer, 
	  inExtQueueFunc);

/*----------*/

  inPtrServer->extQueueFunc = inExtQueueFunc;

/*----------*/

  TRAZA1("Returning from GENIF_server_external_queue_set() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
GENIF_server_external_queue(GENIF_server_t* inPtrServer, int inFlag)
{
  GENIF_channel_t*		ptrChannel;

  int				ret = GENIF_RC_OK;

/*----------*/

  TRAZA2("Entering in GENIF_server_external_queue(%p, %d)", 
          inPtrServer,
	  inFlag);

/*----------*/

  if(inPtrServer->extQueueFunc != NULL)
  {
    inPtrServer->extQueueFlag = inFlag;

    RLST_resetGet(inPtrServer->channel, NULL);

    while((ptrChannel = RLST_getNext(inPtrServer->channel)))
    {
      GENIF_channel_external_queue(ptrChannel, inFlag);
    }
  }

/*----------*/

  TRAZA1("Returning from GENIF_server_external_queue() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
GENIF_server_channel_send
(
  GENIF_server_t*		inPtrServer,
  GENIF_channel_t* 		inPtrChannel,
  GENIF_message_t*		inPtrMessage
)
{
  int				ret = GENIF_RC_OK;

/*----------*/

  TRAZA3("Entering in GENIF_server_channel_send(%p, %p, %p)", 
          inPtrServer, 
          inPtrChannel, 
	  inPtrMessage);

/*----------*/

  if(inPtrChannel->parent == inPtrServer)
  {
    ret = GENIF_channel_send(inPtrChannel, inPtrMessage);
  }

  else
  {
    GENIF_FATAL("ASSERTION!!");
  }

/*----------*/

  TRAZA1("Returning from GENIF_server_channel_send() = %d", ret);

  return ret;
}
    
/*----------------------------------------------------------------------------*/

int
GENIF_server_channel_request
(
  GENIF_server_t*		inPtrServer,
  GENIF_channel_t* 		inPtrChannel,
  GENIF_message_t*		inPtrMessage
)
{
  int				ret = GENIF_RC_OK;

/*----------*/

  TRAZA3("Entering in GENIF_server_channel_request(%p, %p, %p)", 
          inPtrServer, 
          inPtrChannel, 
	  inPtrMessage);

/*----------*/

  if(inPtrChannel->parent == inPtrServer)
  {
    ret = GENIF_channel_request(inPtrChannel, inPtrMessage);
  }

  else
  {
    GENIF_FATAL("ASSERTION!!");
  }

/*----------*/

  TRAZA1("Returning from GENIF_server_channel_request() = %d", ret);

  return ret;
}
    
/*----------------------------------------------------------------------------*/

int
GENIF_server_reply
(
  GENIF_server_t*		inPtrServer,
  GENIF_message_t*		inPtrMessage
)
{
  int				ret = GENIF_RC_OK;

/*----------*/

  TRAZA2("Entering in GENIF_server_reply(%p, %p)", 
          inPtrServer, 
	  inPtrMessage);

/*----------*/

  if(inPtrMessage->channel != NULL)
  {
    if(inPtrMessage->channel->parent == inPtrServer)
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

  TRAZA1("Returning from GENIF_server_reply() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
GENIF_server_acquire_msg
(
  GENIF_server_t*		inPtrServer,
  GENIF_message_t*		inPtrMessage
)
{
  int				ret = GENIF_RC_OK;

/*----------*/

  TRAZA2("Entering in GENIF_server_acquire_msg(%p, %p)", 
          inPtrServer, 
	  inPtrMessage);

/*----------*/

  if(inPtrMessage->channel != NULL)
  {
    if(inPtrMessage->channel->parent == inPtrServer)
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

  TRAZA1("Returning from GENIF_server_acquire_msg() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
GENIF_server_cancel_msg
(
  GENIF_server_t*		inPtrServer,
  GENIF_message_t*		inPtrMessage,
  int*				outStore
)
{
  int				ret = GENIF_RC_OK;

/*----------*/

  TRAZA2("Entering in GENIF_server_cancel_msg(%p, %p)", 
          inPtrServer, 
	  inPtrMessage);

/*----------*/

  if(inPtrMessage->channel != NULL)
  {
    if(inPtrMessage->channel->parent == inPtrServer)
    {
      ret = GENIF_channel_cancel_msg(inPtrMessage->channel, 
                                     inPtrMessage, 
				     outStore);
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

  TRAZA1("Returning from GENIF_server_cancel_msg() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

long
GENIF_server_message_num
(
  GENIF_server_t*		inPtrServer,
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

  TRAZA1("Entering in GENIF_server_message_num(%p)", inPtrServer);

/*----------*/

  if(outOstore != NULL) *outOstore = 0; 
  if(outWstore != NULL) *outWstore = 0; 
  if(outIstore != NULL) *outIstore = 0; 

/*----------*/

  RLST_resetGet(inPtrServer->channel, NULL);

  while((ptrChannel = RLST_getNext(inPtrServer->channel)))
  {
    msgO = 0; msgW = 0; msgO = 0;
    
    msgT += GENIF_channel_message_num(ptrChannel, &msgO, &msgW, &msgI);

    if(outOstore != NULL) *outOstore += msgO;
    if(outWstore != NULL) *outWstore += msgW;
    if(outIstore != NULL) *outIstore += msgI;
  }

/*----------*/

  TRAZA1("Returning from GENIF_server_message_num() = %ld", msgT);

  return msgT;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

long
GENIF_server_channel_num(GENIF_server_t* inPtrServer)
{
  long				channelNum = 0;

  TRAZA1("Entering in GENIF_server_channel_num(%p)", inPtrServer);

/*----------*/

  channelNum = RLST_getNumElem(inPtrServer->channel);

/*----------*/

  TRAZA1("Returning from GENIF_server_channel_num() = %ld", channelNum);

  return channelNum;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static void 
GENIF_server_channel_notify_cb
(
  GENIF_channel_t*		inPtrChannel,
  int				inNotify,
  GENIF_message_t*		inPtrMessage
)
{
  GENIF_server_t*		inPtrServer = inPtrChannel->parent;

/*----------*/

  TRAZA3("Entering in GENIF_server_channel_notify_cb(%p, %d, %p)",
          inPtrChannel,
	  inNotify,
	  inPtrMessage);

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
      case GENIF_NOTIFY_REQUEST_SENT:
      case GENIF_NOTIFY_REQUEST_SENT_CLOSED:
      case GENIF_NOTIFY_REQUEST_SENT_TIMEOUT:
      case GENIF_NOTIFY_RESPONSE_RECEIVED:
      {
        GENIF_server_notify(inPtrServer, inNotify, inPtrChannel, inPtrMessage);

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
        GENIF_server_notify(inPtrServer, inNotify, inPtrChannel, inPtrMessage);

        break;
      }

/*----------*/

      case GENIF_NOTIFY_MESSAGE_ERROR:
      case GENIF_NOTIFY_MESSAGE_CLOSED:
      case GENIF_NOTIFY_MESSAGE_TIMEOUT:
      case GENIF_NOTIFY_MESSAGE_SENT:
      case GENIF_NOTIFY_MESSAGE_RECEIVED:
      case GENIF_NOTIFY_MESSAGE_RECEIVED_CLOSED:
      case GENIF_NOTIFY_MESSAGE_RECEIVED_TIMEOUT:
      {
        GENIF_server_notify(inPtrServer, inNotify, inPtrChannel, inPtrMessage);
	
        break;
      }

/*----------*/

      case GENIF_NOTIFY_UNKNOWN_RECEIVED:
      case GENIF_NOTIFY_UNKNOWN_RECEIVED_CLOSED:
      case GENIF_NOTIFY_UNKNOWN_RECEIVED_TIMEOUT:
      {
        GENIF_server_notify(inPtrServer, inNotify, inPtrChannel, inPtrMessage);

        break;
      }
      
/*----------*/

      case GENIF_NOTIFY_CHANNEL_HEARTBEAT:
      {
        GENIF_server_notify(inPtrServer, inNotify, inPtrChannel, inPtrMessage);

	break;
      }

      case GENIF_NOTIFY_CHANNEL_INACTIVITY:
      {
	SUCESO3("WARNING: CONNECTION (%s, %d) CHANNEL (%d) INACTIVITY", 
		 inPtrChannel->remHost,
		 inPtrChannel->remPort,
		 inPtrChannel->fd);
	
	GENIF_channel_empty(inPtrChannel, GENIF_END_CLOSE);
	
        GENIF_server_notify(inPtrServer, inNotify, inPtrChannel, inPtrMessage);

        GENIF_server_channel_close_cb(inPtrServer, inPtrChannel);
	
	break;
      }

      case GENIF_NOTIFY_CHANNEL_MALFUNCTION:
      {
	SUCESO3("WARNING: CONNECTION (%s, %d) CHANNEL (%d) MALFUNCTION", 
		 inPtrChannel->remHost,
		 inPtrChannel->remPort,
		 inPtrChannel->fd);
	
	GENIF_channel_empty(inPtrChannel, GENIF_END_CLOSE);
	
        GENIF_server_notify(inPtrServer, inNotify, inPtrChannel, inPtrMessage);

        GENIF_server_channel_close_cb(inPtrServer, inPtrChannel);
	
	break;
      }

      case GENIF_NOTIFY_CHANNEL_ERROR:
      case GENIF_NOTIFY_CHANNEL_CLOSED:
      {
        GENIF_server_notify(inPtrServer, inNotify, inPtrChannel, inPtrMessage);

        GENIF_server_channel_close_cb(inPtrServer, inPtrChannel);
	
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

  TRAZA0("Returning from GENIF_server_channel_notify_cb()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static void 
GENIF_server_notify
(
  GENIF_server_t*		inPtrSrv,
  int				inNotify,
  GENIF_channel_t*		inPtrChn,
  GENIF_message_t*		inPtrMsg
)
{
  inPtrSrv->notDelete = GENIF_TRUE;

  inPtrSrv->cbNotify(inPtrSrv, inNotify, inPtrChn, inPtrMsg);

  inPtrSrv->notDelete = GENIF_FALSE;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

