/*__________________________________________________________________________

  MODULO:	Funciones Auxiliares

  CATEGORIA:	UTILIB

  AUTOR:	Ricardo Aguado Mart'n

  VERSION:	1.0

  FECHA:	2001 - 2013
____________________________________________________________________________

  DESCRIPCION:

  OBSERVACIONES: 
____________________________________________________________________________*/


/*__INCLUDES DE LA APLICACION_______________________________________________*/

#include "trace_macros_libs.h"
#include "auxfunctions.h"

/*__INCLUDES DE LA BD_______________________________________________________*/

/*__INCLUDES DEL SISTEMA____________________________________________________*/

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef __linux__ 
#include <dirent.h>
#include <semaphore.h>
#include <netdb.h>
#include <pwd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ipc.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <sys/times.h>
#include <sys/wait.h>
#endif

#ifdef _WIN64
#include <dirent.h>
#include <semaphore.h>
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <windows.h>
#endif

/*__CONSTANTES______________________________________________________________*/

/*__TIPOS___________________________________________________________________*/

/*__VARIABLES GLOBALES______________________________________________________*/

/*__VARIABLES PRIVADAS______________________________________________________*/

//----------------

static RFTR_reftree_t	a_LocalIpTree[1];
static RFTR_reftree_t	a_LocalAddressTree[1];
static int		a_LocalAddressInit = AUXF_FALSE;

//----------------

static void (*a_external_fatal_error)(char*, int, const char*, ...) = NULL;

//----------------

/*__DEFINICIONES ADELANTADAS________________________________________________*/

/*--------------------------------------------------------------------------*/
extern struct tm* localtime_r(const time_t*, struct tm*); /* NO warning */
/*--------------------------------------------------------------------------*/

/*__FUNCIONES PRIVADAS______________________________________________________*/

//----------------

static char* AUXF_trim_s(char* ioStr, char inC, int inLrb);

static int AUXF_local_address_cmp(void* inPtrVoidA, void* inPtrVoidB);

//----------------

/*__FUNCIONES PUBLICAS______________________________________________________*/

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

void
AUXF_T(T_t* outPtrT)
{
  struct timeval 	tval;
  struct timezone	tz;

  gettimeofday(&tval, &tz);

  outPtrT->sec = tval.tv_sec; outPtrT->hth = tval.tv_usec / 10000 + 1;

  localtime_r(&outPtrT->sec, &outPtrT->stm);
  
  strftime(outPtrT->tms, 15, "%Y%m%d%H%M%S", &outPtrT->stm);
}

/*--------------------------------------------------------------------------*/

void
AUXF_T_r
(
  T_t*		inPtrT, 
  int 		inYY, 
  int 		inMM, 
  int		inDD,
  int 		inHH, 
  int 		inMI,
  int		inSS,
  T_t*		outPtrT
)
{
  T_t		aux_;
  T_t*		aux;

  if(inPtrT == NULL)
  {
    aux = &aux_; AUXF_T(aux);
  }

  else
  {
    aux = inPtrT;
  }
  
  localtime_r(&aux->sec, &outPtrT->stm);

  outPtrT->stm.tm_sec   += inSS;
  outPtrT->stm.tm_min   += inMI;
  outPtrT->stm.tm_hour  += inHH;
  outPtrT->stm.tm_mday  += inDD;
  outPtrT->stm.tm_mon   += inMM;
  outPtrT->stm.tm_year  += inYY;
  outPtrT->stm.tm_isdst = -1;

  outPtrT->sec = mktime(&outPtrT->stm); outPtrT->hth = aux->hth;

  strftime(outPtrT->tms, 15, "%Y%m%d%H%M%S", &outPtrT->stm);
}

/*--------------------------------------------------------------------------*/

long
AUXF_T_substract_hth
(
  T_t*			inT1, 
  T_t*			inT2
)
{
  long			hth;

  hth = (inT1->sec - inT2->sec) * 100 + (inT1->hth - inT2->hth);
  
  return hth;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

long
AUXF_sec(void)
{
  return time(NULL);
}

/*--------------------------------------------------------------------------*/

long
AUXF_sec_r
(
  long		inSec, 
  int 		inYY, 
  int 		inMM, 
  int		inDD,
  int 		inHH, 
  int 		inMI, 
  int		inSS
)
{
  long			aux;

  struct tm 		stm;

  aux = inSec == 0 ? time(NULL) : inSec;

  localtime_r(&aux, &stm);

  stm.tm_sec   += inSS;
  stm.tm_min   += inMI;
  stm.tm_hour  += inHH;
  stm.tm_mday  += inDD;
  stm.tm_mon   += inMM;
  stm.tm_year  += inYY;
  stm.tm_isdst = -1;

  return mktime(&stm);
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

long
AUXF_gmt_diff(void)
{
  char			gmt[16];
  char			lct[16];

  time_t		t = time(NULL);
  struct tm 		tm;

  gmtime_r(&t, &tm);
  
  strftime(gmt, 15, "%Y%m%d%H%M%S", &tm);

  localtime_r(&t, &tm);
  
  strftime(lct, 15, "%Y%m%d%H%M%S", &tm);

  return AUXF_tms_substract(gmt, lct);
}

/*--------------------------------------------------------------------------*/

char*
AUXF_tms(char* outTms)
{
  time_t		t = time(NULL);
  struct tm 		tm;

  localtime_r(&t, &tm);
  
  strftime(outTms, 15, "%Y%m%d%H%M%S", &tm);

  return outTms;
}

/*--------------------------------------------------------------------------*/

char*
AUXF_sec_to_tms(long inSec, char* outTms)
{
  struct tm 		tm;

  localtime_r(&inSec, &tm);
  
  strftime(outTms, 15, "%Y%m%d%H%M%S", &tm);

  return outTms;
}

/*--------------------------------------------------------------------------*/

long
AUXF_tms_to_sec(char* inTms)
{
  char			aux[128];

  int			YY;
  int			MM;
  int			DD;
  int			HH;
  int			MI;
  int			SS;

  struct tm 		tm;

  if(inTms == NULL)
  {
    AUXF_tms(aux);
  }

  else
  {
    memcpy(aux, inTms, 14); aux[14] = 0;
  }

  SS  = atoi(aux + 12); aux[12] = 0;
  MI  = atoi(aux + 10); aux[10] = 0;
  HH  = atoi(aux +  8); aux[ 8] = 0;
  DD  = atoi(aux +  6); aux[ 6] = 0;
  MM  = atoi(aux +  4); aux[ 4] = 0;
  YY  = atoi(aux +  0);

  tm.tm_sec   = SS;
  tm.tm_min   = MI;
  tm.tm_hour  = HH;
  tm.tm_mday  = DD;
  tm.tm_mon   = MM -1;
  tm.tm_year  = YY -1900;
  tm.tm_isdst = -1;

  return mktime(&tm);
}

/*--------------------------------------------------------------------------*/

char*
AUXF_tms_r
(
  const char*	inTms, 
  int 		inYY, 
  int 		inMM, 
  int		inDD,
  int 		inHH, 
  int 		inMI, 
  int		inSS,
  char*		outTms
)
{
  char			aux[128];

  int			YY;
  int			MM;
  int			DD;
  int			HH;
  int			MI;
  int			SS;

//time_t 		t;
  struct tm 	tm;

  if(inTms == NULL)
  {
    AUXF_tms(aux);
  }

  else
  {
    memcpy(aux, inTms, 14); aux[14] = 0;
  }

  SS  = atoi(aux + 12); aux[12] = 0;
  MI  = atoi(aux + 10); aux[10] = 0;
  HH  = atoi(aux +  8); aux[ 8] = 0;
  DD  = atoi(aux +  6); aux[ 6] = 0;
  MM  = atoi(aux +  4); aux[ 4] = 0;
  YY  = atoi(aux +  0);

  tm.tm_sec   = SS + inSS;
  tm.tm_min   = MI + inMI;
  tm.tm_hour  = HH + inHH;
  tm.tm_mday  = DD + inDD;
  tm.tm_mon   = MM + inMM -1;
  tm.tm_year  = YY + inYY -1900;
  tm.tm_isdst = -1;

//t = mktime(&tm);

  mktime(&tm);

  strftime(outTms, 15, "%Y%m%d%H%M%S", &tm);

  return outTms;
}

/*--------------------------------------------------------------------------*/

char*
AUXF_tms_f
(
  const char*		inTms,
  const char*		inFormat,
  long			inOutLen,
  char*			outTms
)
{
  char			aux[128];

  time_t 		t;
  struct tm 		tm;
  
  if(inTms == NULL)
  {
    t = time(NULL);

    localtime_r(&t, &tm);
  }

  else
  {
    memcpy(aux, inTms, 14); aux[14] = 0;

    tm.tm_sec   = atoi(aux + 12);        aux[12] = 0;
    tm.tm_min   = atoi(aux + 10);        aux[10] = 0;
    tm.tm_hour  = atoi(aux +  8);        aux[ 8] = 0;
    tm.tm_mday  = atoi(aux +  6);        aux[ 6] = 0;
    tm.tm_mon   = atoi(aux +  4) - 1;    aux[ 4] = 0;
    tm.tm_year  = atoi(aux +  0) - 1900;
    tm.tm_isdst = -1;

    t = mktime(&tm);
  }

  strftime(outTms, inOutLen, inFormat, &tm); 

  return outTms;
}

/*--------------------------------------------------------------------------*/

long
AUXF_tms_substract
(
  const char*   inTms1,
  const char*   inTms2
)
{
  time_t                t1;
  time_t                t2;

  struct tm             tm1;
  struct tm             tm2;

  char                  aux1[128];
  char                  aux2[128];

  if(inTms1 == NULL)
  {
    AUXF_tms(aux1);
  }

  else
  {
    memcpy(aux1, inTms1, 14); aux1[14] = 0;
  }

  tm1.tm_sec   = atoi(aux1 + 12); aux1[12] = 0;
  tm1.tm_min   = atoi(aux1 + 10); aux1[10] = 0;
  tm1.tm_hour  = atoi(aux1 +  8); aux1[ 8] = 0;
  tm1.tm_mday  = atoi(aux1 +  6); aux1[ 6] = 0;
  tm1.tm_mon   = atoi(aux1 +  4); aux1[ 4] = 0;
  tm1.tm_year  = atoi(aux1 +  0);
  tm1.tm_mon  -= 1;
  tm1.tm_year -= 1900;
  tm1.tm_isdst = -1;

  t1 = mktime(&tm1);

  if(inTms2 == NULL)
  {
    AUXF_tms(aux2);
  }

  else
  {
    memcpy(aux2, inTms2, 14); aux2[14] = 0;
  }

  tm2.tm_sec   = atoi(aux2 + 12); aux2[12] = 0;
  tm2.tm_min   = atoi(aux2 + 10); aux2[10] = 0;
  tm2.tm_hour  = atoi(aux2 +  8); aux2[ 8] = 0;
  tm2.tm_mday  = atoi(aux2 +  6); aux2[ 6] = 0;
  tm2.tm_mon   = atoi(aux2 +  4); aux2[ 4] = 0;
  tm2.tm_year  = atoi(aux2 +  0);
  tm2.tm_mon  -= 1;
  tm2.tm_year -= 1900;
  tm2.tm_isdst = -1;

  t2 = mktime(&tm2);
  
  return(t1 - t2);
}

/*--------------------------------------------------------------------------*/

int
AUXF_tms_verify(const char* inTms)
{
  char			aux[16];
  long			len;

  int			YY = 0;
  int			MM = 0;
  int			DD = 0;
  int			HH = 0;
  int			MI = 0;
  int			SS = 0;

  struct tm 		tm;

  int			error = 0;

  memcpy(aux, inTms, 16); aux[15] = 0; len = strlen(aux);

  if(len == 14L)
  {
    SS = atoi(aux + 12); aux[12] = 0;
    MI = atoi(aux + 10); aux[10] = 0;
    HH = atoi(aux +  8); aux[ 8] = 0;
    DD = atoi(aux +  6); aux[ 6] = 0;
    MM = atoi(aux +  4); aux[ 4] = 0;
    YY = atoi(aux +  0);
  }

  else if(len == 8L)
  {
    SS = 0;
    MI = 0;
    HH = 12;
    DD = atoi(aux +  6); aux[ 6] = 0;
    MM = atoi(aux +  4); aux[ 4] = 0;
    YY = atoi(aux +  0);
  }

  else if(len == 6L)
  {
    SS = atoi(aux +  4); aux[ 4] = 0;
    MI = atoi(aux +  2); aux[ 2] = 0;
    HH = atoi(aux +  0); aux[ 0] = 0;
    DD = 1;
    MM = 1;
    YY = 2000;
  }

  else
  {
    error = -1;
  }

  if(!error)
  {
    tm.tm_sec   = SS;
    tm.tm_min   = MI;
    tm.tm_hour  = HH;
    tm.tm_mday  = DD;
    tm.tm_mon   = MM -1;
    tm.tm_year  = YY -1900;
    tm.tm_isdst = -1;

    mktime(&tm);

    if((tm.tm_sec  !=  SS) ||
       (tm.tm_min  !=  MI) ||
       (tm.tm_hour !=  HH) ||
       (tm.tm_mday !=  DD) ||
       (tm.tm_mon  != (MM - 1)) ||
       (tm.tm_year != (YY - 1900)))
    {
      error = -1;
    }
  }

  return error;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

long
AUXF_least_common_multiple(long inA, long inB)
{
  long			a = inA;
  long			b = inB;
  
  while(a != b) if(a > b) {a -= b;} else {b -= a;}

  return inA * inB / a;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

int
AUXF_int_verify(const char* inStr)
{
  char			minInt[]  = "-2147483648";
  char			maxInt[]  = "2147483647";
  long			lenMinInt = 11;
  long			lenMaxInt = 10;

  int			i;
  
  int			end = 0;

  int			error = 0;

  for(i = 0; (inStr[i] != 0) && (!end); i++)
  {
    if((inStr[i] < '0') || (inStr[i] > '9'))
    {
      if((i > 0) || (inStr[i] != '-'))
      {
        end =  1; error = -1;
      }
    }
  }

  if(error == 0)
  {
    if(i >= lenMaxInt)
    {
      if(i > lenMinInt)
      {
        error = -1;
      }

      else if((i == lenMinInt) && (inStr[0] == '-'))
      {
        if(strcmp(inStr, minInt) > 0) error = -1;
      }

      else if((i == lenMaxInt) && (inStr[0] != '-'))
      {
        if(strcmp(inStr, maxInt) > 0) error = -1;
      }

      else
      {
        error = -1;
      }
    }
  }

  return error;
}

/*--------------------------------------------------------------------------*/

int
AUXF_long_verify(const char* inStr)
{
  char			minInt[]  = "-9223372036854775808";
  char			maxInt[]  = "9223372036854775807";
  long			lenMinInt = 20;
  long			lenMaxInt = 19;

  int			i;
  
  int			end = 0;

  int			error = 0;

  for(i = 0; (inStr[i] != 0) && (!end); i++)
  {
    if((inStr[i] < '0') || (inStr[i] > '9'))
    {
      if((i > 0) || (inStr[i] != '-'))
      {
        end =  1; error = -1;
      }
    }
  }

  if(error == 0)
  {
    if(i >= lenMaxInt)
    {
      if(i > lenMinInt)
      {
        error = -1;
      }

      else if((i == lenMinInt) && (inStr[0] == '-'))
      {
        if(strcmp(inStr, minInt) > 0) error = -1;
      }

      else if((i == lenMaxInt) && (inStr[0] != '-'))
      {
        if(strcmp(inStr, maxInt) > 0) error = -1;
      }

      else
      {
        error = -1;
      }
    }
  }

  return error;
}


/*--------------------------------------------------------------------------*/

int
AUXF_float_verify(const char* inStr)
{
  int			i;
  
  int			end = 0;

  int			error = 0;
  
  int                   dotOcurrence = 0;

  for(i = 0; (inStr[i] != 0) && (!end); i++)
  {
    if((inStr[i] < '0') || (inStr[i] > '9'))
    {
      if (inStr[i] == '.' && dotOcurrence == 0)
      {
        dotOcurrence = 1;
      }  
      else if((i > 0) || (inStr[i] != '-'))
      {
        end =  1; error = -1;
      }
    }
  }

  if( strlen(inStr) == 1 && inStr[0] == '.' )
  {
    error = -1;
  }
     
  return error;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

int 
AUXF_ip_verify(const char* inIP)
{
  long			i = 0;
  long			j = 0;
  
  long			len;

  int			n;
  char			c;
  
  char			aux[AUXF_MAXLEN_IP + 1];

  int			state = 0;
  int			error = 0;

  len = strlen(inIP);

  for(i = 0; i <= len && error == 0; i++)
  {
    c = inIP[i];

    switch(state)
    {
      case 0:
      case 1:
      case 2:
      case 3:
      {
        if(c >= '0' && c <= '9')
	{
	  aux[j++] = c; 

	  if(j > 3) error = -1;
	}

	else if(c == '.' || c == 0)
	{
	  if(j == 0)
	  {
	    error = -1;
	  }

	  else
	  {
	    aux[j] = 0; n = atoi(aux); j = 0;

	    if(n > 255)
            {
              error = -1;
            }
            
            else
            {
	      state++;
            }
	  }
	}

	else
	{
	  error = -1;
	}
	
        break;
      }
      
      default:
      {
        error = -1;
	
        break;
      }
    }
  }

  if(state < 4) error = -1;

  return error;
}

/*--------------------------------------------------------------------------*/

char* 
AUXF_ip_normalize(const char* inIP, char* outIP)
{
  long			i = 0;
  long			j = 0;
  
  long			len;

  int			n;
  char			c;
  
  char			aux[AUXF_MAXLEN_IP + 1];

  char			IP[AUXF_MAXLEN_IP + 1];

  int			state = 0;

  int			error = 0;

  len = strlen(inIP);

  for(i = 0; i < len && error == 0; i++)
  {
    c = inIP[i];

    switch(state)
    {
      case 0:
      case 1:
      case 2:
      case 3:
      {
        if(c >= '0' && c <= '9')
	{
	  aux[j] = c; j++; 

	  error = j < 3 ? 0 : -1;
	}

	else if(c == '.')
	{
	  if(j == 0)
	  {
	    error = -1;
	  }

	  else
	  {
	    aux[j] = 0; j = 0; n = atoi(aux);

	    sprintf(IP + state * 4, "%03d.", n);

	    error = n < 256 ? 0 : -1;

	    state++;
	  }
	}

	else
	{
	  error = -1;
	}
	
        break;
      }
      
      default:
      {
        error = -1;
	
        break;
      }
    }
  }

  if(error == 0) {strcpy(outIP, IP);} else {strcpy(outIP, "");}

  return outIP;
}

/*--------------------------------------------------------------------------*/

unsigned int
AUXF_ip_to_number(const char* inIP)
{
  long			i = 0;
  long			j = 0;
  
  long			len;

  int			n;
  char			c;
  
  char			aux[AUXF_MAXLEN_IP + 1];

  unsigned int		IP = 0;

  int			state = 0;

  int			error = 0;

  len = strlen(inIP);

  for(i = 0; i < len && error == 0; i++)
  {
    c = inIP[i];

    switch(state)
    {
      case 0:
      case 1:
      case 2:
      case 3:
      {
        if(c >= '0' && c <= '9')
	{
	  aux[j] = c; j++; 

	  error = j <= 3 ? 0 : -1;
	}

	else if(c == '.')
	{
	  if(j == 0)
	  {
	    error = -1;
	  }

	  else
	  {
	    aux[j] = 0; j = 0; n = atoi(aux);

	    IP += (2 ^ (8 * (3 - state))) * n;

	    error = n < 256 ? 0 : -1;

	    state++;
	  }
	}

	else
	{
	  error = -1;
	}
	
        break;
      }
      
      default:
      {
        error = -1;
	
        break;
      }
    }
  }

  return IP;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

void
AUXF_tval_substract
(
  struct timeval*	tval1, 
  struct timeval*	tval2,
  struct timeval*	tvalo
)
{
  tvalo->tv_sec = tval1->tv_sec - tval2->tv_sec;

  while(tval1->tv_usec >= 1000000)
  {
    tvalo->tv_sec++; tval1->tv_usec -= 1000000;
  }

  while(tval2->tv_usec >= 1000000)
  {
    tvalo->tv_sec--; tval2->tv_usec -= 1000000;
  }

  if(tval1->tv_usec >= tval2->tv_usec)
  {
    tvalo->tv_usec = tval1->tv_usec - tval2->tv_usec;
  }

  else
  {
    tvalo->tv_sec--;
    tvalo->tv_usec = 1000000 + tval1->tv_usec - tval2->tv_usec;
  }
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

static char*
AUXF_trim_s(char* ioStr, char inC, int inLrb)
{
  long		i;
  long		len;

  if((inLrb == 1) || (inLrb == 2))
  {
    len = strlen(ioStr);
    for(i = len - 1; (i >= 0) && (ioStr[i] == inC); i--);
    ioStr[i + 1] = 0;
  }

  if((inLrb == 0) || (inLrb == 2))
  {
    len = strlen(ioStr);
    for(i = 0; (i < len) && (ioStr[i] == inC); i++);
    memmove(ioStr, ioStr + i, len - i + 1);
  }

  return ioStr;
}

/*--------------------------------------------------------------------------*/

char* AUXF_ltrim(char* ioStr) { return AUXF_trim_s(ioStr,' ',0); }
char* AUXF_rtrim(char* ioStr) { return AUXF_trim_s(ioStr,' ',1); }
char* AUXF_btrim(char* ioStr) { return AUXF_trim_s(ioStr,' ',2); }

/*--------------------------------------------------------------------------*/

char* AUXF_ltrim0(char* ioStr) { return AUXF_trim_s(ioStr,'0',0); }
char* AUXF_rtrim0(char* ioStr) { return AUXF_trim_s(ioStr,'0',1); }
char* AUXF_btrim0(char* ioStr) { return AUXF_trim_s(ioStr,'0',2); }

/*--------------------------------------------------------------------------*/

char* AUXF_ltrimC(char* ioStr, char inC) { return AUXF_trim_s(ioStr,inC,0); }
char* AUXF_rtrimC(char* ioStr, char inC) { return AUXF_trim_s(ioStr,inC,1); }
char* AUXF_btrimC(char* ioStr, char inC) { return AUXF_trim_s(ioStr,inC,2); }

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

char* 
AUXF_scape_del(char* ioStr)
{
  long		i,j;
  long		len;

  len = strlen(ioStr);

  for(i = 0, j=0; i < len; i++, j++)
  {
    if(ioStr[i] == '\\') i++; ioStr[j] = ioStr[i];
  }

  ioStr[j] = 0;

  return ioStr;
}

/*--------------------------------------------------------------------------*/

char* 
AUXF_scape_add(char* inStr, char* inCs, long* ioPtrLen, char* outStr)
{
  long		i,j,k;

  long		lenMax;
  long		len;

  lenMax = *ioPtrLen; *ioPtrLen = 0; len = strlen(inStr);

  for(i = 0, j = 0; (i < len) && (j < lenMax - 1); i++, j++)
  {
    for(k = 0; inCs[k] != 0; k++)
    {
      if(inStr[i] == inCs[k]) outStr[j++] = '\\';
    }

    outStr[j] = inStr[i];
  }

  outStr[j] = 0; *ioPtrLen = j; 

  return outStr;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

char*
AUXF_memory_dump
(
  const void*		inPtrMem,
  long			inMemLen,
  long			inStrLen,
  char*			outStr
)
{
  char*			ret = outStr;
  
  const unsigned char*	buf = (const unsigned char*)(inPtrMem);
  
  char			dir[1024 + 1];
  char			oct[1024 + 1];
  char			str[1024 + 1];
  char			aux[1024 + 1];

  long			i;
  
  strcpy(ret, "");
  strcpy(dir, "");
  strcpy(oct, "");
  strcpy(str, "");
  
  for(i = 0; i < inMemLen; i++)
  {
    if((i % 16) == 0)
    {
      sprintf(dir, "%04lX:  ", i);
    }

    if(((i + 1) % 4) == 0)
    {
      sprintf(aux, "%02X  ", buf[i]); strcat(oct, aux);
    }

    else
    {
      sprintf(aux, "%02X ",  buf[i]); strcat(oct, aux);
    }

    if((buf[i] >= 32) && (buf[i] < 127))
    {
      sprintf(aux, "%c", buf[i]); strcat(str, aux);
    }

    else
    {
      strcat(str, ".");
    }

    if((((i + 1) % 16) == 0) || ((i + 1) == inMemLen))
    {
      while(strlen(oct) < 52L) strcat(oct, " ");
      while(strlen(str) < 16L) strcat(str, " ");
      
      strncat(ret,  dir, inStrLen - strlen(ret)); ret[inStrLen - 1] = 0;
      strncat(ret,  oct, inStrLen - strlen(ret)); ret[inStrLen - 1] = 0;
      strncat(ret,  "|", inStrLen - strlen(ret)); ret[inStrLen - 1] = 0;
      strncat(ret,  str, inStrLen - strlen(ret)); ret[inStrLen - 1] = 0;
      strncat(ret,  "|", inStrLen - strlen(ret)); ret[inStrLen - 1] = 0;
      strncat(ret, "\n", inStrLen - strlen(ret)); ret[inStrLen - 1] = 0;
      
      strcpy(dir, "");
      strcpy(oct, "");
      strcpy(str, "");
    }
  }

  ret[strlen(ret) - 1] = 0;

  return ret;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

char*
AUXF_mem_to_hex
(
  const void*		inPtrMem,
  long			inMemLen,
  long			inStrLen,
  char*			outStrHex
)
{
  const unsigned char*	mem = (const unsigned char*)(inPtrMem);
  
  char*			hex = outStrHex;
  long			len = 0;
  
  long			idx;
  
  for(idx = 0; idx < inMemLen; idx++)
  {
    if(len + 2 < inStrLen)
    {
      sprintf(hex + len, "%02X", mem[idx]); len +=2;
    }
  }

  hex[len] = 0; return hex;
}

/*--------------------------------------------------------------------------*/

long
AUXF_hex_to_mem
(
  const char*		inStrHex,
  long			inMemLen,
  void*			outPtrMem
)
{
  const char*		hex = inStrHex;
  long			max;
  
  unsigned char*	mem = (unsigned char*)(outPtrMem);
  long			len = 0;
  
  char			str[3];
  long			idx;

  max = strlen(inStrHex); if(max % 2 != 0) max--;

  for(idx = 0; idx < max; idx+=2)
  {
    str[0] = hex[idx]; str[1] = hex[idx + 1]; str[2] = 0;
    
    if(len < inMemLen)
    {
      sscanf(str, "%hhx", &mem[len++]);
    }
  }

  return len;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

int 
AUXF_file_check
(
  const char*	inFile, 
  long* 	outSize,
  long* 	outTime
)
{
  int		existe;
  struct stat	st;
  
  if(inFile == NULL)
  {
    existe = 0;
  }

  else if(stat(inFile, &st) < 0)
  {
    existe = 0;
  }
  
  else if(S_ISREG(st.st_mode) == 0)
  {
    existe = 0;
  }

  else
  {
    existe = 1;
    
    if(outSize != NULL) *outSize  = st.st_size;
    if(outTime != NULL) *outTime  = st.st_mtime;
  }

  return existe;
}

/*--------------------------------------------------------------------------*/

int 
AUXF_file_copy
(
  const char*		inFileSrc, 
  const char*		inFileDst
)
{
  FILE* 		src;
  FILE*			dst;

  size_t		size;

  char			buffer[4096];

  src = fopen(inFileSrc, "rb");

  if(src == NULL)
  {
    return -1;
  }

  dst = fopen(inFileDst, "wb");

  if(dst == NULL)
  {
    fclose ( src );

    return -1;
  }

  while(!feof(src))
  {
    size = fread(buffer, 1, 4096, src);

    if(size > 0L)
    {
      if(fwrite(buffer, 1, size, dst) < size)
      {
        fclose(src); fclose(dst);

        return -1;
      }
    }
    
    else if(ferror(src))
    {
      fclose(src); fclose(dst);

      return -1;
    }
  }

  fclose(src); fclose(dst);

  return 0;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

double
AUXF_random_get()
{
  static int    	ini = 0;
  struct timeval 	tval;
  struct timezone	tz;
  double        	num;

  if(ini == 0)
  {
    ini = 1;
     
    gettimeofday(&tval, &tz);
    
    srand((unsigned)tval.tv_usec);
  }

  num = ((double) rand()) / RAND_MAX;

  return num;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

char*
AUXF_login_get
(
  long			inMaxLen,
  char*			outLogin
)
{
  struct passwd		pass;
  struct passwd*	pres;

  char			buff[1024];

#ifdef __linux__
  if(getpwuid_r(getuid(), &pass, buff, 1024, &pres))
#else
  if(getpwuid_r(getuid(), &pass, buff, 1024) == NULL)
#endif
  {
    strcpy(outLogin, "");
  }

  else
  {
    strncpy(outLogin, pass.pw_name, inMaxLen); 

    outLogin[inMaxLen] = 0;
  }

  return outLogin;
}

/*--------------------------------------------------------------------------*/

char*
AUXF_machine_get
(
  long			inMaxLen,
  char*			outMachine
)
{
  gethostname(outMachine, inMaxLen); 

  outMachine[inMaxLen] = 0;

  return outMachine;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

int 
AUXF_is_alfanum
(
  const char*	inString 
)
{
  int		alfanum = AUXF_TRUE;

  long		i = 0;

  char		c;

  while((c = inString[i++] != 0) && alfanum)
  {
    alfanum = AUXF_is_alfanum_c(c);
  }

  return alfanum;
}

/*--------------------------------------------------------------------------*/

int 
AUXF_is_alfanum_c
(
  int		inC 
)
{
  int		alfanum = AUXF_FALSE;

  if((inC >= 'a') && (inC <= 'z'))
  {
    alfanum = AUXF_TRUE;
  }

  else if((inC >= 'A') && (inC <= 'Z'))
  {
    alfanum = AUXF_TRUE;
  }

  else if((inC >= '0') && (inC <= '9'))
  {
    alfanum = AUXF_TRUE;
  }

  else if((inC == '-') || (inC == '_'))
  {
    alfanum = AUXF_TRUE;
  }

  return alfanum;
}



/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static int 
AUXF_local_address_cmp(void* inPtrVoidA, void* inPtrVoidB)
{
  AUXF_local_address_t*	inAddrA = (AUXF_local_address_t*)(inPtrVoidA);
  AUXF_local_address_t*	inAddrB = (AUXF_local_address_t*)(inPtrVoidB);

  int				cmp;

//----------------

  cmp = strcmp(inAddrA->addr, inAddrB->addr);

//----------------

  return cmp;
}

/*----------------------------------------------------------------------------*/

char*
AUXF_local_ip_get(char* outAddr) 
{
  AUXF_local_address_t*		ptrLa;

//----------------

  if(a_LocalAddressInit == AUXF_FALSE)
  {
    AUXF_local_address_load();
  }

//----------------

  RFTR_resetGet(a_LocalIpTree, NULL);

  if((ptrLa = RFTR_getNext(a_LocalIpTree)))
  {
    if(strcmp(ptrLa->addr, "127.0.0.1") == 0)
    {
      ptrLa = RFTR_getNext(a_LocalIpTree);
    }
  }

  if(ptrLa == NULL)
  {
    strcpy(outAddr, "127.0.0.1");
  }

  else
  {
    strcpy(outAddr, ptrLa->addr);
  }

//----------------

  return outAddr;
}

/*----------------------------------------------------------------------------*/

int
AUXF_local_ip_check(char* inAddr) 
{
  AUXF_local_address_t*		ptrLa;
  AUXF_local_address_t		la[1];

  int				ret = AUXF_FALSE;

//----------------

  if(a_LocalAddressInit == AUXF_FALSE)
  {
    AUXF_local_address_load();
  }

//----------------

  strncpy(la->addr, inAddr, AUXF_MAXLEN_ADDRESS);

  la->addr[AUXF_MAXLEN_ADDRESS] = 0;

  ptrLa = RFTR_find(a_LocalIpTree, la, NULL, NULL);

  if(ptrLa != NULL)
  {
    ret = AUXF_TRUE;
  }

//----------------

  return ret;
}

/*----------------------------------------------------------------------------*/

int
AUXF_local_address_check(char* inAddr) 
{
  AUXF_local_address_t*		ptrLa;
  AUXF_local_address_t		la[1];

  int				ret = AUXF_FALSE;

//----------------

  if(a_LocalAddressInit == AUXF_FALSE)
  {
    AUXF_local_address_load();
  }

//----------------

  strncpy(la->addr, inAddr, AUXF_MAXLEN_ADDRESS);

  la->addr[AUXF_MAXLEN_ADDRESS] = 0;

  ptrLa = RFTR_find(a_LocalAddressTree, la, NULL, NULL);

  if(ptrLa != NULL)
  {
    ret = AUXF_TRUE;
  }

//----------------

  return ret;
}

/*----------------------------------------------------------------------------*/

void
AUXF_local_address_load(void) 
{
  int				s;

  struct ifconf			ifconf;

  struct ifreq			ifreqs[100];

  int				nif;

  struct sockaddr*		psa;
  struct sockaddr_in*		psai;

  static socklen_t		lsa  = sizeof(struct sockaddr);

  struct hostent*		he;

  struct in_addr 		sin_addr;
  
  AUXF_local_address_t		la;

  AUXF_local_address_t*		pla;
  AUXF_local_address_t*		pip;
  
  static size_t			lla  = sizeof(AUXF_local_address_t);

  long				num = 0;
  long				idx = 0;

  unsigned int			flags = 0;

  int				gai_errno;

  char				node[AUXF_MAXLEN_ADDRESS + 1];

//----------------

  if(a_LocalAddressInit == AUXF_FALSE)
  {
    if(RFTR_initializeRefTree(a_LocalIpTree,
			      &la,
			      &la.tree, 
			      AUXF_local_address_cmp) != 0)
    {
      AUXF_FATAL0("ERROR: RFTR_initializeRefTree()");
    }

    if(RFTR_initializeRefTree(a_LocalAddressTree, 
			      &la,
			      &la.tree, 
			      AUXF_local_address_cmp) != 0)
    {
      AUXF_FATAL0("ERROR: RFTR_initializeRefTree()");
    }

    a_LocalAddressInit = AUXF_TRUE;
  }

//----------------

  while((pip = RFTR_extractMini(a_LocalIpTree))      != NULL) {free(pip);}
  while((pla = RFTR_extractMini(a_LocalAddressTree)) != NULL) {free(pla);}

//----------------

  gethostname(node, AUXF_MAXLEN_ADDRESS); node[AUXF_MAXLEN_ADDRESS] = 0;

  SUCESO1("NODE %s:", node);

  pla = malloc(lla); memset(pla, 0, lla);

  strcpy(pla->addr, node);
  
  if(RFTR_insert(a_LocalAddressTree, pla) != RFTR_RC_OK)
  {
    free(pla);
  }

//----------------

  memset(&ifconf, 0, sizeof(ifconf));

  ifconf.ifc_buf = (char*)(ifreqs); ifconf.ifc_len = sizeof(ifreqs);

  if((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    SUCESO2("ERROR: socket() = %d (%s)", errno, strerror(errno));
  }

// TODO
#ifndef _WIN64
  else if(ioctl(s, SIOCGIFCONF, &ifconf) < 0)
  {
    SUCESO2("ERROR: ioctl() = %d (%s)", errno, strerror(errno));
  }
#endif

  else
  {
    nif = ifconf.ifc_len / sizeof(struct ifreq);
    
    for(idx = 0; idx < nif; idx++)
    {
      pip = malloc(lla); memset(pip, 0, lla);
      pla = malloc(lla); memset(pla, 0, lla);

      psa = &ifreqs[idx].ifr_addr; psai = (struct sockaddr_in*)(psa);

      snprintf(pip->addr, AUXF_MAXLEN_ADDRESS + 1, "%s", inet_ntoa(psai->sin_addr));
      snprintf(pla->addr, AUXF_MAXLEN_ADDRESS + 1, "%s", inet_ntoa(psai->sin_addr));

      if(RFTR_insert(a_LocalIpTree, pip) != RFTR_RC_OK)
      {
        free(pip);
      }

      if(RFTR_insert(a_LocalAddressTree, pla) == RFTR_RC_OK)
      {
        pla = malloc(lla); memset(pla, 0, lla);
      }

      gai_errno = getnameinfo(psa, lsa, pla->addr, AUXF_MAXLEN_ADDRESS, NULL, 0, flags);

      if(gai_errno != 0)
      {
// TODO
#ifndef _WIN64
        if(gai_errno == EAI_SYSTEM) gai_errno = errno; free(pla);
#endif
        SUCESO2("ERROR: getnameinfo() = %d (%s)", gai_errno, gai_strerror(gai_errno));
      }

      else if(RFTR_insert(a_LocalAddressTree, pla) != RFTR_RC_OK)
      {
        free(pla);
      }
    }
  }

  if(s > 0) close(s);

//----------------

  if((he = gethostbyname(node)) == NULL)
  {
    SUCESO2("ERROR: gethostbyname() = %d (%s)", errno, strerror(errno));
  }

  else
  {
    for(idx = 0; he->h_addr_list[idx] != NULL; idx++)
    {
      pla = malloc(lla); memset(pla, 0, lla);
      
      memcpy(&sin_addr, he->h_addr_list[idx], he->h_length);

      snprintf(pla->addr, AUXF_MAXLEN_ADDRESS + 1, "%s", inet_ntoa(sin_addr));

      if(RFTR_insert(a_LocalAddressTree, pla) != RFTR_RC_OK)
      {
        free(pla);
      }
    }

    for(idx = 0; he->h_aliases[idx] != NULL; idx++)
    {
      pla = malloc(lla); memset(pla, 0, lla);
      
      snprintf(pla->addr, AUXF_MAXLEN_ADDRESS + 1, "%s", he->h_aliases[idx]);

      if(RFTR_insert(a_LocalAddressTree, pla) != RFTR_RC_OK)
      {
        free(pla);
      }
    }

    endhostent();
  }

//----------------

  RFTR_resetGet(a_LocalIpTree, NULL); num = 0;

  while((pip = RFTR_getNext(a_LocalIpTree)) != NULL)
  {
    SUCESO2("- LIP(%02ld) = <%s>", num++, pip->addr);
  }

  RFTR_resetGet(a_LocalAddressTree, NULL); num = 0;

  while((pla = RFTR_getNext(a_LocalAddressTree)) != NULL)
  {
    SUCESO2("- LAD(%02ld) = <%s>", num++, pla->addr);
  }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void
AUXF_set_external_fatal_error(void (*inCb)(char*, int, const char*, ...))
{
  a_external_fatal_error = inCb;
}

/*----------------------------------------------------------------------------*/

void
AUXF_fatal_error(char* inFile, int inLine, const char* inErrorStr, ...)
{
  va_list		list;

  char			errorStr[AUXF_MAXLEN_STRING + 1];

//----------------

  va_start(list, inErrorStr);
    
  vsnprintf(errorStr, AUXF_MAXLEN_STRING + 1, inErrorStr, list);

  va_end(list);

  errorStr[AUXF_MAXLEN_STRING] = 0;

//----------------

  TRACE_write(TRACE_TYPE_DEFAULT, 0, "<%s %d> %s", inFile, inLine, errorStr);

  if(a_external_fatal_error)
  {
    a_external_fatal_error(inFile, inLine, "ERROR: PdasLib");
  }

//----------------

  exit(-1);
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

