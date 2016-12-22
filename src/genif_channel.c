/*____________________________________________________________________________

  MODULO:	Canal protocolo peticio'n respuesta generico

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

//---------------- // Channel

static int GENIF_channel_message_end
(
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMessage,
  int				inEndType,
  int				inStoreType
);

static int GENIF_channel_message_timeout(GENIF_channel_t* inChannel);

static void GENIF_channel_mask(GENIF_channel_t* inChannel);

static int GENIF_channel_read_mask(GENIF_channel_t* inChannel);
static int GENIF_channel_write_mask(GENIF_channel_t* inChannel);

static void GENIF_channel_close_cb(GENIF_channel_t* inChannel);
static void GENIF_channel_error_cb(GENIF_channel_t* inChannel, int inError);

static void GENIF_channel_alloc_cb
(
  uv_handle_t*			inHandle,
  size_t			inSsize,
  uv_buf_t*			inUvBuf
);

static void GENIF_channel_read_cb
(
  uv_stream_t*			inStream,
  ssize_t			inReadLen,
  const uv_buf_t*		inEvBuff
);

static void GENIF_channel_write
(
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMessage
);

static void GENIF_channel_write_cb(uv_write_t* inUvReq, int inStatus);

static void GENIF_channel_process_in_msg_cb
(
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMessage
);

static void GENIF_channel_process_out_msg_cb
(
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMessage
);

static void GENIF_channel_notify
(
  GENIF_channel_t*		inChannel,
  int				inNotify,
  GENIF_message_t*		inMessage
);

//----------------

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int 
GENIF_channel_initialize
(
  GENIF_channel_t* 		inChannel,
  int				inFd,
  void*				inParent,
  int				inChannelType,
  GENIF_message_t*		(*inExtQueueFunc)(GENIF_channel_t*),
  void				(*inCbNotify)(GENIF_channel_t*, int,
					      GENIF_message_t*),
  GENIF_channel_config_t*	inConfig,
  GENIF_channel_counter_t*	inCounter,
  GENIF_modifier_t*		inModifier
)
{
  GENIF_message_t		m;

  int				ret = GENIF_RC_OK;

  TRAZA2("Entering in GENIF_channel_initialize(%p, %d)", inChannel, inFd);

//----------------

  if(ret == GENIF_RC_OK)
  {
    memset(inChannel, 0, sizeof(GENIF_channel_t));

    inChannel->loop     = uv_default_loop();

    inChannel->type     = inChannelType;
    inChannel->parent   = inParent;
    inChannel->cbNotify = inCbNotify;

    inChannel->modifier = inModifier;

    inChannel->T = uv_now(inChannel->loop);
    
    inChannel->heartbeatT   = inChannel->T;
    inChannel->inactivityT  = inChannel->T;
    inChannel->malfunctionT = inChannel->T;

    inChannel->extQueueFunc = inExtQueueFunc;

    inChannel->config  = inConfig;
    inChannel->counter = inCounter;
  }

//----------------

  if(ret == GENIF_RC_OK)
  {
    if(RLST_initializeRefList(inChannel->outQueue, &m, &m.list) < 0)
    {
      GENIF_FATAL0("FATAL: RLST_initializeRefList()");
    }

    if(RFTR_initializeRefTree(inChannel->outRef, &m, &m.tree, GENIF_message_cmp) < 0)
    {
      GENIF_FATAL("ERROR: RFTR_initializeRefTree()");
    }

    if(RLST_initializeRefList(inChannel->outRefTout, &m, &m.list) < 0)
    {
      GENIF_FATAL0("FATAL: RLST_initializeRefList()");
    }

    if(RLST_initializeRefList(inChannel->inRef, &m, &m.list) < 0)
    {
      GENIF_FATAL0("FATAL: RLST_initializeRefList()");
    }
  }

//----------------

  if(ret == GENIF_RC_OK)
  {
    GENIF_channel_mask(inChannel);
  }

//----------------

  TRAZA1("Returning from GENIF_channel_initialize() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int 
GENIF_channel_finalize(GENIF_channel_t* inChannel)
{
  int				ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_channel_finalize(%p)", inChannel);

//----------------

  uv_close((uv_handle_t*)(inChannel->channel), NULL);

  GENIF_channel_empty(inChannel, GENIF_END_USER);

//----------------

  TRAZA1("Returning from GENIF_channel_finalize() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int 
GENIF_channel_address(GENIF_channel_t* inChannel)
{
/*
  struct sockaddr		saddr;
  struct sockaddr_in		saddrin;

  static socklen_t		len = sizeof(saddrin);

  socklen_t			len1 = len;
  socklen_t			len2 = len;

  int				rc;
*/
  int				ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_channel_address(%p)", inChannel);
/*
//----------------

  if(inChannel->fd != -1)
  {
    rc = getsockname(inChannel->fd, &saddr, &len1);

    if(rc == 0)
    {
      memcpy(&saddrin, &saddr, len);

      strncpy(inChannel->locHost, inet_ntoa(saddrin.sin_addr), GENIF_MAXLEN_HOST);

      inChannel->locHost[GENIF_MAXLEN_HOST] = 0;

      inChannel->locPort = ntohs(saddrin.sin_port);

      SUCESO3("CHANNEL (%d) LOCAL  ADDRESS (%s, %d)",
               inChannel->fd,
	       inChannel->locHost,
	       inChannel->locPort);
    }

//----------------

    rc = getpeername(inChannel->fd, &saddr, &len2);

    if(rc == 0)
    {
      memcpy(&saddrin, &saddr, len);

      strncpy(inChannel->remHost, inet_ntoa(saddrin.sin_addr), GENIF_MAXLEN_HOST);

      inChannel->remHost[GENIF_MAXLEN_HOST] = 0;

      inChannel->remPort = ntohs(saddrin.sin_port);

      SUCESO3("CHANNEL (%d) REMOTE ADDRESS (%s, %d)", 
               inChannel->fd,
	       inChannel->remHost,
	       inChannel->remPort);
    }
  }

//----------------
*/
  TRAZA1("Returning from GENIF_channel_address() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
GENIF_channel_empty(GENIF_channel_t* inChannel, int inEndType)
{
  GENIF_message_t*		ptrMsg = NULL;

  int				ret = GENIF_RC_OK;

  TRAZA2("Entering in GENIF_channel_empty(%p, %d)", inChannel, inEndType);

//----------------

  if(inChannel->outBuffMsg != NULL)
  {
    ptrMsg = inChannel->outBuffMsg; inChannel->outBuffMsg = NULL;
    
    GENIF_channel_message_end(inChannel, ptrMsg, inEndType, GENIF_O_STORE);
  }

  while((ptrMsg = RLST_extractHead(inChannel->outQueue)))
  {
    GENIF_channel_message_end(inChannel, ptrMsg, inEndType, GENIF_O_STORE);
  }

//----------------

  while((ptrMsg = RLST_extractHead(inChannel->outRefTout)))
  {
    RFTR_extract(inChannel->outRef, ptrMsg);

    GENIF_channel_message_end(inChannel, ptrMsg, inEndType, GENIF_W_STORE);
  }

//----------------

  if(inChannel->inBuffMsg != NULL)
  {
    GENIF_message_delete(inChannel->inBuffMsg);
    
    inChannel->inBuffMsg = NULL;
  }

  while((ptrMsg = RLST_extractHead(inChannel->inRef)))
  {
    GENIF_channel_message_end(inChannel, ptrMsg, inEndType, GENIF_I_STORE);
  }

//----------------

  TRAZA1("Returning from GENIF_channel_empty() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

static int
GENIF_channel_message_end
(
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMessage,
  int				inEndType,
  int				inStoreType
)
{
  char				strMsg[GENIF_MAXLEN_STRING + 1];
  char				strErr[GENIF_MAXLEN_STRING + 1];

  int				notify = GENIF_NOTIFY_NONE;

  int				delete = GENIF_FALSE;

  int				ret = GENIF_RC_OK;

  TRAZA2("Entering in GENIF_channel_message_end(%p, %p)", inChannel, inMessage);

//----------------

  switch(inMessage->type)
  {
    case GENIF_MESSAGE_TYPE_REQUEST:
    {
      switch(inStoreType)
      {
        case GENIF_O_STORE:
	{
	  delete = GENIF_FALSE;
	  
          switch(inEndType)
          {
	    case GENIF_END_USER:
	    {
	      inChannel->counter->outRequestCanceled++;

 	      notify = GENIF_NOTIFY_NONE;

	      strcpy(strErr, "WARNING: CANCELED QUEUED REQUEST");

	      break;
	    }

	    case GENIF_END_CLOSE:
	    {
	      inChannel->counter->outRequestClosed++;

	      notify = GENIF_NOTIFY_REQUEST_CLOSED;

	      break;
	    }

	    case GENIF_END_TIMEOUT:
	    {
	      inChannel->counter->outRequestTimeout++;

	      notify = GENIF_NOTIFY_REQUEST_TIMEOUT;

	      break;
	    }
	  }

	  break;
	}

        case GENIF_W_STORE:
	{
	  delete = GENIF_FALSE;
	  
          switch(inEndType)
          {
	    case GENIF_END_USER:
	    {
	      inChannel->counter->winRequestCanceled++;

 	      notify = GENIF_NOTIFY_NONE;

	      strcpy(strErr, "WARNING: CANCELED SENT REQUEST");

	      break;
	    }

	    case GENIF_END_CLOSE:
	    {
	      inChannel->counter->winRequestClosed++;

	      notify = GENIF_NOTIFY_REQUEST_SENT_CLOSED;

	      break;
	    }

	    case GENIF_END_TIMEOUT:
	    {
	      inChannel->counter->winRequestTimeout++;

	      notify = GENIF_NOTIFY_REQUEST_SENT_TIMEOUT;

	      break;
	    }
	  }

	  break;
	}

        case GENIF_I_STORE:
	{
	  delete = GENIF_TRUE;
	  
          switch(inEndType)
          {
	    case GENIF_END_USER:
	    {
	      inChannel->counter->inRequestCanceled++;

 	      notify = GENIF_NOTIFY_NONE;

	      strcpy(strErr, "WARNING: CANCELED RECEVIED REQUEST");

	      break;
	    }

            case GENIF_END_CLOSE:
	    {
	      inChannel->counter->inRequestClosed++;

	      notify = GENIF_NOTIFY_REQUEST_RECEIVED_CLOSED;

	      break;
	    }

            case GENIF_END_TIMEOUT:
	    {
	      inChannel->counter->inRequestTimeout++;

	      notify = GENIF_NOTIFY_REQUEST_RECEIVED_TIMEOUT;

	      break;
	    }
          }

          break;
        }
      }

      break;
    }
	
//----------------

    case GENIF_MESSAGE_TYPE_RESPONSE:
    {
      switch(inStoreType)
      {
        case GENIF_O_STORE:
	{
	  delete = GENIF_TRUE;
	  
          switch(inEndType)
          {
	    case GENIF_END_USER:
	    {
	      inChannel->counter->outResponseCanceled++;

 	      notify = GENIF_NOTIFY_NONE;

	      strcpy(strErr, "WARNING: CANCELED QUEUED RESPONSE");

	      break;
	    }

	    case GENIF_END_CLOSE:
	    {
	      inChannel->counter->outResponseClosed++;

	      notify = GENIF_NOTIFY_RESPONSE_CLOSED;

	      break;
	    }

	    case GENIF_END_TIMEOUT:
	    {
	      inChannel->counter->outResponseTimeout++;

	      notify = GENIF_NOTIFY_RESPONSE_TIMEOUT;

	      break;
	    }
	  }

	  break;
	}

        case GENIF_W_STORE:
        case GENIF_I_STORE:
	{
	  GENIF_FATAL("ASSERTION!!");

	  break;
	}

      }

      break;
    }

//----------------

    case GENIF_MESSAGE_TYPE_MESSAGE:
    {
      switch(inStoreType)
      {
        case GENIF_O_STORE:
	{
	  delete = GENIF_FALSE;
	  
          switch(inEndType)
          {
	    case GENIF_END_USER:
	    {
	      inChannel->counter->outMessageCanceled++;

 	      notify = GENIF_NOTIFY_NONE;

	      strcpy(strErr, "WARNING: CANCELED QUEUED MESSAGE");

	      break;
	    }

	    case GENIF_END_CLOSE:
	    {
	      inChannel->counter->outMessageClosed++;

	      notify = GENIF_NOTIFY_MESSAGE_CLOSED;

	      break;
	    }

	    case GENIF_END_TIMEOUT:
	    {
	      inChannel->counter->outMessageTimeout++;

	      notify = GENIF_NOTIFY_MESSAGE_TIMEOUT;

	      break;
	    }
	  }

	  break;
	}

        case GENIF_W_STORE:
	{
	  GENIF_FATAL("ASSERTION!!");

	  break;
	}

        case GENIF_I_STORE:
	{
	  delete = GENIF_TRUE;
	  
          switch(inEndType)
          {
	    case GENIF_END_USER:
	    {
 	      notify = GENIF_NOTIFY_NONE;

	      strcpy(strErr, "");

	      break;
	    }

            case GENIF_END_CLOSE:
	    {
	      inChannel->counter->inMessageClosed++;

	      notify = GENIF_NOTIFY_MESSAGE_RECEIVED_CLOSED;

	      break;
	    }

            case GENIF_END_TIMEOUT:
	    {
	      inChannel->counter->inMessageTimeout++;

	      notify = GENIF_NOTIFY_MESSAGE_RECEIVED_TIMEOUT;

	      break;
	    }
          }

          break;
        }
      }

      break;
    }

//----------------

    case GENIF_MESSAGE_TYPE_UNKNOWN:
    {
      switch(inStoreType)
      {
        case GENIF_O_STORE:
        case GENIF_W_STORE:
	{
	  GENIF_FATAL("ASSERTION!!");

	  break;
	}

        case GENIF_I_STORE:
	{
	  delete = GENIF_TRUE;
	  
          switch(inEndType)
          {
            case GENIF_END_USER:
	    {
 	      notify = GENIF_NOTIFY_NONE;

	      strcpy(strErr, "");

	      break;
	    }

            case GENIF_END_CLOSE:
	    {
	      notify = GENIF_NOTIFY_UNKNOWN_RECEIVED_CLOSED;

	      break;
	    }

            case GENIF_END_TIMEOUT:
	    {
	      notify = GENIF_NOTIFY_UNKNOWN_RECEIVED_TIMEOUT;

	      break;
	    }
          }

          break;
        }
      }

      break;
    }
  }

//----------------

  if(notify == GENIF_NOTIFY_NONE)
  {
    if(strlen(strErr) > 0) 
    {
      GENIF_message_dump(inMessage, GENIF_MAXLEN_STRING + 1, strMsg);

      SUCESO2("%s\n%s", strErr, strMsg);
    }
  }

  else { GENIF_channel_notify(inChannel, notify, inMessage); }

  if(delete == GENIF_TRUE)
  {
    GENIF_message_delete(inMessage);
  }

//----------------

  TRAZA1("Returning from GENIF_channel_message_end() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void 
GENIF_channel_timer_cb(GENIF_channel_t* inChannel, long inT)
{
  long				T = inT;

  TRAZA2("Entering in GENIF_channel_timer_cb(%p, %ld)", inChannel, inT);

  if(inChannel->connected == GENIF_TRUE)
  {
    inChannel->heartbeatT   = T;
    inChannel->inactivityT  = T;
    inChannel->malfunctionT = T;
  }

//----------------
  GENIF_channel_message_timeout(inChannel);
//----------------

  if(inChannel->config->heartbeatTout > 0)
  {
    if((inChannel->inFlowCount != 0) || (inChannel->outFlowCount != 0))
    {
      inChannel->heartbeatT = T;
    }

    else if(RLST_getNumElem(inChannel->outQueue))
    {
      inChannel->heartbeatT = T;
    }

    else if(RLST_getNumElem(inChannel->inRef))
    {
      inChannel->heartbeatT = T;
    }

    else if((T - inChannel->heartbeatT) >= inChannel->config->heartbeatTout)
    {
      GENIF_channel_notify(inChannel, GENIF_NOTIFY_CHANNEL_HEARTBEAT, NULL);

      inChannel->heartbeatT = T;
    }
  }

//----------------

  if(inChannel->config->inactivityTout > 0)
  {
    if((inChannel->inFlowCount != 0) || (inChannel->outFlowCount != 0))
    {
      inChannel->inactivityT = T;
    }

    else if(RLST_getNumElem(inChannel->outQueue))
    {
      inChannel->inactivityT = T;
    }

    else if(RLST_getNumElem(inChannel->inRef))
    {
      inChannel->inactivityT = T;
    }

    if((T - inChannel->inactivityT) >= inChannel->config->inactivityTout)
    {
      GENIF_channel_notify(inChannel, GENIF_NOTIFY_CHANNEL_INACTIVITY, NULL);

      inChannel->inactivityT = T;
    }
  }

//----------------

  if(inChannel->config->malfunctionTout > 0)
  {
    if(inChannel->config->outToutsMax > 0)
    {
      if(inChannel->outToutsCount >= inChannel->config->outToutsMax)
      {
	GENIF_channel_notify(inChannel, GENIF_NOTIFY_CHANNEL_MALFUNCTION, NULL);

	inChannel->outToutsCount = 0; inChannel->malfunctionT = T;
      }
    }

    if((T - inChannel->malfunctionT) >= inChannel->config->malfunctionTout)
    {
      inChannel->malfunctionT = T; inChannel->outToutsCount = 0;
    }
  }
  
//----------------

  inChannel->inFlowCount = 0; inChannel->outFlowCount = 0;

  GENIF_channel_mask(inChannel);

//----------------

  TRAZA0("Returning from GENIF_channel_timer_cb()");
}

/*----------------------------------------------------------------------------*/

static int
GENIF_channel_message_timeout(GENIF_channel_t* inChannel)
{
  long				T = uv_now(inChannel->loop);

  long				Tout = 0;

  GENIF_message_t*		ptrMsg = NULL;

  int				endTout = 0;

  int				ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_channel_message_timeout(%p)", inChannel);

//----------------

  if(inChannel->config->outQueueTout > 0)
  {
    if(RLST_getNumElem(inChannel->outQueue))
    {
      Tout = T - inChannel->config->outQueueTout; endTout = 0;
  
      while((ptrMsg = RLST_getHead(inChannel->outQueue)) && (!endTout))
      {
        if(Tout >= ptrMsg->T)
        {
          RLST_extract(inChannel->outQueue, ptrMsg);
  
          GENIF_channel_message_end(inChannel, ptrMsg,
	                            GENIF_END_TIMEOUT,
				    GENIF_O_STORE);
        }
  
        else { endTout = 1; }
      }
    }
  }

//----------------

  if(inChannel->config->outRefTout > 0)
  {
    if(RLST_getNumElem(inChannel->outRefTout))
    {
      Tout = T - inChannel->config->outRefTout; endTout = 0;

      while((ptrMsg = RLST_getHead(inChannel->outRefTout)) && (!endTout))
      {
	if(Tout >= ptrMsg->T)
	{
	  RFTR_extract(inChannel->outRef,     ptrMsg);
	  RLST_extract(inChannel->outRefTout, ptrMsg);

	  GENIF_channel_message_end(inChannel, ptrMsg,
	                            GENIF_END_TIMEOUT,
				    GENIF_W_STORE);

	  inChannel->outToutsCount++;
	}

	else { endTout = 1; }
      }
    }
  }

//----------------

  if(inChannel->config->inRefTout > 0)
  {
    if(RLST_getNumElem(inChannel->inRef))
    {
      Tout = T - inChannel->config->inRefTout; endTout = 0;

      while((ptrMsg = RLST_getHead(inChannel->inRef)) && (!endTout))
      {
	if(Tout >= ptrMsg->T)
	{
	  RLST_extract(inChannel->inRef, ptrMsg);

	  GENIF_channel_message_end(inChannel, ptrMsg,
	                            GENIF_END_TIMEOUT,
				    GENIF_I_STORE);
	}

	else { endTout = 1; }
      }
    }
  }

//----------------

  TRAZA1("Returning from GENIF_channel_message_timeout() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void 
GENIF_channel_read_disable(GENIF_channel_t* inChannel, int inFlag)
{
  TRAZA2("Entering in GENIF_channel_read_disable(%p, %d)", inChannel, inFlag);

//----------------

  if(inChannel->disabledRead != inFlag)
  {
    inChannel->disabledRead = inFlag; GENIF_channel_mask(inChannel);
  }

//----------------

  TRAZA0("Returning from GENIF_channel_read_disable()");
}

/*----------------------------------------------------------------------------*/

void 
GENIF_channel_write_disable(GENIF_channel_t* inChannel, int inFlag)
{
  TRAZA2("Entering in GENIF_channel_write_disable(%p, %d)", inChannel, inFlag);

//----------------

  if(inChannel->disabledWrite != inFlag)
  {
    inChannel->disabledWrite = inFlag; GENIF_channel_mask(inChannel);
  }

//----------------

  TRAZA0("Returning from GENIF_channel_write_disable()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static void 
GENIF_channel_mask(GENIF_channel_t* inChannel)
{
  TRAZA1("Entering in GENIF_channel_mask(%p, %d)", inChannel);

//----------------

  GENIF_channel_read_mask(inChannel);
  GENIF_channel_write_mask(inChannel);

//----------------

  TRAZA0("Returning from GENIF_channel_mask()");
}

/*----------------------------------------------------------------------------*/

static int 
GENIF_channel_read_mask(GENIF_channel_t* inChannel)
{
  int			readMask  = 0;

  TRAZA1("Entering in GENIF_channel_read_mask(%p)", inChannel);

//----------------

  if(inChannel->connected == GENIF_TRUE)
  {
    readMask = 1;

    if(inChannel->disabledRead == GENIF_FLAG_ON)
    {
      readMask = 0;
    }

    if(inChannel->config->inFlowMax > 0)
    {
      if(inChannel->inFlowCount >= inChannel->config->inFlowMax)
      {
        readMask = 0;
      }
    }

    if(inChannel->config->inRefMax > 0)
    {
      if(RLST_getNumElem(inChannel->inRef) >= inChannel->config->inRefMax)
      {
        readMask = 0;
      }
    }

    if(inChannel->readMask != readMask)
    {
      inChannel->readMask = readMask;

      if(inChannel->readMask)
      {
	uv_read_start((uv_stream_t*)(inChannel->channel),
	               GENIF_channel_alloc_cb,
		       GENIF_channel_read_cb);
      }

      else { uv_read_stop((uv_stream_t*)(inChannel->channel)); }
    }
  }

  else { inChannel->readMask = GENIF_FLAG_OFF; }

//----------------

  TRAZA1("Returning from GENIF_channel_read_mask() = %d", readMask);

  return readMask;
}

/*----------------------------------------------------------------------------*/

static int
GENIF_channel_write_mask(GENIF_channel_t* inChannel)
{
  int			writeMask  = 0;

  TRAZA1("Entering in GENIF_channel_write_mask(%p)", inChannel);

//----------------

  if(inChannel->connected == GENIF_TRUE)
  {
    writeMask = 1;

    if(inChannel->config->outFlowMax > 0)
    {
      if(inChannel->outFlowCount >= inChannel->config->outFlowMax)
      {
        writeMask = 0;
      }
    }

    if(inChannel->config->outRefMax > 0)
    {
      if(RLST_getNumElem(inChannel->outRefTout) >= inChannel->config->outRefMax)
      {
        writeMask = 0;
      }
    }

    if(inChannel->writeMask != writeMask)
    {
      inChannel->writeMask = writeMask;
    }
  }

  else { inChannel->writeMask = GENIF_FLAG_OFF; }

//----------------

  TRAZA1("Returning from GENIF_channel_write_mask() = %d", writeMask);

  return writeMask;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static void
GENIF_channel_close_cb
(
  GENIF_channel_t*		inChannel
)
{
  TRAZA1("Entering in GENIF_channel_close_cb(%p)", inChannel);

//----------------

  DEPURA1("WARNING: CHANNEL (%p) CLOSED BY PEER", inChannel);

  inChannel->connected = GENIF_FALSE;

  uv_close((uv_handle_t*)(inChannel->channel), NULL);

  GENIF_channel_empty(inChannel, GENIF_END_CLOSE);

  GENIF_channel_notify(inChannel, GENIF_NOTIFY_CHANNEL_CLOSED, NULL);

//----------------

  TRAZA0("Returning from GENIF_channel_close_cb()");
}

/*----------------------------------------------------------------------------*/

static void
GENIF_channel_error_cb
(
  GENIF_channel_t*		inChannel,
  int				inError
)
{
  TRAZA2("Entering in GENIF_channel_error_cb(%p, %d)", inChannel, inError);

//----------------

  SUCESO2("ERROR: CHANNEL (%p) ERROR (%d)", inChannel, inError);

  inChannel->connected = GENIF_FALSE;

  uv_close((uv_handle_t*)(inChannel->channel), NULL);

  GENIF_channel_empty(inChannel, GENIF_END_CLOSE);

  GENIF_channel_notify(inChannel, GENIF_NOTIFY_CHANNEL_ERROR, NULL);

//----------------

  TRAZA0("Returning from GENIF_channel_error_cb()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static void
GENIF_channel_alloc_cb
(
  uv_handle_t*			inHandle,
  size_t			inSsize,
  uv_buf_t*			inEvBuff
)
{
  GENIF_channel_t*		inChannel = inHandle->data;

  BUFF_part_t* 			part;

  TRAZA3("Entering in GENIF_channel_alloc_cb(%p, %ld, %p)",
          inChannel,
	  inSsize,
	  inEvBuff);

//----------------

  if(inChannel->inBuff->tail == NULL)
  {
    part = BUFF_part_new(inChannel->inBuff);
  }

  else { part = inChannel->inBuff->tail->part; }

  inEvBuff->len = part->type->size - part->len;

  if(inEvBuff->len <= 0)
  {
    part = BUFF_part_new(inChannel->inBuff);

    inEvBuff->len = part->type->size;
  }

  inEvBuff->base = (char*)(part->data + part->len);

//----------------

  TRAZA0("Returning from GENIF_channel_alloc_cb()");
}

/*----------------------------------------------------------------------------*/

static void
GENIF_channel_read_cb
(
  uv_stream_t*			inStream,
  ssize_t			inReadLen,
  const uv_buf_t*		inEvBuff
)
{
  GENIF_channel_t*		inChannel = inStream->data;
  
  GENIF_message_t*		ptrMessage = NULL;

  int				ret = GENIF_RC_OK;

  TRAZA3("Entering in GENIF_channel_read_cb(%p, %ld, %p)",
          inChannel,
	  inReadLen,
	  inEvBuff);

//----------------

  if(inReadLen > 0)
  {
    inChannel->inBuff->idxLen += inReadLen;

    inChannel->inBuff->tail->part->len += inReadLen;
  }

  else if(inReadLen == 0)
  {
    GENIF_channel_close_cb(inChannel);
  }

  else // if(readLen < 0)
  {
    GENIF_channel_error_cb(inChannel, errno);
  }

//----------------

  if(inChannel->inBuff->idxLen > 0)
  {
    if(inChannel->inBuffMsg == NULL)
    {
      inChannel->inBuffMsg = GENIF_message_new(inChannel->modifier);

      inChannel->inBuffMsg->channel = inChannel;
    }

    ret = GENIF_message_decode(inChannel->inBuffMsg, inChannel->inBuff);

    if(ret == GENIF_RC_OK)
    {
      ptrMessage = inChannel->inBuffMsg; inChannel->inBuffMsg = NULL;

      GENIF_channel_process_in_msg_cb(inChannel, ptrMessage);
    }
  
    else if(ret == GENIF_RC_ERROR)
    {
      inChannel->counter->inUnknownMsg++;

      GENIF_message_delete(inChannel->inBuffMsg);

      inChannel->inBuffMsg = NULL;
    }
  }

//----------------

  if(inChannel->inBuff->idxLen >= inChannel->inBuff->type->size)
  {
    SUCESO1("ERROR: Read buffer is full (%ld)", inChannel->inBuff->idxLen);

    GENIF_channel_error_cb(inChannel, -1);
  }

//----------------
  GENIF_channel_mask(inChannel);
//----------------

  TRAZA0("Returning from GENIF_channel_read_cb()");
}

/*----------------------------------------------------------------------------*/

static void
GENIF_channel_process_in_msg_cb
(
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMessage
)
{
  GENIF_message_t*		ptrMsg = NULL;

  char				strMsg[GENIF_MAXLEN_STRING + 1];

  int				rc;

  TRAZA2("Entering in GENIF_channel_process_in_msg_cb(%p, %p)",
          inChannel,
	  inMessage);

//----------------

  switch(inMessage->type)
  {
    case GENIF_MESSAGE_TYPE_REQUEST:
    {
      inChannel->inFlowCount++;

      inChannel->counter->inRequest++;

      inMessage->T = uv_now(inChannel->loop);

      rc = RLST_insertTail(inChannel->inRef, inMessage);

      if(rc != RLST_RC_OK)
      {
	GENIF_FATAL("ERROR: RLST_insertTail()");
      }

      GENIF_channel_notify(inChannel, GENIF_NOTIFY_REQUEST_RECEIVED, inMessage);

      break;
    }

//----------------

    case GENIF_MESSAGE_TYPE_RESPONSE:
    {
      inChannel->counter->inResponse++;

      ptrMsg = RFTR_find(inChannel->outRef, inMessage, NULL, NULL);

      if(ptrMsg != NULL)
      {
	RFTR_extract(inChannel->outRef,     ptrMsg);
	RLST_extract(inChannel->outRefTout, ptrMsg);

	GENIF_message_resp_copy(ptrMsg, inMessage);

	GENIF_channel_notify(inChannel, GENIF_NOTIFY_RESPONSE_RECEIVED, ptrMsg);
      }

      else
      {
        GENIF_message_dump(inMessage, GENIF_MAXLEN_STRING + 1, strMsg);
	
	SUCESO1("ERROR: REQUEST NOT FOUND FOR RESPONSE:\n%s", strMsg);

	inChannel->counter->inResponseError++;
      }

      GENIF_message_delete(inMessage);

      break;
    }

//----------------

    case GENIF_MESSAGE_TYPE_MESSAGE:
    {
      inChannel->inFlowCount++;

      inChannel->counter->inMessage++;

      inMessage->T = uv_now(inChannel->loop);

      rc = RLST_insertTail(inChannel->inRef, inMessage);

      if(rc != RLST_RC_OK)
      {
	GENIF_FATAL("ERROR: RLST_insertTail()");
      }

      GENIF_channel_notify(inChannel, GENIF_NOTIFY_MESSAGE_RECEIVED, inMessage);

      break;
    }

//----------------

    case GENIF_MESSAGE_TYPE_UNKNOWN:
    {
      inChannel->inFlowCount++;

      inChannel->counter->inUnknownMsg++;

      inMessage->T = uv_now(inChannel->loop);

      rc = RLST_insertTail(inChannel->inRef, inMessage);

      if(rc != RLST_RC_OK)
      {
	GENIF_FATAL("ERROR: RLST_insertTail()");
      }

      GENIF_channel_notify(inChannel, GENIF_NOTIFY_UNKNOWN_RECEIVED, inMessage);

      break;
    }

//----------------

    default:
    {
      SUCESO1("ERROR: RECEIVED UNKNOWN MESSAGE TYPE (%d)", inMessage->type);

      inChannel->counter->inUnknownMsg++;
      
      break;
    }
  }

//----------------

  TRAZA0("Returning from GENIF_channel_process_in_msg_cb()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static void
GENIF_channel_write
(
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMessage
)
{
  int				ret = GENIF_RC_OK;

  TRAZA2("Entering in GENIF_channel_write(%p, %p)", inChannel, inMessage);

//----------------

  if(inChannel->writeMask == GENIF_FLAG_OFF)
  {
    if(RLST_insertTail(inChannel->outQueue, inMessage) < 0)
    {
      GENIF_FATAL0("FATAL: RLST_insertTail()");
    }
  }

  else if(inChannel->outBuffMsg != NULL)
  {
    if(RLST_insertTail(inChannel->outQueue, inMessage) < 0)
    {
      GENIF_FATAL0("FATAL: RLST_insertTail()");
    }
  }

//----------------

  else // Write message
  {
    ret = GENIF_message_encode(inChannel->outBuffMsg,
	                       inChannel->outBuff,
			      &inChannel->outBuffNum);

    if(ret != GENIF_RC_OK)
    {
      switch(inChannel->outBuffMsg->type)
      {
        case GENIF_MESSAGE_TYPE_REQUEST:
        {
          inChannel->counter->outRequestError++;

          GENIF_channel_notify(inChannel, GENIF_NOTIFY_REQUEST_ERROR,
                               inChannel->outBuffMsg);

          break;
        }

        case GENIF_MESSAGE_TYPE_RESPONSE:
        {
          inChannel->counter->outResponseError++;

          GENIF_channel_notify(inChannel, GENIF_NOTIFY_RESPONSE_ERROR,
                               inChannel->outBuffMsg);

          GENIF_message_delete(inChannel->outBuffMsg);

          break;
        }

        case GENIF_MESSAGE_TYPE_MESSAGE:
        {
          inChannel->counter->outMessageError++;

          GENIF_channel_notify(inChannel, GENIF_NOTIFY_MESSAGE_ERROR,
                               inChannel->outBuffMsg);

          break;
        }

        default:
        {
          GENIF_FATAL0("ASSERTION!!");

          break;
        }
      }

      inChannel->outBuffMsg = NULL;

      ret = GENIF_RC_OK;
    }

    else // if(ret == GENIF_RC_OK)
    {
      inChannel->outBuffReq->data = inChannel;

      ret = uv_write(inChannel->outBuffReq,
	             (uv_stream_t*)(inChannel->channel),
		     inChannel->outBuff,
		     inChannel->outBuffNum,
		     GENIF_channel_write_cb);

      if(ret < 0) GENIF_channel_error_cb(inChannel, ret);
    }
  }

//----------------

  TRAZA0("Returning from GENIF_channel_write()");
}

/*----------------------------------------------------------------------------*/

static void
GENIF_channel_write_cb(uv_write_t* inUvReq, int inStatus)
{
  GENIF_channel_t*		inChannel = inUvReq->data;
  
  GENIF_message_t*		ptrMessage = NULL;

  TRAZA2("Entering in GENIF_channel_write_cb(%p, %d)", inChannel, inStatus);

//----------------

  ptrMessage = inChannel->outBuffMsg; inChannel->outBuffMsg = NULL;

  if(inStatus == 0)
  {
    GENIF_channel_process_out_msg_cb(inChannel, ptrMessage);
  }

  else // if(inStatus != 0)
  {
    GENIF_channel_error_cb(inChannel, inStatus);
  }

//----------------

  GENIF_channel_mask(inChannel);

  if(inChannel->writeMask != GENIF_FLAG_ON)
  {
    if(inChannel->extQueueFlag == GENIF_FLAG_ON)
    {
      if(inChannel->extQueueFunc != NULL)
      {
	ptrMessage = inChannel->extQueueFunc(inChannel);
      }

      if(ptrMessage == NULL)
      {
        inChannel->extQueueFlag = GENIF_FLAG_OFF;

        ptrMessage = RLST_extractHead(inChannel->outQueue);
      }

      else { ptrMessage->channel = inChannel; }
    }

    else { ptrMessage = RLST_extractHead(inChannel->outQueue); }

    if(ptrMessage != NULL)
    {
      GENIF_channel_write(inChannel, ptrMessage);
    }
  }

//----------------

  TRAZA0("Returning from GENIF_channel_write_cb()");
}

/*----------------------------------------------------------------------------*/

static void
GENIF_channel_process_out_msg_cb
(
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMessage
)
{
  long				T = uv_now(inChannel->loop);

  int				rc;

  TRAZA2("Entering in GENIF_channel_process_out_msg_cb(%p, %p)",
          inChannel,
	  inMessage);

//----------------

  switch(inMessage->type)
  {
    case GENIF_MESSAGE_TYPE_REQUEST:
    {
      inChannel->outFlowCount++;

      inChannel->counter->winRequest++;

      inMessage->T = T;

      rc = RFTR_insert(inChannel->outRef, inMessage);
      
      if(rc != RLST_RC_OK)
      {
	GENIF_FATAL("ERROR: RFTR_insert()");
      }

      rc = RLST_insertTail(inChannel->outRefTout, inMessage);

      if(rc != RLST_RC_OK)
      {
	GENIF_FATAL("ERROR: RLST_insertTail()");
      }

      GENIF_channel_notify(inChannel, GENIF_NOTIFY_REQUEST_SENT, inMessage);

      break;
    }

//----------------

    case GENIF_MESSAGE_TYPE_RESPONSE:
    {
      GENIF_channel_notify(inChannel, GENIF_NOTIFY_RESPONSE_SENT, inMessage);

      GENIF_message_delete(inMessage);

      break;
    }

//----------------

    case GENIF_MESSAGE_TYPE_MESSAGE:
    {
      inChannel->outFlowCount++;

      inMessage->T = T;

      GENIF_channel_notify(inChannel, GENIF_NOTIFY_MESSAGE_SENT, inMessage);

      break;
    }

//----------------

    default:
    {
      SUCESO1("ERROR: SENDING UNKNOWN MESSAGE TYPE (%d)", inMessage->type);

      inChannel->counter->outUnknownMsg++;
      
      break;
    }
  }

//----------------

  TRAZA0("Returning from GENIF_channel_process_out_msg_cb()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
GENIF_channel_external_queue(GENIF_channel_t* inChannel, int inFlag)
{
  int				ret = GENIF_RC_OK;

  TRAZA2("Entering in GENIF_channel_external_queue(%p, %d)", inChannel, inFlag);

//----------------

  if(inChannel->extQueueFlag != inFlag)
  {
    inChannel->extQueueFlag = inFlag; // GENIF_channel_mask(inChannel);
  }

//----------------

  TRAZA1("Returning from GENIF_channel_external_queue() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
GENIF_channel_send
(
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMessage
)
{
  int				ret = GENIF_RC_OK;

  TRAZA2("Entering in GENIF_channel_send(%p, %p)", inChannel, inMessage);

//----------------
  inChannel->counter->outMessage++;
//----------------

  if(inChannel->config->outQueueMax > 0)
  {
    if(RLST_getNumElem(inChannel->outQueue) >= inChannel->config->outQueueMax)
    {
      SUCESO0("ERROR: CHANNEL OUTPUT QUEUE FULL");
    
      inChannel->counter->outMessageError++;
      
      ret = GENIF_RC_QUEUE_FULL;
    }
  }

//----------------

  if(ret == GENIF_RC_OK)
  {
    inMessage->type    = GENIF_MESSAGE_TYPE_MESSAGE;
    inMessage->channel = inChannel;
    inMessage->T       = uv_now(inChannel->loop);

    GENIF_channel_write(inChannel, inMessage);
  }

//----------------

  TRAZA1("Returning from GENIF_channel_send() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
GENIF_channel_request
(
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMessage
)
{
  int				ret = GENIF_RC_OK;

  TRAZA2("Entering in GENIF_channel_request(%p, %p)", inChannel, inMessage);

//----------------
  inChannel->counter->outRequest++;
//----------------

  if(inChannel->config->outQueueMax > 0)
  {
    if(RLST_getNumElem(inChannel->outQueue) >= inChannel->config->outQueueMax)
    {
      SUCESO0("ERROR: CHANNEL OUTPUT QUEUE FULL");
    
      inChannel->counter->outRequestError++;
      
      ret = GENIF_RC_QUEUE_FULL;
    }
  }

//----------------

  if(ret == GENIF_RC_OK)
  {
    inMessage->type    = GENIF_MESSAGE_TYPE_REQUEST;
    inMessage->channel = inChannel;
    inMessage->T       = uv_now(inChannel->loop);

    GENIF_channel_write(inChannel, inMessage);
  }

//----------------

  TRAZA1("Returning from GENIF_channel_request() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
GENIF_channel_reply
(
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMessage
)
{
  GENIF_message_t*		ptrMsg;
  
  int				ret = GENIF_RC_OK;

  TRAZA2("Entering in GENIF_channel_reply(%p, %p)", inChannel, inMessage);

//----------------
  inChannel->counter->outResponse++;
//----------------

  ptrMsg = RLST_extract(inChannel->inRef, inMessage);

  if(ptrMsg == NULL)
  {
    ret = GENIF_RC_NOT_FOUND;
  }

  else
  {
    inMessage->type    = GENIF_MESSAGE_TYPE_RESPONSE;
    inMessage->channel = inChannel;
    inMessage->T       = uv_now(inChannel->loop);
	
    GENIF_channel_write(inChannel, inMessage);
  }

//----------------
  GENIF_channel_mask(inChannel);
//----------------

  TRAZA1("Returning from GENIF_channel_reply() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

GENIF_message_t*
GENIF_channel_get_any_msg
(
  GENIF_channel_t*		inChannel,
  int*				outStore
)
{
  GENIF_message_t*		ptrMsg = NULL;

  TRAZA1("Entering in GENIF_channel_get_any_msg(%p, %d)", inChannel);

//----------------

  if((ptrMsg = RLST_getHead(inChannel->outQueue)) != NULL)
  {
    *outStore = GENIF_O_STORE;
  }
  
  else if((ptrMsg = inChannel->outBuffMsg) != NULL)
  {
    *outStore = GENIF_O_STORE;
  }

  else if((ptrMsg = RLST_getHead(inChannel->outRefTout)) != NULL)
  {
    *outStore = GENIF_W_STORE;
  }

  else if((ptrMsg = RLST_getHead(inChannel->inRef)) != NULL)
  {
    *outStore = GENIF_I_STORE;
  }
  
  else { ptrMsg = NULL; }

//----------------

  TRAZA1("Returning from GENIF_channel_get_any_msg() = %p", ptrMsg);

  return ptrMsg;
}

/*----------------------------------------------------------------------------*/

int
GENIF_channel_unref_msg
(
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMessage
)
{
  int				ret = GENIF_RC_OK;

  TRAZA2("Entering in GENIF_channel_unref_msg(%p, %p)", inChannel, inMessage);

//----------------

  if(RLST_extract(inChannel->inRef, inMessage) == NULL)
  {
    ret = GENIF_RC_NOT_FOUND;
  }

  GENIF_channel_mask(inChannel);

//----------------

  TRAZA1("Returning from GENIF_channel_unref_msg() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
GENIF_channel_cancel_msg
(
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMessage,
  int*				outStore
)
{
  GENIF_message_t*		ptrM1 = NULL;
  GENIF_message_t*		ptrM2 = NULL;
  GENIF_message_t*		ptrM3 = NULL;
  GENIF_message_t*		ptrM4 = NULL;
  GENIF_message_t*		ptrM5 = NULL;

  int				ret = GENIF_RC_OK;

  TRAZA2("Entering in GENIF_channel_cancel_msg(%p, %p)", inChannel, inMessage);

//----------------

  if(inMessage == inChannel->outBuffMsg)
  {
    ptrM1 = inChannel->outBuffMsg; inChannel->outBuffMsg = NULL;
  }
  
  ptrM2 = RLST_extract(inChannel->outQueue,   inMessage);
  ptrM3 = RFTR_extract(inChannel->outRef,     inMessage);
  ptrM4 = RLST_extract(inChannel->outRefTout, inMessage);
  ptrM5 = RLST_extract(inChannel->inRef,      inMessage);

//----------------

  if(ptrM1 != NULL)
  {
    *outStore = GENIF_O_STORE;
    
    ret = GENIF_channel_message_end(inChannel,
	                            inMessage,
                                    GENIF_END_USER, *outStore);
  }

  else if(ptrM2 != NULL)
  {
    *outStore = GENIF_O_STORE;
    
    ret = GENIF_channel_message_end(inChannel,
	                            inMessage,
                                    GENIF_END_USER, *outStore);
  }

  else if((ptrM3 != NULL) || (ptrM4 != NULL))
  {
    *outStore = GENIF_W_STORE;
    
    ret = GENIF_channel_message_end(inChannel,
	                            inMessage,
                                    GENIF_END_USER, *outStore);
  }

  else if(ptrM5 != NULL)
  {
    *outStore = GENIF_I_STORE;
    
    ret = GENIF_channel_message_end(inChannel,
	                            inMessage,
                                    GENIF_END_USER, *outStore);
  }

  else { ret = GENIF_RC_NOT_FOUND; }

//----------------
  GENIF_channel_mask(inChannel);
//----------------

  TRAZA1("Returning from GENIF_channel_cancel_msg() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

long
GENIF_channel_message_num
(
  GENIF_channel_t*		inChannel,
  long*				outOstore,
  long*				outWstore,
  long*				outIstore
)
{
  long				num;
  long				ret = 0;

  TRAZA1("Entering in GENIF_channel_message_num(%p, %d)", inChannel);

//----------------

  if(inChannel->outQueue != NULL)
  {
    num = RLST_getNumElem(inChannel->outQueue);

    if(outOstore != NULL)
    {
      *outOstore = num;
    }

    ret += num;
  }

  else
  {
    if(outOstore != NULL)
    {
      *outOstore = 0;
    }
  }

  if(inChannel->outBuffMsg != NULL)
  {
    if(outOstore != NULL)
    {
      (*outOstore)++;
    }

    ret += 1;
  }

//----------------

  if(inChannel->outRef != NULL)
  {
    num = RLST_getNumElem(inChannel->outRefTout);

    if(outWstore != NULL)
    {
      *outWstore = num;
    }

    ret += num;
  }

  else
  {
    if(outWstore != NULL)
    {
      *outWstore = 0;
    }
  }

//----------------

  if(inChannel->inRef != NULL)
  {
    num = RLST_getNumElem(inChannel->inRef);

    if(outIstore != NULL)
    {
      *outIstore = num;
    }

    ret += num;
  }

  else
  {
    if(outIstore != NULL)
    {
      *outIstore = 0;
    }
  }

//----------------

  TRAZA1("Returning from GENIF_channel_message_num() = %ld", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static void 
GENIF_channel_notify
(
  GENIF_channel_t*		inChannel,
  int				inNotify,
  GENIF_message_t*		inMessage
)
{
  inChannel->notDelete = GENIF_TRUE;

  inChannel->cbNotify(inChannel, inNotify, inMessage);

  inChannel->notDelete = GENIF_FALSE;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

