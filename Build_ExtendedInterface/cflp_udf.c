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

const CHR *VERSION_CFLPUDF_X_="Sancho M. CFLPUDF v.1.0.0.";

extern const ULO INSTRUCTIONS;

/* _________________________________________________________________________ */
/* Functions                                                       SECTION 0 */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include <fcntl.h>
#include <time.h>

#define ECODE_RT__DFTEST_OUT_OF_MEMORY 247
#define ECODE_RT__DFTEST_FILE_IO_FAIL  248

/* This option changes stateless file descriptors to stateful ones: */
#define DFTEST_USE_GLOBALS

#ifdef DFTEST_USE_GLOBALS
/* System may run multiple task job instances in parallel (max. N_IORBP jobs),
   thus, multiple copies of global entities might be needed.
   Additionally, system may run in multithreaded mode (N_CPUPROC threads),
   thus, multiple copies of global entities might be needed as well.
*/
/* Global entities that are sensitive to parallel task job instances
   only:
   - every global entity is an array of [0;get_n_taskjob()[,
     current entity is [get_id_taskjob()].
*/
  struct _dftest_globals_tsk{
    CHR *outfile_name;         /* only serial ordered WRITE operations */
    int outfile_descr;         /* are enabled in our dataflow program */
  } *dftest_globals_tsk_p=NULL;

/* Global entities that are sensitive to parallel task job instances
   and threads:
   - if !am_I_in_the_multithreaded_module() then
       every global entity is an array of [0;get_n_taskjob()[,
       current entity is [get_id_taskjob()];
   - if am_I_in_the_multithreaded_module() then
       every global entity is an array of [0;get_n_taskjob()*get_n_cpuproc()[,
       current entity is [get_id_taskjob()*get_n_cpuproc()+get_id_cpuproc()].
*/
  struct _dftest_globals_thr{
    CHR *inpfile_name;         /* parallel simultaneous READ operations */
    int inpfile_descr;         /* are enabled in our dataflow program */
  } *dftest_globals_thr_p=NULL;
#endif

void dftest_write_data_to_cluster(const void *rt_ctrl, const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  SLO DataClusterInMemory_ptr,SequenceId,RecordsPerDataCluster,RecordSize,i,j;
  CHR *heap_ptr;
  ret_ival(rt_ctrl,dat_ptr,  &DataClusterInMemory_ptr);  /* read arguments */
  ret_ival(rt_ctrl,dat_ptr+1,&SequenceId);               /* from the stack */
  ret_ival(rt_ctrl,dat_ptr+2,&RecordsPerDataCluster);
  ret_ival(rt_ctrl,dat_ptr+3,&RecordSize);
  if(noterror_fast(rt_ctrl)){
    if(0==DataClusterInMemory_ptr)
      rise_error_info_dbg(ECODE_RT__DFTEST_OUT_OF_MEMORY,
        "dftest_write_data_to_cluster(): memory allocation failure",dat_ptr);
    else{
      heap_ptr=(CHR*)DataClusterInMemory_ptr;

      j=sprintf(heap_ptr,"%ld",SequenceId);
      heap_ptr+=j;
      for(j++;j<RecordSize;j++)
        *heap_ptr++=' ';
      *heap_ptr++='\n';

      j=sprintf(heap_ptr,"%ld",RecordsPerDataCluster);
      heap_ptr+=j;
      for(j++;j<RecordSize;j++)
        *heap_ptr++=' ';
      *heap_ptr++='\n';

      for(i=0;i<RecordsPerDataCluster;i++){
        for(j=1;j<RecordSize;j++)
          *heap_ptr++=(CHR)((double)rand()/RAND_MAX*26)+'A';
        *heap_ptr++='\n';
      }
    }
    ret_dat->single=1;
    ret_dat->type='I';
    ret_dat->value.ival=DataClusterInMemory_ptr;  /* return value */
  }
  return;
}

void dftest_write_cluster_to_file(const void *rt_ctrl, const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  SLO DataClusterInMemory_ptr,DataClusterSize,OutputTestFileOrdering_sync;
  CHR *OutputTestFileName=NULL,**FileName_p=&OutputTestFileName;
  int outfile_descr=-1,*file_descr_p=&outfile_descr;
#ifdef DFTEST_USE_GLOBALS
  FileName_p=&dftest_globals_tsk_p[get_id_taskjob()].outfile_name;
  file_descr_p=&dftest_globals_tsk_p[get_id_taskjob()].outfile_descr;
#endif
  ret_ival(rt_ctrl,dat_ptr,  &DataClusterInMemory_ptr);  /* read arguments */
  ret_ival(rt_ctrl,dat_ptr+1,&DataClusterSize);          /* from the stack */
  ret_sval(rt_ctrl,dat_ptr+2,&OutputTestFileName);
  ret_ival(rt_ctrl,dat_ptr+3,&OutputTestFileOrdering_sync);
  if(noterror_fast(rt_ctrl)){
    if(0==DataClusterInMemory_ptr)
      rise_error_info_dbg(ECODE_RT__DFTEST_OUT_OF_MEMORY,
        "dftest_write_cluster_to_file(): memory allocation failure",dat_ptr);
    else
      if(((-1==*file_descr_p||!cmp(OutputTestFileName,*FileName_p)))&&
         (-1==(*file_descr_p=open(equ(FileName_p,OutputTestFileName),
         O_WRONLY|O_APPEND))))
        rise_error_info_dbg(ECODE_RT__DFTEST_FILE_IO_FAIL,
          "dftest_write_cluster_to_file():"
          " file I/O failure while opening output file",dat_ptr);
      else{
        lseek(*file_descr_p,0,SEEK_END);
        if(DataClusterSize!=write(*file_descr_p,(void*)DataClusterInMemory_ptr,
           DataClusterSize))
          rise_error_info_dbg(ECODE_RT__DFTEST_FILE_IO_FAIL,
            "dftest_write_cluster_to_file():"
            " file I/O failure while writing to output file",dat_ptr);
#ifndef DFTEST_USE_GLOBALS
        close(*file_descr_p);
#endif
      }
    ret_dat->single=1;
    ret_dat->type='I';
    ret_dat->value.ival=DataClusterInMemory_ptr;  /* return value */
  }
  free_string(&OutputTestFileName);
  return;
}

void dftest_get_next_cluster_size(const void *rt_ctrl, const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  SLO InputFilePosition,RecordSize,i,DataClusterSize=0;
  CHR *InputTestFileName=NULL,**FileName_p=&InputTestFileName,buff[21];
  int inpfile_descr=-1,*file_descr_p=&inpfile_descr;
#ifdef DFTEST_USE_GLOBALS
  ULO globidx;
  globidx=am_I_in_the_multithreaded_module()?get_id_taskjob()*get_n_cpuproc()+
    get_id_cpuproc():get_id_taskjob();
  FileName_p=&dftest_globals_thr_p[globidx].inpfile_name;
  file_descr_p=&dftest_globals_thr_p[globidx].inpfile_descr;
#endif
  ret_sval(rt_ctrl,dat_ptr,  &InputTestFileName);  /* read arguments */
  ret_ival(rt_ctrl,dat_ptr+1,&InputFilePosition);  /* from the stack */
  ret_ival(rt_ctrl,dat_ptr+2,&RecordSize);
  if(noterror_fast(rt_ctrl)){
    if(((-1==*file_descr_p||!cmp(InputTestFileName,*FileName_p)))&&
       (-1==(*file_descr_p=open(equ(FileName_p,InputTestFileName),O_RDONLY))))
      rise_error_info_dbg(ECODE_RT__DFTEST_FILE_IO_FAIL,
        "dftest_get_next_cluster_size():"
        " file I/O failure while opening input file",dat_ptr);
    else{
      if(lseek(*file_descr_p,0,SEEK_END)>InputFilePosition)
        if(-1==lseek(*file_descr_p,InputFilePosition+RecordSize,SEEK_SET))
          rise_error_info_dbg(ECODE_RT__DFTEST_FILE_IO_FAIL,
            "dftest_get_next_cluster_size():"
            " file I/O failure while seeking in input file",dat_ptr);
        else{
          for(i=0;i<(SLO)sizeof(buff);i++)
            buff[i]=0;
          if(sizeof(buff)!=read(*file_descr_p,(void*)buff,sizeof(buff)))
            rise_error_info_dbg(ECODE_RT__DFTEST_FILE_IO_FAIL,
              "dftest_get_next_cluster_size():"
              " file I/O failure while reading from input file",dat_ptr);
          else
            DataClusterSize=RecordSize*(atol(buff)+2);
        }
#ifndef DFTEST_USE_GLOBALS
      close(*file_descr_p);
#endif
    }
    ret_dat->single=1;
    ret_dat->type='I';
    ret_dat->value.ival=DataClusterSize;  /* return value */
  }
  free_string(&InputTestFileName);
  return;
}

void dftest_read_data_to_cluster(const void *rt_ctrl, const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  SLO DataClusterInMemory_ptr,InputFilePosition,DataClusterSize;
  CHR *InputTestFileName=NULL,**FileName_p=&InputTestFileName;
  int inpfile_descr=-1,*file_descr_p=&inpfile_descr;
#ifdef DFTEST_USE_GLOBALS
  ULO globidx;
  globidx=am_I_in_the_multithreaded_module()?get_id_taskjob()*get_n_cpuproc()+
    get_id_cpuproc():get_id_taskjob();
  FileName_p=&dftest_globals_thr_p[globidx].inpfile_name;
  file_descr_p=&dftest_globals_thr_p[globidx].inpfile_descr;
#endif
  ret_ival(rt_ctrl,dat_ptr,  &DataClusterInMemory_ptr);  /* read arguments */
  ret_sval(rt_ctrl,dat_ptr+1,&InputTestFileName);        /* from the stack */
  ret_ival(rt_ctrl,dat_ptr+2,&InputFilePosition);
  ret_ival(rt_ctrl,dat_ptr+3,&DataClusterSize);
  if(noterror_fast(rt_ctrl)){
    if(0==DataClusterInMemory_ptr)
      rise_error_info_dbg(ECODE_RT__DFTEST_OUT_OF_MEMORY,
        "dftest_read_data_to_cluster(): memory allocation failure",dat_ptr);
    else
      if(((-1==*file_descr_p||!cmp(InputTestFileName,*FileName_p)))&&
         (-1==(*file_descr_p=open(equ(FileName_p,InputTestFileName),
         O_RDONLY))))
        rise_error_info_dbg(ECODE_RT__DFTEST_FILE_IO_FAIL,
          "dftest_read_data_to_cluster():"
          " file I/O failure while opening input file",dat_ptr);
      else{
        if(-1==lseek(*file_descr_p,InputFilePosition,SEEK_SET))
          rise_error_info_dbg(ECODE_RT__DFTEST_FILE_IO_FAIL,
            "dftest_read_data_to_cluster():"
            " file I/O failure while seeking in input file",dat_ptr);
        else
          if(DataClusterSize!=read(*file_descr_p,
             (void*)DataClusterInMemory_ptr,DataClusterSize))
            rise_error_info_dbg(ECODE_RT__DFTEST_FILE_IO_FAIL,
              "dftest_read_data_to_cluster():"
              " file I/O failure while reading from input file",dat_ptr);
#ifndef DFTEST_USE_GLOBALS
        close(*file_descr_p);
#endif
      }
    ret_dat->single=1;
    ret_dat->type='I';
    ret_dat->value.ival=DataClusterInMemory_ptr;  /* return value */
  }
  free_string(&InputTestFileName);
  return;
}

void dftest_process_data_cluster(const void *rt_ctrl, const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  SLO DataClusterInMemory_ptr,RecordSize,DataClusterSize,i;
  ret_ival(rt_ctrl,dat_ptr,  &DataClusterInMemory_ptr);  /* read arguments */
  ret_ival(rt_ctrl,dat_ptr+1,&RecordSize);               /* from the stack */
  ret_ival(rt_ctrl,dat_ptr+2,&DataClusterSize);
  if(noterror_fast(rt_ctrl)){
    if(0==DataClusterInMemory_ptr)
      rise_error_info_dbg(ECODE_RT__DFTEST_OUT_OF_MEMORY,
        "dftest_process_data_cluster(): memory allocation failure",dat_ptr);
    else{
      /* Place your data cluster processing code here (e.g. sorting of the
         data cluster records).
         This is just a stub:
           - time complexity linearly depends on DataClusterSize;
           - data cluster remains unchanged in order to allow one to compare
             input and output test files. */
      for(i=0;i<DataClusterSize*6;i++)
        time(NULL);  /* dummy operation */
    }
    ret_dat->single=1;
    ret_dat->type='I';
    ret_dat->value.ival=DataClusterInMemory_ptr;  /* return value */
  }
  return;
}

SLO _dffib_FibonacciSeamless(const void *rt_ctrl, SLO n){
  return noterror_fast(rt_ctrl)&&n>1?_dffib_FibonacciSeamless(rt_ctrl,n-1)+
    _dffib_FibonacciSeamless(rt_ctrl,n-2):n;
}

void dffib_FibonacciSeamless(const void *rt_ctrl, const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  SLO n;
  ret_ival(rt_ctrl,dat_ptr,&n);
  if(noterror_fast(rt_ctrl)){
    ret_dat->single=1;
    ret_dat->type='I';
    ret_dat->value.ival=_dffib_FibonacciSeamless(rt_ctrl,n);
  }
  return;
}

/* _________________________________________________________________________ */
/* FastLisp Callbacks                                              SECTION 1 */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void startup_callback(void){
#ifdef DFTEST_USE_GLOBALS
  /* Here we initialize our global entities once the system starts. */
  ULO i,globcnt;
  if(!dftest_globals_thr_p){
    globcnt=get_n_taskjob()*(am_I_in_the_multithreaded_module()?
      get_n_cpuproc():1);
    if((dftest_globals_thr_p=(struct _dftest_globals_thr*)malloc(globcnt*
       sizeof(struct _dftest_globals_thr)))==NULL){
      fprintf(stderr,
        "\nstartup_callback() for dftest: memory allocation failure\n");
      exit(1);
    }
    for(i=0;i<globcnt;i++){
      dftest_globals_thr_p[i].inpfile_name=NULL;
      dftest_globals_thr_p[i].inpfile_descr=-1;
    }
  }
  if(!dftest_globals_tsk_p){
    if((dftest_globals_tsk_p=(struct _dftest_globals_tsk*)malloc(
       get_n_taskjob()*sizeof(struct _dftest_globals_tsk)))==NULL){
      fprintf(stderr,
        "\nstartup_callback() for dftest: memory allocation failure\n");
      exit(1);
    }
    for(i=0;i<get_n_taskjob();i++){
      dftest_globals_tsk_p[i].outfile_name=NULL;
      dftest_globals_tsk_p[i].outfile_descr=-1;
    }
  }
#endif
  return;
}

void taskjob_end_callback(ULO id_taskjob){
#ifdef DFTEST_USE_GLOBALS
  /* Here we deinitialize our global entities each time a task job ends. */
  ULO i,globcnt,globidx;
  globcnt=am_I_in_the_multithreaded_module()?get_n_cpuproc():1;
  for(i=0;i<globcnt;i++){
    globidx=id_taskjob*globcnt+i;
    free_string(&dftest_globals_thr_p[globidx].inpfile_name);
    if(-1!=dftest_globals_thr_p[globidx].inpfile_descr){
      close(dftest_globals_thr_p[globidx].inpfile_descr);
      dftest_globals_thr_p[globidx].inpfile_descr=-1;
    }
  }
  free_string(&dftest_globals_tsk_p[id_taskjob].outfile_name);
  if(-1!=dftest_globals_tsk_p[id_taskjob].outfile_descr){
    close(dftest_globals_tsk_p[id_taskjob].outfile_descr);
    dftest_globals_tsk_p[id_taskjob].outfile_descr=-1;
  }
#endif
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
  {"WRITE_DATA_TO_CLUSTER",4,'I',(UCH*)"IIII",&dftest_write_data_to_cluster},
  {"WRITE_CLUSTER_TO_FILE",4,'I',(UCH*)"IISI",&dftest_write_cluster_to_file},
  {"GET_NEXT_CLUSTER_SIZE",3,'I',(UCH*)"SII", &dftest_get_next_cluster_size},
  {"READ_DATA_TO_CLUSTER", 4,'I',(UCH*)"ISII",&dftest_read_data_to_cluster},
  {"PROCESS_DATA_CLUSTER", 3,'I',(UCH*)"III", &dftest_process_data_cluster},
  {"FIBONACCISEAMLESS",1,'I',(UCH*)"I",&dffib_FibonacciSeamless}
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

