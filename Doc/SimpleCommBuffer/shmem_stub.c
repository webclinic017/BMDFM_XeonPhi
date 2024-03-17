/* shmem_stub.c - a stub for the ShMem Pool
                  Shared Memory Pool stub for fastlisp. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CHR char
#define UCH unsigned char
#define SCH signed char
#define USH unsigned short int
#define SSH signed short int
#define ULO unsigned long int
#define SLO signed long int
#define DFL double

#ifdef __cplusplus
extern "C" {
#endif

void shmempool_on(void){
  return;
}

void shmempool_off(void){
  return;
}

UCH is_shmempool_on(void){
  return 0;
}

void *reallocpool(void *ptr, ULO size){
  return realloc(ptr,size);
}

void freepool(void *ptr){
  free(ptr);
  return;
}

CHR *shmempoolLUT_add_key_value(CHR **value, const CHR *key){
  return NULL;
}

CHR *shmempoolLUT_del_key_value(CHR **key){
  return NULL;
}

CHR *shmempoolLUT_get_value(CHR **value, const CHR *key){
  return NULL;
}

#ifdef __cplusplus
} // extern "C"
#endif

