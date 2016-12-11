/*______________________________________________________________________________

  MODULO:	Trazas

  CATEGORIA:	UTILIB

  AUTOR:	Ricardo Aguado Mart'n

  VERSION:	1.0

  FECHA:	2009 - 2013
________________________________________________________________________________

  DESCRIPCION:

  OBSERVACIONES: 
______________________________________________________________________________*/

/*----------*/
/*----------*/

// #define TRACE_OVERWRITE

/*----------*/
/*----------*/

/*__INCLUDES DEL SISTEMA______________________________________________________*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <time.h> 
#include <unistd.h>
#include <sys/time.h>   
#include <sys/types.h>
#include <sys/stat.h>

/*__INCLUDES DE LA BD_________________________________________________________*/

/*__INCLUDES DE LA APLICACION_________________________________________________*/

#include "trace.h"

#ifdef TRACE_OVERWRITE

#include "plat/tracelib.h"

#endif

/*__CONSTANTES________________________________________________________________*/
  
const char* TRACE_type[] =
{
  "APPL",
  "DDBB",
  "LIBS",
  "REFO",
  "COMM",
  "GNIF",
  "PDAS",
  "PFCS",
  "SIPL",
  "AMTA",
  "RADV",

  ""
};

/*__TIPOS_____________________________________________________________________*/

/*__VARIABLES EXTERNAS________________________________________________________*/

/*__VARIABLES GLOBALES________________________________________________________*/

/*__VARIABLES LOCALES_________________________________________________________*/

/*----------*/

#ifdef TRACE_OVERWRITE

extern pthread_mutex_t	g_tracelib_mutex[1];
extern int              g_tracelib_mutex_init;

#endif

/*----------*/

static TRACE_t		a_trace[1];
static int		a_trace_init = 0;

int			g_trace_mt = 0;
int			g_trace_mp = 0;

/*----------*/

/*__FUNCIONES EXTERNAS________________________________________________________*/

extern struct tm *localtime_r(const time_t *timer, struct tm *result);

/*__FUNCIONES PUBLICAS________________________________________________________*/

/*__FUNCIONES PRIVADAS________________________________________________________*/

static void TRACE_thread_id_free(void* inData);

static int TRACE_open(void);

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int
TRACE_init
(
  char*			inTraceBpath,
  char*			inTraceRpath,
  char*			inTraceFname,
  char*			inTraceFextn
)
{
  long			i;
  
  int			ret = TRACE_RC_OK;

/*----------*/

  if(a_trace_init == 0)
  {
    a_trace_init = 1;

    a_trace->fs = NULL;

    for(i = 0; i < TRACE_TYPE_TOTAL; i++) a_trace->level[i] = 0;

    snprintf(a_trace->path, TRACE_MAXLEN_PATH + 1, "%s/%s",
	     inTraceBpath,
	     inTraceRpath);

    strncpy(a_trace->name, inTraceFname, TRACE_MAXLEN_NAME);
    a_trace->name[TRACE_MAXLEN_NAME] = 0;

    strncpy(a_trace->extn, inTraceFextn, TRACE_MAXLEN_EXTN);
    a_trace->extn[TRACE_MAXLEN_EXTN] = 0;

    pthread_mutex_init(a_trace->mutex, NULL);

    pthread_key_create(&a_trace->ptkey, TRACE_thread_id_free);

    a_trace->thrid = 0; TRACE_thread_id_set();

#ifdef TRACE_OVERWRITE

    snprintf(a_trace->file, TRACE_MAXLEN_FILE, "%s/%s%s",
	     a_trace->path,
	     a_trace->name,
	     a_trace->extn);

    if(trace_init(0, a_trace->file, 0, 1, 1) != TRACE_OK)
    {
      TRACE_final(); ret = TRACE_RC_ERROR;
    }

    else
    {
      pthread_mutex_init(g_tracelib_mutex, NULL);

      g_tracelib_mutex_init = 1;
    }

#else

    if(TRACE_open() < 0)
    {
      fprintf(stderr, "ERROR: TRACE_open()\n");

      TRACE_final(); ret = TRACE_RC_ERROR;
    }

#endif

  }

/*----------*/

  return ret;
}

/*----------------------------------------------------------------------------*/

int 
TRACE_final(void)
{
  int			ret = TRACE_RC_OK;

/*----------*/

  if(a_trace_init == 1)
  {
    if(a_trace->fs != NULL)
    {
      fclose(a_trace->fs); a_trace->fs = NULL;
    }

    a_trace_init = 0;
  }

/*----------*/

  return ret;
}

/*----------------------------------------------------------------------------*/

int 
TRACE_thread_id_set(void)
{
  int		tid = 0;
  
  int*		ptid;
  
  if(a_trace_init == 1)
  {
    if((ptid = pthread_getspecific(a_trace->ptkey)) == NULL)
    {
      tid = a_trace->thrid++;
      
      ptid = malloc(sizeof(int)); *ptid = tid;

      pthread_setspecific(a_trace->ptkey, ptid);
    }
  }

  return tid;
}

/*----------------------------------------------------------------------------*/

static void 
TRACE_thread_id_free(void* inData)
{
  free(inData);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static int 
TRACE_open(void)
{
  time_t		t = time(NULL);

  struct tm 		tm;

  int			ret = TRACE_RC_OK;

/*----------*/

  if(a_trace->fs != NULL)
  {
    fclose(a_trace->fs); a_trace->fs = NULL;
  }
    
/*----------*/

  localtime_r(&t, &tm);
  
  strftime(a_trace->tms, TRACE_MAXLEN_TIMESTAMP + 1, "%Y%m%d", &tm);

  snprintf(a_trace->file, TRACE_MAXLEN_FILE, "%s/%s_%s%s",
           a_trace->path,
           a_trace->name, a_trace->tms,
           a_trace->extn);

  a_trace->fs = fopen(a_trace->file, "a");

  if(a_trace->fs == NULL)
  {
    fprintf(stderr, "ERROR: fopen(%s) = %d (%s)\n",
            a_trace->file,
	    errno, 
	    strerror(errno));

    ret = TRACE_RC_ERROR;
  }

/*----------*/

  return ret;
}

/*----------------------------------------------------------------------------*/

int 
TRACE_level_change(int inType, int inLevel)
{
  int			ret = TRACE_RC_OK;

  int 			type  = inType;
  int 			level = inLevel;

  if(type <= 0) type = 0; 

  if(type >= TRACE_TYPE_TOTAL) type = TRACE_TYPE_TOTAL - 1;

  if(level < 0) level = 0; 

  a_trace->level[type] = level;

/*----------*/
#ifdef TRACE_OVERWRITE
/*----------*/

  if(inType == TRACE_TYPE_DEFAULT)
  {
    trace_changeLevel(level);
  }

/*----------*/
#endif
/*----------*/

  return ret;
}

/*----------------------------------------------------------------------------*/

int 
TRACE_level_get(int inType)
{
  int type  = inType;

  if(type < 0) type = 0; 

  if(type > TRACE_TYPE_TOTAL) type = TRACE_TYPE_TOTAL - 1;
  
  return a_trace->level[type];
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int 
TRACE_write(int inType, int inLevel, const char* inTrace, ...)
{
  va_list		list;

  char			line[TRACE_MAXLEN_TRACE + 1];

  time_t		t;

  struct tm 		tm;

  char			tms[TRACE_MAXLEN_TIMESTAMP + 1];

  int			thrid;
  int*			pv;

  int			rc;

  int			ret = TRACE_RC_OK;

/*----------*/
#ifdef TRACE_OVERWRITE
/*----------*/

  int			level = 0;

  if(a_trace_init == 1)
  {
    if(inLevel <= a_trace->level[inType])
    {
//    if(pthread_mutex_lock(a_trace->mutex) == 0)
//    { 
	va_start(list, inTrace);

	vsnprintf(line, TRACE_MAXLEN_TRACE + 1, inTrace, list);

	va_end(list);

	level = trace_getLevel();

	if(inLevel <= level)
	{
	  trace_write(inLevel, "%s", line);
	}

	else
	{
	  trace_write(level, "%s", line);
	}

//      pthread_mutex_unlock(a_trace->mutex);
//    }
    }
  }

/*----------*/
#else
/*----------*/

  if(a_trace_init == 1)
  {
    if(inLevel <= a_trace->level[inType])
    {
      if(pthread_mutex_lock(a_trace->mutex) == 0)
      { 
	t = time(NULL); localtime_r(&t, &tm);
  
	strftime(tms, TRACE_MAXLEN_TIMESTAMP + 1, "%Y%m%d%H%M%S", &tm);

	if(memcmp(a_trace->tms, tms, 8) != 0)
	{
	  TRACE_open();
	}

	if(access(a_trace->file, F_OK) != 0)
	{
	  TRACE_open();
	}

	if(a_trace->fs != NULL)
	{
	  va_start(list, inTrace);
    
	  vsnprintf(line, TRACE_MAXLEN_TRACE + 1, inTrace, list);

	  va_end(list);

	  if(g_trace_mt)
	  {
	    pv = pthread_getspecific(a_trace->ptkey); 

	    thrid = pv == NULL ? 0 : *pv;
	    
	    rc = fprintf(a_trace->fs, "[%s][%02d][%d] %s\n", 
			 tms,
			 thrid,
			 inLevel, 
			 line);
	  }

	  else if(g_trace_mp)
	  {
	    rc = fprintf(a_trace->fs, "[%s][%05d][%d] %s\n", 
			 tms,
			 getpid(),
			 inLevel, 
			 line);
	  }

	  else
	  {
	    rc = fprintf(a_trace->fs, "[%s][%d] %s\n", 
			 tms,
			 inLevel, 
			 line);
	  }

	  if(rc < 0)
	  {
	    fprintf(stderr, "ERROR: fprintf(%s) = %d (%s)\n",
		    a_trace->file,
		    errno, 
		    strerror(errno)); 
	  }

	  if(fflush(a_trace->fs) < 0)
	  {
	    fprintf(stderr, "ERROR: fflush(%s) = %d (%s)\n",
		    a_trace->file,
		    errno, 
		    strerror(errno)); 
	  }
	}

	pthread_mutex_unlock(a_trace->mutex);
      }
    }
  }

/*----------*/
#endif
/*----------*/

  return ret;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

