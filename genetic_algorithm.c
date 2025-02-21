/***************************************************************/
/* This is a simple genetic algorithm implementation where the */
/* evaluation function takes positive values only and the      */
/* fitness of an individual is the same as the value of the    */
/* objective function                                          */
/***************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* Change any of these parameters to match your needs */

#define POPSIZE 50     /* population size */
#define MAXGENS 1000   /* max. number of generations */
#define NVARS 3        /* no. of problem variables */
#define PXOVER 0.8     /* probability of crossover */
#define PMUTATION 0.15 /* probability of mutation */

size_t generation; /* current generation no. */
size_t cur_best;   /* best individual */
FILE *galog;       /* an output file */

struct genotype {        /* genotype (GT), a member of the population */
    double gene[NVARS];  /* a string of variables */
    double fitness;      /* GT's fitness */
    double upper[NVARS]; /* GT's variables upper bound */
    double lower[NVARS]; /* GT's variables lower bound */
    double rfitness;     /* relative fitness */
    double cfitness;     /* cumulative fitness */
};

struct genotype population[POPSIZE + 1];    /* population */
struct genotype newpopulation[POPSIZE + 1]; /* new population; */
                                            /* replaces the */
                                            /* old generation */

/* Declaration of procedures used by this genetic algorithm */

static void __initialize(void);
static double __randval(double, double);
static void __evaluate(void);
static void __keep_the_best(void);
static void __elitist(void);
static void __select(void);
static void __crossover(void);
static void __Xover(int, int);
static void __swap(double *, double *);
static void __mutate(void);
static void __report(void);

/***************************************************************/
/* Initialization function: Initializes the values of genes    */
/* within the variables bounds. It also initializes (to zero)  */
/* all fitness values for each member of the population. It    */
/* reads upper and lower bounds of each variable from the      */
/* input file 'gadata.txt'. It randomly generates values       */
/* between these bounds for each gene of each genotype in the  */
/* population. The format of the input file 'gadata.txt' is    */
/* var1_lower_bound var1_upper bound                           */
/* var2_lower_bound var2_upper bound                           */
/***************************************************************/

static void __initialize(void)
{
    FILE *infile;
    size_t i, j;
    double lbound, ubound;

    if ((infile = fopen("gadata.txt", "r")) == NULL) {
        fprintf(galog, "\nCannot open input file!\n");
        exit(1);
    }

    /* initialize variables within the bounds */

    for (i = 0; i < NVARS; i++) {
        fscanf(infile, "%lf", &lbound);
        fscanf(infile, "%lf", &ubound);
        for (j = 0; j < POPSIZE; j++) {
            population[j].fitness = 0;
            population[j].rfitness = 0;
            population[j].cfitness = 0;
            population[j].lower[i] = lbound;
            population[j].upper[i] = ubound;
            population[j].gene[i]
                = __randval(population[j].lower[i], population[j].upper[i]);
        }
    }

    fclose(infile);
}

/***********************************************************/
/* Random value generator: Generates a value within bounds */
/***********************************************************/

static double __randval(double low, double high)
{
    double val;

    val = ((double)(rand() % 1000) / 1000.0) * (high - low) + low;

    return (val);
}

/*************************************************************/
/* Evaluation function: This takes a user defined function.  */
/* Each time this is changed, the code has to be recompiled. */
/* The current function is:  x[1]~2-x[1]*x[2]+x[3]           */
/*************************************************************/

static void __evaluate(void)
{
    int mem;
    size_t i;
    double x[NVARS + 1];

    for (mem = 0; mem < POPSIZE; mem++) {
        for (i = 0; i < NVARS; i++)
            x[i + 1] = population[mem].gene[i];

        population[mem].fitness = (x[1] * x[1]) - (x[1] * x[2]) + x[3];
    }
}

/***************************************************************/
/* Keep_the_best function: This function keeps track of the    */
/* best member of the population. Note that the last entry in  */
/* the array Population holds a copy of the best individual    */
/***************************************************************/

static void __keep_the_best(void)
{
    int mem;
    size_t i;

    cur_best = 0; /* stores the index of the best individual */
    for (mem = 0; mem < POPSIZE; mem++) {
        if (population[mem].fitness > population[POPSIZE].fitness) {
            cur_best = mem;
            population[POPSIZE].fitness = population[mem].fitness;
        }
    }

    /* once the best member in the population is found, copy the genes */
    for (i = 0; i < NVARS; i++)
        population[POPSIZE].gene[i] = population[cur_best].gene[i];
}

/****************************************************************/
/* Elitist function: The best member of the previous generation */
/* is stored as the last in the array. If the best member of    */
/* the current generation is worse then the best member of the  */
/* previous generation, the latter one would replace the worst  */
/* member of the current population                             */
/****************************************************************/

static void __elitist(void)
{
    size_t i;
    double best, worst;      /* best and worst fitness values */
    int best_mem, worst_mem; /* indexes of the best and worst member */

    best = population[0].fitness;
    worst = population[0].fitness;
    for (i = 0; i < POPSIZE - 1; ++i) {
        if (population[i].fitness > population[i + 1].fitness) {
            if (population[i].fitness >= best) {
                best = population[i].fitness;
                best_mem = i;
            }
            if (population[i + 1].fitness <= worst) {
                worst = population[i + 1].fitness;
                worst_mem = i + 1;
            }
        } else {
            if (population[i].fitness <= worst) {
                worst = population[i].fitness;
                worst_mem = i;
            }
            if (population[i + 1].fitness >= best) {
                best = population[i + 1].fitness;
                best_mem = i + 1;
            }
        }
    }

    /* if best individual from the new population is better than  */
    /* the best individual from the previous population, then     */
    /* copy the best from the new population; else replace the    */
    /* worst individual from the current population with the      */
    /* best one from the previous generation                      */
    if (best >= population[POPSIZE].fitness) {
        for (i = 0; i < NVARS; i++)
            population[POPSIZE].gene[i] = population[best_mem].gene[i];

        population[POPSIZE].fitness = population[best_mem].fitness;
    } else {
        for (i = 0; i < NVARS; i++)
            population[worst_mem].gene[i] = population[POPSIZE].gene[i];

        population[worst_mem].fitness = population[POPSIZE].fitness;
    }
}

/**************************************************************/
/* Selection function: Standard proportional selection for    */
/* maximization problems incorporating elitist model - makes  */
/* sure that the best member survives                         */
/**************************************************************/

static void __select(void)
{
    size_t i, j;
    int mem;
    double sum = 0;
    double p;

    /* find total fitness of the population */
    for (mem = 0; mem < POPSIZE; mem++) {
        sum += population[mem].fitness;
    }

    /* calculate relative fitness */
    for (mem = 0; mem < POPSIZE; mem++) {
        population[mem].rfitness = population[mem].fitness / sum;
    }

    population[0].cfitness = population[0].rfitness;

    /* calculate cumulative fitness */
    for (mem = 1; mem < POPSIZE; mem++) {
        population[mem].cfitness
            = population[mem - 1].cfitness + population[mem].rfitness;
    }

    /* finally select survivors using cumulative fitness. */

    for (i = 0; i < POPSIZE; i++) {
        p = rand() % 1000 / 1000.0;
        if (p < population[0].cfitness)
            newpopulation[i] = population[0];
        else {
            for (j = 0; j < POPSIZE; j++)
                if (p >= population[j].cfitness
                    && p < population[j + 1].cfitness)
                    newpopulation[i] = population[j + 1];
        }
    }
    /* once a new population is created, copy it back */

    for (i = 0; i < POPSIZE; i++)
        population[i] = newpopulation[i];
}

/***************************************************************/
/* Crossover selection: selects two parents that take part in  */
/* the crossover. Implements a single point crossover          */
/***************************************************************/

static void __crossover(void)
{
    int mem, one;
    int first = 0; /* count of the number of members chosen */
    double x;

    for (mem = 0; mem < POPSIZE; ++mem) {
        x = rand() % 10001 / 1000.0;
        if (x < PXOVER) {
            ++first;
            if (first % 2 == 0)
                __Xover(one, mem);
            else
                one = mem;
        }
    }
}

/**************************************************************/
/* Crossover: performs crossover of the two selected parents. */
/**************************************************************/

static void __Xover(int one, int two)
{
    size_t i;
    int point; /* crossover point */

    /* select crossover point */
    if (NVARS > 1) {
        if (NVARS == 2)
            point = 1;
        else
            point = (rand() % (NVARS - 1)) + 1;

        for (i = 0; i < point; i++)
            __swap(&population[one].gene[i], &population[two].gene[i]);
    }
}

/*************************************************************/
/* Swap: A swap procedure that helps in swapping 2 variables */
/*************************************************************/

static void __swap(double *x, double *y)
{
    double temp;

    temp = *x;
    *x = *y;
    *y = temp;
}

/**************************************************************/
/* Mutation: Random uniform mutation. A variable selected for */
/* mutation is replaced by a random value between lower and   */
/* upper bounds of this variable                              */
/**************************************************************/

static void __mutate(void)
{
    size_t i, j;
    double lbound, hbound;
    double x;

    for (i = 0; i < POPSIZE; i++)
        for (j = 0; j < NVARS; j++) {
            x = rand() % 10001 / 1000.0;
            if (x < PMUTATION) {
                /* find the bounds on the variable to be mutated */
                lbound = population[i].lower[j];
                hbound = population[i].upper[j];
                population[i].gene[j] = __randval(lbound, hbound);
            }
        }
}

/***************************************************************/
/* Report function: Reports progress of the simulation. Data   */
/* dumped into the output file are separated by commas         */
/***************************************************************/

static void __report(void)
{
    size_t i;
    double best_val;   /* best population fitness */
    double avg;        /* avg population fitness */
    double stddev;     /* std. deviation of population fitness */
    double sum_square; /* sum of square for std. calc */
    double square_sum; /* square of sum for std. calc */
    double sum;        /* total population fitness */

    sum = 0.0;
    sum_square = 0.0;

    for (i = 0; i < POPSIZE; i++) {
        sum += population[i].fitness;
        sum_square += population[i].fitness * population[i].fitness;
    }

    avg = sum / (double)POPSIZE;
    square_sum = avg * avg * (double)POPSIZE;
    stddev = sqrt((sum_square - square_sum) / (POPSIZE - 1));
    best_val = population[POPSIZE].fitness;

    fprintf(galog, "\n%5zu,      %6.3f, %6.3f, %6.3f \n\n", generation,
        best_val, avg, stddev);
}

/**************************************************************/
/* Main function: Each generation involves selecting the best */
/* members, performing crossover & mutation and then          */
/* evaluating the resulting population, until the terminating */
/* condition is satisfied                                     */
/**************************************************************/

int main(int argc, char *argv[])
{
    size_t i;

    if ((galog = fopen("galog.txt", "w")) == NULL) {
        exit(1);
    }

    generation = 0;

    fprintf(galog, "\n generation best average standard \n");
    fprintf(galog, "number value fitness deviation \n");

    __initialize();
    __evaluate();
    __keep_the_best();
    while (generation < MAXGENS) {
        generation++;
        __select();
        __crossover();
        __mutate();
        __report();
        __evaluate();
        __elitist();
    }

    fprintf(galog, "\n\n Simulation completed\n");
    fprintf(galog, "\n Best member: \n");

    for (i = 0; i < NVARS; i++) {
        fprintf(galog, "\n var(%zu) = %3.3f", i, population[POPSIZE].gene[i]);
    }

    fprintf(galog, "\n\n Best fitness= %3.3f", population[POPSIZE].fitness);
    fclose(galog);
    printf("Success\n");

    return 0;
}

/***************************************************************/
