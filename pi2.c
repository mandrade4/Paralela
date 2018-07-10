#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#define MIN(a,b) ((a)<(b)?(a):(b)
#define MILLION 1000000L
void usuage(char*);
int main (int argc, char *argv[]) 
{	
int	 count;				/* contador de numeros primos local */
double elapsed_time;		/* Tiempo de ejecucion */
int  first;				/* index del primer multiplo */
int  global_count;		/* Contador global de primos */
int  high_value;		/* Valor alto del procesador */
int  i;
int  id;				/* ID number del procesador */
int  index;				/* Index del numero primo actual */
int  low_value;			/* Valor bajo del procesador */
char *marked;			/* numeros de 2..,'n' */
int  n;					/* Sieving from 2, ..., 'n' */
int  p;					/* Numero de procesadores */
int  prime;				/* Numero primo */
int  size;				/* Elemento en 'marked' */
int *fprimes;			/* imprime los primeros 5 primos*/
int *lprimes;			/* Imprime los ultimos 5 primos*/
int j;
struct timeval start;
struct timeval end;
	/* Initializa MPI */
if (MPI_Init (&argc, &argv) || MPI_Comm_rank (MPI_COMM_WORLD, &id) || MPI_Comm_size (MPI_COMM_WORLD, &p))
{
if (!id)
printf("Falla al inicializar MPI.\n");
MPI_Finalize();
exit(1);
}
/* inicia el timer */
MPI_Barrier(MPI_COMM_WORLD);
gettimeofday(&start, NULL);
	/* Revisa la entrada */
if (argc != 2)
{
		if (!id) usuage(argv[0]);
		MPI_Finalize();
		exit (1);
	}
	n = atoi(argv[1]);
	if (n < 2)
{
		printf ("n debe ser mas grande que  1.\n");
		exit(1);
	}
/* Particion de datos por procesador */
	size = n/p;
	low_value = (size*id);
	high_value = size*(id+1);
	printf("%d De %d\n",id,p);
	printf("Lv %d y Hv %d\n", low_value, high_value); 
	/*Datos del ultimo procesador */
	if (id == p-1) {
		high_value = n+1;
		size = high_value - (low_value + 1);
		printf("Ultimo P  Lv %d y Hv %d\n", low_value, high_value);
	}
	/* reserva memoria para el arreglo */
	marked = (char *) malloc (size);
	if (marked == NULL) {
		printf ("No se puede asignar suficiente memoria.\n");
		MPI_Finalize();
		exit (1);
	}
	/*Inicializa array marked*/
	for (i = 0; i < size; i++) marked[i] = 0;
	if (!id) index = 0;
	if (!id) marked[0] = marked[1]= 1; /*0 a 1  marked*/
	prime = 2;
	do {
		if (prime * prime > low_value){
			first = prime * prime - low_value;
			}
		else {
			if (!(low_value % prime)) first = 0;
			else {first = prime - (low_value % prime);}
		}
		/*Marks off prime*x*/
		for (i = first; i < size; i += prime) marked[i] = 1;
		if (!id) {
			while (marked[++index]);
			/*Next primes is the non marked index*/
			prime = index;
		}
		if (p > 1) MPI_Bcast (&prime,  1, MPI_INT, 0, MPI_COMM_WORLD);
	} while (prime * prime <= n);
	/*Count the primes for each process*/
	count = 0;
	for (i = 0; i < size; i++) {
		if (!marked[i]) count++;
	}
	/*Junta los datos de los procesadores*/
	if (p > 1) {
		MPI_Reduce (&count, &global_count, 1, MPI_INT, MPI_SUM,0, MPI_COMM_WORLD);
	} else {
			  global_count = count;
	}
	/* Stop the timer */
	gettimeofday(&end, NULL);
	elapsed_time = (end.tv_sec - start.tv_sec + (double)(end.tv_usec - start.tv_usec)/MILLION);
	/* Print the results */
	/* Imprime los primeros y ultimos 5 numeros primos de cada procesador */
	fprimes = (int *)malloc(sizeof(int) * 5);
	lprimes = (int *)malloc(sizeof(int) * 5);
	for (i = 0; i < 5; i++) {
		fprimes[i] = 0;
		lprimes[i] = 0;
	}
	j = 0;
	for (i = 0; i < size; i++) {
		if (!marked[i]) {
			fprimes[j] = i + low_value;
                       j++;
		}
		if (j > 4) {
			break;
		}
	}
	j = 0;
	for (i = size - 1; i >= 0; i--) {
		if (!marked[i]) {
			lprimes[j] = i + low_value; 
				j++;
		}
		if (j > 4) {
			break;
		}
	}
	printf ("%d: %d %d %d %d %d %d %d %d %d %d\n", id, fprimes[0], fprimes[1], fprimes[2], fprimes[3], fprimes[4], lprimes[4], lprimes[3], lprimes[2], lprimes[1], lprimes[0]);
	MPI_Barrier(MPI_COMM_WORLD);
	free(fprimes);
	free(lprimes);
	if (!id) {
		printf ("Hay %d Primos menores o iguales a %d\n", global_count, n);
	}
	if (!id) {
       		printf("Elapsed Time=%lf\n", elapsed_time);
	}
	free(marked);
	MPI_Finalize ();
	return 0;
}
void usuage(char *p_name) {
	printf ("Usuage: %s n\n\n", p_name);
	printf ("Calculates the number of prime numbers up to n.\n");
}
