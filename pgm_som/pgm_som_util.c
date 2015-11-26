/*******
* This program is used for training a self-organizing map. It receives its inputs either in
* input files, or interactively, from the user. The outputs (the weights and topological
* connections) are written to an output file. usage:
* a) kohonen
* 		or
* b) kohonen input_file data_file output_file
*
* When format a) is used, the program prompts the user for the names of the data and weights
* file, and then proceeds to prompt the user on the structural parameters of the
* self-organizing map.
*******/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#define READFILE 0
#define WRITEFILE 1
#define MAXRAND (1<<16)
#define MAXWIN 2.0

#define _exit(a) exit(a)

#include "postgres.h"        //standard
#include "fmgr.h"            //standard
#include "utils/memutils.h"  //standard
#include "utils/geo_decls.h" //standard
#include "utils/errcodes.h"  //warning

#include "../Util/pgm_matriz.h"
#include "../Util/pgm_matriz_util.h"
#include "../Util/pgm_malloc.h"
#include "pgm_som_util.h"



/* Initialize random generator with number of microseconds since
   January 1, 1970, which is almost a random value in itself.
*/
void initrandom()
{	struct timeval tp;
	struct timezone tzp;
	if( gettimeofday(&tp,&tzp) == -1 )
	{	printf("Could not initialize random number generator\n");
		_exit(0);
	}
	srandom((int)tp.tv_usec);
}


/* Returns a double precision random number between low and high
*/
double get_rand(double low, double high)
{	double x = random() % MAXRAND / (double)MAXRAND;
	return( x*(high-low) + low );
}

/* Clear the number of hits for each node in the network */
void clear_hits(struct network *p)
{	int i;

	for(i=0; i<p->totalnodes; i++)
	    p->nodes[i].num_hits = 0;
}


/* Finds the euclidean distance between 2 vectors of doubles (x & y) both of size n
*/
double get_euclidean(double *x, double *y, int n)
{	double dist = 0;
	int i;

	for( i=0; i<n; i++ )
	    dist += (x[i]-y[i]) * (x[i]-y[i]);

	return(dist);
}


/* Returns the best matching node in p to the input in
*/
int find_best_match(struct network * p,struct input_s in)
{	int best = 0;
	double mindistance, distance;
	int i, first, ration;

	first = 1;
	ration = MAXWIN * (p->num_inputs / (double) p->totalnodes);

	for( i=0; i<p->totalnodes; i++){
        if( !p->conscience || p->nodes[i].num_hits < ration ){
	    	distance = get_euclidean(p->nodes[i].weight, in.input, p->indim);
	    	if( first || distance < mindistance ){
                mindistance = distance;
                best = i;
                first = 0;
            }
	    }
	}
	return(best);
}


/* determines whether nodes x and y are neighbors on a distance <= D. Network
   dimension is n.
*/
int are_neighbors(struct node x,struct node y,int n,int D)
{	int i;
	int distance = 0;

	for( i=0; i<n; i++ )
	    if( x.pos[i] > y.pos[i] )
		distance += x.pos[i] - y.pos[i];
	    else
		distance += y.pos[i] - x.pos[i];

	return(distance <= D);
}


/* Update the neighborhood of node "best", according to input "in" and learning
   rate "eta". Neighborhood is of size "D".
*/
void update_neighborhood(struct network *p, int best,int D, struct input_s in, double eta)
{	int i, j;

	for( i=0; i<p->totalnodes; i++)
	    if( are_neighbors(p->nodes[i], p->nodes[best], p->netdim, D) )
		for( j=0; j<p->indim; j++)
		    p->nodes[i].weight[j] += eta *
					     (in.input[j] - p->nodes[i].weight[j]);

}


/* The SOM training algorithm. The #iterations and learning rate (eta) are linearly
   decreased for each decrease of the neighborhood. For #iterations, and for each
   input (in that iteration), the best match is found, and the neighborhood is
   updated.
*/
void trainnetwork(struct network *p)
{	int D=p->Dstart;
	int iter;
	int iter_interval = p->iter_end - p->iter_start;
	double eta, eta_interval, fraction;
	int i, j, best;

	eta_interval = p->eta_end - p->eta_start;
	if(  p->Dstart != p->Dend )
	    fraction = 1 / (double)(p->Dstart - p->Dend);
	else fraction = 0;

	while( D >= p->Dend )
	{   iter = p->iter_start + (p->Dstart - D) * fraction * iter_interval;
	    eta = p->eta_start + (p->Dstart - D) * fraction * eta_interval;
	    for( i=0; i<iter; i++)
	    {	if( p->conscience )
		    clear_hits(p);
		for( j=0; j<p->num_inputs; j++)
		{   best = find_best_match(p, p->inputs[j]);
		    update_neighborhood(p, best, D, p->inputs[j], eta);
		    p->nodes[best].num_hits++;
		}
	    }
	    D--;
	}
}


/* A recursive function for insertion of node positions in the network. Each
inserted node will have the heading "heading", which is long "num_head". A few
dimensions remain to be inserted, and are kept in "other_dim", with length
"num_other". The insertion starts at point "start" in the linear array of nodes.
*/
void insert(struct network *p, int *heading,int *other_dim,int num_head,int num_other,int start)
{	int i, len, size;

	if( num_other == 0 )
	    for( i=0; i<num_head; i++)
		p->nodes[start].pos[i] = heading[i];
	else
	{   len = 1;
	    for( i=1; i<num_other; i++)
		len *= other_dim[i];
	    size = other_dim[0];
	    for( i=0; i<size; i++)
	    {	heading[num_head] = i;
		insert(p, heading, other_dim+1, num_head+1, num_other-1,
						start + i*len);
	    }
	    heading[num_head] = size;
	}
}


/* Initialization of the network. Assignment of position and random weights
   to each node in the network. Weights are chosen so that all the weights in one
   dimension are between the maximum and minimum value of the inputs in that
   dimension.
*/
void initnetwork(struct network *p)
{	int i,j;
	double *min, *max;

	min = (double *)pgm_malloc(p->indim*sizeof(double));
	max = (double *)pgm_malloc(p->indim*sizeof(double));
	if(min == NULL || max == NULL)
	{   deallocate_network(p);
	    printf("Out of memory\n");
	    _exit(0);
	}

	insert(p, p->num_nodes, p->num_nodes, 0, p->netdim, 0);

	/* For each input dimension, find the min. and max. value, and assign all
	   weights from that input node between those two values.
	*/
	for(i=0; i<p->indim; i++)
	{   min[i] = max[i] = p->inputs[0].input[i];
	    for( j=0; j<p->num_inputs; j++)
		if( p->inputs[j].input[i] > max[i] )
		    max[i] = p->inputs[j].input[i];
		else if( p->inputs[j].input[i] < min[i] )
		    min[i] = p->inputs[j].input[i];
	    /* Otherwise, the get_rand would divide by zero */
	    if( min[i] == max[i] )
		max[i] = min[i] * 1.1;
	}


	for( i=0; i<p->totalnodes; i++)
	    for( j=0; j<p->indim; j++)
		p->nodes[i].weight[j] = get_rand(min[j], max[j]);
}

/* Allocates the structure network, initializes all pointers to NULL, and all
   integers to 0
*/
struct network *getnetwork()
{
	struct network *p;

	p = (struct network *)pgm_malloc(sizeof(struct network));
	if( p == NULL )
		return(NULL);
	p->num_nodes = NULL;
	p->nodes = NULL;
	p->inputs = NULL;
	p->indim = p->netdim = p->totalnodes = p->num_inputs = 0;
	p->Dstart = p->Dend = p->iter_start = p->iter_end = 0;
	p->eta_start = p->eta_end = 0.0;
	return p;
}


/* Deallocates all already allocated arrays and structures in the network
*/
void deallocate_network(struct network *p)
{	int i;

	if( p->num_nodes )
	    pgm_free( p->num_nodes );

	if( p->nodes )
	{   for( i=0; i<p->totalnodes; i++)
	    {	if( p->nodes[i].weight )
		    pgm_free( p->nodes[i].weight );
		if( p->nodes[i].pos )
		    pgm_free( p->nodes[i].pos);
	    }
	    pgm_free( p->nodes);
	}

	if( p->inputs )
	{   for( i=0; i<p->num_inputs; i++)
		if( p->inputs[i].input )
		    pgm_free( p->inputs[i].input );
	    pgm_free( p->inputs);
	}

	pgm_free(p);
}


void run(struct network *p,PGM_Matriz_Double *out){

    int i,j;
	for( i=0; i<p->totalnodes; i++)
	    for( j=0; j<p->indim; j++)
            PGM_ELEM_MATRIZ(out,i,j) = p->nodes[i].weight[j];
}

PGM_Matriz_Double *run_SOM(struct network *p){

    PGM_Matriz_Double *out = pgm_create_matrix_double(p->totalnodes,p->indim);

	initrandom();
	initnetwork(p);
	trainnetwork(p);
	run(p,out);
    return out;
}

