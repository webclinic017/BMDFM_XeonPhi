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

#ifdef _EXTENDED_INTERFACE_LESS_GLOBALS_
  #define VERSION_CFLPUDF___ VERSION_CFLPUDF_X_
  #define _CONST_VOID_PTR_RT_CTRL_comma const void *rt_ctrl,
  #define _RT_CTRL_comma rt_ctrl,
  #define noterror() noterror_fast(rt_ctrl)
#else
  #define _CONST_VOID_PTR_RT_CTRL_comma
  #define _RT_CTRL_comma
#endif

const CHR *VERSION_CFLPUDF___="Sancho M. CFLPUDF v.1.0.0.";

extern const ULO INSTRUCTIONS;

/* _________________________________________________________________________ */
/* Functions                                                       SECTION 0 */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* == GMP Wrapper (C-implementation) ======================== BEGINS HERE == */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                      *
*  IMPORTANT: link against GMP with the "-lgmp" flag!  *
*                                                      *
* * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <gmp.h>

/* gmp.h:
typedef struct{
  int _mp_alloc;     // Number of *limbs* allocated and pointed to by _mp_d.
  int _mp_size;      // abs(_mp_size) is the number of used limbs.
  mp_limb_t *_mp_d;  // Pointer to the limbs.
} __mpz_struct; */

CHR *mpz__serialize(CHR **targ, const __mpz_struct *source){
  if(mk_fst_buff(targ,sizeof(__mpz_struct)+labs((SLO)source->_mp_size)*
     sizeof(mp_limb_t))){
    **((__mpz_struct**)targ)=*source;
    memcpy((void*)(*targ+sizeof(__mpz_struct)),(void*)source->_mp_d,
      labs((SLO)source->_mp_size)*sizeof(mp_limb_t));
    ((__mpz_struct*)*targ)->_mp_alloc=labs((SLO)source->_mp_size);
    ((__mpz_struct*)*targ)->_mp_d=(mp_limb_t*)(*targ+sizeof(__mpz_struct));
  }
  return *targ;
}

UCH mpz__deserialize(__mpz_struct *targ, const CHR *source){
  UCH ret_val=0;
  if((len(source)>=sizeof(__mpz_struct))&&(((__mpz_struct*)source)->_mp_alloc*
     sizeof(mp_limb_t)==len(source)-sizeof(__mpz_struct))){
    *targ=*((__mpz_struct*)source);
    targ->_mp_d=(mp_limb_t*)((__mpz_struct*)source+1);
    ret_val=1;
  }
  return ret_val;
}

CHR *mpz__fromstr(CHR **targ, const CHR *source){
  mpz_t z;
  if((SLO)mpz_init_set_str(z,source,10)<0)
    free_string(targ);
  else
    mpz__serialize(targ,&z[0]);
  mpz_clear(z);
  return *targ;
}

CHR *mpz__tostr(CHR **targ, const CHR *source){
  SLO l;
  CHR *temp=NULL;
  mpz_t z;
  equ(&temp,source);
  free_string(targ);
  if(mpz__deserialize(&z[0],temp))
    if((l=mpz_sizeinbase(z,10))>=0)
      if(mk_fst_buff(targ,l+2)){
        *(*targ+l)=*(*targ+l+1)=0;
        gmp_snprintf(*targ,l+2,"%Zd",z);
        rtrim(targ,*targ);
      }
  free_string(&temp);
  return *targ;
}

/* gmp.h:
typedef struct{
  int _mp_prec;      // Max precision, in number of `mp_limb_t's.
  int _mp_size;      // abs(_mp_size) is the number of used limbs.
  mp_exp_t _mp_exp;  // Exponent, in the base of `mp_limb_t'.
  mp_limb_t *_mp_d;  // Pointer to the limbs.
} __mpf_struct; */

CHR *mpf__serialize(CHR **targ, const __mpf_struct *source){
  if(mk_fst_buff(targ,sizeof(__mpf_struct)+labs((SLO)source->_mp_size)*
     sizeof(mp_limb_t))){
    **((__mpf_struct**)targ)=*source;
    memcpy((void*)(*targ+sizeof(__mpf_struct)),(void*)source->_mp_d,
      labs((SLO)source->_mp_size)*sizeof(mp_limb_t));
    ((__mpf_struct*)*targ)->_mp_prec=labs((SLO)source->_mp_size);
    ((__mpf_struct*)*targ)->_mp_d=(mp_limb_t*)(*targ+sizeof(__mpf_struct));
  }
  return *targ;
}

UCH mpf__deserialize(__mpf_struct *targ, const CHR *source){
  UCH ret_val=0;
  if((len(source)>=sizeof(__mpf_struct))&&(((__mpf_struct*)source)->_mp_prec*
     sizeof(mp_limb_t)==len(source)-sizeof(__mpf_struct))){
    *targ=*((__mpf_struct*)source);
    targ->_mp_d=(mp_limb_t*)((__mpf_struct*)source+1);
    ret_val=1;
  }
  return ret_val;
}

CHR *mpf__fromstr(CHR **targ, const CHR *source){
  mpf_t f;
  ULO prec,prec_=len(source)*34/10;  /* ~ 10==2^3.4 */
  mpf_init2(f,prec_);
  prec=f[0]._mp_prec;
  while(1){
    mpf_clear(f);
    /* ATTENTION: GMP native mpf_set_default_prec() is not thread-safe! */
    mpf_set_default_prec(prec_);
    if((SLO)mpf_init_set_str(f,source,10)<0){
      free_string(targ);
      break;
    }
    if(prec==f[0]._mp_prec){
      mpf__serialize(targ,&f[0]);
      break;
    }
  }
  mpf_clear(f);
  return *targ;
}

CHR *mpf__tostr(CHR **targ, const CHR *source){
  ULO l;
  CHR *temp=NULL,*temp1=NULL,*temp2=NULL;
  mpf_t f;
  equ(&temp,source);
  free_string(targ);
  if(mpf__deserialize(&f[0],temp)){
    l=f[0]._mp_prec*10*8*sizeof(mp_limb_t)/34;
    if(mk_fst_buff(targ,l+2)){
      *(*targ+l)=*(*targ+l+1)=0;
      equ_num(&temp1,l);
      lcat(&temp1,get_std_buff(&temp2,"%."));
      cat(&temp1,get_std_buff(&temp2,"Ff"));
      gmp_snprintf(*targ,l+2,temp1,f);
      free_string(&temp1);
      free_string(&temp2);
      l=len(rtrim(targ,*targ))-1;
      temp1=*targ;
      while(*(temp1+l)=='0')
        l--;
      left(targ,*targ,l+1);
    }
  }
  free_string(&temp);
  return *targ;
}

SCH mpz__cmp(const CHR *op_a, const CHR *op_b){
  SCH ret_val=-2;
  int z_res;
  mpz_t z_a,z_b;
  if(mpz__deserialize(&z_a[0],op_a))
    if(mpz__deserialize(&z_b[0],op_b)){
      z_res=mpz_cmp(z_a,z_b);
      ret_val=z_res<0?-1:(z_res>0);
    }
  return ret_val;
}

CHR *mpz__add(CHR **targ, const CHR *op_a, const CHR *op_b){
  mpz_t z_a,z_b,z_res;
  if(mpz__deserialize(&z_a[0],op_a))
    if(mpz__deserialize(&z_b[0],op_b)){
      mpz_init(z_res);
      mpz_add(z_res,z_a,z_b);
      mpz__serialize(targ,&z_res[0]);
      mpz_clear(z_res);
    }
    else
      free_string(targ);
  else
    free_string(targ);
  return *targ;
}

CHR *mpz__sub(CHR **targ, const CHR *op_a, const CHR *op_b){
  mpz_t z_a,z_b,z_res;
  if(mpz__deserialize(&z_a[0],op_a))
    if(mpz__deserialize(&z_b[0],op_b)){
      mpz_init(z_res);
      mpz_sub(z_res,z_a,z_b);
      mpz__serialize(targ,&z_res[0]);
      mpz_clear(z_res);
    }
    else
      free_string(targ);
  else
    free_string(targ);
  return *targ;
}

CHR *mpz__mul(CHR **targ, const CHR *op_a, const CHR *op_b){
  mpz_t z_a,z_b,z_res;
  if(mpz__deserialize(&z_a[0],op_a))
    if(mpz__deserialize(&z_b[0],op_b)){
      mpz_init(z_res);
      mpz_mul(z_res,z_a,z_b);
      mpz__serialize(targ,&z_res[0]);
      mpz_clear(z_res);
    }
    else
      free_string(targ);
  else
    free_string(targ);
  return *targ;
}

CHR *mpz__div(CHR **targ, const CHR *op_a, const CHR *op_b){
  mpz_t z_a,z_b,z_res;
  if(mpz__deserialize(&z_a[0],op_a))
    if(mpz__deserialize(&z_b[0],op_b)){
      mpz_init(z_res);
      mpz_div(z_res,z_a,z_b);
      mpz__serialize(targ,&z_res[0]);
      mpz_clear(z_res);
    }
    else
      free_string(targ);
  else
    free_string(targ);
  return *targ;
}

CHR *mpz__mod(CHR **targ, const CHR *op_a, const CHR *op_b){
  mpz_t z_a,z_b,z_res;
  if(mpz__deserialize(&z_a[0],op_a))
    if(mpz__deserialize(&z_b[0],op_b)){
      mpz_init(z_res);
      mpz_mod(z_res,z_a,z_b);
      mpz__serialize(targ,&z_res[0]);
      mpz_clear(z_res);
    }
    else
      free_string(targ);
  else
    free_string(targ);
  return *targ;
}

CHR *mpz__neg(CHR **targ, const CHR *op_a){
  mpz_t z_a,z_res;
  if(mpz__deserialize(&z_a[0],op_a)){
    mpz_init(z_res);
    mpz_neg(z_res,z_a);
    mpz__serialize(targ,&z_res[0]);
    mpz_clear(z_res);
  }
  else
    free_string(targ);
  return *targ;
}

CHR *mpz__abs(CHR **targ, const CHR *op_a){
  mpz_t z_a,z_res;
  if(mpz__deserialize(&z_a[0],op_a)){
    mpz_init(z_res);
    mpz_abs(z_res,z_a);
    mpz__serialize(targ,&z_res[0]);
    mpz_clear(z_res);
  }
  else
    free_string(targ);
  return *targ;
}

CHR *mpz__pow_i(CHR **targ, const CHR *op_a, SLO op_b){
  mpz_t z_a,z_res;
  if(mpz__deserialize(&z_a[0],op_a)){
    mpz_init(z_res);
    mpz_pow_ui(z_res,z_a,op_b);
    mpz__serialize(targ,&z_res[0]);
    mpz_clear(z_res);
  }
  else
    free_string(targ);
  return *targ;
}

CHR *mpz__fac_i(CHR **targ, SLO op_a){
  mpz_t z_res;
  mpz_init(z_res);
  mpz_fac_ui(z_res,op_a);
  mpz__serialize(targ,&z_res[0]);
  mpz_clear(z_res);
  return *targ;
}

CHR *mpz__sqrt(CHR **targ, const CHR *op_a){
  mpz_t z_a,z_res;
  if(mpz__deserialize(&z_a[0],op_a)){
    mpz_init(z_res);
    mpz_sqrt(z_res,z_a);
    mpz__serialize(targ,&z_res[0]);
    mpz_clear(z_res);
  }
  else
    free_string(targ);
  return *targ;
}

CHR *mpz__and(CHR **targ, const CHR *op_a, const CHR *op_b){
  mpz_t z_a,z_b,z_res;
  if(mpz__deserialize(&z_a[0],op_a))
    if(mpz__deserialize(&z_b[0],op_b)){
      mpz_init(z_res);
      mpz_and(z_res,z_a,z_b);
      mpz__serialize(targ,&z_res[0]);
      mpz_clear(z_res);
    }
    else
      free_string(targ);
  else
    free_string(targ);
  return *targ;
}

CHR *mpz__ior(CHR **targ, const CHR *op_a, const CHR *op_b){
  mpz_t z_a,z_b,z_res;
  if(mpz__deserialize(&z_a[0],op_a))
    if(mpz__deserialize(&z_b[0],op_b)){
      mpz_init(z_res);
      mpz_ior(z_res,z_a,z_b);
      mpz__serialize(targ,&z_res[0]);
      mpz_clear(z_res);
    }
    else
      free_string(targ);
  else
    free_string(targ);
  return *targ;
}

CHR *mpz__xor(CHR **targ, const CHR *op_a, const CHR *op_b){
  mpz_t z_a,z_b,z_res;
  if(mpz__deserialize(&z_a[0],op_a))
    if(mpz__deserialize(&z_b[0],op_b)){
      mpz_init(z_res);
      mpz_xor(z_res,z_a,z_b);
      mpz__serialize(targ,&z_res[0]);
      mpz_clear(z_res);
    }
    else
      free_string(targ);
  else
    free_string(targ);
  return *targ;
}

CHR *mpz__com(CHR **targ, const CHR *op_a){
  mpz_t z_a,z_res;
  if(mpz__deserialize(&z_a[0],op_a)){
    mpz_init(z_res);
    mpz_com(z_res,z_a);
    mpz__serialize(targ,&z_res[0]);
    mpz_clear(z_res);
  }
  else
    free_string(targ);
  return *targ;
}

SCH mpf__cmp(const CHR *op_a, const CHR *op_b){
  SCH ret_val=-2;
  int f_res;
  mpf_t f_a,f_b;
  if(mpf__deserialize(&f_a[0],op_a))
    if(mpf__deserialize(&f_b[0],op_b)){
      f_res=mpf_cmp(f_a,f_b);
      ret_val=f_res<0?-1:(f_res>0);
    }
  return ret_val;
}

CHR *mpf__add(CHR **targ, const CHR *op_a, const CHR *op_b){
  mpf_t f_a,f_b,f_res;
  if(mpf__deserialize(&f_a[0],op_a))
    if(mpf__deserialize(&f_b[0],op_b)){
      mpf_init2(f_res,(f_a[0]._mp_prec>f_b[0]._mp_prec?f_a[0]._mp_prec:
        f_b[0]._mp_prec)*8*sizeof(mp_limb_t));
      mpf_add(f_res,f_a,f_b);
      mpf__serialize(targ,&f_res[0]);
      mpf_clear(f_res);
    }
    else
      free_string(targ);
  else
    free_string(targ);
  return *targ;
}

CHR *mpf__sub(CHR **targ, const CHR *op_a, const CHR *op_b){
  mpf_t f_a,f_b,f_res;
  if(mpf__deserialize(&f_a[0],op_a))
    if(mpf__deserialize(&f_b[0],op_b)){
      mpf_init2(f_res,(f_a[0]._mp_prec>f_b[0]._mp_prec?f_a[0]._mp_prec:
        f_b[0]._mp_prec)*8*sizeof(mp_limb_t));
      mpf_sub(f_res,f_a,f_b);
      mpf__serialize(targ,&f_res[0]);
      mpf_clear(f_res);
    }
    else
      free_string(targ);
  else
    free_string(targ);
  return *targ;
}

CHR *mpf__mul(CHR **targ, const CHR *op_a, const CHR *op_b){
  mpf_t f_a,f_b,f_res;
  if(mpf__deserialize(&f_a[0],op_a))
    if(mpf__deserialize(&f_b[0],op_b)){
      mpf_init2(f_res,(f_a[0]._mp_prec>f_b[0]._mp_prec?f_a[0]._mp_prec:
        f_b[0]._mp_prec)*8*sizeof(mp_limb_t));
      mpf_mul(f_res,f_a,f_b);
      mpf__serialize(targ,&f_res[0]);
      mpf_clear(f_res);
    }
    else
      free_string(targ);
  else
    free_string(targ);
  return *targ;
}

CHR *mpf__div(CHR **targ, const CHR *op_a, const CHR *op_b){
  mpf_t f_a,f_b,f_res;
  if(mpf__deserialize(&f_a[0],op_a))
    if(mpf__deserialize(&f_b[0],op_b)){
      mpf_init2(f_res,(f_a[0]._mp_prec>f_b[0]._mp_prec?f_a[0]._mp_prec:
        f_b[0]._mp_prec)*8*sizeof(mp_limb_t));
      mpf_div(f_res,f_a,f_b);
      mpf__serialize(targ,&f_res[0]);
      mpf_clear(f_res);
    }
    else
      free_string(targ);
  else
    free_string(targ);
  return *targ;
}

CHR *mpf__neg(CHR **targ, const CHR *op_a){
  mpf_t f_a,f_res;
  if(mpf__deserialize(&f_a[0],op_a)){
    mpf_init2(f_res,f_a[0]._mp_prec*8*sizeof(mp_limb_t));
    mpf_neg(f_res,f_a);
    mpf__serialize(targ,&f_res[0]);
    mpf_clear(f_res);
  }
  else
    free_string(targ);
  return *targ;
}

CHR *mpf__abs(CHR **targ, const CHR *op_a){
  mpf_t f_a,f_res;
  if(mpf__deserialize(&f_a[0],op_a)){
    mpf_init2(f_res,f_a[0]._mp_prec*8*sizeof(mp_limb_t));
    mpf_abs(f_res,f_a);
    mpf__serialize(targ,&f_res[0]);
    mpf_clear(f_res);
  }
  else
    free_string(targ);
  return *targ;
}

CHR *mpf__pow_i(CHR **targ, const CHR *op_a, SLO op_b){
  mpf_t f_a,f_res;
  if(mpf__deserialize(&f_a[0],op_a)){
    mpf_init2(f_res,f_a[0]._mp_prec*8*sizeof(mp_limb_t));
    mpf_pow_ui(f_res,f_a,op_b);
    mpf__serialize(targ,&f_res[0]);
    mpf_clear(f_res);
  }
  else
    free_string(targ);
  return *targ;
}

CHR *mpf__sqrt(CHR **targ, const CHR *op_a){
  mpf_t f_a,f_res;
  if(mpf__deserialize(&f_a[0],op_a)){
    mpf_init2(f_res,f_a[0]._mp_prec*8*sizeof(mp_limb_t));
    mpf_sqrt(f_res,f_a);
    mpf__serialize(targ,&f_res[0]);
    mpf_clear(f_res);
  }
  else
    free_string(targ);
  return *targ;
}

CHR *mpf__ceil(CHR **targ, const CHR *op_a){
  mpf_t f_a,f_res;
  if(mpf__deserialize(&f_a[0],op_a)){
    mpf_init2(f_res,f_a[0]._mp_prec*8*sizeof(mp_limb_t));
    mpf_ceil(f_res,f_a);
    mpf__serialize(targ,&f_res[0]);
    mpf_clear(f_res);
  }
  else
    free_string(targ);
  return *targ;
}

CHR *mpf__floor(CHR **targ, const CHR *op_a){
  mpf_t f_a,f_res;
  if(mpf__deserialize(&f_a[0],op_a)){
    mpf_init2(f_res,f_a[0]._mp_prec*8*sizeof(mp_limb_t));
    mpf_floor(f_res,f_a);
    mpf__serialize(targ,&f_res[0]);
    mpf_clear(f_res);
  }
  else
    free_string(targ);
  return *targ;
}

CHR *mpf__trunc(CHR **targ, const CHR *op_a){
  mpf_t f_a,f_res;
  if(mpf__deserialize(&f_a[0],op_a)){
    mpf_init2(f_res,f_a[0]._mp_prec*8*sizeof(mp_limb_t));
    mpf_trunc(f_res,f_a);
    mpf__serialize(targ,&f_res[0]);
    mpf_clear(f_res);
  }
  else
    free_string(targ);
  return *targ;
}
/* == GMP Wrapper (C-implementation) ========================== ENDS HERE == */

/* == GMP Wrapper (CFLP-implementation) ===================== BEGINS HERE == */
#ifdef ECODE_RT__WRONG_FMT_STRING
  #define ECODE_RT__GMP_PROCESSING_FAIL ECODE_RT__WRONG_FMT_STRING
#else
  #define ECODE_RT__GMP_PROCESSING_FAIL 9
#endif

void func__mpz_fromstr(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(mpz__fromstr(&ret_dat->svalue,ret_dat->svalue)==NULL){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "String to GMP conversion error in mpz_fromstr()!",dat_ptr);
    }
  }
  return;
}

void func__mpz_tostr(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(mpz__tostr(&ret_dat->svalue,ret_dat->svalue)==NULL){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP to String conversion error in mpz_tostr()!",dat_ptr);
    }
  }
  return;
}

void func__mpf_fromstr(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(mpf__fromstr(&ret_dat->svalue,ret_dat->svalue)==NULL){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "String to GMP conversion error in mpf_fromstr()!",dat_ptr);
    }
  }
  return;
}

void func__mpf_tostr(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(mpf__tostr(&ret_dat->svalue,ret_dat->svalue)==NULL){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP to String conversion error in mpf_tostr()!",dat_ptr);
    }
  }
  return;
}

void func__mpz_cmp(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='I';
    if((ret_dat->value.ival=mpz__cmp(ret_dat->svalue,op_b))==-2)
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpz_cmp()!",dat_ptr);
  }
  free_string(&op_b);
  return;
}

void func__mpz_equal(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='I';
    if((ret_dat->value.ival=mpz__cmp(ret_dat->svalue,op_b))==-2)
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpz_equal()!",dat_ptr);
    else
      ret_dat->value.ival=!ret_dat->value.ival;
  }
  free_string(&op_b);
  return;
}

void func__mpz_notequal(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='I';
    if((ret_dat->value.ival=mpz__cmp(ret_dat->svalue,op_b))==-2)
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpz_notequal()!",dat_ptr);
    else
      ret_dat->value.ival=(ret_dat->value.ival!=0);
  }
  free_string(&op_b);
  return;
}

void func__mpz_greater(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='I';
    if((ret_dat->value.ival=mpz__cmp(ret_dat->svalue,op_b))==-2)
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpz_greater()!",dat_ptr);
    else
      ret_dat->value.ival=(ret_dat->value.ival==1);
  }
  free_string(&op_b);
  return;
}

void func__mpz_greaterorequal(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='I';
    if((ret_dat->value.ival=mpz__cmp(ret_dat->svalue,op_b))==-2)
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpz_greaterorequal()!",dat_ptr);
    else
      ret_dat->value.ival=(ret_dat->value.ival>=0);
  }
  free_string(&op_b);
  return;
}

void func__mpz_less(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='I';
    if((ret_dat->value.ival=mpz__cmp(ret_dat->svalue,op_b))==-2)
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpz_less()!",dat_ptr);
    else
      ret_dat->value.ival=(ret_dat->value.ival==-1);
  }
  free_string(&op_b);
  return;
}

void func__mpz_lessorequal(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='I';
    if((ret_dat->value.ival=mpz__cmp(ret_dat->svalue,op_b))==-2)
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpz_lessorequal()!",dat_ptr);
    else
      ret_dat->value.ival=(ret_dat->value.ival<=0);
  }
  free_string(&op_b);
  return;
}

void func__mpz_add(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(mpz__add(&ret_dat->svalue,ret_dat->svalue,op_b)==NULL){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpz_add()!",dat_ptr);
    }
  }
  free_string(&op_b);
  return;
}

void func__mpz_sub(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(mpz__sub(&ret_dat->svalue,ret_dat->svalue,op_b)==NULL){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpz_sub()!",dat_ptr);
    }
  }
  free_string(&op_b);
  return;
}

void func__mpz_mul(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(mpz__mul(&ret_dat->svalue,ret_dat->svalue,op_b)==NULL){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpz_mul()!",dat_ptr);
    }
  }
  free_string(&op_b);
  return;
}

void func__mpz_div(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if((len(op_b)>=sizeof(__mpz_struct))&&!((__mpz_struct*)op_b)->_mp_size){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP division by zero error in mpz_div()!",dat_ptr);
    }
    else
      if(mpz__div(&ret_dat->svalue,ret_dat->svalue,op_b)==NULL){
        mk_fst_buff(&ret_dat->svalue,0);
        rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
          "GMP conversion error in mpz_div()!",dat_ptr);
      }
  }
  free_string(&op_b);
  return;
}

void func__mpz_mod(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if((len(op_b)>=sizeof(__mpz_struct))&&!((__mpz_struct*)op_b)->_mp_size){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP division by zero error in mpz_mod()!",dat_ptr);
    }
    else
      if(mpz__mod(&ret_dat->svalue,ret_dat->svalue,op_b)==NULL){
        mk_fst_buff(&ret_dat->svalue,0);
        rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
          "GMP conversion error in mpz_mod()!",dat_ptr);
      }
  }
  free_string(&op_b);
  return;
}

void func__mpz_neg(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(mpz__neg(&ret_dat->svalue,ret_dat->svalue)==NULL){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpz_neg()!",dat_ptr);
    }
  }
  return;
}

void func__mpz_abs(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(mpz__abs(&ret_dat->svalue,ret_dat->svalue)==NULL){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpz_abs()!",dat_ptr);
    }
  }
  return;
}

void func__mpz_pow_i(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  SLO op_b;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_ival(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(op_b<0){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP negative power operand in mpz_pow_i()!",dat_ptr);
    }
    else
      if(mpz__pow_i(&ret_dat->svalue,ret_dat->svalue,op_b)==NULL){
        mk_fst_buff(&ret_dat->svalue,0);
        rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
          "GMP conversion error in mpz_pow_i()!",dat_ptr);
      }
  }
  return;
}

void func__mpz_fac_i(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_ival(_RT_CTRL_comma dat_ptr,&ret_dat->value.ival);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(ret_dat->value.ival<0){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP negative factorial operand in mpz_fac_i()!",dat_ptr);
    }
    else
      if(mpz__fac_i(&ret_dat->svalue,ret_dat->value.ival)==NULL){
        mk_fst_buff(&ret_dat->svalue,0);
        rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
          "GMP conversion error in mpz_fac_i()!",dat_ptr);
      }
  }
  return;
}

void func__mpz_sqrt(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if((len(ret_dat->svalue)>=sizeof(__mpz_struct))&&
       (((__mpz_struct*)ret_dat->svalue)->_mp_size<0)){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP negative operand in mpz_sqrt()!",dat_ptr);
    }
    else
      if(mpz__sqrt(&ret_dat->svalue,ret_dat->svalue)==NULL){
        mk_fst_buff(&ret_dat->svalue,0);
        rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
          "GMP conversion error in mpz_sqrt()!",dat_ptr);
      }
  }
  return;
}

void func__mpz_and(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(mpz__and(&ret_dat->svalue,ret_dat->svalue,op_b)==NULL){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpz_and()!",dat_ptr);
    }
  }
  free_string(&op_b);
  return;
}

void func__mpz_ior(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(mpz__ior(&ret_dat->svalue,ret_dat->svalue,op_b)==NULL){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpz_ior()!",dat_ptr);
    }
  }
  free_string(&op_b);
  return;
}

void func__mpz_xor(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(mpz__xor(&ret_dat->svalue,ret_dat->svalue,op_b)==NULL){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpz_xor()!",dat_ptr);
    }
  }
  free_string(&op_b);
  return;
}

void func__mpz_com(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(mpz__com(&ret_dat->svalue,ret_dat->svalue)==NULL){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpz_com()!",dat_ptr);
    }
  }
  return;
}

void func__mpf_cmp(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='I';
    if((ret_dat->value.ival=mpf__cmp(ret_dat->svalue,op_b))==-2)
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpf_cmp()!",dat_ptr);
  }
  free_string(&op_b);
  return;
}

void func__mpf_equal(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='I';
    if((ret_dat->value.ival=mpf__cmp(ret_dat->svalue,op_b))==-2)
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpf_equal()!",dat_ptr);
    else
      ret_dat->value.ival=!ret_dat->value.ival;
  }
  free_string(&op_b);
  return;
}

void func__mpf_notequal(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='I';
    if((ret_dat->value.ival=mpf__cmp(ret_dat->svalue,op_b))==-2)
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpf_notequal()!",dat_ptr);
    else
      ret_dat->value.ival=(ret_dat->value.ival!=0);
  }
  free_string(&op_b);
  return;
}

void func__mpf_greater(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='I';
    if((ret_dat->value.ival=mpf__cmp(ret_dat->svalue,op_b))==-2)
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpf_greater()!",dat_ptr);
    else
      ret_dat->value.ival=(ret_dat->value.ival==1);
  }
  free_string(&op_b);
  return;
}

void func__mpf_greaterorequal(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='I';
    if((ret_dat->value.ival=mpf__cmp(ret_dat->svalue,op_b))==-2)
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpf_greaterorequal()!",dat_ptr);
    else
      ret_dat->value.ival=(ret_dat->value.ival>=0);
  }
  free_string(&op_b);
  return;
}

void func__mpf_less(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='I';
    if((ret_dat->value.ival=mpf__cmp(ret_dat->svalue,op_b))==-2)
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpf_less()!",dat_ptr);
    else
      ret_dat->value.ival=(ret_dat->value.ival==-1);
  }
  free_string(&op_b);
  return;
}

void func__mpf_lessorequal(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='I';
    if((ret_dat->value.ival=mpf__cmp(ret_dat->svalue,op_b))==-2)
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpf_lessorequal()!",dat_ptr);
    else
      ret_dat->value.ival=(ret_dat->value.ival<=0);
  }
  free_string(&op_b);
  return;
}

void func__mpf_add(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(mpf__add(&ret_dat->svalue,ret_dat->svalue,op_b)==NULL){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpf_add()!",dat_ptr);
    }
  }
  free_string(&op_b);
  return;
}

void func__mpf_sub(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(mpf__sub(&ret_dat->svalue,ret_dat->svalue,op_b)==NULL){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpf_sub()!",dat_ptr);
    }
  }
  free_string(&op_b);
  return;
}

void func__mpf_mul(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(mpf__mul(&ret_dat->svalue,ret_dat->svalue,op_b)==NULL){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpf_mul()!",dat_ptr);
    }
  }
  free_string(&op_b);
  return;
}

void func__mpf_div(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if((len(op_b)>=sizeof(__mpf_struct))&&!((__mpf_struct*)op_b)->_mp_size){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP division by zero error in mpf_div()!",dat_ptr);
    }
    else
      if(mpf__div(&ret_dat->svalue,ret_dat->svalue,op_b)==NULL){
        mk_fst_buff(&ret_dat->svalue,0);
        rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
          "GMP conversion error in mpf_div()!",dat_ptr);
      }
  }
  free_string(&op_b);
  return;
}

void func__mpf_neg(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(mpf__neg(&ret_dat->svalue,ret_dat->svalue)==NULL){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpf_neg()!",dat_ptr);
    }
  }
  return;
}

void func__mpf_abs(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(mpf__abs(&ret_dat->svalue,ret_dat->svalue)==NULL){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpf_abs()!",dat_ptr);
    }
  }
  return;
}

void func__mpf_pow_i(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  SLO op_b;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_ival(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(op_b<0){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP negative power operand in mpf_pow_i()!",dat_ptr);
    }
    else
      if(mpf__pow_i(&ret_dat->svalue,ret_dat->svalue,op_b)==NULL){
        mk_fst_buff(&ret_dat->svalue,0);
        rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
          "GMP conversion error in mpf_pow_i()!",dat_ptr);
      }
  }
  return;
}

void func__mpf_sqrt(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if((len(ret_dat->svalue)>=sizeof(__mpf_struct))&&
       (((__mpf_struct*)ret_dat->svalue)->_mp_size<0)){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP negative operand in mpf_sqrt()!",dat_ptr);
    }
    else
      if(mpf__sqrt(&ret_dat->svalue,ret_dat->svalue)==NULL){
        mk_fst_buff(&ret_dat->svalue,0);
        rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
          "GMP conversion error in mpf_sqrt()!",dat_ptr);
      }
  }
  return;
}

void func__mpf_ceil(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(mpf__ceil(&ret_dat->svalue,ret_dat->svalue)==NULL){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpf_ceil()!",dat_ptr);
    }
  }
  return;
}

void func__mpf_floor(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(mpf__floor(&ret_dat->svalue,ret_dat->svalue)==NULL){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpf_floor()!",dat_ptr);
    }
  }
  return;
}

void func__mpf_trunc(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(mpf__trunc(&ret_dat->svalue,ret_dat->svalue)==NULL){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__GMP_PROCESSING_FAIL,
        "GMP conversion error in mpf_trunc()!",dat_ptr);
    }
  }
  return;
}
/* == GMP Wrapper (CFLP-implementation) ======================= ENDS HERE == */

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
/* == GMP Wrapper (CFLP-implementation) ===================== BEGINS HERE == */
  {"MPZ_FROMSTR",1,'S',(UCH*)"S",&func__mpz_fromstr},
  {"MPZ",1,'S',(UCH*)"S",&func__mpz_fromstr},
  {"MPZ_TOSTR",1,'S',(UCH*)"S",&func__mpz_tostr},
  {"MPZ_CMP",2,'I',(UCH*)"SS",&func__mpz_cmp},
  {"MPZ_==",2,'I',(UCH*)"SS",&func__mpz_equal},
  {"MPZ_!=",2,'I',(UCH*)"SS",&func__mpz_notequal},
  {"MPZ_>",2,'I',(UCH*)"SS",&func__mpz_greater},
  {"MPZ_>=",2,'I',(UCH*)"SS",&func__mpz_greaterorequal},
  {"MPZ_<",2,'I',(UCH*)"SS",&func__mpz_less},
  {"MPZ_<=",2,'I',(UCH*)"SS",&func__mpz_lessorequal},
  {"MPZ_ADD",2,'S',(UCH*)"SS",&func__mpz_add},
  {"MPZ_+",2,'S',(UCH*)"SS",&func__mpz_add},
  {"MPZ_SUB",2,'S',(UCH*)"SS",&func__mpz_sub},
  {"MPZ_-",2,'S',(UCH*)"SS",&func__mpz_sub},
  {"MPZ_MUL",2,'S',(UCH*)"SS",&func__mpz_mul},
  {"MPZ_*",2,'S',(UCH*)"SS",&func__mpz_mul},
  {"MPZ_DIV",2,'S',(UCH*)"SS",&func__mpz_div},
  {"MPZ_/",2,'S',(UCH*)"SS",&func__mpz_div},
  {"MPZ_MOD",2,'S',(UCH*)"SS",&func__mpz_mod},
  {"MPZ_%",2,'S',(UCH*)"SS",&func__mpz_mod},
  {"MPZ_NEG",1,'S',(UCH*)"S",&func__mpz_neg},
  {"MPZ_0-",1,'S',(UCH*)"S",&func__mpz_neg},
  {"MPZ_ABS",1,'S',(UCH*)"S",&func__mpz_abs},
  {"MPZ_POW_I",2,'S',(UCH*)"SI",&func__mpz_pow_i},
  {"MPZ_**_I",2,'S',(UCH*)"SI",&func__mpz_pow_i},
  {"MPZ_FAC_I",1,'S',(UCH*)"I",&func__mpz_fac_i},
  {"MPZ_FACT_I",1,'S',(UCH*)"I",&func__mpz_fac_i},
  {"MPZ_SQRT",1,'S',(UCH*)"S",&func__mpz_sqrt},
  {"MPZ_SQR",1,'S',(UCH*)"S",&func__mpz_sqrt},
  {"MPZ_AND",2,'S',(UCH*)"SS",&func__mpz_and},
  {"MPZ_&",2,'S',(UCH*)"SS",&func__mpz_and},
  {"MPZ_IOR",2,'S',(UCH*)"SS",&func__mpz_ior},
  {"MPZ_|",2,'S',(UCH*)"SS",&func__mpz_ior},
  {"MPZ_XOR",2,'S',(UCH*)"SS",&func__mpz_xor},
  {"MPZ_^",2,'S',(UCH*)"SS",&func__mpz_xor},
  {"MPZ_COM",1,'S',(UCH*)"S",&func__mpz_com},
  {"MPF_FROMSTR",1,'S',(UCH*)"S",&func__mpf_fromstr},
  {"MPF",1,'S',(UCH*)"S",&func__mpf_fromstr},
  {"MPF_TOSTR",1,'S',(UCH*)"S",&func__mpf_tostr},
  {"MPF_CMP",2,'I',(UCH*)"SS",&func__mpf_cmp},
  {"MPF_==",2,'I',(UCH*)"SS",&func__mpf_equal},
  {"MPF_!=",2,'I',(UCH*)"SS",&func__mpf_notequal},
  {"MPF_>",2,'I',(UCH*)"SS",&func__mpf_greater},
  {"MPF_>=",2,'I',(UCH*)"SS",&func__mpf_greaterorequal},
  {"MPF_<",2,'I',(UCH*)"SS",&func__mpf_less},
  {"MPF_<=",2,'I',(UCH*)"SS",&func__mpf_lessorequal},
  {"MPF_ADD",2,'S',(UCH*)"SS",&func__mpf_add},
  {"MPF_+",2,'S',(UCH*)"SS",&func__mpf_add},
  {"MPF_SUB",2,'S',(UCH*)"SS",&func__mpf_sub},
  {"MPF_-",2,'S',(UCH*)"SS",&func__mpf_sub},
  {"MPF_MUL",2,'S',(UCH*)"SS",&func__mpf_mul},
  {"MPF_*",2,'S',(UCH*)"SS",&func__mpf_mul},
  {"MPF_DIV",2,'S',(UCH*)"SS",&func__mpf_div},
  {"MPF_/",2,'S',(UCH*)"SS",&func__mpf_div},
  {"MPF_NEG",1,'S',(UCH*)"S",&func__mpf_neg},
  {"MPF_0-",1,'S',(UCH*)"S",&func__mpf_neg},
  {"MPF_ABS",1,'S',(UCH*)"S",&func__mpf_abs},
  {"MPF_POW_I",2,'S',(UCH*)"SI",&func__mpf_pow_i},
  {"MPF_**_I",2,'S',(UCH*)"SI",&func__mpf_pow_i},
  {"MPF_SQRT",1,'S',(UCH*)"S",&func__mpf_sqrt},
  {"MPF_SQR",1,'S',(UCH*)"S",&func__mpf_sqrt},
  {"MPF_CEIL",1,'S',(UCH*)"S",&func__mpf_ceil},
  {"MPF_FLOOR",1,'S',(UCH*)"S",&func__mpf_floor},
  {"MPF_TRUNC",1,'S',(UCH*)"S",&func__mpf_trunc}
/* == GMP Wrapper (CFLP-implementation) ======================= ENDS HERE == */
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

