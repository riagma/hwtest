/*
 ============================================================================
 Name        : hwtest.c
 Author      : riagma
 Version     :
 Copyright   : none
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <uv.h>

#include <trace_macros.h>

//----------------

static void timer1sec_cb(uv_timer_t* inTimer)
{
  TRAZA1("Entering in timer1sec_cb(%p)", inTimer);

  TRAZA0("Returning from timer1sec_cb()");
}

//----------------

int main(void)
{
  BUFF_memo_t*		memo;
  BUFF_buff_t*		buff;

  JSON_object_t*	jsobj;

  char*			json;

//----------------

  uv_loop_t		loop[1];
  uv_timer_t		timer1sec[1];

//----------------

  TRACE_level_change(TRACE_TYPE_APPL, 3);

  BUFF_initialize();
  JSON_initialize();

  memo = BUFF_memo_new(1024, 128, 2);
  buff = BUFF_buff_new(memo);

  BUFF_part_new(buff);
  BUFF_part_new(buff);

  jsobj = JSON_object_new();

  json = (char*)(buff->head->part->data);

  strcpy(json, "{ \"cmd\" : { \"type\" : \"test\" , \"prm1\" : \"Hola JSON\", \"prm2\" : 0123456789 }      				   }");

  buff->head->part->len = strlen(json);

  buff->idxElm = buff->head;
  buff->idxOff = 0;
  buff->idxLen = buff->head->part->len;

  JSON_object_decode(jsobj, buff);

  JSON_memo_view();
  JSON_object_delete(jsobj);
  JSON_memo_view();

  BUFF_memo_view();
  BUFF_buff_delete(buff); BUFF_memo_delete(memo);
  BUFF_memo_view();

//----------------

  uv_loop_init(loop);
  printf("uv_loop_init()\n");

  uv_timer_init(loop, timer1sec);
  uv_timer_start(timer1sec, timer1sec_cb, 10, 1000);

  uv_run(loop, UV_RUN_DEFAULT);
  printf("uv_run()\n");

  uv_loop_close(loop);
  printf("uv_loop_close()\n");

//----------------

  return 0;
}
