#ifndef PGM_UTIL_SOM_H
#define PGM_UTIL_SOM_H

struct node
{	double *weight;		/* position of the node in input space */
	int *pos;		/* position of the node in the network */
	int num_hits;		/* #patterns for which this node was a winner */
};

struct input_s
{	double *input;		/* input vector */
};

struct network
{	/* The structure */
	int indim;		/* # input dimensions */
	int netdim;		/* # network dimensions */
	int totalnodes;		/* Superfluous: calculated from the
				   num_nodes array */
	int *num_nodes;		/* #nodes per network dimension */
	struct node *nodes;	/* Linear storage for the nodes. */

	/* The training parameters */
	int Dstart, Dend;
	int iter_start, iter_end;
	double eta_start, eta_end;
	int conscience;		/* Is the conscience mech. active? */

	/* The inputs */
	int num_inputs;		/* Number of inputs */
	struct input_s *inputs;	/* Storage for input vectors */
};


void trainnetwork(struct network *p), initnetwork(struct network *p);
void initrandom(void);
double get_rand(double low, double high);
double get_euclidean(double *x, double *y, int n);
int find_best_match(struct network * p,struct input_s in), are_neighbors(struct node x,struct node y,int n,int D);
void update_neighborhood(struct network *p, int best,int D, struct input_s in, double eta), clear_hits(struct network *p);
struct network *getnetwork(void);
void deallocate_network(struct network *p);
void insert(struct network *p, int *heading,int *other_dim,int num_head,int num_other,int start);
void run(struct network *p,PGM_Matriz_Double *out);
PGM_Matriz_Double *run_SOM(struct network *p);

#endif
