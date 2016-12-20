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
  GENIF_message_t*		inMsg,
  int				inEndType,
  int				inStoreType
);

static int GENIF_channel_message_timeout(GENIF_channel_t* inChannel);

static void GENIF_channel_mask(GENIF_channel_t* inChannel);

static int GENIF_channel_read_mask(GENIF_channel_t* inChannel);
static int GENIF_channel_write_mask(GENIF_channel_t* inChannel);

static void GENIF_channel_close_cb(GENIF_channel_t* inChannel);
static void GENIF_channel_error_cb(GENIF_channel_t* inChannel, int inError);

static void GENIF_channel_read_cb(int inFd, void* inChannel);
static void GENIF_channel_write_cb(int inFd, void* inChannel);

static void GENIF_channel_process_in_msg_cb
(
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMsg
);

static void GENIF_channel_process_out_msg_cb
(
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMsg
);

static void GENIF_channel_notify
(
  GENIF_channel_t*		inChannel,
  int				inNotify,
  GENIF_message_t*		inMsg
);

//----------------

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int 
GENIF_channel_pre_initialize
(
  GENIF_channel_t* 		inChannel,
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
  long				T = time(NULL);

  GENIF_message_t		msg;

  int				rc;

  int				ret = GENIF_RC_OK;

  TRAZA1("Entering in GENIF_channel_pre_initialize(%p)", inChannel);

//----------------

  if(ret == GENIF_RC_OK)
  {
	inChannel->loop     = uv_default_loop();

    inChannel->fd       = -1;
    inChannel->type     = inChannelType;
    inChannel->parent   = inParent;
    inChannel->cbNotify = inCbNotify;

    inChannel->modifier = inModifier;

    inChannel->extRef = NULL;
    inChannel->state  = 0;

    inChannel->locHost[0] =  0;
    inChannel->locPort    = -1;

    inChannel->remHost[0] =  0;
    inChannel->remPort    = -1;

    inChannel->T = T;
    
    inChannel->readMask   = GENIF_FLAG_OFF;
    inChannel->writeMask  = GENIF_FLAG_OFF;

    inChannel->disabledRead  = GENIF_FLAG_OFF;
    inChannel->disabledWrite = GENIF_FLAG_OFF;

    inChannel->heartbeatT   = T;
    inChannel->inactivityT  = T;
    inChannel->malfunctionT = T;

    inChannel->outToutsCount = 0;
    inChannel->outFlowCount  = 0;
    inChannel->inFlowCount   = 0;

//  inChannel->outBuff;
    inChannel->outBuffIdx = 0;
    inChannel->outBuffLen = 0;
    inChannel->outBuffMsg = NULL;
    
//  inChannel->inBuff;
    inChannel->inBuffIdx = 0;
    inChannel->inBuffLen = 0;
    inChannel->inBuffMsg = NULL;

    inChannel->extQueueFunc = inExtQueueFunc;
    inChannel->extQueueFlag = GENIF_FLAG_OFF;

//  inChannel->outQueue;
//  inChannel->outRefTout;
//  inChannel->outRef;
//  inChannel->inRef;

    inChannel->config  = inConfig;
    inChannel->counter = inCounter;

    inChannel->notDelete = GENIF_FALSE;
  }

//----------------

  if(ret == GENIF_RC_OK)
  {
    rc = RLST_initializeRefList(inChannel->outQueue, &msg, &msg.list);

    if(rc < 0)
    {
      GENIF_FATAL("ERROR: RLST_initializeRefList()");
    }
  }
  
//----------------

  if(ret == GENIF_RC_OK)
  {
    rc = RFTR_initializeRefTree(inChannel->outRef, &msg, &msg.tree,
                                GENIF_message_cmp);

    if(rc < 0)
    {
      GENIF_FATAL("ERROR: RFTR_initializeRefTree()");
    }
  }
  
//----------------

  if(ret == GENIF_RC_OK)
  {
    rc = RLST_initializeRefList(inChannel->outRefTout, &msg, &msg.list);

    if(rc < 0)
    {
      GENIF_FATAL("ERROR: RLST_initializeRefList()");
    }
  }
  
//----------------

  if(ret == GENIF_RC_OK)
  {
    rc = RLST_initializeRefList(inChannel->inRef, &msg, &msg.list);

    if(rc < 0)
    {
      GENIF_FATAL("ERROR: RLST_initializeRefList()");
    }
  }
  
//----------------

  TRAZA1("Returning from GENIF_channel_pre_initialize() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int 
GENIF_channel_fd_initialize
(
  GENIF_channel_t* 		inChannel,
  int				inFd
)
{
  int				ret = GENIF_RC_OK;

  TRAZA2("Entering in GENIF_channel_fd_initialize(%p, %d)", inChannel, inFd);

//----------------

  inChannel->fd = inFd;
    
  setChannelUserData(inChannel->fd, inChannel);

  setChannelReadF(inChannel->fd, GENIF_channel_read_cb);
  setChannelWriteF(inChannel->fd, GENIF_channel_write_cb);

  GENIF_channel_mask(inChannel);

//----------------

  TRAZA1("Returning from GENIF_channel_fd_initialize() = %d", ret);

  return ret;
}

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
  long				T = time(NULL);

  GENIF_message_t		msg;

  int				rc;

  int				ret = GENIF_RC_OK;

  TRAZA2("Entering in GENIF_channel_initialize(%p, %d)", inChannel, inFd);

//----------------

  if(ret == GENIF_RC_OK)
  {
	inChannel->loop     = uv_default_loop();

    inChannel->fd       = inFd;
    inChannel->type     = inChannelType;
    inChannel->parent   = inParent;
    inChannel->cbNotify = inCbNotify;

    inChannel->modifier = inModifier;

    inChannel->extRef = NULL;
    inChannel->state  = 0;

    inChannel->locHost[0] =  0;
    inChannel->locPort    = -1;

    inChannel->remHost[0] =  0;
    inChannel->remPort    = -1;

    inChannel->T = T;
    
    inChannel->readMask   = GENIF_FLAG_OFF;
    inChannel->writeMask  = GENIF_FLAG_OFF;

    inChannel->disabledRead  = GENIF_FLAG_OFF;
    inChannel->disabledWrite = GENIF_FLAG_OFF;

    inChannel->heartbeatT   = T;
    inChannel->inactivityT  = T;
    inChannel->malfunctionT = T;

    inChannel->outToutsCount = 0;
    inChannel->outFlowCount  = 0;
    inChannel->inFlowCount   = 0;

//  inChannel->outBuff;
    inChannel->outBuffIdx = 0;
    inChannel->outBuffLen = 0;
    inChannel->outBuffMsg = NULL;
    
//  inChannel->inBuff;
    inChannel->inBuffIdx = 0;
    inChannel->inBuffLen = 0;
    inChannel->inBuffMsg = NULL;

    inChannel->extQueueFunc = inExtQueueFunc;
    inChannel->extQueueFlag = GENIF_FLAG_OFF;

//  inChannel->outQueue;
//  inChannel->outRefTout;
//  inChannel->outRef;
//  inChannel->inRef;

    inChannel->config  = inConfig;
    inChannel->counter = inCounter;

    inChannel->notDelete = GENIF_FALSE;
  }

//----------------

  if(ret == GENIF_RC_OK)
  {
    rc = RLST_initializeRefList(inChannel->outQueue, &msg, &msg.list);

    if(rc < 0)
    {
      GENIF_FATAL("ERROR: RLST_initializeRefList()");
    }
  }
  
//----------------

  if(ret == GENIF_RC_OK)
  {
    rc = RFTR_initializeRefTree(inChannel->outRef, &msg, &msg.tree,
                                GENIF_message_cmp);

    if(rc < 0)
    {
      GENIF_FATAL("ERROR: RFTR_initializeRefTree()");
    }
  }
  
//----------------

  if(ret == GENIF_RC_OK)
  {
    rc = RLST_initializeRefList(inChannel->outRefTout, &msg, &msg.list);

    if(rc < 0)
    {
      GENIF_FATAL("ERROR: RLST_initializeRefList()");
    }
  }
  
//----------------

  if(ret == GENIF_RC_OK)
  {
    rc = RLST_initializeRefList(inChannel->inRef, &msg, &msg.list);

    if(rc < 0)
    {
      GENIF_FATAL("ERROR: RLST_initializeRefList()");
    }
  }
  
//----------------

  if(ret == GENIF_RC_OK)
  {
    setChannelUserData(inChannel->fd, inChannel);

    setChannelReadF(inChannel->fd, GENIF_channel_read_cb);
    setChannelWriteF(inChannel->fd, GENIF_channel_write_cb);

    GENIF_channel_mask(inChannel);
  }

//----------------

  TRAZA1("Returning from GENIF_channel_initialize() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int 
GENIF_channel_fd_finalize
(
  GENIF_channel_t* 		inChannel
)
{
  int				ret = GENIF_RC_OK;

//----------------

  TRAZA2("Entering in GENIF_channel_fd_finalize(%p, %d)", 
          inChannel,
	  inChannel->fd);

//----------------

  if(inChannel->fd != -1)
  {
    forceEndChannel(inChannel->fd); close(inChannel->fd); inChannel->fd = -1;
  }

//----------------

  GENIF_channel_empty(inChannel, GENIF_END_USER);

//----------------

  TRAZA1("Returning from GENIF_channel_fd_initialize() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int 
GENIF_channel_finalize(GENIF_channel_t* inChannel)
{
  int				ret = GENIF_RC_OK;

//----------------

  TRAZA2("Entering in GENIF_channel_finalize(%p, %d)", 
          inChannel,
	  inChannel->fd);

//----------------

  if(inChannel->fd != -1)
  {
    forceEndChannel(inChannel->fd); close(inChannel->fd); inChannel->fd = -1;
  }

//----------------

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
  struct sockaddr		saddr;
  struct sockaddr_in		saddrin;

  static socklen_t		len = sizeof(saddrin);

  socklen_t			len1 = len;
  socklen_t			len2 = len;

  int				rc;
  
  int				ret = GENIF_RC_OK;

//----------------

  TRAZA2("Entering in GENIF_channel_address(%p, %d)", 
          inChannel,
	  inChannel->fd);

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

//----------------

  TRAZA2("Entering in GENIF_channel_empty(%p, %d)", 
          inChannel,
	  inChannel->fd);

//----------------

  if(inChannel->outBuffMsg != NULL)
  {
    ptrMsg = inChannel->outBuffMsg;

    inChannel->outBuffMsg = NULL; inChannel->outBuffLen = 0;
    
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
  GENIF_message_t*		inMsg,
  int				inEndType,
  int				inStoreType
)
{
  char				strMsg[GENIF_MAXLEN_STRING + 1];
  char				strErr[GENIF_MAXLEN_STRING + 1];

  int				notify = GENIF_NOTIFY_NONE;

  int				delete = GENIF_FALSE;

  int				ret = GENIF_RC_OK;

//----------------

  TRAZA3("Entering in GENIF_channel_message_end(%p, %d, %p)", 
          inChannel,
	  inChannel->fd,
	  inMsg);


  switch(inMsg->type)
  {

//----------------

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
      GENIF_message_dump(inMsg, GENIF_MAXLEN_STRING + 1, strMsg);

      SUCESO2("%s\n%s", strErr, strMsg);
    }
  }

  else
  {
    GENIF_channel_notify(inChannel, notify, inMsg);
  }

  if(delete == GENIF_TRUE)
  {
    GENIF_message_delete(inMsg);
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

//----------------

  TRAZA3("Entering in GENIF_channel_timer_cb(%p, %d, %ld)", 
          inChannel,
	  inChannel->fd,
	  inT);

//----------------

  if(inChannel->fd == -1)
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

//----------------

  GENIF_channel_mask(inChannel);

//----------------

  TRAZA0("Returning from GENIF_channel_timer_cb()");
}

/*----------------------------------------------------------------------------*/

static int
GENIF_channel_message_timeout(GENIF_channel_t* inChannel)
{
  long				T = time(NULL);

  long				Tout = 0;

  GENIF_message_t*		ptrMsg = NULL;

  int				endTout = 0;

  int				ret = GENIF_RC_OK;

//----------------

  TRAZA2("Entering in GENIF_channel_message_timeout(%p, %d)", 
          inChannel,
	  inChannel->fd);

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
  TRAZA3("Entering in GENIF_channel_read_disable(%p, %d, %d)", 
          inChannel,
	  inChannel->fd, inFlag);

  if(inChannel->disabledRead != inFlag)
  {
    inChannel->disabledRead = inFlag;

    GENIF_channel_mask(inChannel);
  }

  TRAZA0("Returning from GENIF_channel_read_disable()");
}

/*----------------------------------------------------------------------------*/

void 
GENIF_channel_write_disable(GENIF_channel_t* inChannel, int inFlag)
{
  TRAZA3("Entering in GENIF_channel_write_disable(%p, %d, %d)", 
          inChannel,
	  inChannel->fd, inFlag);

  if(inChannel->disabledWrite != inFlag)
  {
    inChannel->disabledWrite = inFlag;

    GENIF_channel_mask(inChannel);
  }

  TRAZA0("Returning from GENIF_channel_write_disable()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static void 
GENIF_channel_mask(GENIF_channel_t* inChannel)
{
  TRAZA2("Entering in GENIF_channel_mask(%p, %d)", 
          inChannel,
	  inChannel->fd);

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

  TRAZA2("Entering in GENIF_channel_read_mask(%p, %d)", 
          inChannel,
	  inChannel->fd);

//----------------

  if(inChannel->fd != -1)
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

//----------------

    if(inChannel->readMask != readMask)
    {
      inChannel->readMask = readMask;

      if(inChannel->readMask)
      {
        setReadMask(inChannel->fd);
      }

      else 
      {
        unsetReadMask(inChannel->fd);
      }
    }
  }

//----------------

  else
  {
    inChannel->readMask = GENIF_FLAG_OFF;
  }

//----------------

  TRAZA1("Returning from GENIF_channel_read_mask() = %d", readMask);

  return readMask;
}

/*----------------------------------------------------------------------------*/

static int
GENIF_channel_write_mask(GENIF_channel_t* inChannel)
{
  int			writeMask  = 0;

  TRAZA2("Entering in GENIF_channel_write_mask(%p, %d)", 
          inChannel,
	  inChannel->fd);

//----------------

  if(inChannel->fd != -1)
  {
    if(inChannel->outBuffLen > 0)
    {
      writeMask = 1;
    }

    if(RLST_getNumElem(inChannel->outQueue) > 0)
    {
      writeMask = 1;
    }

    if(inChannel->extQueueFlag == GENIF_FLAG_ON)
    {
      writeMask = 1;
    }

    if(inChannel->disabledWrite == GENIF_FLAG_ON)
    {
      writeMask = 0;
    }

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

//----------------

    if(inChannel->writeMask != writeMask)
    {
      inChannel->writeMask = writeMask;

      if(inChannel->writeMask)
      {
        setWriteMask(inChannel->fd);
      }

      else 
      {
        unsetWriteMask(inChannel->fd);
      }
    }
  }

//----------------

  else
  {
    inChannel->writeMask = GENIF_FLAG_OFF;
  }

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
  TRAZA2("Entering in GENIF_channel_close_cb(%p, %d)", 
          inChannel,
	  inChannel->fd);

//----------------

  DEPURA1("WARNING: CHANNEL (%d) CLOSED BY PEER", inChannel->fd);

  if(inChannel->fd != -1)
  {
    forceEndChannel(inChannel->fd); close(inChannel->fd); inChannel->fd = -1;
  }

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
  TRAZA3("Entering in GENIF_channel_error_cb(%p, %d, %d)",
          inChannel,
	  inChannel->fd,
          inError);

//----------------

  SUCESO2("ERROR: CHANNEL (%d) ERROR (%d)", inChannel->fd, inError);

  if(inChannel->fd != -1)
  {
    forceEndChannel(inChannel->fd); close(inChannel->fd); inChannel->fd = -1;
  }

  GENIF_channel_empty(inChannel, GENIF_END_CLOSE);

  GENIF_channel_notify(inChannel, GENIF_NOTIFY_CHANNEL_ERROR, NULL);

//----------------

  TRAZA0("Returning from GENIF_channel_error_cb()");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static void
GENIF_channel_read_cb(int inFd, void* inVoidChn)
{
  GENIF_channel_t*		inChannel = inVoidChn;
  
  GENIF_message_t*		ptrMsg = NULL;

  long				readLen;

  long				decodeLen;
  int				decodeEnd = GENIF_FALSE;

  int				rc;

  TRAZA2("Entering in GENIF_channel_read_cb(%p, %d)", inChannel, inFd);

//----------------

  readLen = GENIF_MAXLEN_READ_BUFFER - inChannel->inBuffLen;

  readLen = read(inFd, inChannel->inBuff + inChannel->inBuffLen, readLen);

  if(readLen < 0)
  {
    if(errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
    {
      SUCESO3("ERROR: read(%d) = %d (%s)", inFd, errno, strerror(errno));

      GENIF_channel_error_cb(inChannel, errno);
    }
  }

  else if(readLen == 0)
  {
    GENIF_channel_close_cb(inChannel);
  }

  else // if(readLen > 0)
  {
    inChannel->inBuffLen += readLen;
  }

//----------------

  while(inChannel->inBuffLen > 0 && decodeEnd == GENIF_FALSE)
  {
    if(inChannel->inBuffMsg == NULL)
    {
      inChannel->inBuffMsg = GENIF_message_new(inChannel->modifier);

      inChannel->inBuffMsg->channel = inChannel;
    }

    rc = GENIF_message_decode(inChannel->inBuffMsg,
			      inChannel->inBuff +
			      inChannel->inBuffIdx,
			      inChannel->inBuffLen, &decodeLen);

    if(decodeLen != 0)
    {
      inChannel->inBuffIdx += decodeLen;
      inChannel->inBuffLen -= decodeLen;
    }

    if(rc == GENIF_RC_OK)
    {
      ptrMsg = inChannel->inBuffMsg; inChannel->inBuffMsg = NULL;

      GENIF_channel_process_in_msg_cb(inChannel, ptrMsg);
    }
  
    else if(rc == GENIF_RC_INCOMPLETE)
    {
      decodeEnd = GENIF_TRUE;
    }

    else // if(rc == GENIF_RC_ERROR)
    {
      inChannel->counter->inUnknownMsg++;

      GENIF_message_delete(inChannel->inBuffMsg);

      inChannel->inBuffMsg = NULL;
    }

    GENIF_channel_mask(inChannel);

    if(inChannel->readMask == GENIF_FLAG_OFF)
    {
      decodeEnd = GENIF_TRUE;
    }
  }

//----------------

  if(inChannel->inBuffLen >= GENIF_MAXLEN_READ_BUFFER)
  {
    SUCESO2("ERROR: Read buffer is full (%d:%ld)", inFd, inChannel->inBuffLen);

    GENIF_channel_error_cb(inChannel, -1);
  }

  else if(inChannel->inBuffIdx > 0)
  {
    if(inChannel->inBuffLen > 0)
    {
      memmove(inChannel->inBuff,
	      inChannel->inBuff +
	      inChannel->inBuffIdx,
	      inChannel->inBuffLen);
    }
    
    inChannel->inBuffIdx = 0;
  }

//----------------

  TRAZA0("Returning from GENIF_channel_read_cb()");
}

/*----------------------------------------------------------------------------*/

static void
GENIF_channel_process_in_msg_cb
(
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMsg
)
{
  GENIF_message_t*		ptrMsg = NULL;

  char				strMsg[GENIF_MAXLEN_STRING + 1];

  int				rc;

//----------------

  TRAZA2("Entering in GENIF_channel_process_in_msg_cb(%p, %p)",
          inChannel,
	  inMsg);

  switch(inMsg->type)
  {

//----------------

    case GENIF_MESSAGE_TYPE_REQUEST:
    {
      inChannel->inFlowCount++;

      inChannel->counter->inRequest++;

      inMsg->T = time(NULL);

      rc = RLST_insertTail(inChannel->inRef, inMsg);

      if(rc != RLST_RC_OK)
      {
	GENIF_FATAL("ERROR: RLST_insertTail()");
      }

      GENIF_channel_notify(inChannel, GENIF_NOTIFY_REQUEST_RECEIVED, inMsg);

      break;
    }

//----------------

    case GENIF_MESSAGE_TYPE_RESPONSE:
    {
      inChannel->counter->inResponse++;

      ptrMsg = RFTR_find(inChannel->outRef, inMsg, NULL, NULL);

      if(ptrMsg != NULL)
      {
	RFTR_extract(inChannel->outRef,     ptrMsg);
	RLST_extract(inChannel->outRefTout, ptrMsg);

	GENIF_message_resp_copy(ptrMsg, inMsg);

	GENIF_channel_notify(inChannel, GENIF_NOTIFY_RESPONSE_RECEIVED, ptrMsg);
      }

      else
      {
        GENIF_message_dump(inMsg, GENIF_MAXLEN_STRING + 1, strMsg);
	
	SUCESO1("ERROR: REQUEST NOT FOUND FOR RESPONSE:\n%s", strMsg);

	inChannel->counter->inResponseError++;
      }

      GENIF_message_delete(inMsg);

      break;
    }

//----------------

    case GENIF_MESSAGE_TYPE_MESSAGE:
    {
      inChannel->inFlowCount++;

      inChannel->counter->inMessage++;

      inMsg->T = time(NULL);

      rc = RLST_insertTail(inChannel->inRef, inMsg);

      if(rc != RLST_RC_OK)
      {
	GENIF_FATAL("ERROR: RLST_insertTail()");
      }

      GENIF_channel_notify(inChannel, GENIF_NOTIFY_MESSAGE_RECEIVED, inMsg);

      break;
    }

//----------------

    case GENIF_MESSAGE_TYPE_UNKNOWN:
    {
      inChannel->inFlowCount++;

      inChannel->counter->inUnknownMsg++;

      inMsg->T = time(NULL);

      rc = RLST_insertTail(inChannel->inRef, inMsg);

      if(rc != RLST_RC_OK)
      {
	GENIF_FATAL("ERROR: RLST_insertTail()");
      }

      GENIF_channel_notify(inChannel, GENIF_NOTIFY_UNKNOWN_RECEIVED, inMsg);

      break;
    }

//----------------

    default:
    {
      SUCESO1("ERROR: RECEIVED UNKNOWN MESSAGE TYPE (%d)", inMsg->type);

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
GENIF_channel_write_cb(int inFd, void* inVoidChn)
{
  GENIF_channel_t*		inChannel = inVoidChn;
  
  GENIF_message_t*		ptrMsg = NULL;

  long				writeLen;
  long				writeMax = 0;
  int				writeEnd = GENIF_FALSE;

  int				rc;

  TRAZA2("Entering in GENIF_channel_write_cb(%p, %d)", inChannel, inFd);

//----------------

  while(writeEnd == GENIF_FALSE)
  {
    if((inChannel->outBuffLen == 0) && (inChannel->outBuffMsg == NULL))
    {
      if(inChannel->extQueueFlag == GENIF_FLAG_ON)
      {
	if(inChannel->extQueueFunc != NULL)
	{
	  inChannel->outBuffMsg = inChannel->extQueueFunc(inChannel);
	}

	if(inChannel->outBuffMsg == NULL)
	{
	  inChannel->extQueueFlag = GENIF_FLAG_OFF;

	  inChannel->outBuffMsg = RLST_extractHead(inChannel->outQueue);
	} 

	else { inChannel->outBuffMsg->channel = inChannel; }
      }

      else { inChannel->outBuffMsg = RLST_extractHead(inChannel->outQueue); }
    }

//----------------

    if((inChannel->outBuffLen == 0) && (inChannel->outBuffMsg != NULL))
    {
      inChannel->outBuffIdx = 0;
      
      rc = GENIF_message_encode(inChannel->outBuffMsg, GENIF_MAXLEN_WRITE_BUFFER,
				inChannel->outBuff,
			       &inChannel->outBuffLen);

      if(rc != GENIF_RC_OK)
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
	    GENIF_FATAL("ASSERTION!!");

	    break;
	  }
	}

	inChannel->outBuffMsg = NULL;
	inChannel->outBuffLen = 0;
      }
    }

//----------------

    if((inChannel->outBuffLen != 0) && (inChannel->outBuffMsg != NULL))
    {
      writeLen = write(inFd, 
                       inChannel->outBuff +
		       inChannel->outBuffIdx,
		       inChannel->outBuffLen);

      if(writeLen < 0) 
      {
	if(errno == EINTR)
	{
	  SUCESO3("WARNING: write(%d) = %d (%s)", 
	           inFd, 
		   errno, 
		   strerror(errno));
	}

	else if(errno == EAGAIN || errno == EWOULDBLOCK)
	{
	  DEPURA3("WARNING: write(%d) = %d (%s)", 
	           inFd, 
		   errno, 
		   strerror(errno));

	  writeEnd = GENIF_TRUE;
	}

	else
	{
	  SUCESO3("ERROR: write(%d) = %d (%s)", 
	           inFd, 
		   errno, 
		   strerror(errno));

	  GENIF_channel_error_cb(inChannel, errno);

	  writeEnd = GENIF_TRUE;
	}
      }

      else
      {
	inChannel->outBuffLen -= writeLen;
	inChannel->outBuffIdx += writeLen; writeMax += writeLen;

        if(writeMax >= GENIF_MAXLEN_READ_BUFFER)
	{
	  writeEnd = GENIF_TRUE;
	}
      }
    }

//----------------

    if((inChannel->outBuffLen == 0) && (inChannel->outBuffMsg != NULL))
    {
      ptrMsg = inChannel->outBuffMsg; inChannel->outBuffMsg = NULL;

      GENIF_channel_process_out_msg_cb(inChannel, ptrMsg);
    }

//----------------

    GENIF_channel_mask(inChannel);

    if(inChannel->writeMask == GENIF_FLAG_OFF)
    {
      writeEnd = GENIF_TRUE;
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
  GENIF_message_t*		inMsg
)
{
  long				T = time(NULL);

  int				rc;

//----------------

  TRAZA2("Entering in GENIF_channel_process_out_msg_cb(%p, %p)",
          inChannel,
	  inMsg);

  switch(inMsg->type)
  {

//----------------

    case GENIF_MESSAGE_TYPE_REQUEST:
    {
      inChannel->outFlowCount++;

      inChannel->counter->winRequest++;

      inMsg->T = T;

      rc = RFTR_insert(inChannel->outRef, inMsg);
      
      if(rc != RLST_RC_OK)
      {
	GENIF_FATAL("ERROR: RFTR_insert()");
      }

      rc = RLST_insertTail(inChannel->outRefTout, inMsg);

      if(rc != RLST_RC_OK)
      {
	GENIF_FATAL("ERROR: RLST_insertTail()");
      }

      GENIF_channel_notify(inChannel, GENIF_NOTIFY_REQUEST_SENT, inMsg);

      break;
    }

//----------------

    case GENIF_MESSAGE_TYPE_RESPONSE:
    {
      GENIF_channel_notify(inChannel, GENIF_NOTIFY_RESPONSE_SENT, inMsg);

      GENIF_message_delete(inMsg);

      break;
    }

//----------------

    case GENIF_MESSAGE_TYPE_MESSAGE:
    {
      inChannel->outFlowCount++;

      inMsg->T = T;

      GENIF_channel_notify(inChannel, GENIF_NOTIFY_MESSAGE_SENT, inMsg);

      break;
    }

//----------------

    default:
    {
      SUCESO1("ERROR: SENDING UNKNOWN MESSAGE TYPE (%d)", inMsg->type);

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

//----------------

  TRAZA3("Entering in GENIF_channel_external_queue(%p, %d, %d)", 
          inChannel,
	  inChannel->fd,
	  inFlag);

//----------------

  if(inChannel->extQueueFlag != inFlag)
  {
    inChannel->extQueueFlag = inFlag; GENIF_channel_mask(inChannel);
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
  GENIF_message_t*		inMsg
)
{
  long				T = time(NULL);

  int				rc;
  
  int				ret = GENIF_RC_OK;

//----------------

  TRAZA3("Entering in GENIF_channel_send(%p, %d, %p)", 
          inChannel,
	  inChannel->fd,
	  inMsg);

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
    inMsg->type    = GENIF_MESSAGE_TYPE_MESSAGE;
    inMsg->channel = inChannel;
    inMsg->T       = T;

    rc = RLST_insertTail(inChannel->outQueue, inMsg);

    if(rc != RLST_RC_OK)
    {
      GENIF_FATAL("ERROR: RLST_insertTail()");
    }
  }

//----------------

  GENIF_channel_mask(inChannel);

//----------------

  TRAZA1("Returning from GENIF_channel_send() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
GENIF_channel_request
(
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMsg
)
{
  long				T = time(NULL);

  int				rc;
  
  int				ret = GENIF_RC_OK;

//----------------

  TRAZA3("Entering in GENIF_channel_request(%p, %d, %p)", 
          inChannel,
	  inChannel->fd,
	  inMsg);

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
    inMsg->type    = GENIF_MESSAGE_TYPE_REQUEST;
    inMsg->channel = inChannel;
    inMsg->T       = T;

    rc = RLST_insertTail(inChannel->outQueue, inMsg);

    if(rc != RLST_RC_OK)
    {
      GENIF_FATAL("ERROR: RLST_insertTail()");
    }
  }

//----------------

  GENIF_channel_mask(inChannel);

//----------------

  TRAZA1("Returning from GENIF_channel_request() = %d", ret);

  return ret;
}

/*----------------------------------------------------------------------------*/

int
GENIF_channel_reply
(
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMsg
)
{
  long				T = time(NULL);

  GENIF_message_t*		ptrMsg;
  
  int				rc;
  
  int				ret = GENIF_RC_OK;

//----------------

  TRAZA3("Entering in GENIF_channel_reply(%p, %d, %p)", 
          inChannel,
	  inChannel->fd,
	  inMsg);

//----------------

  inChannel->counter->outResponse++;

//----------------

  ptrMsg = RLST_extract(inChannel->inRef, inMsg);

  if(ptrMsg == NULL)
  {
    ret = GENIF_RC_NOT_FOUND;
  }

  else
  {
    inMsg->type    = GENIF_MESSAGE_TYPE_RESPONSE;
    inMsg->channel = inChannel;
    inMsg->T       = T;
	
    rc = RLST_insertTail(inChannel->outQueue, inMsg);

    if(rc != RLST_RC_OK)
    {
      GENIF_FATAL("ERROR: RLST_insertTail()");
    }
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

//----------------

  TRAZA2("Entering in GENIF_channel_get_any_msg(%p, %d)", 
          inChannel,
	  inChannel->fd);

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
  
  else
  {
    ptrMsg = NULL;
  }

//----------------

  TRAZA1("Returning from GENIF_channel_get_any_msg() = %p", ptrMsg);

  return ptrMsg;
}

/*----------------------------------------------------------------------------*/

int
GENIF_channel_unref_msg
(
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMsg
)
{
  int				ret = GENIF_RC_OK;

//----------------

  TRAZA3("Entering in GENIF_channel_unref_msg(%p, %d, %p)",
          inChannel,
	  inChannel->fd,
	  inMsg);

//----------------

  if(RLST_extract(inChannel->inRef, inMsg) == NULL)
  {
    ret = GENIF_RC_NOT_FOUND;
  }

//----------------

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
  GENIF_message_t*		inMsg,
  int*				outStore
)
{
  GENIF_message_t*		ptrM1 = NULL;
  GENIF_message_t*		ptrM2 = NULL;
  GENIF_message_t*		ptrM3 = NULL;
  GENIF_message_t*		ptrM4 = NULL;
  GENIF_message_t*		ptrM5 = NULL;

  int				ret = GENIF_RC_OK;

//----------------

  TRAZA3("Entering in GENIF_channel_cancel_msg(%p, %d, %p)", 
          inChannel,
	  inChannel->fd,
	  inMsg);

//----------------

  if(inMsg == inChannel->outBuffMsg)
  {
    ptrM1 = inChannel->outBuffMsg;

    inChannel->outBuffMsg = NULL; inChannel->outBuffLen = 0;
  }
  
  ptrM2 = RLST_extract(inChannel->outQueue,   inMsg);
  ptrM3 = RFTR_extract(inChannel->outRef,     inMsg);
  ptrM4 = RLST_extract(inChannel->outRefTout, inMsg);
  ptrM5 = RLST_extract(inChannel->inRef,      inMsg);

//----------------

  if(ptrM1 != NULL)
  {
    *outStore = GENIF_O_STORE;
    
    ret = GENIF_channel_message_end(inChannel, inMsg,
                                    GENIF_END_USER, *outStore);
  }

  else if(ptrM2 != NULL)
  {
    *outStore = GENIF_O_STORE;
    
    ret = GENIF_channel_message_end(inChannel, inMsg,
                                    GENIF_END_USER, *outStore);
  }

  else if((ptrM3 != NULL) || (ptrM4 != NULL))
  {
    *outStore = GENIF_W_STORE;
    
    ret = GENIF_channel_message_end(inChannel, inMsg,
                                    GENIF_END_USER, *outStore);
  }

  else if(ptrM5 != NULL)
  {
    *outStore = GENIF_I_STORE;
    
    ret = GENIF_channel_message_end(inChannel, inMsg,
                                    GENIF_END_USER, *outStore);
  }

  else
  {
    ret = GENIF_RC_NOT_FOUND;
  }

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

//----------------

  TRAZA2("Entering in GENIF_channel_message_num(%p, %d)", 
          inChannel,
	  inChannel->fd);


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
  GENIF_message_t*		inMsg
)
{
  inChannel->notDelete = GENIF_TRUE;

  inChannel->cbNotify(inChannel, inNotify, inMsg);

  inChannel->notDelete = GENIF_FALSE;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

