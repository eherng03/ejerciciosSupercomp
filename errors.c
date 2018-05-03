#include <stdio.h>
#include <math.h> 

#ifdef _OPENMP
#include <omp.h>
#endif

#define N 100
 
float work(int a)
{
  return a+1;
}


int main(void)
{
  float a[N], b[N], x, f, sum, sum_expected, psum;
  int i, chunk;
  int num_threads;

  /* Example 1 (this example contains an OpenMP parallelization error) */
  /* --------- */

  a[0] = 0;
# pragma ordered parallel for
  for (i=1; i<N; i++)
  {
    a[i] = 2.0*i*(i-1);
    b[i] = a[i] - a[i-1];
  } /* end of omp parallel for */ 

  /* testing the correctness of the numerical result: */
  sum=0; for (i=1; i<N; i++) { sum = sum + b[i]; }
  sum_expected = a[N-1]-a[0];
  printf("Exa.1: sum  computed=%8.1f,  expected=%8.1f,  difference=%8.5f \n", 
                                 sum,  sum_expected, sum-sum_expected); 


  /* Example 2 (this example contains an OpenMP parallelization error) */
  /* --------- */
/*Solution: private (a)*/

  a[0] = 0;
# pragma omp parallel
  { 
#   pragma omp for
    for (i=1; i<N; i++)
    {
      a[i] = 3.0*i*(i+1);
    } /* end of omp for nowait */ 
#   pragma omp for private (i)
    for (i=1; i<N; i++)
    {
      b[i] = a[i] - a[i-1];
    } /* end of omp for */ 
  } /* end of omp parallel */ 

  /* testing the correctness of the numerical result: */
  sum=0; for (i=1; i<N; i++) { sum = sum + b[i]; }
  sum_expected = a[N-1]-a[0];
  printf("Exa.2: sum  computed=%8.1f,  expected=%8.1f,  difference=%8.5f \n", 
                                 sum,  sum_expected, sum-sum_expected); 


  /* Example 3 (this example contains an OpenMP parallelization error) */
  /* --------- */

# pragma ordered parallel for
  for (i=1; i<N; i++)
  {
    x = sqrt(b[i]) - 1;
    a[i] = x*x + 2*x + 1;
  } /* end of omp parallel for */ 

  /* testing the correctness of the numerical result: */
  sum=0; for (i=1; i<N; i++) { sum = sum + a[i]; }
  /* sum_expected = same as in Exa.2 */
  printf("Exa.3: sum  computed=%8.1f,  expected=%8.1f,  difference=%8.5f \n", 
                                 sum,  sum_expected, sum-sum_expected); 

  /* Example 4 (this example contains an OpenMP parallelization error) */
  /* --------- */

  f = 2;
# pragma omp parallel for lastprivate(x) firstprivate(f)
  for (i=1; i<N; i++)
  {
    x = f * b[i];
    a[i] = x - 7;
  } /* end of omp parallel for */ 
  a[0] = x; 

  /* testing the correctness of the numerical result: */
  printf("Exa.4: a[0] computed=%8.1f,  expected=%8.1f,  difference=%8.5f \n", 
                                a[0],        2*b[N-1],   a[0] - 2*b[N-1] );


  /* Example 5 (this example contains an OpenMP parallelization error) */
  /* ----SOLUTION: reduction(+:sum)----- */

  sum = 0; 
# pragma omp parallel for reduction(+:sum)
  for (i=1; i<N; i++)
  {
    sum = sum + b[i];
  } /* end of omp parallel for */ 
  /* testing the correctness of the numerical result: */
  /* sum_expected = same as in Exa.2 */
  printf("Exa.5: sum  computed=%8.1f,  expected=%8.1f,  difference=%8.5f \n", 
                                 sum,  sum_expected, sum-sum_expected); 


  /* Example 6 (this example is correct, but the Intel Thread Checker will */
  /* ---------  report a run time analysis "Caution": */
 
  /* Run time analysis report:
      Severity:     Caution 
      Short descr.: OpenMP -- cannot be private.
      Description:  OpenMP -- The access  at "psum = psum + b[i]" cannot be 
                    private because it expects the value previously defined 
                    at "psum = psum + b[i]" in the serial execution.
      Meaning:      The read access "psum+" at "psum = psum + b[i]" cannot 
                    be private because it expects the value previously 
                    defined at "psum=" in previous iteration of this loop,
                    and this write-access may be in another thread.            
  */ 
/* Solution: sin cambios, esta bien.*/

  sum = 0; 
# pragma omp parallel private(psum)
  { 
    psum = 0; 
#   pragma omp for
      for (i=1; i<N; i++)
      {
        psum = psum + b[i]; //Da warning porque psum puede estar ejecutandose en dos hilos
      } /* end of omp for */ 
#   pragma omp critical
    { 
      sum = sum + psum;
    } /* end of omp critical */ 
  } /* end of omp parallel */ 

  /* testing the correctness of the numerical result: */
  /* sum_expected = same as in Exa.2 */
  printf("Exa.6: sum  computed=%8.1f,  expected=%8.1f,  difference=%8.5f \n", 
                                 sum,  sum_expected, sum-sum_expected); 


  /* Example 7 (this example is correct, but the Intel Thread Checker will */
  /* ---------  report a "compile time warning", a "run time error"        */
  /*            and same "Caution" as in Example 6) */

  /* Compile time warning: 
       Variable "num_threads" in OpenMP schedule clause should appear on shared list,
       pragma omp parallel private(psum, num_threads).
     In C/C++, by using an automatic variable "int num_threads" defined 
       at the beginning of the block of the parallel region,
       this warning will not appear. 
     Run time error: 
       The Intel Thread Checker beta-version contains the following bug:
       omp_get_num_threads() returns 2 although only 1 thread is used.
  */ 
/* Solution: declarar num_threads, y que no sea privada, es la misma en 
todos los hilos*/
 

 b[0] = 0; 
  sum = 0; 
# pragma omp parallel private(psum)
  { int num_threads;
#   ifdef _OPENMP
      num_threads=omp_get_num_threads();
#	if (_TCHECK == 200104)
		num_threads=1; /*Correcting bug in the Intel Thread Checker*/
#	endif
#   else
      num_threads=1;
#   endif
    psum = 0; 
#   pragma omp for schedule(static,(N-1)/num_threads+1)
      for (i=0; i<N; i++)
      {
        psum = psum + b[i];
      } /* end of omp for */ 
#   pragma omp for ordered schedule(static,1)
      for (i=0;i<num_threads;i++)
      {
#       pragma omp ordered
          sum = sum + psum;
      } /* end of omp for */
  } /* end of omp parallel */ 

  /* testing the correctness of the numerical result: */
  /* sum_expected = same as in Exa.2 */
  printf("Exa.7: sum  computed=%8.1f,  expected=%8.1f,  difference=%8.5f \n", 
                                 sum,  sum_expected, sum-sum_expected); 

}
