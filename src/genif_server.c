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
#include <string.h>

#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#ifdef _WIN64
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#endif

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

//----------------

static void GENIF_server_timer_cb(uv_timer_t* inTimer);

static void GENIF_server_accept_cb
( 
  uv_stream_t*		inUvServer,
  int				inUvStatus
);

static void GENIF_server_channel_close_cb
( 
  GENIF_server_t*		inServer,
  GENIF_channel_t*		inChannel
);

static void GENIF_server_channel_notify_cb
(
  GENIF_channel_t*		inChannel,
  int				inNotify,
  GENIF_message_t*		inMessage
);

static void GENIF_server_notify
(
  GENIF_server_t*		inSrv,
  int				inNotify,
  GENIF_channel_t*		inChn,
  GENIF_message_t*		inMsg
);

//----------------

/*__FUNCIONES PUBLICAS________________________________________________________*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
GENIF_server_initialize
(
  GENIF_server_t*			inServer,
  int						inPort,
  void*						inParent,
  void						(*inCbNotify)(GENIF_server_t*, int,
                                          GENIF_channel_t*,
				                          GENIF_message_t*),
  GENIF_server_config_t*	inConfig,
  GENIF_modifier_t*			inModifier
)
{
  GENIF_channel_t			channel;
  struct sockaddr_in 		addr[1];

  int						ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_server_initialize(%p)", inServer);

//----------------
  GENIF_factory_initialize();
//----------------

  if(ret == GENIF_RC_OK)
  {
    memset(inServer, 0, sizeof(GENIF_server_t));

    inServer->loop = uv_default_loop();

    inServer->locPort = inPort;
    strcpy(inServer->locHost, "0.0.0.0");

    inServer->parent   = inParent;
    inServer->cbNotify = inCbNotify;

    inServer->modifier = inModifier;

    inServer->counter = &inServer->counterS;
  }

//----------------

  if(ret == GENIF_RC_OK)
  {
	if(uv_timer_init(inServer->loop, inServer->timer) < 0)
	{
	  GENIF_FATAL0("FATAL: uv_timer_init()");
	}

    inServer->timer->data = inServer;

	if(uv_timer_start(inServer->timer, GENIF_server_timer_cb, 1000, 1000) < 0)
    {
      GENIF_FATAL0("FATAL: uv_timer_start()");
    }
  }

//----------------

  if(ret == GENIF_RC_OK)
  {
    if(RLST_initializeRefList(inServer->channel, &channel, &channel.list) < 0)
    {
      GENIF_FATAL("FATAL: RLST_initializeRefList()");
    }
  }
  
//----------------

  if(ret == GENIF_RC_OK)
  {
	if(uv_tcp_init(inServer->loop, inServer->server) < 0)
	{
	  GENIF_FATAL0("FATAL: uv_tcp_init()");
	}

	inServer->server->data = inServer;

	if(uv_ip4_addr(inServer->locHost, inServer->locPort, addr) < 0)
	{
	  GENIF_FATAL2("FATAL: uv_ip4_addr(%s, %s)",
                    inServer->locHost,
					inServer->locPort);
	}

	if(uv_tcp_bind(inServer->server, (struct sockaddr*)(addr), 0) < 0)
	{
	  GENIF_FATAL2("FATAL: uv_tcp_bind(%s, %s)",
                    inServer->locHost,
					inServer->locPort);
	}

	if(uv_listen((uv_stream_t*)(inServer->server), 5, GENIF_server_accept_cb) < 0)
	{
	  GENIF_FATAL2("FATAL: uv_listen(%s, %s)",
                    inServer->locHost,
					inServer->locPort);
	}
  }
  
//----------------

  TRAZA1("Returning from GENIF_server_initialize() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int 
GENIF_server_finalize(GENIF_server_t* inServer)
{
  GENIF_channel_t*		ptrChannel;
  
  int					ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_server_final(%p)", inServer);

//----------------

  while((ptrChannel = RLST_extractHead(inServer->channel)))
  {
    if(GENIF_channel_finalize(ptrChannel) != GENIF_RC_OK)
    {
      SUCESO0("ERROR: GENIF_channel_finalize()");
    }

    GENIF_channel_delete(ptrChannel);
  }
  
  if(uv_timer_stop(inServer->timer) < 0)
  {
    SUCESO0("FATAL: uv_timer_stop()");
  }

  uv_close((uv_handle_t*)(inServer->server), NULL);

//----------------

  TRAZA0("Returning from GENIF_server_final()");

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
GENIF_server_reconfig
(
  GENIF_server_t*			inServer,
  GENIF_server_config_t*	inConfig
)
{
  int				ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_server_reconfig(%p)", inServer);

//----------------

  memcpy(&inServer->config, inConfig, sizeof(GENIF_server_config_t));

//----------------

  TRAZA1("Returning from GENIF_server_reconfig() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
GENIF_server_count
(
  GENIF_server_t*			inServer,
  GENIF_server_counter_t*	outPtrCounter
)
{
  static long				size = sizeof(GENIF_server_counter_t);
  
  int						ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_server_count(%p)", inServer);

//----------------

  memcpy(outPtrCounter, inServer->counter, size);

  memset(inServer->counter, 0, size);

//----------------

  TRAZA1("Returning from GENIF_server_count() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static void 
GENIF_server_timer_cb(uv_timer_t* inTimer)
{
  GENIF_server_t*		inServer = inTimer->data;

  long					T = uv_now(inServer->loop);
  
  GENIF_channel_t*		ptrChannel;
  
  long					Tm, Om, Wm, Im, Fi, Fo;

  TRAZA1("Entering in GENIF_server_timer_cb(%p)", inServer);

//----------------

  if(uv_timer_again(inTimer) < 0)
  {
    GENIF_FATAL0("ERROR: setTimer()");
  }

//----------------

  Fi = 0; Fo = 0; RLST_resetGet(inServer->channel, NULL);

  while((ptrChannel = RLST_getNext(inServer->channel)))
  {
    Fi += ptrChannel->inFlowCount; Fo += ptrChannel->outFlowCount;
    
    GENIF_channel_timer_cb(ptrChannel, T);
  }

//----------------

  if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 1)
  {
    Tm = GENIF_server_message_num(inServer, &Om, &Wm, &Im);

    PRUEBA8("%s:%d -> T=%ld, O=%ld, W=%ld, I=%ld, Fi=%ld, Fo=%ld",
    	     inServer->locHost,
	     inServer->locPort,
	     Tm, Om, Wm, Im, Fi, Fo);
  }

//----------------

  if(inServer->emptyFlag == GENIF_FLAG_ON)
  {
    if(GENIF_server_message_num(inServer, NULL, NULL, NULL) == 0)
    {
      inServer->emptyFlag = GENIF_FLAG_OFF;

      GENIF_server_notify(inServer, GENIF_NOTIFY_SERVER_EMPTY, NULL, NULL);
    }
  }
  
//----------------

  TRAZA0("Returning from GENIF_server_timer_cb()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static void
GENIF_server_accept_cb
( 
  uv_stream_t*			inUvServer,
  int				inUvStatus
)
{
  GENIF_server_t*		inServer = inUvServer->data;

  GENIF_channel_t*		ptrChannel = NULL;

  uv_tcp_t 			channel[1];

  struct sockaddr_storage 	addr[1];
//int				port;
  int				alen;

  int				ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_server_accept_cb(%p)", inServer);

//----------------

  if(inUvStatus < 0)
  {
    SUCESO3("ERROR: GENIF_server_accept_cb(%s, %d) = %d",
             inServer->locHost,
             inServer->locPort,
             inUvStatus);

    ret = GENIF_RC_ERROR;
  }

//----------------

  if(ret == GENIF_RC_OK)
  {
    if((inServer->config.channelMax >  0) &&
       (inServer->config.channelMax <= RLST_getNumElem(inServer->channel)))
    {
      SUCESO1("WARNING: REACHED MAXNUM OF CHANNELS %ld",
               inServer->config.channelMax);

      uv_accept((uv_stream_t*)(inServer->server), (uv_stream_t*)(channel));

      uv_tcp_getpeername(channel, (struct sockaddr*)(addr), &alen);
/*
      SUCESO2("WARNING: REFUSED CONNECTION FROM (%s, %d)",
    	       inet_ntoa(addr->sin_addr),
    	       port = ntohs(addr->sin_port));
*/
  	  uv_close((uv_handle_t*)(channel), NULL);
    }
  }

//----------------

  else if(inServer->disabledListen)
  {
    SUCESO0("WARNING: SERVER LISTEN DISABLED");

    uv_accept((uv_stream_t*)(inServer->server), (uv_stream_t*)(channel));

    uv_tcp_getpeername(channel, (struct sockaddr*)(addr), &alen);
/*
    SUCESO2("WARNING: REFUSED CONNECTION FROM (%s, %d)",
  	     inet_ntoa(addr->sin_addr),
	     port = ntohs(addr->sin_port));
*/
    uv_close((uv_handle_t*)(channel), NULL);
  }

//----------------

  if(ret == GENIF_RC_ERROR)
  {
    ptrChannel = GENIF_channel_new();

    if(GENIF_channel_initialize(ptrChannel, 0, inServer,
                                GENIF_CHANNEL_TYPE_SERVER, NULL,
				GENIF_server_channel_notify_cb,
				&inServer->config.channel,
				&inServer->counter->channel,
				inServer->modifier) < 0)
    {
      GENIF_FATAL("ERROR: GENIF_channel_initialize()");
    }

    if(uv_accept((uv_stream_t*)(inServer->server),
	         (uv_stream_t*)(ptrChannel->channel)) < 0)
    {
      SUCESO2("ERROR: uv_accept(%s, %s)",
	       inServer->locHost,
	       inServer->locPort);

      uv_close((uv_handle_t*)(ptrChannel->channel), NULL);

      GENIF_channel_delete(ptrChannel); ptrChannel = NULL;

      ret = GENIF_RC_ERROR;
    }
  }

//----------------

  if(ret == GENIF_RC_ERROR)
  {
    inServer->counter->channelOpen++;
    
    if(RLST_insertTail(inServer->channel, ptrChannel) < 0)
    {
      GENIF_FATAL("FATAL: RLST_insertTail()");
    }

//  STRCPY(ptrChannel->remHost, inet_ntoa(addr.sin_addr), GENIF_MAXLEN_HOST);
    STRCPY(ptrChannel->remHost, "", GENIF_MAXLEN_HOST);

    ptrChannel->remPort = 0; //port;

    GENIF_channel_read_disable(ptrChannel, inServer->disabledInput);
    GENIF_channel_write_disable(ptrChannel, inServer->disabledOutput);

    DEPURA2("ACCEPTED CONNECTION FROM (%s, %d)",
             ptrChannel->remHost,
             ptrChannel->remPort);
	     
    GENIF_server_notify(inServer, GENIF_NOTIFY_CHANNEL_ACCEPTED,
                        ptrChannel,
			            NULL);

    if(RLST_getNumElem(inServer->channel) == 1)
    {
      inServer->disconnFlag = GENIF_FLAG_OFF;

      GENIF_server_notify(inServer, GENIF_NOTIFY_SERVER_CONNECTED,
                          NULL,
			              NULL);
    }
  }

//----------------

  TRAZA0("Returning from GENIF_server_accept_cb()");
}

/*----------------------------------------------------------------------------*/

void
GENIF_server_channel_close
( 
  GENIF_server_t*		inServer,
  GENIF_channel_t*		inChannel
)
{
  TRAZA2("Entering in GENIF_server_channel_close(%p, %p)", 
          inServer,
	  inChannel);

//----------------

  if(RLST_extract(inServer->channel, inChannel))
  {
    GENIF_channel_finalize(inChannel);
    GENIF_channel_delete(inChannel);
  }

  if(RLST_getNumElem(inServer->channel) == 0)
  {
    inServer->disconnFlag = GENIF_FLAG_ON;
  }

//----------------

  TRAZA0("Returning from GENIF_server_channel_close()");
}

/*----------------------------------------------------------------------------*/

static void
GENIF_server_channel_close_cb
( 
  GENIF_server_t*		inServer,
  GENIF_channel_t*		inChannel
)
{
  TRAZA2("Entering in GENIF_server_channel_close_cb(%p, %p)", 
          inServer,
	  inChannel);

//----------------

  if(RLST_extract(inServer->channel, inChannel))
  {
    GENIF_channel_finalize(inChannel);
    GENIF_channel_delete(inChannel);
  }

  if(RLST_getNumElem(inServer->channel) == 0)
  {
    if(inServer->disconnFlag == GENIF_FLAG_OFF)
    {
      inServer->disconnFlag = GENIF_FLAG_ON;

      GENIF_server_notify(inServer, GENIF_NOTIFY_SERVER_DISCONNECTED,
                          NULL,
			              NULL);
    }
  }

//----------------

  TRAZA0("Returning from GENIF_server_channel_close_cb()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void
GENIF_server_empty_notify(GENIF_server_t* inServer)
{
  TRAZA1("Entering in GENIF_server_empty_notify(%p)", inServer);

//----------------

  inServer->emptyFlag = GENIF_FLAG_ON;

//----------------

  TRAZA0("Returning from GENIF_server_empty_notify()");
}

/*----------------------------------------------------------------------------*/

void 
GENIF_server_listen_disable(GENIF_server_t* inServer, int inFlag)
{
  TRAZA2("Entering in GENIF_server_listen_disable(%p, %d)",
          inServer,
	  inFlag);

//----------------

  inServer->disabledListen = inFlag;

//----------------

  TRAZA0("Returning from GENIF_server_listen_disable()");
}

/*----------------------------------------------------------------------------*/

void 
GENIF_server_input_disable(GENIF_server_t* inServer, int inFlag)
{
  GENIF_channel_t*		ptrChannel = NULL;

  TRAZA2("Entering in GENIF_server_input_disable(%p, %d)",
          inServer,
	      inFlag);

//----------------

  if(inServer->disabledInput != inFlag)
  {
    inServer->disabledInput = inFlag;

    RLST_resetGet(inServer->channel, NULL);
    
    while((ptrChannel = RLST_getNext(inServer->channel)))
    {
      GENIF_channel_read_disable(ptrChannel, inFlag);
    }
  }

//----------------

  TRAZA0("Returning from GENIF_server_input_disable()");
}

/*----------------------------------------------------------------------------*/

void 
GENIF_server_output_disable(GENIF_server_t* inServer, int inFlag)
{
  GENIF_channel_t*		ptrChannel = NULL;

  TRAZA2("Entering in GENIF_server_output_disable(%p, %d)",
          inServer,
	      inFlag);

//----------------

  if(inServer->disabledOutput != inFlag)
  {
    inServer->disabledOutput = inFlag;

    RLST_resetGet(inServer->channel, NULL);
    
    while((ptrChannel = RLST_getNext(inServer->channel)))
    {
      GENIF_channel_write_disable(ptrChannel, inFlag);
    }
  }

//----------------

  TRAZA0("Returning from GENIF_server_output_disable()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
GENIF_server_external_queue_set
(
  GENIF_server_t*	inServer,
  GENIF_message_t*	(*inExtQueueFunc)(GENIF_server_t*, GENIF_channel_t*)
)
{
  int			ret = GENIF_RC_OK;

  TRAZA2("Entering in GENIF_server_external_queue_set(%p, %p)", 
          inServer,
	  inExtQueueFunc);

//----------------

  inServer->extQueueFunc = inExtQueueFunc;

//----------------

  TRAZA1("Returning from GENIF_server_external_queue_set() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
GENIF_server_external_queue(GENIF_server_t* inServer, int inFlag)
{
  GENIF_channel_t*		ptrChannel;

  int				ret = GENIF_RC_OK;

  TRAZA2("Entering in GENIF_server_external_queue(%p, %d)", 
          inServer,
	      inFlag);

//----------------

  if(inServer->extQueueFunc != NULL)
  {
    inServer->extQueueFlag = inFlag;

    RLST_resetGet(inServer->channel, NULL);

    while((ptrChannel = RLST_getNext(inServer->channel)))
    {
      GENIF_channel_external_queue(ptrChannel, inFlag);
    }
  }

//----------------

  TRAZA1("Returning from GENIF_server_external_queue() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
GENIF_server_channel_send
(
  GENIF_server_t*		inServer,
  GENIF_channel_t* 		inChannel,
  GENIF_message_t*		inMessage
)
{
  int				ret = GENIF_RC_OK;

  TRAZA3("Entering in GENIF_server_channel_send(%p, %p, %p)", 
          inServer,
          inChannel,
	  inMessage);

//----------------

  if(inChannel->parent == inServer)
  {
    ret = GENIF_channel_send(inChannel, inMessage);
  }

  else
  {
    GENIF_FATAL("ASSERTION!!");
  }

//----------------

  TRAZA1("Returning from GENIF_server_channel_send() = %d", ret);

  return ret;
}
    
/*----------------------------------------------------------------------------*/

int
GENIF_server_channel_request
(
  GENIF_server_t*		inServer,
  GENIF_channel_t* 		inChannel,
  GENIF_message_t*		inMessage
)
{
  int				ret = GENIF_RC_OK;

  TRAZA3("Entering in GENIF_server_channel_request(%p, %p, %p)", 
          inServer,
          inChannel,
	  inMessage);

//----------------

  if(inChannel->parent == inServer)
  {
    ret = GENIF_channel_request(inChannel, inMessage);
  }

  else
  {
    GENIF_FATAL("ASSERTION!!");
  }

//----------------

  TRAZA1("Returning from GENIF_server_channel_request() = %d", ret);

  return ret;
}
    
/*----------------------------------------------------------------------------*/

int
GENIF_server_reply
(
  GENIF_server_t*		inServer,
  GENIF_message_t*		inMessage
)
{
  int				ret = GENIF_RC_OK;

  TRAZA2("Entering in GENIF_server_reply(%p, %p)", 
          inServer,
	  inMessage);

//----------------

  if(inMessage->channel != NULL)
  {
    if(inMessage->channel->parent == inServer)
    {
      ret = GENIF_channel_reply(inMessage->channel, inMessage);
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

//----------------

  TRAZA1("Returning from GENIF_server_reply() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
GENIF_server_unref_msg
(
  GENIF_server_t*		inServer,
  GENIF_message_t*		inMessage
)
{
  int				ret = GENIF_RC_OK;

  TRAZA2("Entering in GENIF_server_unref_msg(%p, %p)",
          inServer,
	  inMessage);

//----------------

  if(inMessage->channel != NULL)
  {
    if(inMessage->channel->parent == inServer)
    {
      ret = GENIF_channel_unref_msg(inMessage->channel, inMessage);
    }

    else { ret = GENIF_RC_NOT_FOUND; }
  }

  else { ret = GENIF_RC_NOT_FOUND; }

//----------------

  TRAZA1("Returning from GENIF_server_unref_msg() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
GENIF_server_cancel_msg
(
  GENIF_server_t*		inServer,
  GENIF_message_t*		inMessage,
  int*				outStore
)
{
  int				ret = GENIF_RC_OK;

//----------------

  TRAZA2("Entering in GENIF_server_cancel_msg(%p, %p)", 
          inServer,
	  inMessage);

//----------------

  if(inMessage->channel != NULL)
  {
    if(inMessage->channel->parent == inServer)
    {
      ret = GENIF_channel_cancel_msg(inMessage->channel,
                                     inMessage,
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

//----------------

  TRAZA1("Returning from GENIF_server_cancel_msg() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

long
GENIF_server_message_num
(
  GENIF_server_t*		inServer,
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

  TRAZA1("Entering in GENIF_server_message_num(%p)", inServer);

//----------------

  if(outOstore != NULL) *outOstore = 0; 
  if(outWstore != NULL) *outWstore = 0; 
  if(outIstore != NULL) *outIstore = 0; 

//----------------

  RLST_resetGet(inServer->channel, NULL);

  while((ptrChannel = RLST_getNext(inServer->channel)))
  {
    msgO = 0; msgW = 0; msgO = 0;
    
    msgT += GENIF_channel_message_num(ptrChannel, &msgO, &msgW, &msgI);

    if(outOstore != NULL) *outOstore += msgO;
    if(outWstore != NULL) *outWstore += msgW;
    if(outIstore != NULL) *outIstore += msgI;
  }

//----------------

  TRAZA1("Returning from GENIF_server_message_num() = %ld", msgT);

  return msgT;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

long
GENIF_server_channel_num(GENIF_server_t* inServer)
{
  long				channelNum = 0;

  TRAZA1("Entering in GENIF_server_channel_num(%p)", inServer);

//----------------

  channelNum = RLST_getNumElem(inServer->channel);

//----------------

  TRAZA1("Returning from GENIF_server_channel_num() = %ld", channelNum);

  return channelNum;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static void 
GENIF_server_channel_notify_cb
(
  GENIF_channel_t*		inChannel,
  int				inNotify,
  GENIF_message_t*		inMessage
)
{
  GENIF_server_t*		inServer = inChannel->parent;

//----------------

  TRAZA3("Entering in GENIF_server_channel_notify_cb(%p, %d, %p)",
          inChannel,
	  inNotify,
	  inMessage);

//----------------

  if(inNotify < GENIF_NOTIFY_TOTAL)
  {
    DEPURA1("%s", GENIF_notify_name[inNotify]);

    switch(inNotify)
    {

//----------------

      case GENIF_NOTIFY_REQUEST_ERROR:
      case GENIF_NOTIFY_REQUEST_CLOSED:
      case GENIF_NOTIFY_REQUEST_TIMEOUT:
      case GENIF_NOTIFY_REQUEST_SENT:
      case GENIF_NOTIFY_REQUEST_SENT_CLOSED:
      case GENIF_NOTIFY_REQUEST_SENT_TIMEOUT:
      case GENIF_NOTIFY_RESPONSE_RECEIVED:
      {
        GENIF_server_notify(inServer, inNotify, inChannel, inMessage);

        break;
      }
      
//----------------

      case GENIF_NOTIFY_REQUEST_RECEIVED:
      case GENIF_NOTIFY_REQUEST_RECEIVED_CLOSED:
      case GENIF_NOTIFY_REQUEST_RECEIVED_TIMEOUT:
      case GENIF_NOTIFY_RESPONSE_ERROR:
      case GENIF_NOTIFY_RESPONSE_CLOSED:
      case GENIF_NOTIFY_RESPONSE_TIMEOUT:
      case GENIF_NOTIFY_RESPONSE_SENT:
      {
        GENIF_server_notify(inServer, inNotify, inChannel, inMessage);

        break;
      }

//----------------

      case GENIF_NOTIFY_MESSAGE_ERROR:
      case GENIF_NOTIFY_MESSAGE_CLOSED:
      case GENIF_NOTIFY_MESSAGE_TIMEOUT:
      case GENIF_NOTIFY_MESSAGE_SENT:
      case GENIF_NOTIFY_MESSAGE_RECEIVED:
      case GENIF_NOTIFY_MESSAGE_RECEIVED_CLOSED:
      case GENIF_NOTIFY_MESSAGE_RECEIVED_TIMEOUT:
      {
        GENIF_server_notify(inServer, inNotify, inChannel, inMessage);
	
        break;
      }

//----------------

      case GENIF_NOTIFY_UNKNOWN_RECEIVED:
      case GENIF_NOTIFY_UNKNOWN_RECEIVED_CLOSED:
      case GENIF_NOTIFY_UNKNOWN_RECEIVED_TIMEOUT:
      {
        GENIF_server_notify(inServer, inNotify, inChannel, inMessage);

        break;
      }
      
//----------------

      case GENIF_NOTIFY_CHANNEL_HEARTBEAT:
      {
        GENIF_server_notify(inServer, inNotify, inChannel, inMessage);

	break;
      }

      case GENIF_NOTIFY_CHANNEL_INACTIVITY:
      {
	SUCESO2("WARNING: CONNECTION (%s, %d) INACTIVITY",
		 inChannel->remHost,
		 inChannel->remPort);
	
	GENIF_channel_empty(inChannel, GENIF_END_CLOSE);
	
        GENIF_server_notify(inServer, inNotify, inChannel, inMessage);

        GENIF_server_channel_close_cb(inServer, inChannel);
	
	break;
      }

      case GENIF_NOTIFY_CHANNEL_MALFUNCTION:
      {
	SUCESO2("WARNING: CONNECTION (%s, %d) MALFUNCTION",
		 inChannel->remHost,
		 inChannel->remPort);
	
	GENIF_channel_empty(inChannel, GENIF_END_CLOSE);
	
        GENIF_server_notify(inServer, inNotify, inChannel, inMessage);

        GENIF_server_channel_close_cb(inServer, inChannel);
	
	break;
      }

      case GENIF_NOTIFY_CHANNEL_ERROR:
      case GENIF_NOTIFY_CHANNEL_CLOSED:
      {
        GENIF_server_notify(inServer, inNotify, inChannel, inMessage);

        GENIF_server_channel_close_cb(inServer, inChannel);
	
	break;
      }

//----------------

      default:
      {
        SUCESO1("ERROR: UNEXPECTED %s", GENIF_notify_name[inNotify]);

        GENIF_FATAL("ASSERTION!!");

	break;
      }
    }
  }

//----------------

  else
  {
    SUCESO1("ERROR: UNKNOWN (%d)", inNotify);

    GENIF_FATAL("ASSERTION!!");
  }

//----------------

  TRAZA0("Returning from GENIF_server_channel_notify_cb()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static void 
GENIF_server_notify
(
  GENIF_server_t*		inSrv,
  int				inNotify,
  GENIF_channel_t*		inChn,
  GENIF_message_t*		inMsg
)
{
  inSrv->notDelete = GENIF_TRUE;

  inSrv->cbNotify(inSrv, inNotify, inChn, inMsg);

  inSrv->notDelete = GENIF_FALSE;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

