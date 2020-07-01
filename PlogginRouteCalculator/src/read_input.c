#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "read_input.h"

/*!
 * This function will parse the command line and saves it into instance
 * @param	argc is the number if string pointed by argv
 * @param	argv is a pointer to a pointer of strings
 * @param	inst pointer to an instance
 * @modify	saves the value passed in argv into inst
 */
void parse_command_line(int argc, char **argv, instance *inst) {
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-file") == 0) {
			strcpy(inst->input_file, argv[++i]);
			printf("Input file name: %s\n", inst->input_file);
			continue;
		}
	}
}

/*!
* This fuction will read the file saved into inst and saves
* its value into the instance itself
* @param	inst the instance where the name of file is saved and
*					where the the value will be saved
*/
void read_input(instance *inst) {
	//Declaration of local variable
	char *parameter_name;
	char line[180];
	char *token;
	int active_session = 0; //This is useful when we have to read the node's coordinates

	//Initialization of the values into inst
	inst->nnodes = -1;

	//Opens the files that will be read
	FILE *file = fopen(inst->input_file, "r");

	//Cicles for each line into the input file
	while (fgets(line, sizeof(line), file) != NULL) {
		if (strlen(line) <= 1) { continue; }

		parameter_name = strtok(line, " :");

		//Will save the number of nodes and alloc the memory for the 
		//coordinate's value
		if (strncmp(parameter_name, "DIMENSION", 9) == 0) {
			active_session = 0;
			inst->nnodes = atoi(strtok(NULL, " :"));
			inst->x_coord = (double *)calloc(inst->nnodes, sizeof(double));
			inst->y_coord = (double *)calloc(inst->nnodes, sizeof(double));

			//Print of debug
			printf("Number of nodes: %d\n", inst->nnodes);
		}

		//Will redirect the parser to the active session so we can parse 
		//the nodes coordinates
		if (strncmp(parameter_name, "NODE_COORD_SECTION", 18) == 0) {
			active_session = 1;
			continue;
		}

		//Will end the reading of the file
		if (strncmp(parameter_name, "EOF", 3) == 0) {
			active_session = 0;
			break;
		}

		//Parse the coordinates
		if (active_session == 1) {
			int nodes_number = atoi(parameter_name);

			//X's coordinates
			token = strtok(NULL, " :,");
			inst->x_coord[nodes_number] = atof(token);

			//Y's coordinates
			token = strtok(NULL, " :,");
			inst->y_coord[nodes_number] = atof(token);

			continue;
		}
	}
}