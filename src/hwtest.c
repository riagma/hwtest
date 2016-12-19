/*
 ============================================================================
 Name        : hwtest.c
 Author      : riagma
 Version     :
 Copyright   : none
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <uv.h>

#include <trace_macros.h>
#include <jsonif.h>

//----------------

int main(void)
{
  BUFF_memo_t*		memo;
  BUFF_buff_t*		buff;

  JSON_object_t*	jsobj;

  char*				json;

//----------------

  uv_loop_t *loop = malloc(sizeof(uv_loop_t));

  uv_loop_init(loop);
  printf("uv_loop_init()\n");

  uv_run(loop, UV_RUN_DEFAULT);
  printf("uv_run()\n");

  uv_loop_close(loop);
  printf("uv_loop_close()\n");

  free(loop);

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

//----------------

  return 0;
}
