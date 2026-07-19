#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#define SQUARE(a) ((a)*(a))
#define MAXX 10000
#define MINX 0

typedef struct polygon_s {
	int size;
	double ** m;
	double ** d;
	double * x;
	double * y;
} polygon_t;

void process_all(polygon_t *);
polygon_t * allocate_polygon_data(int);
int read_polygon(polygon_t **);
double * allocate_vector(int, char);
double ** allocate_square_matrix(int, char);
void init(polygon_t *);
void free_polygon(polygon_t *);

void process_all(polygon_t * p) {
	int span, a, b, i_offset, j_offset, i, j;
	double temp;

	/*
	 * Calcula primeiro os intervalos menores. Quando m[a][b] for calculado,
	 * todos os subproblemas já estarão prontos.
	 * A equipe de threads é criada uma única vez
	 */
	#pragma omp parallel private(span, b, i_offset, j_offset, i, j, temp)
	{
		for (span = 3; span < p->size; span += 2) {
			/*
			 * Para o mesmo span, cada valor de a calcula uma célula diferente.
			 * A barreira implícita garante que o span termine antes do próximo.
			 */
			#pragma omp for schedule(static)
			for (a = 0; a < p->size; a++) {
				b = (a + span) % p->size;
				p->m[a][b] = (2 * (MAXX - MINX) * p->size);

				for (i_offset = 1; i_offset < span; i_offset += 2) {
					i = (a + i_offset) % p->size;

					for (j_offset = i_offset + 1; j_offset < span;
							j_offset += 2) {
						j = (a + j_offset) % p->size;
						temp = p->m[a][i] + p->m[i][j] + p->m[j][b]
								+ p->d[a][i] + p->d[i][j] + p->d[j][b];

						if (p->m[a][b] > temp)
							p->m[a][b] = temp;
					}
				}
			}
		}
	}
}

double * allocate_vector(int size, char name) {
	double * v;
	if ((v = (double *) malloc(sizeof(double) * size)) == NULL ) {
		fprintf(stderr, "Error allocating memory for polygon data (%c). \n",
				name);
		exit(1);
	}
	return v;
}

double ** allocate_square_matrix(int size, char name) {
	double ** m;
	if ((m = (double **) malloc(sizeof(double *) * size)) == NULL ) {
		fprintf(stderr, "Error allocating memory for polygon data (%c). \n",
				name);
		exit(1);
	}
	int i;
	for (i = 0; i < size; i++) {
		if ((m[i] = (double *) malloc(sizeof(double) * size)) == NULL ) {
			fprintf(stderr, "Error allocating memory for polygon data (%c). \n",
					name);
			exit(1);
		}
	}
	return m;
}

polygon_t * allocate_polygon_data(int n) {
	polygon_t * p;
	if ((p = (polygon_t *) malloc(sizeof(polygon_t))) == NULL ) {
		fprintf(stderr, "Error allocating memory for polygon data. \n");
		exit(1);
	}
	int size = 2 * n;
	p->x = allocate_vector(size, 'x');
	p->y = allocate_vector(size, 'y');
	p->m = allocate_square_matrix(size, 'm');
	p->d = allocate_square_matrix(size, 'd');
	p->size = size;
	return p;
}

int read_polygon(polygon_t ** p) {

	int n, i, status = 0;
	if ((status = scanf("%d", &n)) == 1) {
		*p = allocate_polygon_data(n);
		n *= 2;

		for (i = 0; i < n; i++) {
			scanf("%lf %lf", &((*p)->x[i]), &((*p)->y[i]));
		}
	}
	return (status > 0);
}

void init(polygon_t * p) {
	int i, j;

	/*
	 * Cada par (i, j) escreve em uma posição diferente das matrizes.
	 * Pode ser paralelizado
	 */
	#pragma omp parallel for collapse(2) schedule(static)
	for (i = 0; i < p->size; i++) {
		for (j = 0; j < p->size; j++) {
			if ((i + 1) % p->size == j) {
				p->d[i][j] = 0;
				p->m[i][j] = 0;
			} else {
				p->d[i][j] = sqrt(
						SQUARE((p->x[i])-(p->x[j]))
								+ SQUARE((p->y[i])-(p->y[j])));
				p->m[i][j] = -1;
			}
		}
	}

}

void free_polygon(polygon_t *p) {
	free(p->x);
	free(p->y);
	int i;
	for (i = 0; i < (p->size); i++) {
		free(p->d[i]);
		free(p->m[i]);
	}
	free(p->d);
	free(p->m);
	free(p);
}

int main() {
	int i;
	double smaller, temp;
	polygon_t * p;

	while (read_polygon(&p)) {
		init(p);
		process_all(p);

		smaller = (2 * (MAXX - MINX) * (p->size));

		for (i = 0; i < p->size; i++) {
			temp = p->m[(i + 3) % p->size][i]
					+ p->d[(i + 3) % p->size][i];
			if (temp < smaller)
				smaller = temp;
		}

		free_polygon(p);

		printf("%.4f\n", smaller);
	}

	return 0;
}
