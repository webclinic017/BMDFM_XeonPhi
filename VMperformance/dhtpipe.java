/* dhtpipe.java - Pipeline calculation of the 2D nonseparative Hartley
                  transform

  Direct Hartley transform:

            N-1  M-1              2Pi      2Pi
   H[p,q] =  E    E  x[n,m] cas ( --- pn + --- qm ), p=0..N-1, q=0..M-1.
            n=0  m=0               N        M

  Inverse Hartley transform:

                 1
   Hinv[p,q] =  --- H[p,q].
                N*M
*/

import java.lang.Math;
import java.util.Random;

public class dhtpipe{

  public static void generate(double array[], int n, int m){
    int i,j;
    Random r=new Random();
    for(i=0;i<n;i++){
      for(j=0;j<m;j++){
        array[i*m+j]=r.nextDouble();
      }
    }
    return;
  }

  public static void dht(double target_array[], int n, int m,
     double source_array[]){
    int i,j,p,q;
    double pi,c1,s1,sum,tmp;
    pi=3.1415926535897932;
    c1=2*pi/n;
    s1=2*pi/m;
    for(p=0;p<n;p++){
      for(q=0;q<m;q++){
        sum=0;
        for(i=0;i<n;i++){
          for(j=0;j<m;j++){
            sum+=(source_array[i*m+j]*(Math.cos(tmp=c1*p*i+s1*q*j)
              +Math.sin(tmp)));
          }
        }
        target_array[p*m+q]=sum;
      }
    }
    return;
  }

  public static void idht(double target_array[], int n, int m,
     double source_array[]){
    int i,j,p,q;
    double pi,c1,s1,sum,tmp;
    pi=3.1415926535897932;
    c1=2*pi/n;
    s1=2*pi/m;
    for(p=0;p<n;p++){
      for(q=0;q<m;q++){
        sum=0;
        for(i=0;i<n;i++){
          for(j=0;j<m;j++){
            sum+=(source_array[i*m+j]*(Math.cos(tmp=c1*p*i+s1*q*j)
              +Math.sin(tmp)));
          }
        }
        target_array[p*m+q]=sum/n/m;
      }
    }
    return;
  }

  public static boolean compare(double array0[], double array1[], int n,
     int m){
    boolean result=true;
    if((Math.abs(array0[0]-array1[0])>1.e-10)
       ||(Math.abs(array0[n*m-1]-array1[n*m-1])>1.e-10)){
      result=false;
    }
    return result;
  }

  public static void main(String[] args){
    boolean cmp_res;
    int i,m,n,numb;
    double inp_array_addr[],dht_array_addr[],idht_array_addr[];
    System.out.println(
      "Pipeline calculation of the 2D nonseparative Hartley transform.\n");

    System.out.print("M-value of M*N-matrix: ");
    // m = Integer.parseInt(System.in.readln());
    m=32; System.out.println(""+m);

    System.out.print("N-value of M*N-matrix: ");
    // n = Integer.parseInt(System.in.readln());
    n=32; System.out.println(""+n);

    System.out.print("How many input data packs: ");
    // numb = Integer.parseInt(System.in.readln());
    numb=10; System.out.println(""+numb);

    for(i=1;i<=numb;i++){
      System.out.print("Sequence "+i+": ");
      System.out.flush();
      inp_array_addr=new double[m*n];
      generate(inp_array_addr,n,m);
      dht_array_addr=new double[m*n];
      dht(dht_array_addr,n,m,inp_array_addr);
      idht_array_addr=new double[m*n];
      idht(idht_array_addr,n,m,dht_array_addr);
      cmp_res=compare(inp_array_addr,idht_array_addr,n,m);
      System.out.println(cmp_res?"Ok":"Fail");
    }
    return;
  }
}

