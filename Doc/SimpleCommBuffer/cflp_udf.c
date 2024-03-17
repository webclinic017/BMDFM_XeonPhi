/* cflp_udf.c - FastLisp User Defined Functions written in C
                Sancho Mining 07-09-2000 20:51:42.51pm */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef _NOT_UNIX_
#include <unistd.h>
#endif
#include <string.h>
#include "cflp_udf.h"
#include "simple_comm_buff.h"

#ifdef __cplusplus
extern "C" {
#endif

const CHR *VERSION_CFLPUDF___="Sancho M. CFLPUDF v.1.0.0.";

extern const ULO INSTRUCTIONS;

/* _________________________________________________________________________ */
/* Functions                                                       SECTION 0 */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

class simple_comm_buff *Simple_Comm_Buff=NULL;

void simple_comm_buff_write(const ULO *dat_ptr, struct fastlisp_data *ret_dat){
  CHR *str_data=NULL;
  SLO sync;
  ret_sval(dat_ptr,&str_data);
  ret_ival(dat_ptr+1,&sync);
  if(noterror())
    if(Simple_Comm_Buff==NULL)
      rise_error_info(0,"No Shared Memory Pool is available!");
    else{
      ret_dat->single=1;
      ret_dat->type='I';
      ret_dat->value.ival=(SLO)Simple_Comm_Buff->write((const CHR*)str_data);
    }
  free_string(&str_data);
  return;
}

void simple_comm_buff_read(const ULO *dat_ptr, struct fastlisp_data *ret_dat){
  SLO sync;
  ret_dat->disable_ptr=1;
  ret_ival(dat_ptr,&sync);
  if(noterror())
    if(Simple_Comm_Buff==NULL)
      rise_error_info(0,"No Shared Memory Pool is available!");
    else{
      ret_dat->single=1;
      ret_dat->type='S';
      Simple_Comm_Buff->read(&ret_dat->svalue);
    }
  return;
}

/* _________________________________________________________________________ */
/* FastLisp Callbacks                                              SECTION 1 */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

extern void shmempool_on(void);
extern void shmempool_off(void);
extern UCH is_shmempool_on(void);
extern CHR *shmempoolLUT_add_key_value(CHR **value, const CHR *key);
extern CHR *shmempoolLUT_del_key_value(CHR **key);
extern CHR *shmempoolLUT_get_value(CHR **value, const CHR *key);

#define LUT_KEY_FOR_SIMPLE_COMM_BUFF "Simple Comm Buff"

void startup_callback(void){
  CHR *lut_key=NULL,*lut_value=NULL;
  UCH shmempool;
  if(am_I_in_the_BMDFMsrv_module()){
    Simple_Comm_Buff=new simple_comm_buff;
    shmempool=is_shmempool_on();
    shmempool_off();
    get_std_buff(&lut_key,LUT_KEY_FOR_SIMPLE_COMM_BUFF);
    equ_num(&lut_value,(SLO)Simple_Comm_Buff);
    shmempool_on();
    shmempoolLUT_add_key_value(&lut_value,lut_key);
    shmempool_off();
    free_string(&lut_key);
    free_string(&lut_value);
    if(shmempool)
      shmempool_on();
  }
  else
    if(am_I_in_the_CPUPROC_module()){
      shmempool=is_shmempool_on();
      shmempool_off();
      get_std_buff(&lut_key,LUT_KEY_FOR_SIMPLE_COMM_BUFF);
      shmempool_on();
      shmempoolLUT_get_value(&lut_value,lut_key);
      shmempool_off();
      Simple_Comm_Buff=(simple_comm_buff*)atol(lut_value);
      free_string(&lut_key);
      free_string(&lut_value);
      if(shmempool)
        shmempool_on();
    }
  return;
}

void taskjob_end_callback(ULO id_taskjob){
  CHR *lut_key=NULL;
  UCH shmempool;
  if(am_I_in_the_BMDFMsrv_module()){
    delete Simple_Comm_Buff;
    Simple_Comm_Buff=NULL;
    shmempool=is_shmempool_on();
    shmempool_off();
    get_std_buff(&lut_key,LUT_KEY_FOR_SIMPLE_COMM_BUFF);
    shmempool_on();
    shmempoolLUT_del_key_value(&lut_key);
    shmempool_off();
    free_string(&lut_key);
    if(shmempool)
      shmempool_on();
  }
  return;
}

/* The BMDFMldr module is capable of invoking/evaluating VM language
   expressions from C/C++ code (1-Capable;0-Unable).*/
UCH BMDFMldr_capable_call_VMcode_from_C=0;

void user_io_callback(SLO usr_id, CHR **usr_buff){
  /* This is just a stub. Place your own code here. */
  /* The following is a default behavior: */
  CHR *temp=NULL,*temp1=NULL,*temp2=NULL;
  equ(&temp,*usr_buff);
  if(cmp(temp,get_std_buff(&temp1,"PWD"))){
    mk_fst_buff(&temp1,4096);
    if(getcwd((char*)temp1,(size_t)len(temp1)))
      get_std_buff(usr_buff,temp1);
  }
  else
    if(cmp(head(&temp2,temp),get_std_buff(&temp1,"GetEnv"))){
      tail(&temp1,temp);
      get_std_buff(usr_buff,getenv(temp1));
    }
    else{
      lcat(usr_buff,get_std_buff(&temp,", usr_buff=\""));
      lcat(usr_buff,equ_num(&temp,usr_id));
      lcat(usr_buff,get_std_buff(&temp,"USER_IO: usr_id="));
      cat(usr_buff,get_std_buff(&temp,"\"."));
    }
  free_string(&temp);
  free_string(&temp1);
  free_string(&temp2);
  return;
}

/* _________________________________________________________________________ */
/* FastLisp Database Register                                      SECTION 2 */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

INSTRUCTION_STRU INSTRUCTION_SET[]={
  {"SIMPLE_COMM_BUFF_WRITE",2,'I',(UCH*)"SI",&simple_comm_buff_write},
  {"SIMPLE_COMM_BUFF_READ", 1,'S',(UCH*)"I", &simple_comm_buff_read}
};
const ULO INSTRUCTIONS=sizeof(INSTRUCTION_SET)/sizeof(INSTRUCTION_STRU);

/* _________________________________________________________________________ */
/* Invocation of Function Main                                     SECTION 3 */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

extern int _Main_(int argc, char *argv[]);

int main(int argc, char *argv[]){
  return _Main_(argc,argv);
}

#ifdef __cplusplus
} // extern "C"
#endif

