/* dhtpipe.c - Pipeline calculation of the 2D nonseparative Hartley transform

  Direct Hartley transform:

            N-1  M-1              2Pi      2Pi
   H[p,q] =  E    E  x[n,m] cas ( --- pn + --- qm ), p=0..N-1, q=0..M-1.
            n=0  m=0               N        M

  Inverse Hartley transform:

                 1
   Hinv[p,q] =  --- H[p,q].
                N*M
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define UCH unsigned char
#define SCH signed char
#define USH unsigned short int
#define SSH signed short int
#define ULO unsigned long int
#define SLO signed long int
#define DFL double

void generate(DFL *array, SLO n, SLO m){
  SLO i,j;
  for(i=0;i<n;i++)
    for(j=0;j<m;j++)
      *(array+i*m+j)=1.*rand()/RAND_MAX;
  return;
}

void dht(DFL *target_array, SLO n, SLO m, DFL *source_array){
  SLO i,j,p,q;
  DFL pi,c1,s1,sum,tmp;
  pi=3.1415926535897932;
  c1=2*pi/n;
  s1=2*pi/m;
  for(p=0;p<n;p++)
    for(q=0;q<m;q++){
      sum=0;
      for(i=0;i<n;i++)
        for(j=0;j<m;j++)
          sum+=(*(source_array+i*m+j)*(cos(tmp=c1*p*i+s1*q*j)+sin(tmp)));
      *(target_array+p*m+q)=sum;
    }
  return;
}

void idht(DFL *target_array, SLO n, SLO m, DFL *source_array){
  SLO i,j,p,q;
  DFL pi,c1,s1,sum,tmp;
  pi=3.1415926535897932;
  c1=2*pi/n;
  s1=2*pi/m;
  for(p=0;p<n;p++)
    for(q=0;q<m;q++){
      sum=0;
      for(i=0;i<n;i++)
        for(j=0;j<m;j++)
          sum+=(*(source_array+i*m+j)*(cos(tmp=c1*p*i+s1*q*j)+sin(tmp)));
      *(target_array+p*m+q)=sum/n/m;
    }
  return;
}

UCH compare(DFL *array0, DFL *array1, SLO n, SLO m){
  UCH result=1;
  if((fabs(*array0-*array1)>1.e-10)
     ||(fabs(*(array0+n*m-1)-*(array1+n*m-1))>1.e-10))
    result=0;
  return result;
}

int main(void){
  UCH cmp_res;
  SLO i,m,n,numb;
  DFL *inp_array_addr,*dht_array_addr,*idht_array_addr;
  printf(
    "Pipeline calculation of the 2D nonseparative Hartley transform.\n\n");

  printf("M-value of M*N-matrix: ");
  // scanf("%ld",&m);
  m=32; printf("%ld\n",m);

  printf("N-value of M*N-matrix: ");
  // scanf("%ld",&n);
  n=32; printf("%ld\n",n);

  printf("How many input data packs: ");
  // scanf("%ld",&numb);
  numb=10; printf("%ld\n",numb);

  for(i=1;i<=numb;i++){
    printf("Sequence %ld:",i);
    fflush(stdout);
    inp_array_addr=(DFL*)malloc(m*n*sizeof(DFL));
    generate(inp_array_addr,n,m);
    dht_array_addr=(DFL*)malloc(m*n*sizeof(DFL));
    dht(dht_array_addr,n,m,inp_array_addr);
    idht_array_addr=(DFL*)malloc(m*n*sizeof(DFL));
    idht(idht_array_addr,n,m,dht_array_addr);
    cmp_res=compare(inp_array_addr,idht_array_addr,n,m);
    printf(" %s.\n",cmp_res?"Ok":"Fail");
    free((void*)inp_array_addr);
    free((void*)dht_array_addr);
    free((void*)idht_array_addr);
  }
  exit(0);
}

