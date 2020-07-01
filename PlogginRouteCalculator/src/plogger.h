#ifndef PLOGGER_H_
#define PLOGGER_H_

#include <ilcplex/cplex.h>

//Struct that will contain the parameters of the tsp problem
typedef struct {

	//Variables of the input files
	int nnodes;
	double *x_coord;
	double *y_coord;
	char input_file[1000];

	//Variable that will contain global data
	double *solution;
	double nvariables;
	int *successors;
	int *component;
	int ncomp;

	time_t start_time;

} instance;

void print_error(const char *err);
int TSPopt(instance *inst);
double distance(int i, int j, instance *inst);
int xpos(int i, int j, instance *inst);
double rad2deg(double rad);
double deg2rad(double deg);
void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_solution(instance *inst);
void create_indicators(instance *inst, CPXENVptr env, CPXLPptr lp);
void addManualCut(instance *inst, CPXENVptr env, CPXLPptr lp);
void chg_bounds(instance *inst, CPXENVptr env, CPXLPptr lp, double upper_bound);


#endif // !PLOGGER_H_