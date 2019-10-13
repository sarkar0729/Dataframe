#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include "common.h"
#include "dyn_array.h"
#include "dataframe.h"
#include "sql.h"

// gcc -std=gnu99 -c csvql.c

#define DEBUG 0

// This program reads a CSV file and stores as table in compressed form
// It supports querying the table in SQL

int
main(int argc, char *argv[])
{
FILE *cmd_fp;
boolean interactive;
char *temp;
static char linebuf[8192];
dyn_array_ptr dyn_array_p;
dataframe_ptr df;
value search_val;
uint32_t n;

	if (argc != 2) {
  		fprintf(stderr, "Must specify the name of the file to be loaded\n");
  		exit(1);
	}

	df = dataframe_load_from_csv(argv[1]);
	if (df == NULL) {
  		fprintf(stderr, "Failed to create dataframe from csv file %s\n", argv[1]);
  		exit(1);
	}

	// RESOLVE: Temporary for testing, to be removed
	dyn_array_p = dataframe_get_column(df, "column_2");
	search_val.data_type = FLOAT_TYPE;
	search_val.data.float_data = 0.35;
	n = count_dyn_array(dyn_array_p, search_val, OP_LT);

	if (1)
		printf("count_dyn_array returned %d\n", n);

	if (DEBUG)
		dump_dataframe(df);

	interactive = TRUE;

	while (1) {
		if (interactive) {
			printf ("> ");
    		fflush (stdout);
			if ((fgets (linebuf, sizeof (linebuf) - 1, stdin)) == NULL) {
				putchar ('\n');
				fflush (stdout);
				exit (0);
			}
		} else {
			if ((fgets (linebuf, sizeof (linebuf) - 1, cmd_fp)) == NULL) {
				/* End of input */
				break;
			}
			linebuf[sizeof(linebuf) - 1] = '\0';
		}
		
		// Eliminate the newline character
		temp = strrchr(linebuf, '\n');
		if (temp != NULL)
			*temp = '\0';

		if (execute_sql_command(linebuf, df) == FALSE) {
			break;
		}
	}

	return 0;
}

