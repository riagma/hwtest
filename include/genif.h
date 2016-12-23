/*____________________________________________________________________________

  MODULO:	Objetos protocolo peticio'n respuesta generico

  CATEGORIA:	GENIF

  AUTOR:	Ricardo Aguado Mart'n

  VERSION:	1.0

  FECHA:	2007 - 2013
______________________________________________________________________________

  DESCRIPCION:

  OBSERVACIONES: 
______________________________________________________________________________*/

#ifndef __GENIF_H__
#define __GENIF_H__

/*__INCLUDES DEL SISTEMA______________________________________________________*/

#include <uv.h>

/*__INCLUDES DE LA BD_________________________________________________________*/

/*__INCLUDES DE LA APLICACION_________________________________________________*/

#include "reflist.h"
#include "reftree.h"
#include "refmemo.h"

#include "buffer.h"

/*__CONSTANTES________________________________________________________________*/
  
//----------------

#define	GENIF_MAXLEN_HOST			256
#define	GENIF_MAXLEN_STRING			4096
#define	GENIF_MAXLEN_DUMP			131072

#define	GENIF_MAXNUM_MESSAGE_ID			2147483647

#define	GENIF_MAXNUM_BUFFERS			8

//----------------

#define	GENIF_FLAG_ON				1
#define	GENIF_FLAG_OFF				0

#define	GENIF_ON				1
#define	GENIF_OFF				0

#define	GENIF_TRUE				1
#define	GENIF_FALSE				0

//----------------

enum e_GENIF_CHANNEL_TYPE
{
  GENIF_CHANNEL_TYPE_NONE,
  GENIF_CHANNEL_TYPE_CLIENT,
  GENIF_CHANNEL_TYPE_SERVER,

  GENIF_CHANNEL_TYPE_TOTAL
};

extern const char* a_GENIF_CHANNEL_TYPE[];

//----------------

enum e_GENIF_MESSAGE_TYPE
{
  GENIF_MESSAGE_TYPE_UNKNOWN,
  GENIF_MESSAGE_TYPE_MESSAGE,
  GENIF_MESSAGE_TYPE_REQUEST,
  GENIF_MESSAGE_TYPE_RESPONSE,

  GENIF_MESSAGE_TYPE_TOTAL
};

extern const char* a_GENIF_MESSAGE_TYPE[];

//----------------

enum e_GENIF_RC
{
  GENIF_RC_INCOMPLETE				= -6,
  GENIF_RC_CHANNEL_CLOSED			= -5,
  GENIF_RC_QUEUE_FULL				= -4,
  GENIF_RC_ALREADY_EXISTS			= -3,
  GENIF_RC_NOT_FOUND				= -2,
  GENIF_RC_ERROR				= -1,
  GENIF_RC_OK					=  0
};

//----------------

enum e_GENIF_END_TYPE
{
  GENIF_END_USER,
  GENIF_END_CLOSE,
  GENIF_END_TIMEOUT,

  GENIF_END_TOTAL
};

//----------------

enum e_GENIF_STORE_TYPE
{
  GENIF_O_STORE,
  GENIF_W_STORE,
  GENIF_I_STORE,

  GENIF_STORE_TOTAL
};

//----------------

enum e_GENIF_NOTIFY
{
  GENIF_NOTIFY_NONE,

  GENIF_NOTIFY_REQUEST_ERROR,			// 01
  GENIF_NOTIFY_REQUEST_CLOSED,			// 02
  GENIF_NOTIFY_REQUEST_TIMEOUT,			// 03
  GENIF_NOTIFY_REQUEST_SENT,			// 04
  GENIF_NOTIFY_REQUEST_SENT_CLOSED,		// 05
  GENIF_NOTIFY_REQUEST_SENT_TIMEOUT,		// 06
		
  GENIF_NOTIFY_REQUEST_RECEIVED,		// 07
  GENIF_NOTIFY_REQUEST_RECEIVED_CLOSED,		// 08
  GENIF_NOTIFY_REQUEST_RECEIVED_TIMEOUT,	// 09

  GENIF_NOTIFY_RESPONSE_ERROR,			// 10
  GENIF_NOTIFY_RESPONSE_CLOSED,			// 11
  GENIF_NOTIFY_RESPONSE_TIMEOUT,		// 12
  GENIF_NOTIFY_RESPONSE_SENT,			// 13

  GENIF_NOTIFY_RESPONSE_RECEIVED,		// 14

  GENIF_NOTIFY_MESSAGE_ERROR,			// 15
  GENIF_NOTIFY_MESSAGE_CLOSED,			// 16
  GENIF_NOTIFY_MESSAGE_TIMEOUT,			// 17
  GENIF_NOTIFY_MESSAGE_SENT,			// 18
		
  GENIF_NOTIFY_MESSAGE_RECEIVED,		// 19
  GENIF_NOTIFY_MESSAGE_RECEIVED_CLOSED,		// 20
  GENIF_NOTIFY_MESSAGE_RECEIVED_TIMEOUT,	// 21

  GENIF_NOTIFY_UNKNOWN_RECEIVED,		// 22
  GENIF_NOTIFY_UNKNOWN_RECEIVED_CLOSED,		// 23
  GENIF_NOTIFY_UNKNOWN_RECEIVED_TIMEOUT,	// 24
	
  GENIF_NOTIFY_CHANNEL_ACCEPTED,		// 25
  GENIF_NOTIFY_CHANNEL_CONNECTED,		// 26

  GENIF_NOTIFY_CHANNEL_HEARTBEAT,		// 27
  GENIF_NOTIFY_CHANNEL_INACTIVITY,		// 28
  GENIF_NOTIFY_CHANNEL_MALFUNCTION,		// 29

  GENIF_NOTIFY_CHANNEL_ERROR,			// 30
  GENIF_NOTIFY_CHANNEL_CLOSED,			// 31

  GENIF_NOTIFY_CLIENT_CONNECTED,		// 32
  GENIF_NOTIFY_CLIENT_DISCONNECTED,		// 33

  GENIF_NOTIFY_CLIENT_EMPTY,			// 34

  GENIF_NOTIFY_SERVER_CONNECTED,		// 35
  GENIF_NOTIFY_SERVER_DISCONNECTED,		// 36

  GENIF_NOTIFY_SERVER_EMPTY,			// 37

  GENIF_NOTIFY_TOTAL				// 38
};

extern const char* GENIF_notify_name[];

//----------------

/*__TIPOS_____________________________________________________________________*/

//----------------
//----------------

typedef struct GENIF_modifier_tag		GENIF_modifier_t;

typedef struct GENIF_message_tag		GENIF_message_t;
typedef struct GENIF_channel_tag		GENIF_channel_t;

typedef struct GENIF_client_tag			GENIF_client_t;
typedef struct GENIF_server_tag			GENIF_server_t;

//----------------
//---------------- Modifier

struct GENIF_modifier_tag
{
  void*	(*message_new)(GENIF_message_t*);
  void	(*message_delete)(void*);

  int	(*message_cmp)(void*, void*);

  int	(*message_encode)(void*, int, uv_buf_t [], int*);
  int	(*message_decode)(void*, BUFF_buff_t*, int*);

  void  (*message_resp_copy)(void*, void*);

  void	(*message_view)(void*, int);
  char*	(*message_dump)(void*, long, char*);

  void* modifier;
};

//----------------
//---------------- Message

struct GENIF_message_tag
{
  int				type;
  void*				modMsg;
  GENIF_channel_t*		channel;
  GENIF_modifier_t*		modifier;

  uv_write_t 			uvReq[1];

  long				T;
  unsigned long			O;

  int				notDelete;

  RLST_element_t		list;
  RFTR_element_t		tree;
  MEMO_element_t		memo;
};

//----------------
//---------------- Channel

typedef struct GENIF_channel_config_tag
{
  long				heartbeatTout;
  long				inactivityTout;
  long				malfunctionTout;

  long				inFlowMax;
  long				outFlowMax;
  long				outToutsMax;

  long				outQueueMax;
  long				outQueueTout;

  long				outRefMax;
  long				outRefTout;

  long				inRefMax;
  long				inRefTout;

} GENIF_channel_config_t;

//----------------

typedef struct GENIF_channel_counter_tag 
{
  long				outRequest;
  long				outRequestError;
  long				outRequestClosed;
  long				outRequestTimeout;
  long				outRequestCanceled;

  long				winRequest;
  long				winRequestError;
  long				winRequestClosed;
  long				winRequestTimeout;
  long				winRequestCanceled;

  long				inRequest;
  long				inRequestError;
  long				inRequestClosed;
  long				inRequestTimeout;
  long				inRequestCanceled;

  long				outResponse;
  long				outResponseError;
  long				outResponseClosed;
  long				outResponseTimeout;
  long				outResponseCanceled;

  long				inResponse;
  long				inResponseError;

  long				outMessage;
  long				outMessageError;
  long				outMessageClosed;
  long				outMessageTimeout;
  long				outMessageCanceled;

  long				inMessage;
  long				inMessageError;
  long				inMessageClosed;
  long				inMessageTimeout;

  long				inUnknownMsg;
  long				outUnknownMsg;

} GENIF_channel_counter_t;

//----------------

struct GENIF_channel_tag
{
  uv_loop_t*			loop;
  uv_tcp_t 			channel[1];
  uv_connect_t			connreq[1];

  int				type;
  void*				parent;
  void			      (*cbNotify)(GENIF_channel_t*, int,
                                          GENIF_message_t*);

  GENIF_modifier_t*		modifier;

  void*				extRef;
  int				state;

  char				locHost[GENIF_MAXLEN_HOST + 1];
  int				locPort;

  char				remHost[GENIF_MAXLEN_HOST + 1];
  int				remPort;

  long				T;

  int				connected;

  int				readMask;
  int				writeMask;
  
  int				disabledRead;
  int				disabledWrite;

  long				heartbeatT;
  long				inactivityT;
  long				malfunctionT;

  long				outToutsCount;
  long				outFlowCount;
  long				inFlowCount;

  uv_buf_t 			outBuff[GENIF_MAXNUM_BUFFERS];
  int				outBuffNum;
  GENIF_message_t*		outBuffMsg;
  uv_write_t 			outBuffReq[1];

  BUFF_buff_t*			inBuff;
  GENIF_message_t*		inBuffMsg;

  GENIF_message_t*	      (*extQueueFunc)(GENIF_channel_t*);
  int				extQueueFlag;

  GENIF_message_t*		outMsg;

  RLST_reflist_t		outQueue[1];

  RLST_reflist_t		outRefTout[1];
  RFTR_reftree_t		outRef[1];

  RLST_reflist_t		inRef[1];

  GENIF_channel_config_t*	config;
  GENIF_channel_counter_t*	counter;

  int				notDelete;

  RLST_element_t		list;
  RFTR_element_t		tree;
  MEMO_element_t		memo;
};

//----------------
//---------------- Client

typedef struct GENIF_client_config_tag 
{
  long				connectTout;
  long				reconnectTout;

  long				channelMax;

  long				outRefMax;
  long				outFlowMax;

  long				outQueueMax;
  long				outQueueTout;

  GENIF_channel_config_t	channel;

} GENIF_client_config_t;

//----------------

typedef struct GENIF_client_counter_tag 
{
  long				channelOpen;

  GENIF_channel_counter_t	channel;

} GENIF_client_counter_t;

//----------------

struct GENIF_client_tag 
{
  uv_loop_t*			loop;
  uv_timer_t			timer[1];

  char				remHost[GENIF_MAXLEN_HOST + 1];
  int				remPort;

  char				locHost[GENIF_MAXLEN_HOST + 1];
  int				locPort;
  int				locFlag;
  
  void*				parent;

  void			      (*cbNotify)(GENIF_client_t*, int,
                                          GENIF_channel_t*,
					  GENIF_message_t*);

  GENIF_modifier_t*		modifier;

  void*				extRef;

  int				disconnFlag;

  long				reconnectTime;
  int				reconnectFlag;

  RLST_reflist_t		channel[1];
  RLST_reflist_t		chnltmp[1];

  GENIF_message_t*	      (*extQueueFunc)(GENIF_client_t*, GENIF_channel_t*);
  int				extQueueFlag;

  RLST_reflist_t		outQueue[1];

  int				emptyFlag;

  long				outRefCount;
  long				outFlowCount;

  int				disabledInput;
  int				disabledOutput;

  int				inputMask;
  int				outputMask;
  
  GENIF_client_config_t		config;
  
  GENIF_client_counter_t	counterS;
  GENIF_client_counter_t*	counter;

  int				notDelete;

  RLST_element_t		list;
  RFTR_element_t		tree;
  MEMO_element_t		memo;
};

//----------------
//---------------- Server

typedef struct GENIF_server_config_tag 
{
  long				channelMax;

  GENIF_channel_config_t	channel;

} GENIF_server_config_t;

//----------------

typedef struct GENIF_server_counter_tag 
{
  long				channelOpen;

  GENIF_channel_counter_t	channel;

} GENIF_server_counter_t;

//----------------

struct GENIF_server_tag 
{
  uv_loop_t*			loop;
  uv_tcp_t 			server[1];

  char				locHost[GENIF_MAXLEN_HOST + 1];
  int				locPort;

  int				listenFd;

  void*				parent;
  void				(*cbNotify)(GENIF_server_t*, int,
                                            GENIF_channel_t*,
				            GENIF_message_t*);

  GENIF_modifier_t*		modifier;

  void*				extRef;

  uv_timer_t			timer[1];

  int				disconnFlag;

  RLST_reflist_t		channel[1];

  GENIF_message_t*		(*extQueueFunc)(GENIF_server_t*, 
                                                GENIF_channel_t*);
  int				extQueueFlag;

  int				emptyFlag;

  int				disabledListen;

  int				disabledInput;
  int				disabledOutput;

  GENIF_server_config_t		config;

  GENIF_server_counter_t	counterS;
  GENIF_server_counter_t*	counter;
  
  int				notDelete;

  RLST_element_t		list;
  RFTR_element_t		tree;
  MEMO_element_t		memo;
};

//----------------

/*__INCLUDES DE LA APLICACION_________________________________________________*/

/*__VARIABLES EXTERNAS________________________________________________________*/

/*__VARIABLES GLOBALES________________________________________________________*/

/*__VARIABLES LOCALES_________________________________________________________*/

/*__FUNCIONES EXTERNAS________________________________________________________*/

/*__FUNCIONES PUBLICAS________________________________________________________*/

//----------------
//---------------- Factory

void GENIF_factory_initialize(void);
void GENIF_factoty_memo_view();

GENIF_message_t* GENIF_message_new(GENIF_modifier_t* inModifier);
void GENIF_message_delete(GENIF_message_t* inMessage);

GENIF_channel_t* GENIF_channel_new(void);
void GENIF_channel_delete(GENIF_channel_t* inChannel);

GENIF_client_t* GENIF_client_new(void);
void GENIF_client_delete(GENIF_client_t* inClient);

GENIF_server_t* GENIF_server_new(void);
void GENIF_server_delete(GENIF_server_t* inServer);

//----------------
//---------------- Message

int GENIF_message_cmp(void* inMessageA, void* inMessageB);

//----------------

int GENIF_message_encode
(
    GENIF_message_t*		inMessage,
    uv_buf_t 			outBuff[],
    int*			outBnum
);

int GENIF_message_decode
(
  GENIF_message_t*		inMessage,
  BUFF_buff_t*			inBuffer
);

void GENIF_message_resp_copy
(
  GENIF_message_t*		inMessage,
  GENIF_message_t*		inResp
);

//----------------

void GENIF_message_view(GENIF_message_t* inMessage, int inTraceLevel);

char* GENIF_message_dump
(
  GENIF_message_t*		inMessage,
  long				inStrLen,
  char*				outStr
);

//----------------
//---------------- Channel

int GENIF_channel_initialize
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
);

int GENIF_channel_finalize(GENIF_channel_t* inChannel);

//----------------

int GENIF_channel_address(GENIF_channel_t* inChn);

//----------------

void GENIF_channel_timer_cb(GENIF_channel_t* inChannel, long inT);

//----------------

void GENIF_channel_read_disable(GENIF_channel_t* inChannel, int inFlag);
void GENIF_channel_write_disable(GENIF_channel_t* inChannel, int inFlag);

//----------------

void GENIF_channel_write
(
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMessage
);

//----------------

int GENIF_channel_external_queue(GENIF_channel_t* inChannel, int inFlag);

int GENIF_channel_send
(
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMessage
);

int GENIF_channel_request
(
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMessage
);

int GENIF_channel_reply
(
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMessage
);

//----------------

int GENIF_channel_empty(GENIF_channel_t* inChn, int inEndType);

GENIF_message_t* GENIF_channel_get_any_msg
(
  GENIF_channel_t*		inChannel,
  int*				outStore
);

int GENIF_channel_unref_msg
(
  GENIF_channel_t*		inChn,
  GENIF_message_t*		inMessage
);

int GENIF_channel_cancel_msg
(
  GENIF_channel_t*		inChannel,
  GENIF_message_t*		inMessage,
  int*				outStore
);

long GENIF_channel_message_num
(
  GENIF_channel_t*		inChannel,
  long*				outOstore,
  long*				outWstore,
  long*				outIstore
);

//----------------
//---------------- Client

int GENIF_client_initialize
(
  GENIF_client_t*		inClient,
  char*				inHost,
  int				inPort,
  void*				inParent,
  void				(*inCbNotify)(GENIF_client_t*, int,
                                              GENIF_channel_t*,
					      GENIF_message_t*),
  GENIF_client_config_t*	inConfig,
  GENIF_modifier_t*		inModifier
);

int GENIF_client_finalize(GENIF_client_t* inClient);

//----------------

int GENIF_client_initialize_hp
(
  GENIF_client_t*		inClient,
  char*				inHost,
  int				inPort,
  char*				inLocalHost,
  int				inLocalPort,
  void*				inParent,
  void				(*inCbNotify)(GENIF_client_t*, int,
                                              GENIF_channel_t*,
					      GENIF_message_t*),
  GENIF_client_config_t*	inConfig,
  GENIF_modifier_t*		inModifier
);

//----------------

int GENIF_client_reconfig
(
  GENIF_client_t*		inClient,
  GENIF_client_config_t*	inConfig
);

int GENIF_client_count
(
  GENIF_client_t*		inClient,
  GENIF_client_counter_t*	outCounter
);

//----------------

void GENIF_client_channel_close
( 
  GENIF_client_t*		inClient,
  GENIF_channel_t*		inChannel
);

//----------------

void GENIF_client_empty_notify(GENIF_client_t* inClient);

void GENIF_client_input_disable(GENIF_client_t* inClient, int inFlag);

void GENIF_client_output_disable(GENIF_client_t* inClient, int inFlag);

//----------------

int GENIF_client_external_queue_set
(
  GENIF_client_t*	inClient,
  GENIF_message_t*	(*inExtQueueFunc)(GENIF_client_t*, GENIF_channel_t*)
);

int GENIF_client_external_queue(GENIF_client_t* inClient, int inFlag);

//----------------

int GENIF_client_send
(
  GENIF_client_t*		inClient,
  GENIF_message_t*		inMessage
);

int GENIF_client_request
(
  GENIF_client_t*		inClient,
  GENIF_message_t*		inMessage
);

int GENIF_client_prior_request
(
  GENIF_client_t*		inClient,
  GENIF_message_t*		inMessage
);

int GENIF_client_channel_send
(
  GENIF_client_t*		inClient,
  GENIF_channel_t* 		inChannel,
  GENIF_message_t*		inMessage
);

int GENIF_client_channel_request
(
  GENIF_client_t*		inClient,
  GENIF_channel_t* 		inChannel,
  GENIF_message_t*		inMessage
);

int GENIF_client_reply
(
  GENIF_client_t*		inClient,
  GENIF_message_t*		inMessage
);

int GENIF_client_unref_msg
(
  GENIF_client_t*		inClient,
  GENIF_message_t*		inMessage
);

int GENIF_client_cancel_msg
(
  GENIF_client_t*		inClient,
  GENIF_message_t*		inMessage,
  int*				outStore
);

long GENIF_client_message_num
(
  GENIF_client_t*		inClient,
  long*				outOstore,
  long*				outWstore,
  long*				outIstore
);

//----------------
//---------------- Server

int GENIF_server_initialize
(
  GENIF_server_t*		inServer,
  int				inPort,
  void*				inParent,
  void				(*inCbNotify)(GENIF_server_t*, int,
                                              GENIF_channel_t*,
				              GENIF_message_t*),
  GENIF_server_config_t*	inConfig,
  GENIF_modifier_t*		inModifier
);

int GENIF_server_finalize(GENIF_server_t* inServer);

//----------------

void GENIF_server_channel_close
( 
  GENIF_server_t*		inServer,
  GENIF_channel_t*		inChannel
);

//----------------

int GENIF_server_reconfig
(
  GENIF_server_t*		inServer,
  GENIF_server_config_t*	inConfig
);

int GENIF_server_count
(
  GENIF_server_t*		inServer,
  GENIF_server_counter_t*	outCounter
);

//----------------

void GENIF_server_empty_notify(GENIF_server_t* inServer);

void GENIF_server_listen_disable(GENIF_server_t* inServer, int inFlag);

void GENIF_server_input_disable(GENIF_server_t* inServer, int inFlag);

void GENIF_server_output_disable(GENIF_server_t* inServer, int inFlag);

//----------------

int GENIF_server_external_queue_set
(
  GENIF_server_t*	inServer,
  GENIF_message_t*	(*inExtQueueFunc)(GENIF_server_t*, GENIF_channel_t*)
);

int GENIF_server_external_queue(GENIF_server_t* inServer, int inFlag);

//----------------

int GENIF_server_channel_send
(
  GENIF_server_t*		inServer,
  GENIF_channel_t* 		inChannel,
  GENIF_message_t*		inMessage
);

int GENIF_server_channel_request
(
  GENIF_server_t*		inServer,
  GENIF_channel_t* 		inChannel,
  GENIF_message_t*		inMessage
);

int GENIF_server_reply
(
  GENIF_server_t*		inServer,
  GENIF_message_t*		inMessage
);

int GENIF_server_unref_msg
(
  GENIF_server_t*		inServer,
  GENIF_message_t*		inMessage
);

int GENIF_server_cancel_msg
(
  GENIF_server_t*		inServer,
  GENIF_message_t*		inMessage,
  int*				outStore
);

long GENIF_server_message_num
(
  GENIF_server_t*		inServer,
  long*				outOstore,
  long*				outWstore,
  long*				outIstore
);

//----------------

long GENIF_server_channel_num
(
  GENIF_server_t*		inServer
);

//----------------

void GENIF_set_fatal_error(void (*)(const char*));
void GENIF_fatal_error(char*, int, const char*, ...);

#define GENIF_FATAL(cad) GENIF_fatal_error((char *)__FILE__,(int)__LINE__,cad);

#define GENIF_FATAL0(cad) \
	GENIF_fatal_error((char *)__FILE__,(int)__LINE__, cad)

#define GENIF_FATAL1(cad,arg1) \
	GENIF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1))

#define GENIF_FATAL2(cad,arg1,arg2) \
	GENIF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2))

#define GENIF_FATAL3(cad,arg1,arg2,arg3) \
	GENIF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3))

#define GENIF_FATAL4(cad,arg1,arg2,arg3,arg4) \
	GENIF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3),(arg4))

#define GENIF_FATAL5(cad,arg1,arg2,arg3,arg4,arg5) \
	GENIF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3),(arg4),(arg5))

#define GENIF_FATAL6(cad,arg1,arg2,arg3,arg4,arg5,arg6) \
	GENIF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6))

#define GENIF_FATAL7(cad,arg1,arg2,arg3,arg4,arg5,arg6,arg7) \
	GENIF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6),(arg7))

#define GENIF_FATAL8(cad,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) \
	GENIF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6),(arg7),(arg8))

#define GENIF_FATAL9(cad,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9) \
	GENIF_fatal_error((char *)__FILE__,(int)__LINE__, cad,\
	(arg1),(arg2),(arg3),(arg4),(arg5),(arg6),(arg7),(arg8),(arg9))

//----------------

/*__FUNCIONES PRIVADAS________________________________________________________*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

#endif

