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
#include <string.h>

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

static int GENIF_client_connect(GENIF_client_t* inClient);

static void GENIF_client_connect_cb(uv_connect_t* inConnReq, int inStatus);

//----------------

static void GENIF_client_timer_cb(uv_timer_t* inTimer);

static void GENIF_client_mask(GENIF_client_t* inClient);

static void GENIF_client_input_mask(GENIF_client_t* inClient);
static void GENIF_client_output_mask(GENIF_client_t* inClient);

static void GENIF_client_channel_close_cb
( 
  GENIF_client_t*		inClient,
  GENIF_channel_t*		inChannel
);

static GENIF_message_t* GENIF_client_channel_write_cb
(
  GENIF_channel_t*		inChannel
);

static void GENIF_client_channel_notify_cb
(
  GENIF_channel_t*		inChannel,
  int				inNotify,
  GENIF_message_t*		inMessage
);

static void GENIF_client_notify
(
  GENIF_client_t*		inClient,
  int				inNotify,
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMessage
);

//----------------

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
GENIF_client_initialize
(
  GENIF_client_t*			inClient,
  char*					inRemHost,
  int					inRemPort,
  void*					inParent,
  void					(*inCbNotify)(GENIF_client_t*, int,
                                                      GENIF_channel_t*,
                                                      GENIF_message_t*),
  GENIF_client_config_t*		inConfig,
  GENIF_modifier_t*			inModifier
)
{
  GENIF_channel_t			c;
  GENIF_message_t			m;

  int					ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_client_initialize(%p)", inClient);

//----------------
  GENIF_factory_initialize();
//----------------

  if(ret == GENIF_RC_OK)
  {
    memset(inClient, 0, sizeof(GENIF_client_t));

    inClient->loop = uv_default_loop();

    STRCPY(inClient->remHost, inRemHost, GENIF_MAXLEN_HOST);
    inClient->remPort = inRemPort;

    inClient->parent   = inParent;
    inClient->cbNotify = inCbNotify;

    inClient->modifier = inModifier;

    inClient->inputMask  = GENIF_FLAG_ON;
    inClient->outputMask = GENIF_FLAG_ON;

    memcpy(&inClient->config, inConfig, sizeof(GENIF_client_config_t));

    inClient->counter = &inClient->counterS;
  }

//----------------

  if(ret == GENIF_RC_OK)
  {
    if(uv_timer_init(inClient->loop, inClient->timer) < 0)
    {
      GENIF_FATAL0("FATAL: uv_timer_init()");
    }

    inClient->timer->data = inClient;

    if(uv_timer_start(inClient->timer, GENIF_client_timer_cb, 1000, 1000) < 0)
    {
      GENIF_FATAL0("FATAL: uv_timer_start()");
    }
  }

//----------------

  if(ret == GENIF_RC_OK)
  {
    if(RLST_initializeRefList(inClient->channel, &c, &c.list) < 0)
    {
      GENIF_FATAL0("FATAL: RLST_initializeRefList()");
    }

    if(RLST_initializeRefList(inClient->chnltmp, &c, &c.list) < 0)
    {
      GENIF_FATAL0("FATAL: RLST_initializeRefList()");
    }
  
    if(RLST_initializeRefList(inClient->outQueue, &m, &m.list) < 0)
    {
      GENIF_FATAL0("FATAL: RLST_initializeRefList()");
    }
  }
  
//----------------

  if(ret == GENIF_RC_OK)
  {
    if(GENIF_client_connect(inClient) < 0)
    {
      SUCESO0("FATAL: GENIF_client_connect()");

      GENIF_client_finalize(inClient);

      ret = GENIF_RC_ERROR;
    }
  }

//----------------

  TRAZA1("Returning from GENIF_client_initialize() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int 
GENIF_client_finalize(GENIF_client_t* inClient)
{
  GENIF_channel_t*		ptrChannel;

  GENIF_message_t*		ptrMessage;

  char				strMessage[GENIF_MAXLEN_STRING + 1];

  int				ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_client_finalize(%p)", inClient);

//----------------

  while((ptrMessage = RLST_extractHead(inClient->outQueue)) != NULL)
  {
    GENIF_message_dump(ptrMessage, GENIF_MAXLEN_STRING + 1, strMessage);
      
    SUCESO1("WARNING: CANCELED QUEUED MESSAGE %s", strMessage);
  }

//----------------

  while((ptrChannel = RLST_extractHead(inClient->chnltmp)))
  {
    if(GENIF_channel_finalize(ptrChannel) != GENIF_RC_OK)
    {
      SUCESO0("ERROR: GENIF_channel_finalize()");
    }

    GENIF_channel_delete(ptrChannel);
  }

  while((ptrChannel = RLST_extractHead(inClient->channel)))
  {
    if(GENIF_channel_finalize(ptrChannel) != GENIF_RC_OK)
    {
      SUCESO0("ERROR: GENIF_channel_finalize()");
    }

    GENIF_channel_delete(ptrChannel);
  }
  
//----------------

  if(uv_timer_stop(inClient->timer) < 0)
  {
    SUCESO0("FATAL: uv_timer_stop()");
  }

//----------------

  TRAZA1("Returning from GENIF_client_finalize() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
GENIF_client_initialize_hp
(
  GENIF_client_t*		inClient,
  char*				inRemHost,
  int				inRemPort,
  char*				inLocHost,
  int				inLocPort,
  void*				inParent,
  void				(*inCbNotify)(GENIF_client_t*, int,
                                              GENIF_channel_t*,
					      GENIF_message_t*),
  GENIF_client_config_t*	inConfig,
  GENIF_modifier_t*		inModifier
)
{
  GENIF_channel_t		c;
  GENIF_message_t		m;

  int				ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_client_initialize(%p)", inClient);

//----------------
  GENIF_factory_initialize();
//----------------

  if(ret == GENIF_RC_OK)
  {
    memset(inClient, 0, sizeof(GENIF_client_t));

    inClient->loop = uv_default_loop();

    STRCPY(inClient->remHost, inRemHost, GENIF_MAXLEN_HOST);
    inClient->remPort = inRemPort;

    STRCPY(inClient->locHost, inLocHost, GENIF_MAXLEN_HOST);
    inClient->locPort = inLocPort;

    inClient->locFlag = GENIF_FLAG_ON;

    inClient->parent   = inParent;
    inClient->cbNotify = inCbNotify;

    inClient->modifier = inModifier;

    inClient->inputMask  = GENIF_FLAG_ON;
    inClient->outputMask = GENIF_FLAG_ON;

    memcpy(&inClient->config, inConfig, sizeof(GENIF_client_config_t));

    inClient->counter = &inClient->counterS;
  }

//----------------

  if(ret == GENIF_RC_OK)
  {
    if(uv_timer_init(inClient->loop, inClient->timer) < 0)
    {
      GENIF_FATAL0("FATAL: uv_timer_init()");
    }

    inClient->timer->data = inClient;

    if(uv_timer_start(inClient->timer, GENIF_client_timer_cb, 1000, 1000) < 0)
    {
      GENIF_FATAL0("FATAL: uv_timer_start()");
    }
  }

//----------------

  if(ret == GENIF_RC_OK)
  {
    if(RLST_initializeRefList(inClient->channel, &c, &c.list) < 0)
    {
      GENIF_FATAL0("FATAL: RLST_initializeRefList()");
    }

    if(RLST_initializeRefList(inClient->chnltmp, &c, &c.list) < 0)
    {
      GENIF_FATAL0("FATAL: RLST_initializeRefList()");
    }
  
    if(RLST_initializeRefList(inClient->outQueue, &m, &m.list) < 0)
    {
      GENIF_FATAL0("FATAL: RLST_initializeRefList()");
    }
  }
  
//----------------

  if(ret == GENIF_RC_OK)
  {
    if(GENIF_client_connect(inClient) < 0)
    {
      SUCESO0("ERROR: GENIF_client_connect()");

      GENIF_client_finalize(inClient);

      ret = GENIF_RC_ERROR;
    }
  }

//----------------

  TRAZA1("Returning from GENIF_client_initialize() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
GENIF_client_reconfig
(
  GENIF_client_t*		inClient,
  GENIF_client_config_t*	inConfig
)
{
  int				ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_client_reconfig(%p)", inClient);

//----------------

  memcpy(&inClient->config, inConfig, sizeof(GENIF_client_config_t));

  GENIF_client_connect(inClient);

//----------------

  TRAZA1("Returning from GENIF_client_reconfig() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
GENIF_client_count
(
  GENIF_client_t*		inClient,
  GENIF_client_counter_t*	outCounter
)
{
  static long			size = sizeof(GENIF_client_counter_t);

  int				ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_client_count(%p)", inClient);

//----------------

  memcpy(outCounter, inClient->counter, size);

  memset(inClient->counter, 0, size);

//----------------

  TRAZA1("Returning from GENIF_client_count() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static void 
GENIF_client_timer_cb(uv_timer_t* inTimer)
{
  GENIF_client_t*		inClient = inTimer->data;

  long				T = uv_now(inClient->loop);
  
  GENIF_channel_t*		ptrChannel;

  GENIF_message_t*		ptrMessage;

  long				Tout = 0;

  int				notify;

  int				endTout;

  long				Tm, Om, Wm, Im, Fi, Fo;

  TRAZA1("Entering in GENIF_client_timer_cb(%p)", inClient);

//----------------

  if(uv_timer_again(inTimer) < 0)
  {
    GENIF_FATAL0("FATAL: setTimer()");
  }

//----------------

  Fi = 0; Fo = 0; 

  RLST_resetGet(inClient->channel, NULL);

  while((ptrChannel = RLST_getNext(inClient->channel)))
  {
    Fi += ptrChannel->inFlowCount; Fo += ptrChannel->outFlowCount;
    
    GENIF_channel_timer_cb(ptrChannel, T);
  }

//----------------

  if(TRACE_level_get(TRACE_TYPE_DEFAULT) >= 1)
  {
    Tm = GENIF_client_message_num(inClient, &Om, &Wm, &Im);

    PRUEBA9("%s:%d (%d) -> T=%ld, O=%ld, W=%ld, I=%ld, Fi=%ld, Fo=%ld",
	    inClient->remHost,
	    inClient->remPort,
	    inClient->disabledOutput,
	    Tm, Om, Wm, Im, Fi, Fo); 
  }

//----------------

  if(inClient->config.reconnectTout > 0)
  {
    if(inClient->reconnectFlag == GENIF_FLAG_ON)
    {
      Tout = T - inClient->config.reconnectTout;

      if(Tout >= inClient->reconnectTime)
      {
    	GENIF_client_connect(inClient);
      }
    }
  }

//----------------

  if(inClient->config.outQueueTout > 0)
  {
    if(RLST_getNumElem(inClient->outQueue))
    {
      Tout = T - inClient->config.outQueueTout;

      RLST_resetGet(inClient->outQueue, NULL); endTout = 0;

      while((ptrMessage = RLST_getHead(inClient->outQueue)) && (!endTout))
      {
        if(Tout >= ptrMessage->T)
        {
	  RLST_extractHead(inClient->outQueue);
  
	  if(ptrMessage->type == GENIF_MESSAGE_TYPE_MESSAGE)
	  {
	    inClient->counter->channel.outMessageTimeout++;

	    notify = GENIF_NOTIFY_MESSAGE_TIMEOUT;

	    GENIF_client_notify(inClient, notify, NULL, ptrMessage);
	  }

	  else
	  {
	    inClient->counter->channel.outRequestTimeout++;

	    notify = GENIF_NOTIFY_REQUEST_TIMEOUT;
	  }

	  GENIF_client_notify(inClient, notify, NULL, ptrMessage);
        }

        else { endTout = 1; }
      }
    }
  }

//----------------

  inClient->outFlowCount = 0;

  GENIF_client_mask(inClient);

//----------------

  if(inClient->emptyFlag == GENIF_FLAG_ON)
  {
    if(GENIF_client_message_num(inClient, NULL, NULL, NULL) == 0)
    {
      inClient->emptyFlag = GENIF_FLAG_OFF;

      GENIF_client_notify(inClient, GENIF_NOTIFY_CLIENT_EMPTY, NULL, NULL);
    }
  }
  
//----------------

  TRAZA0("Returning from GENIF_client_timer_cb()");
}

/*----------------------------------------------------------------------------*/

static int
GENIF_client_connect(GENIF_client_t* inClient)
{
  GENIF_channel_t*		ptrChannel;

  struct sockaddr_in 		addr[1];

  long				num;

  int				ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_client_connect(%p)", inClient);

//----------------

  inClient->reconnectFlag = GENIF_FLAG_OFF;
  inClient->reconnectTime = 0;

//----------------

  num = RLST_getNumElem(inClient->channel);
  
  for(; num < inClient->config.channelMax; num++)
  {
    ptrChannel = GENIF_channel_new();

    if(GENIF_channel_initialize(ptrChannel, 0, inClient,
                                GENIF_CHANNEL_TYPE_CLIENT,
				GENIF_client_channel_write_cb,
				GENIF_client_channel_notify_cb,
			       &inClient->config.channel,
			       &inClient->counter->channel,
				inClient->modifier) < 0)
    {
      GENIF_FATAL0("FATAL: GENIF_channel_initialize()");
    }

    if(uv_tcp_init(inClient->loop, ptrChannel->channel) < 0)
    {
      GENIF_FATAL0("FATAL: uv_tcp_init()");
    }

    ptrChannel->channel->data = ptrChannel;
    ptrChannel->connreq->data = ptrChannel;

    if(inClient->locFlag == GENIF_FLAG_ON)
    {
      if(uv_ip4_addr(inClient->locHost, inClient->locPort, addr) < 0)
      {
        GENIF_FATAL2("FATAL: uv_ip4_addr(%s, %s)",
                      inClient->locHost,
                      inClient->locPort);
      }

      if(uv_tcp_bind(ptrChannel->channel, (struct sockaddr*)(addr), 0) < 0)
      {
        GENIF_FATAL2("FATAL: uv_tcp_bind(%s, %s)",
		      inClient->locHost,
		      inClient->locPort);
      }
    }

    if(uv_ip4_addr(inClient->remHost, inClient->remPort, addr) < 0)
    {
      GENIF_FATAL2("FATAL: uv_ip4_addr(%s, %s)",
                    inClient->remHost,
                    inClient->remPort);
    }

    if(uv_tcp_connect(ptrChannel->connreq,
                      ptrChannel->channel, (struct sockaddr*)(addr),
		      GENIF_client_connect_cb) < 0)
    {
      GENIF_FATAL2("FATAL: uv_tcp_connect(%s, %s)",
	            inClient->remHost,
		    inClient->remPort);
    }

    if(ret == GENIF_RC_OK)
    {
      DEPURA2("OPENING CONNECTION (%s, %d)",
	       inClient->remHost,
	       inClient->remPort);

      if(RLST_insertTail(inClient->chnltmp, ptrChannel) != RLST_RC_OK)
      {
        GENIF_FATAL0("FATAL: RLST_insertTail()");
      }

      inClient->notDelete = GENIF_TRUE;
    }

    else // if(ret != GENIF_RC_OK)
    {
      SUCESO2("ERROR: OPENING CONNECTION (%s, %d)",
	       inClient->remHost,
	       inClient->remPort);

      uv_close((uv_handle_t*)(ptrChannel->channel), NULL);

      GENIF_channel_delete(ptrChannel);

      inClient->reconnectFlag = GENIF_FLAG_ON;
      inClient->reconnectTime = uv_now(inClient->loop);

      ret = GENIF_RC_ERROR;
    }
  }

//----------------

  TRAZA1("Returning from GENIF_client_connect() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

static void
GENIF_client_connect_cb(uv_connect_t* inConnReq, int inStatus)
{
  GENIF_channel_t*		inChannel = inConnReq->data;
  GENIF_client_t*		inClient = inChannel->parent;
  
  TRAZA2("Entering in GENIF_client_connect_cb(%p, %d)", inClient, inStatus);

//----------------

  RLST_extract(inClient->chnltmp, inChannel);

  if(RLST_getNumElem(inClient->chnltmp) <= 0)
  {
    inClient->notDelete = GENIF_FALSE;
  }

//----------------

  if(inClient->timer == NULL)
  {
    SUCESO2("WARNING: CONNECT CALLBACK FOR A FINALIZED CLIENT (%s, %d)",
    	     inClient->remHost,
	     inClient->remPort);

    GENIF_client_channel_close(inClient, inChannel);
  }

  else if(inStatus == 0)
  {
    DEPURA2("CONNECTION (%s, %d) ESTABLISHED CHANNEL",
	     inClient->remHost,
	     inClient->remPort);

    inClient->counter->channelOpen++;
    
    GENIF_channel_read_disable(inChannel, inClient->disabledInput);
    GENIF_channel_write_disable(inChannel, inClient->disabledOutput);

    GENIF_client_notify(inClient, GENIF_NOTIFY_CHANNEL_CONNECTED,
                        inChannel,
			NULL);
    
    if(RLST_getNumElem(inClient->channel) == 1)
    {
      inClient->disconnFlag = GENIF_FLAG_OFF;

      GENIF_client_notify(inClient, GENIF_NOTIFY_CLIENT_CONNECTED,
                          NULL,
			  NULL);
    }

    if(RLST_getNumElem(inClient->outQueue) > 0)
    {
      GENIF_channel_external_queue(inChannel, GENIF_FLAG_ON);
    }
  }
  
//----------------
  
  else // if(inStatus != 0)
  {
    SUCESO3("ERROR: OPENING CONNECTION (%s, %d) ERROR (%d)", 
	     inClient->remHost,
	     inClient->remPort, inStatus);

    uv_close((uv_handle_t*)(inChannel->channel), NULL);

    GENIF_channel_delete(inChannel);

    inClient->reconnectFlag = GENIF_FLAG_ON;
    inClient->reconnectTime = uv_now(inClient->loop);

    if(RLST_getNumElem(inClient->channel) == 0)
    {
      if(inClient->disconnFlag == GENIF_FLAG_OFF)
      {
        inClient->disconnFlag = GENIF_FLAG_ON;

        GENIF_client_notify(inClient, GENIF_NOTIFY_CLIENT_DISCONNECTED,
                            NULL,
			    NULL);
      }
    }
  }

//----------------

  TRAZA0("Returning from GENIF_client_connect_cb()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void
GENIF_client_channel_close
( 
  GENIF_client_t*		inClient,
  GENIF_channel_t*		inChannel
)
{
  TRAZA2("Entering in GENIF_client_channel_close(%p, %p)", 
          inClient,
	  inChannel);

//----------------

  if(RLST_extract(inClient->channel, inChannel))
  {
    GENIF_channel_finalize(inChannel);
    GENIF_channel_delete(inChannel);
    
    inClient->reconnectFlag = GENIF_FLAG_ON;
    inClient->reconnectTime = uv_now(inClient->loop);
  }

//----------------

  if(RLST_getNumElem(inClient->channel) == 0)
  {
    inClient->disconnFlag = GENIF_FLAG_ON;
  }

//----------------

  TRAZA0("Returning from GENIF_client_channel_close()");
}

/*----------------------------------------------------------------------------*/

static void
GENIF_client_channel_close_cb
( 
  GENIF_client_t*		inClient,
  GENIF_channel_t*		inChannel
)
{
  TRAZA2("Entering in GENIF_client_channel_close_cb(%p, %p)", 
          inClient,
	  inChannel);

//----------------

  if(RLST_extract(inClient->channel, inChannel))
  {
    GENIF_channel_finalize(inChannel);
    GENIF_channel_delete(inChannel);

    inClient->reconnectFlag = GENIF_FLAG_ON;
    
    inClient->reconnectTime = uv_now(inClient->loop);
  }

//----------------

  if(RLST_getNumElem(inClient->channel) == 0)
  {
    if(inClient->disconnFlag == GENIF_FLAG_OFF)
    {
      inClient->disconnFlag = GENIF_FLAG_ON;

      GENIF_client_notify(inClient, GENIF_NOTIFY_CLIENT_DISCONNECTED,
                          NULL,
			  NULL);
    }
  }

//----------------

  TRAZA0("Returning from GENIF_client_channel_close_cb()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void
GENIF_client_empty_notify(GENIF_client_t* inClient)
{
  TRAZA1("Entering in GENIF_client_empty_notify(%p)", inClient);

//----------------

  inClient->emptyFlag = GENIF_FLAG_ON;

//----------------

  TRAZA0("Returning from GENIF_client_empty_notify()");
}

/*----------------------------------------------------------------------------*/

void 
GENIF_client_input_disable(GENIF_client_t* inClient, int inFlag)
{
  TRAZA2("Entering in GENIF_client_input_disable(%p, %d)",
          inClient,
	  inFlag);

//----------------

  if(inClient->disabledInput != inFlag)
  {
    inClient->disabledInput = inFlag;

    GENIF_client_mask(inClient);
  }

//----------------

  TRAZA0("Returning from GENIF_client_input_disable()");
}

/*----------------------------------------------------------------------------*/

void 
GENIF_client_output_disable(GENIF_client_t* inClient, int inFlag)
{
  TRAZA2("Entering in GENIF_client_output_disable(%p, %d)",
          inClient,
	  inFlag);

//----------------

  if(inClient->disabledOutput != inFlag)
  {
    inClient->disabledOutput = inFlag;

    GENIF_client_mask(inClient);
  }

//----------------

  TRAZA0("Returning from GENIF_client_output_disable()");
}

/*----------------------------------------------------------------------------*/

static void
GENIF_client_mask(GENIF_client_t* inClient)
{
  int				inputMask;
  int				outputMask;
  
  GENIF_channel_t*		ptrChannel;

  TRAZA1("Entering in GENIF_client_mask(%p)", inClient);

//----------------

  inputMask = GENIF_FLAG_ON;

  if(inClient->disabledInput == GENIF_FLAG_ON)
  {
    inputMask = GENIF_FLAG_OFF;
  }

  if(inClient->inputMask != inputMask)
  {
    inClient->inputMask = inputMask;

    RLST_resetGet(inClient->channel, NULL);
    
    if(inputMask == GENIF_FLAG_ON)
    {
      while((ptrChannel = RLST_getNext(inClient->channel)))
      {
        GENIF_channel_read_disable(ptrChannel, GENIF_FLAG_OFF);
      }
    }

    else
    {
      while((ptrChannel = RLST_getNext(inClient->channel)))
      {
        GENIF_channel_read_disable(ptrChannel, GENIF_FLAG_ON);
      }
    }
  }
  
//----------------

  outputMask = GENIF_FLAG_ON;

  if(inClient->disabledOutput == GENIF_FLAG_ON)
  {
    outputMask = GENIF_FLAG_OFF;
  }

  if(inClient->config.outRefMax > 0)
  {
    if(inClient->outRefCount >= inClient->config.outRefMax)
    {
      outputMask = GENIF_FLAG_OFF;
    }
  }

  if(inClient->config.outFlowMax > 0)
  {
    if(inClient->outFlowCount >= inClient->config.outFlowMax)
    {
      outputMask = GENIF_FLAG_OFF;
    }
  }

  if(inClient->outputMask != outputMask)
  {
    inClient->outputMask = outputMask;
    
    RLST_resetGet(inClient->channel, NULL);
    
    if(outputMask == GENIF_FLAG_ON)
    {
      while((ptrChannel = RLST_getNext(inClient->channel)))
      {
        GENIF_channel_write_disable(ptrChannel, GENIF_FLAG_OFF);
      }
    }

    else
    {
      while((ptrChannel = RLST_getNext(inClient->channel)))
      {
        GENIF_channel_write_disable(ptrChannel, GENIF_FLAG_ON);
      }
    }
  }
  
//----------------

  TRAZA0("Returning from GENIF_client_mask()");
}

/*----------------------------------------------------------------------------*/

static void
GENIF_client_input_mask(GENIF_client_t* inClient)
{
  int				inputMask;

  GENIF_channel_t*		ptrChannel;

  TRAZA1("Entering in GENIF_client_input_mask(%p)", inClient);

//----------------

  inputMask = GENIF_FLAG_ON;

  if(inClient->disabledInput == GENIF_FLAG_ON)
  {
    inputMask = GENIF_FLAG_OFF;
  }

  if(inClient->inputMask != inputMask)
  {
    inClient->inputMask = inputMask;

    RLST_resetGet(inClient->channel, NULL);

    if(inputMask == GENIF_FLAG_ON)
    {
      while((ptrChannel = RLST_getNext(inClient->channel)))
      {
        GENIF_channel_read_disable(ptrChannel, GENIF_FLAG_OFF);
      }
    }

    else
    {
      while((ptrChannel = RLST_getNext(inClient->channel)))
      {
        GENIF_channel_read_disable(ptrChannel, GENIF_FLAG_ON);
      }
    }
  }

//----------------

  TRAZA0("Returning from GENIF_client_input_mask()");
}

/*----------------------------------------------------------------------------*/

static void
GENIF_client_output_mask(GENIF_client_t* inClient)
{
  int				outputMask;

  GENIF_channel_t*		ptrChannel;

  TRAZA1("Entering in GENIF_client_output_mask(%p)", inClient);

//----------------

  outputMask = GENIF_FLAG_ON;

  if(inClient->disabledOutput == GENIF_FLAG_ON)
  {
    outputMask = GENIF_FLAG_OFF;
  }

  if(inClient->config.outRefMax > 0)
  {
    if(inClient->outRefCount >= inClient->config.outRefMax)
    {
      outputMask = GENIF_FLAG_OFF;
    }
  }

  if(inClient->config.outFlowMax > 0)
  {
    if(inClient->outFlowCount >= inClient->config.outFlowMax)
    {
      outputMask = GENIF_FLAG_OFF;
    }
  }

  if(inClient->outputMask != outputMask)
  {
    inClient->outputMask = outputMask;
  }

//----------------

  TRAZA0("Returning from GENIF_client_output_mask()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
GENIF_client_external_queue_set
(
  GENIF_client_t*	inClient,
  GENIF_message_t*	(*inExtQueueFunc)(GENIF_client_t*, GENIF_channel_t*)
)
{
  int			ret = GENIF_RC_OK;

  TRAZA2("Entering in GENIF_client_external_queue_set(%p, %p)", 
          inClient,
	  inExtQueueFunc);

//----------------

  inClient->extQueueFunc = inExtQueueFunc;

//----------------

  TRAZA1("Returning from GENIF_client_external_queue_set() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
GENIF_client_external_queue(GENIF_client_t* inClient, int inFlag)
{
  GENIF_channel_t*		ptrChannel;

  int				ret = GENIF_RC_OK;

  TRAZA2("Entering in GENIF_client_external_queue(%p, %d)", 
          inClient,
	  inFlag);

//----------------

  if(inClient->extQueueFunc != NULL)
  {
    inClient->extQueueFlag = inFlag;

    RLST_resetGet(inClient->channel, NULL);

    while((ptrChannel = RLST_getNext(inClient->channel)))
    {
      GENIF_channel_external_queue(ptrChannel, inFlag);
    }
  }

//----------------

  TRAZA1("Returning from GENIF_client_external_queue() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
GENIF_client_send
(
  GENIF_client_t*		inClient,
  GENIF_message_t*		inMessage
)
{
  GENIF_channel_t*		ptrChannel;

  int				sent = GENIF_FALSE;

  int				ret = GENIF_RC_OK;

  TRAZA2("Entering in GENIF_client_send(%p, %p)", inClient, inMessage);

//----------------

  if(inClient->outputMask == GENIF_FLAG_ON)
  {
    RLST_resetGet(inClient->channel, NULL);

    ptrChannel = RLST_getNext(inClient->channel);

    while(ptrChannel != NULL)
    {
      if(ptrChannel->outBuffMsg == NULL)
      {
	ret = GENIF_channel_send(ptrChannel, inMessage);

	ptrChannel = NULL; sent = GENIF_TRUE;
      }

      else
      {
	GENIF_channel_external_queue(ptrChannel, GENIF_FLAG_ON);

	ptrChannel = RLST_getNext(inClient->channel);
      }
    }
  }

//----------------

  if(sent == GENIF_FALSE)
  {
    inClient->counter->channel.outMessage++;

    if(inClient->config.outQueueMax > 0)
    {
      if(RLST_getNumElem(inClient->outQueue) >= inClient->config.outQueueMax)
      {
	inClient->counter->channel.outMessageError++;

	ret = GENIF_RC_QUEUE_FULL;
      }
    }

    if(ret == GENIF_RC_OK)
    {
      inMessage->type    = GENIF_MESSAGE_TYPE_MESSAGE;
      inMessage->channel = NULL;
      inMessage->T       = uv_now(inClient->loop);

      if(RLST_insertTail(inClient->outQueue, inMessage) < 0)
      {
	GENIF_FATAL0("FATAL: RLST_insertTail()");
      }
    }
  }

//----------------
  GENIF_client_mask(inClient);
//----------------

  TRAZA1("Returning from GENIF_client_send() = %d", ret);

  return ret;
}
    
/*----------------------------------------------------------------------------*/

int
GENIF_client_request
(
  GENIF_client_t*		inClient,
  GENIF_message_t*		inMessage
)
{
  GENIF_channel_t*		ptrChannel;

  int				sent = GENIF_FALSE;

  int				ret = GENIF_RC_OK;

  TRAZA2("Entering in GENIF_client_request(%p, %p)", inClient, inMessage);

//----------------

  inClient->counter->channel.outRequest++;

  if(inClient->config.outQueueMax > 0)
  {
    if(RLST_getNumElem(inClient->outQueue) >= inClient->config.outQueueMax)
    {
      inClient->counter->channel.outRequestError++;

      ret = GENIF_RC_QUEUE_FULL;
    }
  }

//----------------

  if(ret == GENIF_RC_OK)
  {
    inMessage->type    = GENIF_MESSAGE_TYPE_REQUEST;
    inMessage->channel = NULL;
    inMessage->T       = uv_now(inClient->loop);

    if(inClient->outputMask == GENIF_FLAG_ON)
    {
      RLST_resetGet(inClient->channel, NULL);

      ptrChannel = RLST_getNext(inClient->channel);

      while(ptrChannel != NULL)
      {
        if(ptrChannel->outBuffMsg == NULL)
        {
          inClient->outFlowCount++; inClient->outRefCount++;

          inMessage->channel = ptrChannel;

          GENIF_channel_write(ptrChannel, inMessage);

          ptrChannel = NULL; sent = GENIF_TRUE;
        }

        else
        {
          GENIF_channel_external_queue(ptrChannel, GENIF_FLAG_ON);

          ptrChannel = RLST_getNext(inClient->channel);
        }
      }
    }

    if(sent == GENIF_FALSE)
    {
      if(RLST_insertTail(inClient->outQueue, inMessage) < 0)
      {
        GENIF_FATAL0("FATAL: RLST_insertTail()");
      }
    }
    
    else { GENIF_client_output_mask(inClient); }
  }

//----------------

  TRAZA1("Returning from GENIF_client_request() = %d", ret);

  return ret;
}
    
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static void
GENIF_client_dispatch(GENIF_client_t* inClient)
{
  GENIF_channel_t*		ptrChannel;
  GENIF_message_t*		ptrMessage;

  TRAZA1("Entering in GENIF_client_dispatch(%p)", inClient);

//----------------

  if(inClient->outputMask == GENIF_FLAG_ON)
  {
    RLST_resetGet(inClient->channel, NULL);

    ptrChannel = RLST_getNext(inClient->channel);

    ptrMessage = RLST_gettHead(inClient->outQueue);

    while(ptrChannel != NULL && ptrMessage != NULL)
    {
      if(ptrChannel->writeMask == GENIF_TRUE && ptrChannel->outBuffMsg == NULL)
      {
	ptrMessage = RLST_extractHead(inClient->outQueue);

	if(ptrMessage != NULL)
	{
          inClient->outFlowCount++; inClient->outRefCount++;

          ptrMessage->channel = ptrChannel;

          GENIF_channel_write(ptrChannel, ptrMessage);

          GENIF_client_output_mask(inClient);

          if(inClient->outputMask == GENIF_FLAG_ON)
          {
            ptrChannel = RLST_getNext(inClient->channel);
          }

          else { ptrChannel = NULL; }
	}
      }

      else // if(ptrChannel->outBuffMsg != NULL)
      {
	GENIF_channel_external_queue(ptrChannel, GENIF_FLAG_ON);

	ptrChannel = RLST_getNext(inClient->channel);
      }
    }
  }

//----------------

  TRAZA0("Returning from GENIF_client_dispatch()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
GENIF_client_channel_send
(
  GENIF_client_t*		inClient,
  GENIF_channel_t* 		inChannel,
  GENIF_message_t*		inMessage
)
{
  int				ret = GENIF_RC_OK;

  TRAZA3("Entering in GENIF_client_channel_send(%p, %p, %p)", 
          inClient,
          inChannel,
	  inMessage);

//----------------

  if(inChannel->parent == inClient)
  {
    ret = GENIF_channel_send(inChannel, inMessage);
  }

  else { GENIF_FATAL("ASSERTION!!"); }

//----------------

  TRAZA1("Returning from GENIF_client_channel_send() = %d", ret);

  return ret;
}
    
/*----------------------------------------------------------------------------*/

int
GENIF_client_channel_request
(
  GENIF_client_t*		inClient,
  GENIF_channel_t* 		inChannel,
  GENIF_message_t*		inMessage
)
{
  int				ret = GENIF_RC_OK;

  TRAZA3("Entering in GENIF_client_channel_request(%p, %p, %p)", 
          inClient,
          inChannel,
	  inMessage);

//----------------

  if(inChannel->parent == inClient)
  {
    ret = GENIF_channel_request(inChannel, inMessage);
  }

  else { GENIF_FATAL("ASSERTION!!"); }

//----------------

  TRAZA1("Returning from GENIF_client_channel_request() = %d", ret);

  return ret;
}
    
/*----------------------------------------------------------------------------*/

int
GENIF_client_reply
(
  GENIF_client_t*		inClient,
  GENIF_message_t*		inMessage
)
{
  int				ret = GENIF_RC_OK;

  TRAZA2("Entering in GENIF_client_reply(%p, %p)", inClient, inMessage);

//----------------

  if(inMessage->channel != NULL)
  {
    if(inMessage->channel->parent == inClient)
    {
      ret = GENIF_channel_reply(inMessage->channel, inMessage);
    }

    else { GENIF_FATAL("ASSERTION!!"); }
  }

  else { GENIF_FATAL("ASSERTION!!"); }

//----------------

  TRAZA1("Returning from GENIF_client_reply() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
GENIF_client_unref_msg
(
  GENIF_client_t*		inClient,
  GENIF_message_t*		inMessage
)
{
  int				ret = GENIF_RC_OK;

  TRAZA2("Entering in GENIF_client_unref_msg(%p, %p)", inClient, inMessage);

//----------------

  if(RLST_extract(inClient->outQueue, inMessage) == NULL)
  {
    if(inMessage->channel != NULL)
    {
      if(inMessage->channel->parent == inClient)
      {
	ret = GENIF_channel_unref_msg(inMessage->channel, inMessage);
      }

      else { ret = GENIF_RC_NOT_FOUND; }
    }

    else { ret = GENIF_RC_NOT_FOUND; }
  }

//----------------

  TRAZA1("Returning from GENIF_client_unref_msg() = %d", ret);

  return ret;
}
    
/*----------------------------------------------------------------------------*/

int
GENIF_client_cancel_msg
(
  GENIF_client_t*		inClient,
  GENIF_message_t*		inMessage,
  int*				outStore
)
{
  GENIF_message_t*		ptrMessage;

  int				ret = GENIF_RC_OK;

  TRAZA2("Entering in GENIF_client_cancel_msg(%p, %p)", 
          inClient,
	  inMessage);

//----------------

  ptrMessage = RLST_extract(inClient->outQueue, inMessage);

  if(ptrMessage != NULL)
  {
    inClient->counter->channel.outRequestCanceled++;

    *outStore = GENIF_O_STORE;
  }

//----------------

  else if(inMessage->channel != NULL)
  {
    if(inMessage->channel->parent == inClient)
    {
      ret = GENIF_channel_cancel_msg(inMessage->channel,
				     inMessage,
				     outStore);

      if((ret == GENIF_RC_OK) && (*outStore == GENIF_W_STORE))
      {
        inClient->outRefCount--;

        GENIF_client_mask(inClient);
      }
    }

    else { ret = GENIF_RC_NOT_FOUND; }
  }

  else { ret = GENIF_RC_NOT_FOUND; }

//----------------

  TRAZA1("Returning from GENIF_client_cancel_msg() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

long
GENIF_client_message_num
(
  GENIF_client_t*		inClient,
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
  
  TRAZA1("Entering in GENIF_client_message_num(%p)", inClient);

//----------------

  if(outOstore != NULL) *outOstore = 0; 
  if(outWstore != NULL) *outWstore = 0; 
  if(outIstore != NULL) *outIstore = 0; 

//----------------

  msgT = RLST_getNumElem(inClient->outQueue);

  if(outOstore != NULL) *outOstore += msgT;

//----------------

  RLST_resetGet(inClient->channel, NULL);

  while((ptrChannel = RLST_getNext(inClient->channel)))
  {
    msgT += GENIF_channel_message_num(ptrChannel, &msgO, &msgW, &msgI);

    if(outOstore != NULL) *outOstore += msgO;
    if(outWstore != NULL) *outWstore += msgW;
    if(outIstore != NULL) *outIstore += msgI;
  }

//----------------

  TRAZA1("Returning from GENIF_client_message_num() = %ld", msgT);

  return msgT;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static GENIF_message_t* 
GENIF_client_channel_write_cb(GENIF_channel_t* inChannel)
{
  GENIF_client_t*		inClient = inChannel->parent;

  GENIF_message_t*		ptrMsgChn;
  GENIF_message_t*		ptrMsgCli;

  GENIF_message_t*		ptrMessage = NULL;

  TRAZA2("Entering in GENIF_client_channel_write_cb(%p, %p)", 
          inChannel,
	  inClient);

//----------------

  ptrMsgChn = RLST_getHead(inChannel->outQueue);
  ptrMsgCli = RLST_getHead(inClient->outQueue);

  if(ptrMsgCli == NULL)
  {
    if(inClient->extQueueFlag == GENIF_FLAG_ON)
    {
      if(inClient->extQueueFunc != NULL)
      {
        ptrMessage = inClient->extQueueFunc(inClient, inChannel);
      }

      if(ptrMessage == NULL)
      {
        inClient->extQueueFlag = GENIF_FLAG_OFF;
      }

      else { inChannel->counter->outRequest++; }
    }
  }

  else if(ptrMsgChn == NULL)
  {
    ptrMessage = RLST_extractHead(inClient->outQueue);
  }

  else if(ptrMsgCli->O < ptrMsgChn->O)
  {
    ptrMessage = RLST_extractHead(inClient->outQueue);
  }

  else { ptrMessage = RLST_extractHead(inChannel->outQueue); }

//----------------

  TRAZA1("Returning from GENIF_client_channel_write_cb() = %p", ptrMessage);

  return ptrMessage;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static void 
GENIF_client_channel_notify_cb
(
  GENIF_channel_t*		inChannel,
  int				inNotify,
  GENIF_message_t*		inMessage
)
{
  GENIF_client_t*		inClient = inChannel->parent;

  TRAZA3("Entering in GENIF_client_channel_notify_cb(%p, %p, %d)", 
          inChannel,
	  inMessage,
	  inNotify);

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
      {
        inClient->outRefCount--;
	GENIF_client_mask(inClient);

        GENIF_client_notify(inClient, inNotify, inChannel, inMessage);
	
        break;
      }
      
      case GENIF_NOTIFY_REQUEST_SENT:
      {
        // inClient->outRefCount++; inClient->outFlowCount++;
	// GENIF_client_mask(inClient);

        GENIF_client_notify(inClient, inNotify, inChannel, inMessage);
	
        break;
      }
      
      case GENIF_NOTIFY_REQUEST_SENT_CLOSED:
      case GENIF_NOTIFY_REQUEST_SENT_TIMEOUT:
      {
        inClient->outRefCount--;
	GENIF_client_mask(inClient);
	
        GENIF_client_notify(inClient, inNotify, inChannel, inMessage);
	
        break;
      }
      
      case GENIF_NOTIFY_RESPONSE_RECEIVED:
      {
        inClient->outRefCount--;
	GENIF_client_mask(inClient);
	
        GENIF_client_notify(inClient, inNotify, inChannel, inMessage);
	
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
        GENIF_client_notify(inClient, inNotify, inChannel, inMessage);
	
        break;
      }

//----------------

      case GENIF_NOTIFY_MESSAGE_ERROR:
      case GENIF_NOTIFY_MESSAGE_CLOSED:
      case GENIF_NOTIFY_MESSAGE_TIMEOUT:
      {
        GENIF_client_notify(inClient, inNotify, inChannel, inMessage);
	
        break;
      }
      
      case GENIF_NOTIFY_MESSAGE_SENT:
      {
        // inClient->outFlowCount++;
	// GENIF_client_mask(inClient);

        GENIF_client_notify(inClient, inNotify, inChannel, inMessage);
	
        break;
      }
      

      case GENIF_NOTIFY_MESSAGE_RECEIVED:
      case GENIF_NOTIFY_MESSAGE_RECEIVED_CLOSED:
      case GENIF_NOTIFY_MESSAGE_RECEIVED_TIMEOUT:
      {
        GENIF_client_notify(inClient, inNotify, inChannel, inMessage);
	
        break;
      }

//----------------

      case GENIF_NOTIFY_UNKNOWN_RECEIVED:
      case GENIF_NOTIFY_UNKNOWN_RECEIVED_CLOSED:
      case GENIF_NOTIFY_UNKNOWN_RECEIVED_TIMEOUT:
      {
        GENIF_client_notify(inClient, inNotify, inChannel, inMessage);
	
        break;
      }
      
//----------------

      case GENIF_NOTIFY_CHANNEL_HEARTBEAT:
      {
        GENIF_client_notify(inClient, inNotify, inChannel, inMessage);
	
	break;
      }

      case GENIF_NOTIFY_CHANNEL_INACTIVITY:
      {
	SUCESO2("WARNING: CONNECTION (%s, %d) INACTIVITY",
		 inClient->remHost,
		 inClient->remPort);

        GENIF_channel_empty(inChannel, GENIF_END_CLOSE);

        GENIF_client_notify(inClient, inNotify, inChannel, inMessage);

        GENIF_client_channel_close_cb(inClient, inChannel);

	break;
      }

      case GENIF_NOTIFY_CHANNEL_MALFUNCTION:
      {
	SUCESO2("WARNING: CONNECTION (%s, %d) MALFUNCTION",
		 inClient->remHost,
		 inClient->remPort);

        GENIF_channel_empty(inChannel, GENIF_END_CLOSE);

        GENIF_client_notify(inClient, inNotify, inChannel, inMessage);

        GENIF_client_channel_close_cb(inClient, inChannel);

	break;
      }

      case GENIF_NOTIFY_CHANNEL_ERROR:
      case GENIF_NOTIFY_CHANNEL_CLOSED:
      {
        GENIF_client_notify(inClient, inNotify, inChannel, inMessage);

        GENIF_client_channel_close_cb(inClient, inChannel);
	
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

  TRAZA0("Returning from GENIF_client_channel_notify_cb()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static void 
GENIF_client_notify
(
  GENIF_client_t*		inClient,
  int					inNotify,
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMessage
)
{
  inClient->notDelete = GENIF_TRUE;

  inClient->cbNotify(inClient, inNotify, inChannel, inMessage);

  if(RLST_getNumElem(inClient->channel) <= 0)
  {
    inClient->notDelete = GENIF_FALSE;
  }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

