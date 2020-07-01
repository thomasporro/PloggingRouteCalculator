#define _CRT_SECURE_NO_DEPRECATE
#define EPS 1e-5
#define BINARY_VARIABLE 'B'
#define EQUAL 'E'
#define LESS_EQUAL 'L'
#define GREAT_EQUAL 'G'
#define PI 3.14159265358979323846
#define UPPER_BOUND 520.0
#define LOWER_BOUND 480.0
#define DIVIDER 2.0

#include <ilcplex/cplex.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include "plogger.h"

/*!
* Prints an error and exits from the program
* @param	err pointer to a char's string that will be print
*/
void print_error(const char *err) {
	printf("\n\n ERROR: %s \n\n", err);
	fflush(NULL);
	exit(1);
}

/*!
* Calculate the solution of the problem built into an instance
* @param	inst is a pointer to the instance where is stored the problem
* @return	0 if the solution is found. Other values otherwise
*/
int TSPopt(instance *inst) {
	//Open the CPLEX enviroment
	int error;
	CPXENVptr env = CPXopenCPLEX(&error);
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP");

	//Build the problem's number
	build_model(inst, env, lp);

	//Obtain the number of variable and print them on screen
	int cur_numcols = CPXgetnumcols(env, lp);
	inst->nvariables = cur_numcols;
	printf("Number of variables: %d\n", cur_numcols);


	//Allocate the memory for keeping the solution's informations into the istance's structure
	inst->ncomp = 999999;
	inst->solution = (double *)calloc(cur_numcols, sizeof(double));
	inst->successors = (int*)calloc(inst->nnodes, sizeof(int));
	inst->component = (int*)calloc(inst->nnodes, sizeof(int));

	inst->start_time = time(NULL);

	//Cicle that add a simple SEC when the number of components is greater than 2
	int cycles = 1;
	while (inst->ncomp >= 2) {
		//Calculate the solution of the problem
		printf("CALCULATING THE SOLUTION (CYCLE N.%d) ...\n", cycles);
		CPXmipopt(env, lp);
		printf("SOLUTION CALCULATED (CYCLE N.%d)\n", cycles);
		printf("Number of costraints (CYCLE N.%d): %d\n", cycles, CPXgetnumrows(env, lp));
		double time_passed = difftime(time(NULL), inst->start_time);
		printf("DEBUG-> Time passed: %f s\n", time_passed);


		//If the problem have a solution it saves it into the instance's structure
		if (CPXgetx(env, lp, inst->solution, 0, cur_numcols - 1)) {
			print_error("Failed to optimize MIP.\n");
		}

		double objval = -1;
		if (CPXgetobjval(env, lp, &objval)) {
			print_error("Failed to optimize MIP.\n");
		}
		printf("DEBUG-> objval: %f\n", -objval);


		//Build the solution and saves the components and successors into the instance's structure
		build_solution(inst);

		//Update cycle's counter
		cycles++;
		if (inst->ncomp >= 2) {
			addManualCut(inst, env, lp);
		}

		printf("\n\n");
	}

	//Frees the memory
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env);

	return error;
}

/*!
* Calculates the distance between two nodes using the geo distance
* @param	i is the index of the first node
* @param	j is the index of the second node
* @param	inst is a pointer to the instance where the nodes are stored
* @return	double that indicates the geo distance
*/
double distance(int i, int j, instance *inst) {
	double theta, dist;

	theta = inst->y_coord[i] - inst->y_coord[j];
	dist = sin(deg2rad(inst->x_coord[i]))*sin(deg2rad(inst->x_coord[j])) + cos(deg2rad(inst->x_coord[i]))*cos(deg2rad(inst->x_coord[j]))*cos(deg2rad(theta));
	dist = acos(dist);
	dist = rad2deg(dist);
	dist = dist * 60 * 1.1515;
	dist = dist * 1.609344 * 1000;
	return dist;
}

double deg2rad(double deg) {
	return (deg * PI / 180);
}

double rad2deg(double rad) {
	return (rad * 180 / PI);
}

/*!
* Calculates the position of the variable x(i, j) into the CPLEX problem for
* an undirected graph
* @param	i is the index of the first node
* @param	j is the index of the second node
* @param	inst is a pointer to the instance where the nodes are stored
* @return	an integer indicating the position of the variable into the CPLEX problem
*/
int xpos(int i, int j, instance *inst) {
	if (i == j) print_error(" i == j in xpos");
	if (i > j) return xpos(j, i, inst);
	int pos = i * inst->nnodes + j - ((i + 1)*(i + 2) / 2);
	return pos;
}

/*!
* Function that build the model for an undirected graph
* @param	inst is a pointer to the instance of the problem created using plogger.h
* @param	env is the enviroment of CPLEX
* @param	lp is the problem written in CPLEX
*/
void build_model(instance *inst, CPXENVptr env, CPXLPptr lp) {
	//Seed for the random number generator
	srand(1237030);

	//Variables used to name the variables and constraints into CPLEX
	char **cname = (char **)calloc(1, sizeof(char*));
	cname[0] = (char *)calloc(100, sizeof(char));

	//Add binary variables x(i, j) for i < j
	for (int i = 0; i < inst->nnodes; i++) {
		for (int j = i + 1; j < inst->nnodes; j++) {
			//Variables used to create the variable into CPLEX
			sprintf(cname[0], "x(%d,%d)", i + 1, j + 1);
			char variable_type = BINARY_VARIABLE;
			double obj = -rand() % 2;
			double lb = 0.0;
			double ub = 0.0;
			//Creates the columns of the new variable
			if (CPXnewcols(env, lp, 1, &obj, &lb, &ub, &variable_type, cname)) print_error("wrong CPXnewcols on x var.s");
			if (CPXgetnumcols(env, lp) - 1 != xpos(i, j, inst)) print_error("wrong position for x var.s");
		}
	}

	//Create the indicators and changes the bound of the variables
	create_indicators(inst, env, lp);
	chg_bounds(inst, env, lp, UPPER_BOUND);

	//Upper limit for the length of the path
	sprintf(cname[0], "upper");
	int lastrow = CPXgetnumrows(env, lp);
	double rhs = UPPER_BOUND;
	char sense = 'L';
	if (CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname)) print_error("wrong CPXnewrows [intelligent Systems]");
	int numcoefs = 1112286;
	int *collist = (int *)calloc(1112286, sizeof(int));
	int *rowlist = (int *)calloc(1112286, sizeof(int));
	double *vallist = (double *)calloc(1112286, sizeof(double));
	int counter = 0;
	for (int i = 0; i < inst->nnodes; i++) {
		for (int j = i + 1; j < inst->nnodes; j++) {
			collist[counter] = xpos(i, j, inst);
			vallist[counter] = distance(i, j, inst);
			rowlist[counter] = lastrow;
			counter++;
		}
	}
	if (CPXchgcoeflist(env, lp, numcoefs, rowlist, collist, vallist)) print_error("wrong CPXchgcoef [intelligent Systems]");

	//Lower limit for the length of the path
	sprintf(cname[0], "lower");
	lastrow = CPXgetnumrows(env, lp);
	rhs = LOWER_BOUND;
	sense = 'G';
	if (CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname)) print_error("wrong CPXnewrows [intelligent Systems]");
	counter = 0;
	for (int i = 0; i < inst->nnodes; i++) {
		for (int j = i + 1; j < inst->nnodes; j++) {
			rowlist[counter] = lastrow;
			counter++;
		}
	}
	if (CPXchgcoeflist(env, lp, numcoefs, rowlist, collist, vallist)) print_error("wrong CPXchgcoef [intelligent Systems]");
	free(collist);
	free(vallist);
	free(rowlist);


	printf("END OF BUILDING CONSTRAINTS\n");

	CPXwriteprob(env, lp, "model.lp", NULL);
	printf("END OF WRITING THE PROBLEM ON model.lp\n\n");
	free(cname[0]);
	free(cname);
}


/*!
* Function that build the solution for an undirected graph
* @param	inst is a pointer to the instance of the problem created using plogger.h
*/
void build_solution(instance *inst) {
	//Verify if each node has exactly 2 edges
	int *node_degree = (int*)calloc(inst->nnodes, sizeof(int));
	//In this cycle I modify the degree of a node if the  value of the edge is greater
	//than a tollerance value defined as EPS
	for (int i = 0; i < inst->nnodes; i++) {
		for (int j = i + 1; j < inst->nnodes; j++) {

			//Verify the value of the edge between the nodes i and j
			int k = xpos(i, j, inst);
			if (fabs(inst->solution[k]) > EPS && fabs(inst->solution[k] - 1.0) > EPS) {
				print_error("wrong xstar in build_sol()");
			}

			//Modify the values of the node's degree
			if (inst->solution[k] > 0.5) {
				++node_degree[i];
				++node_degree[j];
			}
		}
	}

	//Check if the degree of each node is equal at 2
	int counter = 0;
	for (int i = 0; i < inst->nnodes; i++) {
		if (node_degree[i] == 2) {
			//printf("wrong degree in build_sol() in the node %d", i);
			//exit(1);
			counter++;
		}
		//printf("node %d degree = %d\n", i, node_degree[i]);
	}
	free(node_degree);

	//Let's set the values of successors and components and the number of components
	inst->ncomp = 0;
	for (int i = 0; i < inst->nnodes; i++) {
		inst->successors[i] = -2;
		inst->component[i] = -2;
	}

	//Modified the value of the selected variables so we can
	//recognize them from the unselected ones
	//int start = -1;
	int succ = -1;
	int c = 0;
	for (int i = 0; i < inst->nnodes; i++) {
		for (int j = 0; j < inst->nnodes; j++) {
			if (i == j) continue;
			if (inst->solution[xpos(i, j, inst)] > 0.5) {
				inst->successors[i] = -1;
				inst->component[i] = -1;
				inst->successors[j] = -1;
				inst->component[j] = -1;
				c++;
			}
		}
		//if (start != -1) break;
	}

	double dist = 0.0;

	//We build the successors and components to indicate the edges of the solution
	for (int start = 0; start < inst->nnodes; start++) {
		//If the component of start has been already assigned skip this for
		if (inst->component[start] >= 0) continue;
		//I upgrade the number of components
		if (inst->successors[start] == -1) {
			inst->ncomp++;
			int i = start;
			while (inst->component[start] == -1) {
				for (int j = 0; j < inst->nnodes; j++) {
					if (i != j && inst->solution[xpos(i, j, inst)] > 0.5 && inst->component[j] == -1 && inst->successors[j] != i) {
						dist += distance(i, j, inst);
						inst->successors[i] = j;
						inst->component[j] = inst->ncomp;
						i = j;
						break;
					}
				}
			}
		}
	}

	printf("DEBUG-> ncomp: %d\n", inst->ncomp);

	if (inst->ncomp == 1) {
		printf("\n\nLENGHT OF THE PATH: %f\n\n", dist);
	}
	return;
}

/*!
* Function that build the Big-M constraint using the indicator constraints
* @param	inst is a pointer to the instance of the problem created using plogger.h
* @param	env is the enviroment of CPLEX
* @param	lp is the problem written in CPLEX
*/
void create_indicators(instance *inst, CPXENVptr env, CPXLPptr lp) {
	int first_node;
	int second_node;
	char line[180];
	char *token;

	//Count of the lines to alloc the memory after
	int number_of_lines = 0;
	FILE *count_lines = fopen("dataset/edges.txt", "r");
	while (fgets(line, sizeof(line), count_lines) != NULL) {
		if (strlen(line) <= 1) { continue; }
		number_of_lines++;
	}
	fclose(count_lines);

	//Alloc the memory that I need
	char *lu = (char *)calloc(number_of_lines, sizeof(char));
	double *bd = (double*)calloc(number_of_lines, sizeof(double));
	int *indices = (int *)calloc(number_of_lines, sizeof(int));
	int counter = 0;

	//Watch which variable will be active
	//Used to overcame the problem of memory leak
	int *variables_active = (int*)calloc(1112286, sizeof(int));
	for (int i = 0; i < 1112286; i++) {
		variables_active[i] = -1;
	}
	FILE *indicators = fopen("dataset/edges.txt", "r");
	while (fgets(line, sizeof(line), indicators) != NULL) {
		if (strlen(line) <= 1) { continue; }

		first_node = atoi(strtok(line, " "));
		second_node = atoi(strtok(NULL, " ,:"));

		if (first_node == second_node) continue;
		variables_active[xpos(first_node, second_node, inst)] = 1;
	}
	fclose(indicators);


	//Variables needed to create the indicators
	char sense = 'E';
	double rhs = 2;
	int complemented = 0;
	int nzcnt = inst->nnodes;
	int *linind = (int*)calloc(nzcnt, sizeof(int));
	double *linval = (double*)calloc(nzcnt, sizeof(double));

	FILE *file = fopen("dataset/edges.txt", "r");

	while (fgets(line, sizeof(line), file) != NULL) {
		if (strlen(line) <= 1) { continue; }

		//Set the value of nnz to 0
		nzcnt = 0;

		first_node = atoi(strtok(line, " "));
		second_node = atoi(strtok(NULL, " ,:"));

		if (first_node == second_node) continue;

		lu[counter] = 'U';
		bd[counter] = 1;
		indices[counter] = xpos(first_node, second_node, inst);
		counter++;

		for (int j = 0; j < inst->nnodes; j++) {
			if (first_node == j) continue;
			if (variables_active[xpos(first_node, j, inst)] == 1) {
				linind[nzcnt] = xpos(first_node, j, inst);
				linval[nzcnt] = 1;
				nzcnt++;
			}
		}

		for (int j = 0; j < inst->nnodes; j++) {
			if (first_node == j) continue;
			if (variables_active[xpos(first_node, j, inst)] == 1) {
				CPXaddindconstr(env, lp, xpos(first_node, j, inst), complemented, nzcnt, rhs, sense, linind, linval, NULL);
			}
		}

		nzcnt = 0;
		for (int j = 0; j < inst->nnodes; j++) {
			if (second_node == j) continue;
			if (variables_active[xpos(second_node, j, inst)] == 1) {
				linind[nzcnt] = xpos(second_node, j, inst);
				linval[nzcnt] = 1;
				nzcnt++;
			}
		}

		for (int j = 0; j < inst->nnodes; j++) {
			if (second_node == j) continue;
			if (variables_active[xpos(second_node, j, inst)] == 1) {
				CPXaddindconstr(env, lp, xpos(second_node, j, inst), complemented, nzcnt, rhs, sense, linind, linval, NULL);
			}
		}
	}
	if (CPXchgbds(env, lp, number_of_lines + 1, indices, lu, bd)) print_error("Failed to fix the value of the variables\n");

	lu[0] = 'L';
	bd[0] = 1;
	if (CPXchgbds(env, lp, 1, indices, lu, bd)) print_error("Failed to fix the value of the variables\n");
	fclose(file);


	free(lu);
	free(bd);
	free(indices);
	free(linind);
	free(linval);
}

/*!
* Function that build new costraints to avoid the cycles found
* @param	inst is a pointer to the instance of the problem created using plogger.h
* @param	env is the enviroment of CPLEX
* @param	lp is the problem written in CPLEX
*/
void addManualCut(instance *inst, CPXENVptr env, CPXLPptr lp) {
	char **cname = (char **)calloc(1, sizeof(char*));
	cname[0] = (char *)calloc(100, sizeof(char));

	for (int i = 1; i <= inst->ncomp; i++) {
		int *nodes = (int*)calloc(inst->nnodes, sizeof(int));
		int number_of_nodes = 0;
		for (int j = 0; j < inst->nnodes; j++) {
			if (inst->component[j] == i) {
				nodes[number_of_nodes] = j;
				number_of_nodes++;
			}
		}

		//Creates a new constraint
		int lastrow = CPXgetnumrows(env, lp);
		sprintf(cname[0], "SEC constraints");
		double rhs = number_of_nodes - 1;
		char sense = 'L';
		if (CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname)) print_error(" wrong CPXnewrows [addManualCut]");

		int counter = 0;
		int i = nodes[0];
		while (counter < number_of_nodes) {
			if (CPXchgcoef(env, lp, lastrow, xpos(i, inst->successors[i], inst), 1.0)) print_error("wrong CPXchgcoef [addManualCut]");
			i = inst->successors[i];
			counter++;
		}
		free(nodes);
	}
	CPXwriteprob(env, lp, "model.lp", NULL);
	free(cname[0]);
	free(cname);
}

/*!
* Remove the variable further than UPPER_BOUND/DIVIDER
* @param	inst is a pointer to the instance of the problem created using plogger.h
* @param	env is the enviroment of CPLEX
* @param	lp is the problem written in CPLEX
*/
void chg_bounds(instance *inst, CPXENVptr env, CPXLPptr lp, double upper_bound) {
	int counter = 0;
	char *l = (char *)calloc(2224572, sizeof(char));
	double *b = (double*)calloc(2224572, sizeof(double));
	int *index = (int *)calloc(2224572, sizeof(int));

	for (int i = 0; i < inst->nnodes; i++) {
		if (i == 10) continue;
		if (distance(10, i, inst) > upper_bound / DIVIDER) {
			for (int j = 0; j < inst->nnodes; j++) {
				if (i == j) continue;
				l[counter] = 'U';
				b[counter] = 0;
				index[counter] = xpos(i, j, inst);
				counter++;
			}
		}
	}
	printf("DEBUG-> Removed the nodes further than: %f\n", upper_bound / DIVIDER);

	if (CPXchgbds(env, lp, counter + 1, index, l, b)) print_error("Failed to fix the value of the variables\n");
	free(l);
	free(b);
	free(index);
}