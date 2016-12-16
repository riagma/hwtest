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
#include <uv.h>

#include <trace_macros.h>
#include <jsonif.h>

int main(void)
{
  uv_loop_t *loop = malloc(sizeof(uv_loop_t));

  TRACE_level_change(TRACE_TYPE_APPL, 3);
  JSON_initialize();

  uv_loop_init(loop);
  printf("uv_loop_init()\n");

  uv_run(loop, UV_RUN_DEFAULT);
  printf("uv_run()\n");

  uv_loop_close(loop);
  printf("uv_loop_close()\n");

  free(loop);

  return 0;
}
