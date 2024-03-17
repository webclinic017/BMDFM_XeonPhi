/* simple_comm_buff.cpp - Simple Communication Buffer
                          Implementation of the simple communication buffer. */

#include "simple_comm_buff.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void shmempool_on(void);
extern void shmempool_off(void);
extern UCH is_shmempool_on(void);
extern void *reallocpool(void *ptr, ULO size);
extern void freepool(void *ptr);

simple_comm_buff::simple_comm_buff(){
  _simple_comm_buff=NULL;
  sem_init(&wr_sem,1,1);
  sem_init(&rd_sem,1,0);
}

simple_comm_buff::~simple_comm_buff(){
  UCH shmempool;
  shmempool=is_shmempool_on();
  shmempool_on();
  free_string(&_simple_comm_buff);
  sem_destroy(&wr_sem);
  sem_destroy(&rd_sem);
  if(!shmempool)
    shmempool_off();
}

void *simple_comm_buff::operator new(size_t size){
  UCH shmempool;
  shmempool=is_shmempool_on();
  shmempool_on();
  void *ptr=reallocpool(NULL,(ULO)size);
  if(!shmempool)
    shmempool_off();
  return ptr;
}

void *simple_comm_buff::operator new[](size_t size){
  UCH shmempool;
  shmempool=is_shmempool_on();
  shmempool_on();
  void *ptr=reallocpool(NULL,(ULO)size);
  if(!shmempool)
    shmempool_off();
  return ptr;
}

void simple_comm_buff::operator delete(void *ptr){
  UCH shmempool;
  shmempool=is_shmempool_on();
  shmempool_on();
  freepool(ptr);
  if(!shmempool)
    shmempool_off();
  return;
}

void simple_comm_buff::operator delete[](void *ptr){
  UCH shmempool;
  shmempool=is_shmempool_on();
  shmempool_on();
  freepool(ptr);
  if(!shmempool)
    shmempool_off();
  return;
}

CHR simple_comm_buff::write(const CHR *str_data){
  UCH not_error=0,shmempool;
  if(str_data!=NULL){
     shmempool=is_shmempool_on();
     shmempool_on();
     while((not_error=noterror())&&sem_wait(&wr_sem));
     if(not_error){
       equ(&_simple_comm_buff,str_data);
       while(sem_post(&rd_sem));
     }
     if(!shmempool)
       shmempool_off();
  }
  return not_error;
}

CHR *simple_comm_buff::read(CHR **str_data){
  UCH not_error;
  CHR *ret_val=NULL;
  if(str_data!=NULL){
     while((not_error=noterror())&&sem_wait(&rd_sem));
     if(not_error){
       equ(str_data,_simple_comm_buff);
       while(sem_post(&wr_sem));
       ret_val=*str_data;
     }
  }
  return ret_val;
}

#ifdef __cplusplus
} // extern "C"
#endif

