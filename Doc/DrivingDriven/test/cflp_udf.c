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

#ifdef __cplusplus
extern "C" {
#endif

const CHR *VERSION_CFLPUDF___="Sancho M. CFLPUDF v.1.0.0.";

extern const ULO INSTRUCTIONS;

/* _________________________________________________________________________ */
/* Functions                                                       SECTION 0 */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include <fcntl.h>

/* .---------------------------------------------------------------------.
   | 0. DRIVING SENDS SHELL COMMANDS TO PIPE BMDFMpipeDriving2Driven<i>  |
   |      "echo args | fastlisp -q exec.flx"                             |
   |      "echo args | BMDFMldr -q exec.flz"                             |
   | 1. SHELL EXECUTES COMMANDS                                          |
   |      "echo args | fastlisp -q exec.flx >BMDFMpipeDriven2Driving<i>" |
   |      "echo args | BMDFMldr -q exec.flz >BMDFMpipeDriven2Driving<i>" |
   | 2. DRIVING RECEIVES RESULTS FROM DRIVEN                             |
   `---------------------------------------------------------------------' */
void bmdfm2bmdfm_offload(const ULO *dat_ptr, struct fastlisp_data *ret_dat){
  CHR *exec=NULL,*args=NULL;  /* "echo args | fastlisp -q exec.flx" */
                              /* "echo args | BMDFMldr -q exec.flz" */
  CHR *temp=NULL,*temp1=NULL;
  CHR *Driving2Driven_pipe_name=NULL,*Driven2Driving_pipe_name=NULL,
      *result=NULL;
  int descr,Driving2Driven_pipe_descr,Driven2Driving_pipe_descr;
  ret_dat->disable_ptr=1;
  ret_sval(dat_ptr,&exec);
  ret_sval(dat_ptr+1,&args);
  if(noterror()){
    cat(&exec,get_std_buff(&temp,am_I_in_the_fastlisp_module()?
      ".flx":".flz"));
    if(-1==(descr=open(exec,O_RDONLY))){
      get_std_buff(&temp,"*** Error while opening the `");
      cat(&temp,exec);
      cat(&temp,get_std_buff(&temp1,"' file!"));
      rise_error_info(1,temp);
    }
    else{
      close(descr);
      lcat(&exec,get_std_buff(&temp,am_I_in_the_fastlisp_module()?
        "fastlisp -q ":"BMDFMldr -q "));
      get_std_buff(&Driving2Driven_pipe_name,"BMDFMpipeDriving2Driven");
      cat(&Driving2Driven_pipe_name,equ_num(&temp,get_id_cpuproc()));
      get_std_buff(&Driven2Driving_pipe_name,"BMDFMpipeDriven2Driving");
      cat(&Driven2Driving_pipe_name,temp);
      if(-1==(Driving2Driven_pipe_descr=open(Driving2Driven_pipe_name,
         O_RDWR))){
        get_std_buff(&temp,"*** Error while opening the `");
        cat(&temp,Driving2Driven_pipe_name);
        cat(&temp,get_std_buff(&temp1,"' pipe!"));
        rise_error_info(1,temp);
      }
      else
        if(-1==(Driven2Driving_pipe_descr=open(Driven2Driving_pipe_name,
           O_RDWR))){
          close(Driving2Driven_pipe_descr);
          get_std_buff(&temp,"*** Error while opening the `");
          cat(&temp,Driven2Driving_pipe_name);
          cat(&temp,get_std_buff(&temp1,"' pipe!"));
          rise_error_info(1,temp);
        }
        else{
          get_std_buff(&temp,"echo ");
          cat(&temp,args);
          cat(&temp,get_std_buff(&temp1," | "));
          cat(&temp,exec);
          cat(&temp,sch2str(&temp1,'\n'));
          write(Driving2Driven_pipe_descr,(void*)temp,len(temp));
          space(&result,0);
          while(noterror()&&!len(result))
            while(noterror()&&(1==read(Driven2Driving_pipe_descr,(void*)temp1,
               1))){
              if(*temp1=='\n')
                break;
              cat(&result,temp1);
            }
          close(Driving2Driven_pipe_descr);
          close(Driven2Driving_pipe_descr);
        }
    }
    ret_dat->single=1;
    ret_dat->type='S';
    equ(&ret_dat->svalue,result);
    free_string(&exec);
    free_string(&args);
    free_string(&Driving2Driven_pipe_name);
    free_string(&Driven2Driving_pipe_name);
    free_string(&result);
    free_string(&temp);
    free_string(&temp1);
  }
  return;
}

SLO _dffib_FibonacciSeamless(SLO n){
  return noterror()&&n>1?_dffib_FibonacciSeamless(n-1)+
    _dffib_FibonacciSeamless(n-2):n;
}

void dffib_FibonacciSeamless(const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  SLO n;
  ret_ival(dat_ptr,&n);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='I';
    ret_dat->value.ival=_dffib_FibonacciSeamless(n);
  }
  return;
}

/* _________________________________________________________________________ */
/* FastLisp Callbacks                                              SECTION 1 */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void startup_callback(void){
  /* This is just a stub. Place your own code here. */
  return;
}

void taskjob_end_callback(ULO id_taskjob){
  /* This is just a stub. Place your own code here. */
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
  {"FIBONACCISEAMLESS",  1,'I',(UCH*)"I", &dffib_FibonacciSeamless},
  {"BMDFM2BMDFM_OFFLOAD",2,'S',(UCH*)"SS",&bmdfm2bmdfm_offload}
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

