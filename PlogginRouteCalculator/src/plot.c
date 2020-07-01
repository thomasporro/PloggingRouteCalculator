#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <gnuplot_c.h>
#include "plot.h"

/*!
* Plot a graph using the passed command and the gnuplot enviroment
* @param	commands it's a vector of strings where are written the command of gnuplot
* @param	n_commands is an integer indicating the number of command passed into commands
* @param	inst is a pointer to the instance of the problem created using plogger.h
*/
void plot(char **commands, int n_commands, instance *inst) {
	//Open the gnuplot enviroment and write the data into a data
	FILE *gnuplotPipe = _popen("C:/UNIPD/ro2/gnuplot/bin/gnuplot.exe -persistent", "w");
	FILE * temp = fopen("data.dat", "w");

	//I iterate the plotting for each component
	for (int i = 1; i <= inst->ncomp; i++) {
		//Varibales used to identify all the nodes into a component
		int start = -1;
		int succ = -1;

		//Finds the first node in that component and then close the loop
		for (int j = 1; j < inst->nnodes; j++) {
			if (inst->component[j] == i) {
				fprintf(temp, "%lf %lf \n", inst->x_coord[j], inst->y_coord[j]);
				start = j;
				succ = inst->successors[j];
				break;
			}
		}

		//Actually plot the solution by writing it into a temporary file
		while (succ != start) {
			fprintf(temp, "%lf %lf \n", inst->x_coord[succ], inst->y_coord[succ]);
			if (start == inst->successors[succ]) {
				fprintf(temp, "%lf %lf \n", inst->x_coord[start], inst->y_coord[start]);
			}
			succ = inst->successors[succ];
		}
		fprintf(temp, "\n");
	}

	//Closing the file
	fclose(temp);

	//Executing the commands passed into the function
	for (int i = 0; i < n_commands; i++) {
		fprintf(gnuplotPipe, "%s \n", commands[i]);
	}

	//Closing the pipeline of gnuplot
	_pclose(gnuplotPipe);
}