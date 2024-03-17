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

/* == WChar String Processing (C-implementation) ============ BEGINS HERE == */
#include <wchar.h>
#include <wctype.h>  /* for towupper() and towlower() */

#define WCR wchar_t

WCR *wmk_fst_buff(WCR **buff, ULO size){
  mk_fst_buff((CHR**)buff,(size+1)*sizeof(WCR));
  *(*buff+size)=(WCR)0;
  return *buff;
}

WCR *wfree_string(WCR **targ){
  return (WCR*)free_string((CHR**)targ);
}

ULO wlen(const WCR *string){
  return len((CHR*)string)<sizeof(WCR)?0:len((CHR*)string)/sizeof(WCR)-1;
}

SLO wprnwidth(const WCR *string){
  SLO w=0;
  ULO i,l=wlen(string);
  int wcw;
  for(i=0;i<l;i++){
    if((wcw=wcwidth(*(string+i)))<0){
      w=-1;
      break;
    }
    w+=*(string+i)?wcw:2;  /* "\0" */
  }
  return w;
}

WCR *wequ(WCR **targ, const WCR *source){
  ULO l;
  if(*targ!=source){
    l=wlen(source);
    wmk_fst_buff(targ,l);
    memcpy((void*)*targ,(void*)source,l*sizeof(WCR));
  }
  return *targ;
}

WCR *wcat(WCR **targ, const WCR *source){
  ULO l0=wlen(*targ),l1=wlen(source);
  WCR *temp=NULL;
  wmk_fst_buff(&temp,l0+l1);
  memcpy((void*)temp,(void*)*targ,l0*sizeof(WCR));
  memcpy((void*)(temp+l0),(void*)source,l1*sizeof(WCR));
  wfree_string(targ);
  return (*targ=temp);
}

WCR *wlcat(WCR **targ, const WCR *source){
  ULO l0=wlen(*targ),l1=wlen(source);
  WCR *temp=NULL;
  wmk_fst_buff(&temp,l0+l1);
  memcpy((void*)temp,(void*)source,l1*sizeof(WCR));
  memcpy((void*)(temp+l1),(void*)*targ,l0*sizeof(WCR));
  wfree_string(targ);
  return (*targ=temp);
}

WCR *wfromstr_(WCR **targ, const CHR *source){
  SLO l;
  if((l=(SLO)mbstowcs(NULL,source,0))<0)
    wfree_string(targ);
  else{
    wmk_fst_buff(targ,l);
    if(l&&((SLO)mbstowcs(*targ,source,l+1)<0))
      wfree_string(targ);
  }
  return *targ;
}

WCR *wfromstr(WCR **targ, const CHR *source){
  SLO i,l=len(source);
  WCR *target=NULL,*wchr_nul=NULL,*temp=NULL;
  wmk_fst_buff(&target,0);
  wmk_fst_buff(&wchr_nul,1);
  *wchr_nul=(WCR)0;
  for(i=0;i<l;i++){
    if(wfromstr_(&temp,source+i)==NULL){
      wfree_string(&target);
      break;
    }
    wcat(&target,temp);
    while(*(source+i))
      i++;
    if(i<l)
      wcat(&target,wchr_nul);
  }
  wfree_string(&wchr_nul);
  wfree_string(&temp);
  wfree_string(targ);
  return (*targ=target);
}

CHR *wtostr_(CHR **targ, const WCR *source){
  SLO l;
  if((l=(SLO)wcstombs(NULL,source,0))<0)
    free_string(targ);
  else{
    mk_fst_buff(targ,l);
    if(l&&((SLO)wcstombs(*targ,source,l+1)<0))
      free_string(targ);
  }
  return *targ;
}

CHR *wtostr(CHR **targ, const WCR *source){
  SLO i,l=wlen(source);
  CHR *target=NULL,*chr_nul=NULL,*temp=NULL;
  mk_fst_buff(&target,0);
  sch2str(&chr_nul,0);
  for(i=0;i<l;i++){
    if(wtostr_(&temp,source+i)==NULL){
      free_string(&target);
      break;
    }
    cat(&target,temp);
    while(*(source+i))
      i++;
    if(i<l)
      cat(&target,chr_nul);
  }
  free_string(&chr_nul);
  free_string(&temp);
  free_string(targ);
  return (*targ=target);
}

UCH wcmp(const WCR *string1, const WCR *string2){
  UCH ret_val=0;
  ULO i,l;
  if((l=wlen(string1))==wlen(string2)){
    for(i=0;i<l;i++)
      if(*(string1+i)!=*(string2+i))
        break;
    if(i==l)
      ret_val=1;
  }
  return ret_val;
}

SCH wcmp_s(const WCR *string1, const WCR *string2){
  SCH ret_val=0;
  ULO i,l;
  if(string1!=string2)
    if(string1==NULL)
      ret_val=-1;
    else
      if(string2==NULL)
        ret_val=1;
      else{
        l=wlen(string1);
        i=wlen(string2);
        if(l>i)
          l=i;
        for(i=0;i<l;i++)
          if(*(string1+i)<*(string2+i)){
            ret_val=-1;
            break;
          }
          else
            if(*(string1+i)>*(string2+i)){
              ret_val=1;
              break;
            }
        if(i==l)
          ret_val=wlen(string1)<wlen(string2)?-1:(wlen(string1)>wlen(string2));
      }
  return ret_val;
}

WCR *wspace(WCR **targ, ULO pos){
  ULO i;
  wmk_fst_buff(targ,pos);
  for(i=0;i<pos;i++)
    *(*targ+i)=(WCR)L' ';
  return *targ;
}

WCR *wreplicate(WCR **targ, const WCR *source, ULO num){
  ULO i;
  WCR *temp=NULL;
  if(*targ==source){
    wmk_fst_buff(&temp,0);
    for(i=0;i<num;i++)
      wcat(&temp,source);
    wfree_string(targ);
    *targ=temp;
  }
  else
    if(wlen(source)==1){
      wmk_fst_buff(targ,num);
      for(i=0;i<num;i++)
        *(*targ+i)=*source;
    }
    else{
      wmk_fst_buff(targ,0);
      for(i=0;i<num;i++)
        wcat(targ,source);
    }
  return *targ;
}

WCR *wsubstr(WCR **targ, const WCR *source, ULO from, ULO pos){
  ULO l;
  WCR *temp=NULL;
  if(!from){
    from=1;
    if(pos)
      pos--;
  }
  if(from<=(l=wlen(source)))
    pos=(l-from+1)<pos?l-from+1:pos;
  else{
    from=l;
    pos=0;
  }
  if(*targ==source){
    wequ(&temp,source);
    wmk_fst_buff(targ,pos);
    memcpy((void*)*targ,(void*)(temp+from-1),pos*sizeof(WCR));
    *(*targ+pos)=(WCR)0;
    wfree_string(&temp);
  }
  else{
    wmk_fst_buff(targ,pos);
    memcpy((void*)*targ,(void*)(source+from-1),pos*sizeof(WCR));
    *(*targ+pos)=(WCR)0;
  }
  return *targ;
}

WCR *wleft(WCR **targ, const WCR *source, ULO pos){
  wsubstr(targ,source,1,pos);
  return *targ;
}

WCR *wleftr(WCR **targ, const WCR *source, ULO posr){
  ULO i;
  i=wlen(source);
  if(posr>i)
    posr=i;
  wsubstr(targ,source,1,i-posr);
  return *targ;
}

WCR *wright(WCR **targ, const WCR *source, ULO pos){
  ULO i;
  i=wlen(source);
  if(pos>i)
    pos=i;
  wsubstr(targ,source,i-pos+1,pos);
  return *targ;
}

WCR *wrightl(WCR **targ, const WCR *source, ULO posl){
  wsubstr(targ,source,posl+1,wlen(source));
  return *targ;
}

ULO wat(const WCR *pattern, const WCR *among){
  ULO i=0,j,l;
  if(((l=wlen(among))>=(j=wlen(pattern)))&&j){
    l-=j;
    for(i=0;i<=l;i++)
      if(!memcmp((void*)pattern,(void*)(among+i),j*sizeof(WCR)))
        break;
    i=i>l?0:i+1;
  }
  return i;
}

ULO wrat(const WCR *pattern, const WCR *among){
  ULO i=0,j,l;
  if(((l=wlen(among))>=(j=wlen(pattern)))&&j){
    l=l-j+1;
    for(i=l;i>0;i--)
      if(!memcmp((void*)pattern,(void*)(among+i-1),j*sizeof(WCR)))
        break;
    if(!i)
      i=0;
  }
  return i;
}

WCR *wstrtran(WCR **targ, const WCR *source, const WCR *pattern,
   const WCR *subst){
  ULO i,l;
  WCR *temp=NULL,*temp1=NULL,*temp2=NULL;
  l=wlen(pattern);
  if((*targ==pattern)||(*targ==subst)){
    wequ(&temp,source);
    wmk_fst_buff(&temp2,0);
    while(wlen(temp))
      if((i=wat(pattern,temp))){
        wleft(&temp1,temp,i-1);
        wcat(&temp2,temp1);
        wcat(&temp2,subst);
        wrightl(&temp,temp,i-1+l);
      }
      else{
        wcat(&temp2,temp);
        wmk_fst_buff(&temp,0);
      }
    wfree_string(targ);
    *targ=temp2;
    wfree_string(&temp);
    wfree_string(&temp1);
  }
  else
    if((l==1)&&(wlen(subst)==1)){
      l=wlen(wequ(targ,source));
      for(i=0;i<l;i++)
        if(*(*targ+i)==*pattern)
          *(*targ+i)=*subst;
    }
    else{
      wequ(&temp,source);
      wmk_fst_buff(targ,0);
      while(wlen(temp))
        if((i=wat(pattern,temp))){
          wleft(&temp1,temp,i-1);
          wcat(targ,temp1);
          wcat(targ,subst);
          wrightl(&temp,temp,i-1+l);
        }
        else{
          wcat(targ,temp);
          wmk_fst_buff(&temp,0);
        }
      wfree_string(&temp);
      wfree_string(&temp1);
    }
  return *targ;
}

WCR *wltrim(WCR **targ, const WCR *source){
  ULO i=0,l;
  l=wlen(source);
  while((i<l)&&(*(source+i)<=(WCR)L' '))
    i++;
  wrightl(targ,source,i);
  return *targ;
}

WCR *wrtrim(WCR **targ, const WCR *source){
  ULO i;
  i=wlen(source);
  while(i&&(*(source+i-1)<=(WCR)L' '))
    i--;
  wleft(targ,source,i);
  return *targ;
}

WCR *walltrim(WCR **targ, const WCR *source){
  wltrim(targ,wrtrim(targ,source));
  return *targ;
}

WCR *wpack(WCR **targ, const WCR *source){
  ULO i,l;
  WCR *temp=NULL,*temp1=NULL;
  walltrim(targ,source);
  l=wlen(*targ);
  for(i=0;i<l;i++)
    if(*(*targ+i)<(WCR)L' ')
      *(*targ+i)=(WCR)L' ';
  wmk_fst_buff(&temp,2);
  *temp=*(temp+1)=(WCR)0;
  wmk_fst_buff(&temp1,1);
  *temp1=(WCR)0;
  while(l>wlen(wstrtran(targ,*targ,temp,temp1)))
    l=wlen(*targ);
  wfree_string(&temp);
  wfree_string(&temp1);
  return *targ;
}

UCH wnotempty(const WCR *string){
  ULO i,l;
  UCH ret_val=0;
  l=wlen(string);
  for(i=0;i<l;i++)
    if(*(string+i)>(WCR)L' '){
      ret_val=1;
      break;
    }
  return ret_val;
}

WCR *whead(WCR **targ, const WCR *source){
  ULO i;
  WCR *temp=NULL;
  wpack(targ,source);
  wmk_fst_buff(&temp,1);
  *temp=(WCR)L' ';
  if((i=wat(temp,*targ)))
    wleft(targ,*targ,i-1);
  wrtrim(targ,*targ);
  wfree_string(&temp);
  return *targ;
}

WCR *wtail(WCR **targ, const WCR *source){
  ULO i;
  WCR *temp=NULL;
  i=wlen(wltrim(&temp,source))-wlen(whead(targ,temp));
  wltrim(targ,wright(targ,temp,i));
  wfree_string(&temp);
  return *targ;
}

WCR *wlsp_head(WCR **targ, const WCR *source){
  ULO i,l,bracket=0;
  UCH flag=1;
  l=wlen(source);
  for(i=0;i<l;i++)
    if(*(source+i)>(WCR)L' ')
      break;
  for(;i<l;i++)
    if(*(source+i)<=(WCR)L' '){
      if(!bracket&&flag)
        break;
    }
    else
      if(*(source+i)==(WCR)L'"')
        flag=!flag;
      else
        if(flag){
          if(*(source+i)==(WCR)L'(')
            bracket++;
          else
            if(*(source+i)==(WCR)L')')
              if(bracket){
                bracket--;
                if(!bracket){
                  i++;
                  break;
                }
              }
              else{
                flag=0;
                break;
              }
        }
        else
          if(*(source+i)==(WCR)L'\\')
            i++;
  walltrim(targ,wleft(targ,source,i));
  return *targ;
}

WCR *wlsp_tail(WCR **targ, const WCR *source){
  ULO i;
  WCR *temp=NULL;
  wlsp_head(targ,wltrim(&temp,source));
  i=wlen(temp)-wlen(*targ);
  wltrim(targ,wright(targ,temp,i));
  wfree_string(&temp);
  return *targ;
}

WCR *wupper(WCR **targ, const WCR *source){
  ULO i,l;
  l=wlen(wequ(targ,source));
  for(i=0;i<l;i++)
    *(*targ+i)=(WCR)towupper((wint_t)*(*targ+i));
  return *targ;
}

WCR *wlower(WCR **targ, const WCR *source){
  ULO i,l;
  l=wlen(wequ(targ,source));
  for(i=0;i<l;i++)
    *(*targ+i)=(WCR)towlower((wint_t)*(*targ+i));
  return *targ;
}

WCR *wrev(WCR **targ, const WCR *source){
  ULO i,l;
  WCR *temp=NULL;
  if(*targ==source){
    wequ(&temp,wequ(targ,source));
    l=wlen(temp);
    for(i=0;i<l;i++)
      *(*targ+i)=*(temp+l-i-1);
    wfree_string(&temp);
  }
  else{
    wequ(targ,source);
    l=wlen(source);
    for(i=0;i<l;i++)
      *(*targ+i)=*(source+l-i-1);
  }
  return *targ;
}

WCR *wpadl(WCR **targ, const WCR *source, ULO width){
  WCR *temp=NULL;
  wequ(targ,source);
  while((SLO)width<wprnwidth(wleft(targ,wrtrim(targ,*targ),width)))
    wleftr(targ,*targ,1);
  wcat(targ,wspace(&temp,width-wprnwidth(*targ)));
  wfree_string(&temp);
  return *targ;
}

WCR *wpadr(WCR **targ, const WCR *source, ULO width){
  WCR *temp=NULL;
  wltrim(targ,source);
  while((SLO)width<wprnwidth(wleft(targ,*targ,width)))
    wleftr(targ,*targ,1);
  wlcat(targ,wspace(&temp,width-wprnwidth(*targ)));
  wfree_string(&temp);
  return *targ;
}

WCR *wpadc(WCR **targ, const WCR *source, ULO width){
  WCR *temp=NULL;
  wltrim(targ,source);
  while((SLO)width<wprnwidth(wleft(targ,wrtrim(targ,*targ),width)))
    wleftr(targ,*targ,1);
  wlcat(targ,wspace(&temp,width-wprnwidth(wcat(targ,wspace(&temp,(width-
    wprnwidth(*targ))>>1)))));
  wfree_string(&temp);
  return *targ;
}
/* == WChar String Processing (C-implementation) ============== ENDS HERE == */

/* == WChar String Processing (CFLP-implementation) ========= BEGINS HERE == */
#ifdef ECODE_RT__WRONG_FMT_STRING
  #define ECODE_RT__WCHAR_PROCESSING_FAIL ECODE_RT__WRONG_FMT_STRING
#else
  #define ECODE_RT__WCHAR_PROCESSING_FAIL 9
#endif

void func__wlen(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='I';
    ret_dat->value.ival=wlen((WCR*)ret_dat->svalue);
  }
  return;
}

void func__wprnwidth(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='I';
    if((ret_dat->value.ival=wprnwidth((WCR*)ret_dat->svalue))<0){
      rise_error_info_dbg(ECODE_RT__WCHAR_PROCESSING_FAIL,
        "WChar processing error while calculating printable width!",dat_ptr);
    }
  }
  return;
}

void func__wfromstr(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(wfromstr((WCR**)&ret_dat->svalue,ret_dat->svalue)==NULL){
      wmk_fst_buff((WCR**)&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__WCHAR_PROCESSING_FAIL,
        "MultiByte String to WChar String conversion error!",dat_ptr);
    }
  }
  return;
}

void func__wtostr(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    if(wtostr(&ret_dat->svalue,(WCR*)ret_dat->svalue)==NULL){
      mk_fst_buff(&ret_dat->svalue,0);
      rise_error_info_dbg(ECODE_RT__WCHAR_PROCESSING_FAIL,
        "WChar String to MultiByte String conversion error!",dat_ptr);
    }
  }
  return;
}

void func__wcat(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    wcat((WCR**)&ret_dat->svalue,(WCR*)op_b);
  }
  free_string(&op_b);
  return;
}

void func__wcmp(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='I';
    ret_dat->value.ival=wcmp_s((WCR*)ret_dat->svalue,(WCR*)op_b);
  }
  free_string(&op_b);
  return;
}

void func__wspace(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_ival(_RT_CTRL_comma dat_ptr,&ret_dat->value.ival);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    wspace((WCR**)&ret_dat->svalue,ret_dat->value.ival);
  }
  return;
}

void func__wreplicate(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_ival(_RT_CTRL_comma dat_ptr+1,&ret_dat->value.ival);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    wreplicate((WCR**)&ret_dat->svalue,(WCR*)ret_dat->svalue,
      ret_dat->value.ival);
  }
  return;
}

void func__wsubstr(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  SLO op_c;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_ival(_RT_CTRL_comma dat_ptr+1,&ret_dat->value.ival);
  ret_ival(_RT_CTRL_comma dat_ptr+2,&op_c);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    wsubstr((WCR**)&ret_dat->svalue,(WCR*)ret_dat->svalue,ret_dat->value.ival,
      op_c);
  }
  return;
}

void func__wleft(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_ival(_RT_CTRL_comma dat_ptr+1,&ret_dat->value.ival);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    wleft((WCR**)&ret_dat->svalue,(WCR*)ret_dat->svalue,ret_dat->value.ival);
  }
  return;
}

void func__wleftr(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_ival(_RT_CTRL_comma dat_ptr+1,&ret_dat->value.ival);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    wleftr((WCR**)&ret_dat->svalue,(WCR*)ret_dat->svalue,ret_dat->value.ival);
  }
  return;
}

void func__wright(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_ival(_RT_CTRL_comma dat_ptr+1,&ret_dat->value.ival);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    wright((WCR**)&ret_dat->svalue,(WCR*)ret_dat->svalue,ret_dat->value.ival);
  }
  return;
}

void func__wrightl(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_ival(_RT_CTRL_comma dat_ptr+1,&ret_dat->value.ival);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    wrightl((WCR**)&ret_dat->svalue,(WCR*)ret_dat->svalue,ret_dat->value.ival);
  }
  return;
}

void func__wat(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='I';
    ret_dat->value.ival=wat((WCR*)ret_dat->svalue,(WCR*)op_b);
  }
  free_string(&op_b);
  return;
}

void func__wrat(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='I';
    ret_dat->value.ival=wrat((WCR*)ret_dat->svalue,(WCR*)op_b);
  }
  free_string(&op_b);
  return;
}

void func__wstrtran(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  CHR *op_b=NULL,*op_c=NULL;
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_sval(_RT_CTRL_comma dat_ptr+1,&op_b);
  ret_sval(_RT_CTRL_comma dat_ptr+2,&op_c);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    wstrtran((WCR**)&ret_dat->svalue,(WCR*)ret_dat->svalue,(WCR*)op_b,
      (WCR*)op_c);
  }
  free_string(&op_b);
  free_string(&op_c);
  return;
}

void func__wltrim(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    wltrim((WCR**)&ret_dat->svalue,(WCR*)ret_dat->svalue);
  }
  return;
}

void func__wrtrim(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    wrtrim((WCR**)&ret_dat->svalue,(WCR*)ret_dat->svalue);
  }
  return;
}

void func__walltrim(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    walltrim((WCR**)&ret_dat->svalue,(WCR*)ret_dat->svalue);
  }
  return;
}

void func__wpack(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    wpack((WCR**)&ret_dat->svalue,(WCR*)ret_dat->svalue);
  }
  return;
}

void func__wnotempty(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='I';
    ret_dat->value.ival=wnotempty((WCR*)ret_dat->svalue);
  }
  return;
}

void func__whead(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    whead((WCR**)&ret_dat->svalue,(WCR*)ret_dat->svalue);
  }
  return;
}

void func__wtail(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    wtail((WCR**)&ret_dat->svalue,(WCR*)ret_dat->svalue);
  }
  return;
}

void func__wlsp_head(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    wlsp_head((WCR**)&ret_dat->svalue,(WCR*)ret_dat->svalue);
  }
  return;
}

void func__wlsp_tail(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    wlsp_tail((WCR**)&ret_dat->svalue,(WCR*)ret_dat->svalue);
  }
  return;
}

void func__wupper(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    wupper((WCR**)&ret_dat->svalue,(WCR*)ret_dat->svalue);
  }
  return;
}

void func__wlower(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    wlower((WCR**)&ret_dat->svalue,(WCR*)ret_dat->svalue);
  }
  return;
}

void func__wrev(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    wrev((WCR**)&ret_dat->svalue,(WCR*)ret_dat->svalue);
  }
  return;
}

void func__wpadl(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_ival(_RT_CTRL_comma dat_ptr+1,&ret_dat->value.ival);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    wpadl((WCR**)&ret_dat->svalue,(WCR*)ret_dat->svalue,ret_dat->value.ival);
  }
  return;
}

void func__wpadr(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_ival(_RT_CTRL_comma dat_ptr+1,&ret_dat->value.ival);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    wpadr((WCR**)&ret_dat->svalue,(WCR*)ret_dat->svalue,ret_dat->value.ival);
  }
  return;
}

void func__wpadc(_CONST_VOID_PTR_RT_CTRL_comma const ULO *dat_ptr,
   struct fastlisp_data *ret_dat){
  ret_dat->disable_ptr=1;
  ret_sval(_RT_CTRL_comma dat_ptr,&ret_dat->svalue);
  ret_ival(_RT_CTRL_comma dat_ptr+1,&ret_dat->value.ival);
  if(noterror()){
    ret_dat->single=1;
    ret_dat->type='S';
    wpadc((WCR**)&ret_dat->svalue,(WCR*)ret_dat->svalue,ret_dat->value.ival);
  }
  return;
}
/* == WChar String Processing (CFLP-implementation) =========== ENDS HERE == */

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
/* == WChar String Processing (CFLP-implementation) ========= BEGINS HERE == */
  {"WLEN",1,'I',(UCH*)"S",&func__wlen},
  {"WPRNWIDTH",1,'I',(UCH*)"S",&func__wprnwidth},
  {"WFROMSTR",1,'S',(UCH*)"S",&func__wfromstr},
  {"W",1,'S',(UCH*)"S",&func__wfromstr},
  {"WTOSTR",1,'S',(UCH*)"S",&func__wtostr},
  {"WCAT",2,'S',(UCH*)"SS",&func__wcat},
  {"WCMP",2,'I',(UCH*)"SS",&func__wcmp},
  {"WSPACE",1,'S',(UCH*)"I",&func__wspace},
  {"WREPLICATE",2,'S',(UCH*)"SI",&func__wreplicate},
  {"WSUBSTR",3,'S',(UCH*)"SII",&func__wsubstr},
  {"WLEFT",2,'S',(UCH*)"SI",&func__wleft},
  {"WLEFTR",2,'S',(UCH*)"SI",&func__wleftr},
  {"WRIGHT",2,'S',(UCH*)"SI",&func__wright},
  {"WRIGHTL",2,'S',(UCH*)"SI",&func__wrightl},
  {"WAT",2,'I',(UCH*)"SS",&func__wat},
  {"WRAT",2,'I',(UCH*)"SS",&func__wrat},
  {"WSTRTRAN",3,'S',(UCH*)"SSS",&func__wstrtran},
  {"WLTRIM",1,'S',(UCH*)"S",&func__wltrim},
  {"WRTRIM",1,'S',(UCH*)"S",&func__wrtrim},
  {"WALLTRIM",1,'S',(UCH*)"S",&func__walltrim},
  {"WPACK",1,'S',(UCH*)"S",&func__wpack},
  {"WNOTEMPTY",1,'I',(UCH*)"S",&func__wnotempty},
  {"WHEAD",1,'S',(UCH*)"S",&func__whead},
  {"WTAIL",1,'S',(UCH*)"S",&func__wtail},
  {"WLSP_HEAD",1,'S',(UCH*)"S",&func__wlsp_head},
  {"WLSP_TAIL",1,'S',(UCH*)"S",&func__wlsp_tail},
  {"WUPPER",1,'S',(UCH*)"S",&func__wupper},
  {"WLOWER",1,'S',(UCH*)"S",&func__wlower},
  {"WREV",1,'S',(UCH*)"S",&func__wrev},
  {"WPADL",2,'S',(UCH*)"SI",&func__wpadl},
  {"WPADR",2,'S',(UCH*)"SI",&func__wpadr},
  {"WPADC",2,'S',(UCH*)"SI",&func__wpadc}
/* == WChar String Processing (CFLP-implementation) =========== ENDS HERE == */
};
const ULO INSTRUCTIONS=sizeof(INSTRUCTION_SET)/sizeof(INSTRUCTION_STRU);

/* _________________________________________________________________________ */
/* Invocation of Function Main                                     SECTION 3 */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/*
$ export LANG=en_US
$ export LC_ALL=en_US.UTF-8
$ locale
LANG=en_US
LC_CTYPE="en_US.UTF-8"
LC_NUMERIC="en_US.UTF-8"
LC_TIME="en_US.UTF-8"
LC_COLLATE="en_US.UTF-8"
LC_MONETARY="en_US.UTF-8"
LC_MESSAGES="en_US.UTF-8"
LC_PAPER="en_US.UTF-8"
LC_NAME="en_US.UTF-8"
LC_ADDRESS="en_US.UTF-8"
LC_TELEPHONE="en_US.UTF-8"
LC_MEASUREMENT="en_US.UTF-8"
LC_IDENTIFICATION="en_US.UTF-8"
LC_ALL=en_US.UTF-8
*/
#include <locale.h>

extern int _Main_(int argc, char *argv[]);

int main(int argc, char *argv[]){
  setlocale(LC_CTYPE,"");
  return _Main_(argc,argv);
}

#ifdef __cplusplus
} // extern "C"
#endif

