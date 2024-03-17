/* cflp_udf.h - FastLisp User Defined Functions written in C
                Sancho Mining 07-09-2000 20:51:42.51pm */

#ifndef _CFLP_UDF_H
#define _CFLP_UDF_H

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

/* _________________________________________________________________________ */
/* Section: String Functions                                                 */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* String format:

  <ULO StrSize><Str><ZeroChar><ZeroChar_Justification_To_ULO_Size>
  CHR *StrPtr -'

*/

CHR *mk_std_buff(CHR **buff, ULO size);
CHR *mk_std_buff_secure(CHR **buff, ULO size);
CHR *mk_fst_buff(CHR **buff, ULO size);
CHR *mk_fst_buff_secure(CHR **buff, ULO size);
CHR *get_std_buff(CHR **targ, const CHR *buff);
CHR *get_std_buff_secure(CHR **targ, const CHR *buff);
UCH notempty(const CHR *string);
ULO len(const CHR *string);
ULO at(const CHR *pattern, const CHR *among);
ULO rat(const CHR *pattern, const CHR *among);
UCH cmp(const CHR *string1, const CHR *string2);
SCH cmp_s(const CHR *string1, const CHR *string2);
CHR *equ(CHR **targ, const CHR *source);
CHR *equ_secure(CHR **targ, const CHR *source);
CHR *equ_num(CHR **targ, SLO num);
CHR *equ_unum(CHR **targ, ULO num);
CHR *equ_numhex(CHR **targ, ULO num);
CHR *equ_fnum(CHR **targ, DFL fnum);
CHR *cat(CHR **targ, const CHR *source);
CHR *lcat(CHR **targ, const CHR *source);
CHR *space(CHR **targ, ULO pos);
CHR *replicate(CHR **targ, const CHR *source, ULO num);
CHR *left(CHR **targ, const CHR *source, ULO pos);
CHR *leftr(CHR **targ, const CHR *source, ULO posr);
CHR *right(CHR **targ, const CHR *source, ULO pos);
CHR *rightl(CHR **targ, const CHR *source, ULO posl);
CHR *substr(CHR **targ, const CHR *source, ULO from, ULO pos);
CHR *strtran(CHR **targ, const CHR *source, const CHR *pattern,
  const CHR *subst);
CHR *ltrim(CHR **targ, const CHR *source);
CHR *rtrim(CHR **targ, const CHR *source);
CHR *alltrim(CHR **targ, const CHR *source);
CHR *pack(CHR **targ, const CHR *source);
CHR *head(CHR **targ, const CHR *source);
CHR *tail(CHR **targ, const CHR *source);
CHR *lsp_head(CHR **targ, const CHR *source);
CHR *lsp_tail(CHR **targ, const CHR *source);
CHR *upper(CHR **targ, const CHR *source);
CHR *lower(CHR **targ, const CHR *source);
CHR *upper_utf8(CHR **targ, const CHR *source);
CHR *lower_utf8(CHR **targ, const CHR *source);
UCH is_utf8char(const CHR *utf8char);
CHR *ltrim_utf8(CHR **targ, const CHR *source);
CHR *rtrim_utf8(CHR **targ, const CHR *source);
CHR *alltrim_utf8(CHR **targ, const CHR *source);
CHR *rev(CHR **targ, const CHR *source);
CHR *padl(CHR **targ, const CHR *source, ULO width);
CHR *padr(CHR **targ, const CHR *source, ULO width);
CHR *padc(CHR **targ, const CHR *source, ULO width);
CHR *strraw(CHR **targ, const CHR *source);
CHR *strunraw(CHR **targ, const CHR *source);
CHR *strdump(CHR **targ, const CHR *source);
CHR *string_time(CHR **targ);
CHR *strings_version(CHR **targ);
CHR *sch2str(CHR **targ, SCH num);
CHR *ssh2str(CHR **targ, SSH num);
CHR *slo2str(CHR **targ, SLO num);
CHR *ptr2str(CHR **targ, void *ptr);
CHR *dfl2str(CHR **targ, DFL num);
SCH str2sch(const CHR *string);
SSH str2ssh(const CHR *string);
SLO str2slo(const CHR *string);
void *str2ptr(const CHR *string);
DFL str2dfl(const CHR *string);
ULO crcsum(const CHR *string);
CHR *free_string(CHR **targ);

/* Sample code:

  CHR *str0=NULL,*str1=NULL,*str2=NULL;
  get_std_buff(&str0,"To be or not to be");
  get_std_buff(&str1,"be");
  get_std_buff(&str2,"compute");
  upper(&str0,strtran(&str0,str0,str1,str2));
  printf("`%s'\n",str0);
  free_string(&str0);
  free_string(&str1);
  free_string(&str2);

Result:

  `TO COMPUTE OR NOT TO COMPUTE'

*/


/* _________________________________________________________________________ */
/* Section: Data/Function Types, Helpers                                     */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* universal data structure */
struct fastlisp_data{
  UCH disable_ptr;
  UCH single;
  UCH type;          /* 0, 'I', 'F', 'S', 'Z' */
  UCH arraytype;     /* 0, 'I', 'F', 'S', 'Z' */
  union{
    SLO ival;
    DFL fval;
  } value;
  CHR *svalue;
  ULO indices_numb;
  UCH *aready_tags;  /* 0, 'I', 'F', 'S', 'Z' for arraytype!=0 */
  union{
    struct fastlisp_data *mix;
    SLO *ival;
    DFL *fval;
    CHR **svalue;
  } array;
};


/* user C function type declaration */
typedef void (*fcall)(const ULO*, struct fastlisp_data*);

/* get integer or pointer value */
void ret_ival(const ULO *dat_ptr, SLO *targ);

/* get float value */
void ret_fval(const ULO *dat_ptr, DFL *targ);

/* get string value */
void ret_sval(const ULO *dat_ptr, CHR **targ);


/* copy the universal data structure */
void copy_flp_data(struct fastlisp_data *dest,
  const struct fastlisp_data *source, ULO indices_numb);

/* delete the universal data structure */
void free_flp_data(struct fastlisp_data *ret_dat);


/* _________________________________________________________________________ */
/* Section: Processing Units and Task Jobs                                   */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* get the number of configured parallel processing units
   ('N_CPUPROC' configuration parameter) */
ULO get_n_cpuproc(void);

/* get ID of the current parallel processing unit
   (within the range of [0; 'N_CPUPROC'[) */
ULO get_id_cpuproc(void);

/* get the number of maximal parallel task jobs
   ('N_IORBP' configuration parameter) */
ULO get_n_taskjob(void);

/* get ID of the current task job
   (within the range of [0; 'N_IORBP'[) */
ULO get_id_taskjob(void);


/* _________________________________________________________________________ */
/* Section: Modules                                                          */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

UCH am_I_in_the_fastlisp_module(void);

UCH am_I_in_the_BMDFMldr_module(void);

UCH am_I_in_the_BMDFMsrv_module(void);

UCH am_I_in_the_CPUPROC_module(void);

UCH am_I_in_the_multithreaded_module(void);


/* _________________________________________________________________________ */
/* Section: Callbacks                                                        */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* called at startup */
void startup_callback(void);

/* called at task job end */
void taskjob_end_callback(ULO id_taskjob);

/* (user_io <IVal> <SVal>) callee */
void user_io_callback(SLO usr_id, CHR **usr_buff);


/* _________________________________________________________________________ */
/* Section: User Threads                                                     */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* helper functions to be called from each created user thread
   (if this thread uses BMDFM functionality) */
void setup_bmdfm_thread_safe_functionality_for_local_thread(void);
void cleanup_bmdfm_thread_safe_functionality_for_local_thread(void);


/* _________________________________________________________________________ */
/* Section: Runtime Error Handling                                           */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* check whether no runtime error occurred */
UCH noterror(void);

/* set runtime error */
CHR *set_error(CHR **targ_errtext);

/* rise runtime error */
void rise_error(void);

/* set runtime error with dbg info */
CHR *set_error_dbg(CHR **targ_errtext, const ULO *dat_ptr);

/* rise runtime error with dbg info */
void rise_error_dbg(const ULO *dat_ptr);

/* set runtime error with info */
CHR *set_error_info(UCH errcode, CHR **targ_errtext, const CHR *src_errtext);

/* rise runtime error with info */
void rise_error_info(UCH errcode, const CHR *src_errtext);

/* set runtime error with info and with dbg info */
CHR *set_error_info_dbg(UCH errcode, CHR **targ_errtext,
  const CHR *src_errtext, const ULO *dat_ptr);

/* rise runtime error with info and with dbg info */
void rise_error_info_dbg(UCH errcode, const CHR *src_errtext,
  const ULO *dat_ptr);

/* get runtime error code */
UCH get_error_code(void);

/* get runtime error text */
CHR *get_error_text(CHR **targ_errtext);

/* Reserved runtime error codes:
  ECODE_RT__INT_DIVZERO        1
  ECODE_RT__INT_MODZERO        2
  ECODE_RT__FLOAT_DIVZERO      3
  ECODE_RT__FLOAT_POWER        4
  ECODE_RT__FLOAT_LOGARITHM    5
  ECODE_RT__FLOAT_SQRT         6
  ECODE_RT__AHEAP_WRONGDESCR   7
  ECODE_RT__AHEAP_OUTOFRANGE   8
  ECODE_RT__WRONG_FMT_STRING   9
  ECODE_RT__VAR_NOTINIT       10
  ECODE_RT__ARR_NOTINIT       11
  ECODE_RT__ARR_MEMBSNOTINIT  12
  ECODE_RT__ARR_NEGINDEX      13
  ECODE_RT__ARR_ZEROMEMB      14
  ECODE_RT__ARR_WRONGINDICES  15
  ECODE_RT__ARR_TYPEMISMATCH  16
  ECODE_RT__RESERVED6        249
  ECODE_RT__RESERVED5        250
  ECODE_RT__RESERVED4        251
  ECODE_RT__RESERVED3        252
  ECODE_RT__RESERVED2        253
  ECODE_RT__RESERVED1        254
  ECODE_RT__RESERVED0        255
*/


/* _________________________________________________________________________ */
/* Section: Instruction Database                                             */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* instruction database structure */
typedef struct{
  const CHR *fnc_name;
  const SSH operands;
  const UCH ret_type;    /* I=integer, F=float, S=string, Z=nil */
  const UCH *op_type;
  const fcall func_ptr;
} INSTRUCTION_STRU;

/*****************************************************************************/


/*
Example of a User Defined Function:

  void my_function(const ULO *dat_ptr, struct fastlisp_data *ret_dat){
    const ULO *tmp_ptr;
    SLO n,result=0;
    CHR *str=NULL;
    DFL *f_array,koef;
    struct fastlisp_data dat={0,1,0,0,{0},NULL,1,NULL,{NULL}};

    ret_ival(dat_ptr,&n);                // arg0: integer (n)
    ret_ival(dat_ptr+1,(SLO*)&f_array);  // arg1: float ptr (f_array)
    ret_fval(dat_ptr+2,&koef);           // arg2: float (koef)
    ret_sval(dat_ptr+3,&str);            // arg3: string (str)
    tmp_ptr=*((ULO**)(dat_ptr+4));       // arg4: any data as
    (*(fcall)*tmp_ptr)(tmp_ptr+1,&dat);  //       universal data (dat)

    if(noterror()){
      // data processing to compute `result':
      ret_dat->single=1;
      ret_dat->type='I';
      ret_dat->value.ival=result;

      // or an error occurred:
      rise_error_info_dbg(USER_DEFINED_UNRESERVED_ERRCODE,"ERROR_TEXT",
        dat_ptr);
    }

    if(dat.disable_ptr)
      free_flp_data(&dat);
    free_string(&str);

    return;
  }

  // Users may implement their own ret_val() like:
  //
  //   void ret_val(const ULO *dat_ptr, struct fastlisp_data *ret_dat){
  //     dat_ptr=*((ULO**)dat_ptr);
  //     (*(fcall)*dat_ptr)(dat_ptr+1,ret_dat);
  //     return;
  //   }
  //
  // or like:
  //
  //   void ret_val(const ULO *dat_ptr, struct fastlisp_data *ret_dat){
  //     if(noterror()){
  //       dat_ptr=*((ULO**)dat_ptr);
  //       (*(fcall)*dat_ptr)(dat_ptr+1,ret_dat);
  //     }
  //     return;
  //   }
  //
  // in order to simplify reading universal data args, e.g.:
  //
  //   ret_val(dat_ptr+4,&dat);

  INSTRUCTION_STRU INSTRUCTION_SET[]={
    // ...
    {"MY_FUNCTION",5,'I',(UCH*)"IIFSI",&my_function},
    // ...
  };
  const ULO INSTRUCTIONS=sizeof(INSTRUCTION_SET)/sizeof(INSTRUCTION_STRU);
*/


/* ####################
   ##                ##
   ##  UNDOCUMENTED  ##
   ##                ##
   ####################


.---------------------------------------------------.
|                                                   |
|  Evaluation of a fastlisp expression from C code  |
|                                                   |
`---------------------------------------------------'

extern void func__mapcar(const ULO*, struct fastlisp_data*);
extern void func__dummy_s(const ULO*, struct fastlisp_data*);

#ifndef POSIXMTHREAD_NOT_SUPPORTED
__thread
#endif
struct{
  CHR *flp_expr;
  CHR *bytecode;
} flpeval_cache={NULL,NULL};
CHR flp_eval(CHR *flp_expr, struct fastlisp_data *ret_dat){
  CHR success=0,*flp_fnc=NULL,*temp=NULL;
  struct fastlisp_data res={1,1,0,0,{0},NULL,1,NULL,{NULL}};
  get_std_buff(&flp_fnc,flp_expr);
  if(notempty(flp_fnc)&&cmp(flp_fnc,flpeval_cache.flp_expr))
    equ(&flp_fnc,flpeval_cache.bytecode);
  lcat(&flp_fnc,slo2str(&temp,len(flp_fnc)));
  lcat(&flp_fnc,ptr2str(&temp,(void*)&func__dummy_s));
  lcat(&flp_fnc,temp);
  *((CHR**)flp_fnc)=flp_fnc+sizeof(ULO);
  func__mapcar((ULO*)flp_fnc,&res);
  if((res.array.mix+2)->value.ival||(res.array.mix+4)->value.ival)
    copy_flp_data(ret_dat,&res,0);
  else{
    copy_flp_data(ret_dat,res.array.mix+1,0);
    get_std_buff(&flpeval_cache.flp_expr,flp_expr);
    equ(&flpeval_cache.bytecode,(res.array.mix+8)->svalue);
    success=1;
  }
  free_flp_data(&res);
  free_string(&flp_fnc);
  free_string(&temp);
  return success;
}

This can be useful (for example) when a C code allocates a heap in the
Shared Memory Pool, hiding all implementation/synchronization details:

  SLO descr;
  struct fastlisp_data res={1,1,0,0,{0},NULL,1,NULL,{NULL}};
  if(flp_eval("(asyncheap_create 1024)",&res))
    descr=res.value.ival;  // can be a return value for the functions-consumers
                           // processed even on different CPUs
  free_flp_data(&res);


.-------------------------------------------.
|                                           |
|  Allocate/Free shared memory from C code  |
|                                           |
`-------------------------------------------'

#ifdef _TO_BE_LINKED_AGAINST_CPUPROC_
  #define FLP_MALLOC par_func__asyncheap_create_j
  #define FLP_FREE par_func__asyncheap_delete_j
#else
  #define FLP_MALLOC func__asyncheap_create_j
  #define FLP_FREE func__asyncheap_delete_j
#endif

extern void FLP_MALLOC(const ULO*, struct fastlisp_data*);
extern void FLP_FREE(const ULO*, struct fastlisp_data*);
extern void func__dummy_i(const ULO*, struct fastlisp_data*);

void *flp_malloc(SLO bytes){
  CHR *flp_fnc=NULL,*temp=NULL;
  SLO addr;
  struct fastlisp_data res={1,1,0,0,{0},NULL,1,NULL,{NULL}};
  slo2str(&flp_fnc,bytes);
  lcat(&flp_fnc,ptr2str(&temp,(void*)&func__dummy_i));
  lcat(&flp_fnc,temp);
  *((CHR**)flp_fnc)=flp_fnc+sizeof(ULO);
  FLP_MALLOC((ULO*)flp_fnc,&res);
  addr=res.value.ival;
  free_flp_data(&res);
  free_string(&flp_fnc);
  free_string(&temp);
  return (void*)addr;
}

void flp_free(SLO addr){
  CHR *flp_fnc=NULL,*temp=NULL;
  struct fastlisp_data res={1,1,0,0,{0},NULL,1,NULL,{NULL}};
  slo2str(&flp_fnc,addr);
  lcat(&flp_fnc,ptr2str(&temp,(void*)&func__dummy_i));
  lcat(&flp_fnc,temp);
  *((CHR**)flp_fnc)=flp_fnc+sizeof(ULO);
  FLP_FREE((ULO*)flp_fnc,&res);
  free_flp_data(&res);
  free_string(&flp_fnc);
  free_string(&temp);
  return;
}


.------------------------------------------------------------------------.
|                                                                        |
|  Allocate/Free shared memory directly (e.g. from an external process)  |
|                                                                        |
`------------------------------------------------------------------------'

// Add the following code to your cflp_udf.c
// Link against one of BMDFMldr.o, BMDFMsrv.o, CPUPROC.o like e.g.:
//                     gcc -o MyProg cflp_udf.c CPUPROC.o -lpthread -lm

extern UCH attach_mempool(int sharedID, ULO mntaddr);
extern void shmempool_on(void);
extern void shmempool_off(void);
extern UCH is_shmempool_on(void);
extern void *reallocpool(void *ptr, ULO size);
extern ULO getalignedsizepool(void *ptr);
extern void addmcastrefpool(void *ptr);
extern void freepool(void *ptr);
extern UCH detach_mempool(void);
extern CHR *shmempoolLUT_add_key_value(CHR **value, const CHR *key);
extern CHR *shmempoolLUT_del_key_value(CHR **key);
extern CHR *shmempoolLUT_get_value(CHR **value, const CHR *key);
extern CHR *shmempoolLUT_get_entire_contents(CHR **contents);
extern void shmempoolLUT_purge_entire_contents(void);

int main(int argc, char *argv[]){
  CHR *BM_DFM_CONNECTION_FILE_path=NULL,*info=NULL,*temp=NULL,*temp1=NULL,
      *lut_key=NULL,*lut_value=NULL;
  ULO mntaddr;
  int f_descr,sharedID;
  struct _entry_point_struct{
    CHR *str0;
    CHR *str1;
    CHR *strN;
  } entry_point_struct={NULL,NULL,NULL},*entry_point_struct_ptr;

  if((BM_DFM_CONNECTION_FILE_path=getenv("BM_DFM_CONNECTION_FILE_path"))==NULL)
    BM_DFM_CONNECTION_FILE_path=(CHR*)"/tmp/.BMDFMsrv";

  if((f_descr=open(BM_DFM_CONNECTION_FILE_path,0))==-1){
    fprintf(stderr,"Fail opening file `%s'.\n",BM_DFM_CONNECTION_FILE_path);
    exit(1);
  }
  mk_fst_buff(&info,1024);
  read(f_descr,(void*)info,1024);
  close(f_descr);
  if(!cmp(left(&temp,info,9),get_std_buff(&temp1,"BMDFMsrv "))){
    fprintf(stderr,"`%s' is not the BM_DFM connection file.\n",
      BM_DFM_CONNECTION_FILE_path);
    exit(1);
  }
  tail(&info,info);
  head(&temp,info);
  tail(&info,info);
  sharedID=(int)atoi(temp);
  head(&temp,info);
  mntaddr=(ULO)atol(temp);

  shmempool_on();
  if(!attach_mempool(sharedID,mntaddr)){
    fprintf(stderr,"Cannot attach the shared memory pool.\n");
    exit(1);
  }

  if((entry_point_struct_ptr=(struct _entry_point_struct*)reallocpool(NULL,
     sizeof(struct _entry_point_struct)))==NULL){
    fprintf(stderr,"Memory allocation in the shared memory pool failed.\n");
    exit(1);
  }
  get_std_buff_secure(&entry_point_struct.str0,"String 0: I am in ShMemPool.");
  get_std_buff_secure(&entry_point_struct.str1,"String 1: I am in ShMemPool.");
  get_std_buff_secure(&entry_point_struct.strN,"String N: I am in ShMemPool.");
  *entry_point_struct_ptr=entry_point_struct;

  // Allocated entries are persistent.
  // Keep entry_point_struct_ptr somehow available for others, e.g.:
  //
  //   shmempool_off();
  //   get_std_buff(&lut_key,"Key for our test allocations");
  //   equ_num(&lut_value,(SLO)entry_point_struct_ptr);
  //   shmempool_on();
  //   shmempoolLUT_add_key_value(&lut_value,lut_key);
  //   shmempool_off();
  //
  // A consumer can initialize entry_point_struct_ptr like:
  //   shmempool_off();
  //   get_std_buff(&lut_key,"Key for our test allocations");
  //   shmempool_on();
  //   shmempoolLUT_get_value(&lut_value,lut_key);
  //   shmempool_off();
  //   entry_point_struct_ptr=(struct _entry_point_struct*)atol(lut_value);

  shmempool_on();
  if(!detach_mempool()){
    fprintf(stderr,"Cannot detach the shared memory pool.\n");
    exit(1);
  }
  shmempool_off();

  return 0;
}


.-------------------------------------------------------------------------.
|                                                                         |
|  Another nice example of producing ordered non-standard i/o during the  |
|  out-of-order processing                                                |
|                                                                         |
`-------------------------------------------------------------------------'

  // C level
  CHR devicewrite_udf(SLO enable, SLO *chunk){
    devicewrite(chunk);  // Non-standard output.
    return enable;  // Repeater.
  }

  # VM level
  (setq wenable 1)
  (for i 1 1 number_of_raw_chunks (progn
    (setq chunk (process (index chunks i)))  # Out-of-order processing.
    (setq wenable (devicewrite_udf wenable chunk))  # Ordered processing due
                                                    # to the data dependencies.
  ))


.------------------------------------------------------------------------.
|                                                                        |
|  Direct access to internal COPs of ISA and ISA extention               |
|                                                                        |
`------------------------------------------------------------------------'

#ifdef _EXTENDED_INTERFACE_LESS_GLOBALS_
  #define _CONST_VOID_PTR_RT_CTRL_comma const void *rt_ctrl,
  #define _RT_CTRL_comma rt_ctrl,
  #define noterror() noterror_fast(rt_ctrl)
#else
  #define _CONST_VOID_PTR_RT_CTRL_comma
  #define _RT_CTRL_comma
#endif

extern void _internal_ISA_COP_mnemonics_get(CHR **lst);
extern void _internal_ISA_COP_mnemonics_set(const CHR *list);
extern UCH _internal_ISA_COP_mnemonics_vars_ASCIIorUTF8_get(void);
extern void _internal_ISA_COP_mnemonics_vars_ASCIIorUTF8_set(UCH ASCIIorUTF8);
extern void _internal_ISA_COPs_get(CHR **tab);
extern void _internal_ISA_COPs_set(const CHR *table);
extern void _internal_ISAext_COPs_get(CHR **tab);
extern void _internal_ISAext_COPs_set(const CHR *table);

void func__idoubled(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_ival(_RT_CTRL_comma dat_ptr,&ret_dat->value.ival);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='I';
    ret_dat->value.ival*=2;
  }
  return;
}
void func__idoubled_j(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  dat_ptr=*((ULO**)dat_ptr);
  (*(fcall)*dat_ptr)(_RT_CTRL_comma dat_ptr+1,ret_dat);
  if(noterror())
    ret_dat->value.ival*=2;
  return;
}

void func__fusion_cmp(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  const ULO *tmp_ptr;
  SLO opb_i;
  DFL opb_f;
  CHR *opb_s=NULL;
  ret_dat->disable_ptr=1;
  tmp_ptr=*((ULO**)dat_ptr);
  (*(fcall)*tmp_ptr)(_RT_CTRL_comma tmp_ptr+1,ret_dat);
  if(noterror())
    switch(ret_dat->type){
      case 'I':
        ret_ival(_RT_CTRL_comma dat_ptr+1,&opb_i);
        if(noterror()){
          ret_dat->single=1;
          ret_dat->type='I';
          ret_dat->value.ival=(ret_dat->value.ival==opb_i);
        }
        break;
      case 'F':
        ret_fval(_RT_CTRL_comma dat_ptr+1,&opb_f);
        if(noterror()){
          ret_dat->single=1;
          ret_dat->type='I';
          ret_dat->value.ival=(ret_dat->value.fval==opb_f);
        }
        break;
      case 'S':
        ret_sval(_RT_CTRL_comma dat_ptr+1,&opb_s);
        if(noterror()){
          ret_dat->single=1;
          ret_dat->type='I';
          ret_dat->value.ival=cmp(ret_dat->svalue,opb_s);
          free_string(&opb_s);
        }
        break;
      case 'Z':
        tmp_ptr=*((ULO**)dat_ptr+1);
        (*(fcall)*tmp_ptr)(_RT_CTRL_comma tmp_ptr+1,ret_dat);
        ret_dat->single=1;
        ret_dat->value.ival=(ret_dat->type=='Z');
        ret_dat->type='I';
    }
  return;
}
void func__fusion_cmp_i(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  const ULO *tmp_ptr;
  SLO op_a;
  ret_dat->disable_ptr=1;
  tmp_ptr=*((ULO**)dat_ptr);
  (*(fcall)*tmp_ptr)(_RT_CTRL_comma tmp_ptr+1,ret_dat);
  op_a=ret_dat->value.ival;
  tmp_ptr=*((ULO**)(dat_ptr+1));
  (*(fcall)*tmp_ptr)(_RT_CTRL_comma tmp_ptr+1,ret_dat);
  if(noterror())
    ret_dat->value.ival=(op_a==ret_dat->value.ival);
  return;
}
void func__fusion_cmp_f(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  const ULO *tmp_ptr;
  DFL op_a;
  ret_dat->disable_ptr=1;
  tmp_ptr=*((ULO**)dat_ptr);
  (*(fcall)*tmp_ptr)(_RT_CTRL_comma tmp_ptr+1,ret_dat);
  op_a=ret_dat->value.fval;
  tmp_ptr=*((ULO**)(dat_ptr+1));
  (*(fcall)*tmp_ptr)(_RT_CTRL_comma tmp_ptr+1,ret_dat);
  if(noterror()){
    ret_dat->type='I';
    ret_dat->value.ival=(op_a==ret_dat->value.fval);
  }
  return;
}
void func__fusion_cmp_s(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  const ULO *tmp_ptr;
  CHR *op_a=NULL;
  ret_dat->disable_ptr=1;
  tmp_ptr=*((ULO**)dat_ptr);
  (*(fcall)*tmp_ptr)(_RT_CTRL_comma tmp_ptr+1,ret_dat);
  equ(&op_a,ret_dat->svalue);
  tmp_ptr=*((ULO**)(dat_ptr+1));
  (*(fcall)*tmp_ptr)(_RT_CTRL_comma tmp_ptr+1,ret_dat);
  if(noterror()){
    ret_dat->type='I';
    ret_dat->value.ival=cmp(op_a,ret_dat->svalue);
  }
  free_string(&op_a);
  return;
}
void func__fusion_cmp_z(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  const ULO *tmp_ptr;
  ret_dat->disable_ptr=1;
  tmp_ptr=*((ULO**)dat_ptr);
  (*(fcall)*tmp_ptr)(_RT_CTRL_comma tmp_ptr+1,ret_dat);
  tmp_ptr=*((ULO**)(dat_ptr+1));
  (*(fcall)*tmp_ptr)(_RT_CTRL_comma tmp_ptr+1,ret_dat);
  if(noterror()){
    ret_dat->type='I';
    ret_dat->value.ival=1;
  }
  return;
}

int main(int argc, char *argv[]){
  CHR *ISA_mnemonics=NULL,*ISA_COPs=NULL,*ISAext_COPs=NULL,
      *temp=NULL,*temp1=NULL;
  char buff[200];

  _internal_ISA_COP_mnemonics_get(&ISA_mnemonics);
  lcat(&ISA_mnemonics,space(&temp,1));
  cat(&ISA_mnemonics,temp);
  strtran(&ISA_mnemonics,ISA_mnemonics,
    get_std_buff(&temp," MAPCAR "),get_std_buff(&temp1," MAPBUS "));
  alltrim(&ISA_mnemonics,ISA_mnemonics);
  _internal_ISA_COP_mnemonics_set(ISA_mnemonics);
  _internal_ISA_COP_mnemonics_get(&ISA_mnemonics);
  fprintf(stderr,"%s\n",ISA_mnemonics);

  _internal_ISA_COPs_get(&ISA_COPs);
  snprintf(buff,sizeof(buff),"(IDOUBLED 1 I \"I\" %ld %ld 0 0 0)",
    (SLO)&func__idoubled,(SLO)&func__idoubled_j);
  cat(&ISA_COPs,get_std_buff(&temp,buff));
  snprintf(buff,sizeof(buff),"(I*2 1 I \"I\" %ld %ld 0 0 0)",
    (SLO)&func__idoubled,(SLO)&func__idoubled_j);
  cat(&ISA_COPs,get_std_buff(&temp,buff));
  snprintf(buff,sizeof(buff),"(FUSION_CMP 2 I \"EE\" %ld %ld %ld %ld %ld)",
    (SLO)&func__fusion_cmp,(SLO)&func__fusion_cmp_i,(SLO)&func__fusion_cmp_f,
    (SLO)&func__fusion_cmp_s,(SLO)&func__fusion_cmp_z);
  cat(&ISA_COPs,get_std_buff(&temp,buff));
  snprintf(buff,sizeof(buff),"(MERGE_CMP 2 I \"EE\" %ld %ld %ld %ld %ld)",
    (SLO)&func__fusion_cmp,(SLO)&func__fusion_cmp_i,(SLO)&func__fusion_cmp_f,
    (SLO)&func__fusion_cmp_s,(SLO)&func__fusion_cmp_z);
  cat(&ISA_COPs,get_std_buff(&temp,buff));
  _internal_ISA_COPs_set(ISA_COPs);
  _internal_ISA_COPs_get(&ISA_COPs);
  fprintf(stderr,"%s\n",ISA_COPs);

  _internal_ISAext_COPs_get(&ISAext_COPs);
  snprintf(buff,sizeof(buff),"(IDOUBLED_ 1 I \"I\" %ld)",(SLO)&func__idoubled);
  cat(&ISAext_COPs,get_std_buff(&temp,buff));
  snprintf(buff,sizeof(buff),"(I*2_ 1 I \"I\" %ld)",(SLO)&func__idoubled);
  cat(&ISAext_COPs,get_std_buff(&temp,buff));
  _internal_ISAext_COPs_set(ISAext_COPs);
  _internal_ISAext_COPs_get(&ISAext_COPs);
  fprintf(stderr,"%s\n",ISAext_COPs);

  free_string(&ISA_mnemonics);
  free_string(&ISA_COPs);
  free_string(&ISAext_COPs);
  free_string(&temp);
  free_string(&temp1);

  return _Main_(argc,argv);
}

# Testbench for direct access to internal COPs of ISA and ISA extention:
(setq var_int 2)
(setq var_float 2.)
(setq var_str "2")
(setq var_nil nil)
(outf "(i*2_ 2) = %ld\n" (i*2_ var_int))
(outf "(i*2_ 2.) = %ld\n" (i*2_ var_float))
(outf "(i*2 2) = %ld\n" (i*2 var_int))
(outf "(i*2 2.) = %ld\n" (i*2 var_float))
(outf "(fusion_cmp 2 2.) = %ld\n" (fusion_cmp var_int var_float))
(outf "(fusion_cmp 2 2) = %ld\n" (fusion_cmp var_int var_int))
(outf "(fusion_cmp 2. 2.) = %ld\n" (fusion_cmp var_float var_float))
(outf "(fusion_cmp \"2\" \"2\") = %ld\n" (fusion_cmp var_str var_str))
(outf "(fusion_cmp nil nil) = %ld\n" (fusion_cmp var_nil var_nil))
""

(comments

Preprocessed by fastlisp:
*************************
  (SETQ@I VAR_INT@I 2)
  (SETQ@F VAR_FLOAT@F 2.)
  (SETQ@S VAR_STR@S "2")
  (SETQ@Z VAR_NIL@Z NIL)
  (OUTF "(i*2_ 2) = %ld\n" (I*2_@J VAR_INT@I))
  (OUTF "(i*2_ 2.) = %ld\n" (I*2_ VAR_FLOAT@F))
  (OUTF "(i*2 2) = %ld\n" (I*2@J VAR_INT@I))
  (OUTF "(i*2 2.) = %ld\n" (I*2 VAR_FLOAT@F))
  (OUTF "(fusion_cmp 2 2.) = %ld\n" (FUSION_CMP VAR_INT@I VAR_FLOAT@F))
  (OUTF "(fusion_cmp 2 2) = %ld\n" (FUSION_CMP@I VAR_INT@I VAR_INT@I))
  (OUTF "(fusion_cmp 2. 2.) = %ld\n" (FUSION_CMP@F VAR_FLOAT@F VAR_FLOAT@F))
  (OUTF "(fusion_cmp \"2\" \"2\") = %ld\n" (FUSION_CMP@S VAR_STR@S VAR_STR@S))
  (OUTF "(fusion_cmp nil nil) = %ld\n" (FUSION_CMP@Z VAR_NIL@Z VAR_NIL@Z))

Preprocessed by BMDFMldr:
*************************
  (SETQ@I MAIN:VAR_INT@I 2)
  (SETQ@F MAIN:VAR_FLOAT@F 2.)
  (SETQ@S MAIN:VAR_STR@S "2")
  (SETQ@Z MAIN:VAR_NIL@Z NIL)
  (SETQ@I MAIN:TMP__000000001 (I*2_@J MAIN:VAR_INT@I))
  (SETQ@S MAIN:TMP__000000002@S (OUTF "(i*2_ 2) = %ld\n" MAIN:TMP__000000001))
  (SETQ@I MAIN:TMP__000000001 (I*2_ MAIN:VAR_FLOAT@F))
  (SETQ@S MAIN:TMP__000000002@S (OUTF "(i*2_ 2.) = %ld\n" MAIN:TMP__000000001))
  (SETQ@S MAIN:TMP__000000001 (OUTF "(i*2 2) = %ld\n" (I*2@J MAIN:VAR_INT@I)))
  (SETQ@S MAIN:TMP__000000001 (OUTF "(i*2 2.) = %ld\n" (I*2 MAIN:VAR_FLOAT@F)))
  (SETQ@S
    MAIN:TMP__000000001
    (OUTF
      "(fusion_cmp 2 2.) = %ld\n"
      (FUSION_CMP MAIN:VAR_INT@I MAIN:VAR_FLOAT@F)
    )
  )
  (SETQ@S
    MAIN:TMP__000000001
    (OUTF
      "(fusion_cmp 2 2) = %ld\n"
      (FUSION_CMP@I MAIN:VAR_INT@I MAIN:VAR_INT@I)
    )
  )
  (SETQ@S
    MAIN:TMP__000000001
    (OUTF
      "(fusion_cmp 2. 2.) = %ld\n"
      (FUSION_CMP@F MAIN:VAR_FLOAT@F MAIN:VAR_FLOAT@F)
    )
  )
  (SETQ@S
    MAIN:TMP__000000001
    (OUTF
      "(fusion_cmp \"2\" \"2\") = %ld\n"
      (FUSION_CMP@S MAIN:VAR_STR@S MAIN:VAR_STR@S)
    )
  )
  (SETQ@S
    MAIN:TMP__000000001
    (OUTF
      "(fusion_cmp nil nil) = %ld\n"
      (FUSION_CMP@Z MAIN:VAR_NIL@Z MAIN:VAR_NIL@Z)
    )
  )

Output:
*******
  (i*2_ 2) = 4
  (i*2_ 2.) = 4
  (i*2 2) = 4
  (i*2 2.) = 4
  (fusion_cmp 2 2.) = 1
  (fusion_cmp 2 2) = 1
  (fusion_cmp 2. 2.) = 1
  (fusion_cmp "2" "2") = 1
  (fusion_cmp nil nil) = 1

) # end comments

*/

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* cflp_udf.h */

