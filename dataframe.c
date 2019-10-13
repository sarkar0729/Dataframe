#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include "common.h"
#include "dyn_array.h"
#include "dataframe.h"

// gcc -std=gnu99 -c dataframe.c

#define DEBUG 0

#define MAX_COLUMNS 64
#define MAX_ROWS 65536
#define SIZE_OF_LINE 1023

typedef struct {
char name[MAX_NAME_LENGTH];
uint8_t column_count;
uint32_t row_count;
dyn_array_ptr column[MAX_COLUMNS];
} dataframe;

dataframe_ptr dataframe_create(char *df_name, char **csv_header, uint8_t *csv_types, uint32_t row_count, uint32_t column_count);

// RESOLVE: Need to support floating point numbers in scientific notation

int
get_type(char *str)
{
int type;
int this_type;

	type = UNKNOWN_TYPE;

	while (*str != '\0') {
		if (isdigit(*str))
			this_type = 1;
		else if (*str == '.')
			this_type = 2;
		else
			this_type = 3;

		str++;

		if (type == UNKNOWN_TYPE) {
			if (this_type == 1)
				type = INTEGER_TYPE;
			else if (this_type == 2)
				type = FLOAT_TYPE;
			else
				type = STRING_TYPE;

			continue;
		} else {
			// Change type to string, if any non-numeric character
			// Change to float, if currently integer and a '.' encountered
			// Flage error if two dots encountered in a float
			// If a digit is encountered, the type remains same
			if (this_type == 3)
				type = STRING_TYPE;
			else if (this_type == 2) {
				if (type == INTEGER_TYPE)
					type = FLOAT_TYPE;
				else if (type == FLOAT_TYPE)
					return (UNKNOWN_TYPE);
			}

			continue;
		}
	}

	return type;
}

char *csv_table[MAX_ROWS][MAX_COLUMNS];

dataframe_ptr
dataframe_load_from_csv(char *name)
{
FILE *fp;
int bytes_read;
int field_count;
dataframe_ptr df;
dataframe *this_df;
dyn_array_ptr dyn_array_p;
char *csv_header[MAX_COLUMNS];
uint8_t csv_types[MAX_COLUMNS];
char *string_array[MAX_ROWS];
uint32_t row_count;
uint32_t column_count;
char *save_ptr = NULL;
char *token;
char *temp;
char *table_name;
int token_length;
char *my_string;
char line[SIZE_OF_LINE + 1]; // Add a byte for the \0 at the end of string
int nbytes;

	if ((fp = fopen(name, "r")) == NULL) {
		fprintf(stderr, "Could not open file %s\n", name);
		return NULL;
	}

	for (int i = 0; i < MAX_ROWS; i++)
		for (int j = 0; j < MAX_COLUMNS; j++)
			csv_table[i][j] = NULL;

	for (int j = 0; j < MAX_COLUMNS; j++)
		csv_header[j] = NULL;

	// read_csv

	row_count = 0;
	column_count = 0;
	nbytes = SIZE_OF_LINE + 1; // Add a byte for the \0 at the end of string
	my_string = line;

	for (;;) {
		bytes_read = getline(&my_string, (size_t *) &nbytes, fp);

		if (bytes_read == -1)
			break;

		// Eliminate the newline character, if any
		temp = strrchr(my_string, '\n');
		if (temp != NULL)
			*temp = '\0';

		if (row_count == 0 && my_string[0] == '#') {
			// read_csv_header
			field_count = 0;
			token = strtok_r(my_string + 1, ",", &save_ptr);

			while (token != NULL) {
				token_length = strlen(token);
				temp = (char *) malloc (token_length + 1);
				strcpy(temp, token);
				csv_header[field_count] = temp;

				token = strtok_r(NULL, ",", &save_ptr);   
				field_count++;
			}

			column_count = field_count;

			if (1) {
				printf("Column names: ");
				for (int i = 0; i < field_count; i++)
					printf("%s ", csv_header[i]);
				printf("\n");
			}

			continue;
		}

		token = strtok_r(my_string, ",", &save_ptr);

		// RESOLVE: Get rid of leading and trailing blank

		field_count = 0;
		while (token != NULL) {
			token_length = strlen(token);
			temp = (char *) malloc (token_length + 1);
			strcpy(temp, token);
			csv_table[row_count][field_count] = temp;

			token = strtok_r(NULL, ",", &save_ptr);   
			field_count++;
		}

		if (column_count == 0) {
			column_count = field_count;
		} else {
			if (column_count != field_count) {
				printf("Found line with wrong number of fields:");
				printf("row_count = %d column_count = %d, field_count = %d\n", row_count, column_count, field_count);
				return FALSE;
			}
		}

		row_count++;
	}

	if (DEBUG) {
		// dump_csv_table
		for (int i = 0; i < row_count; i++) {
			for (int j = 0; j < column_count; j++) {
				printf("csv_table[%d][%d]: %s\n", i, j, csv_table[i][j]);
			}
			printf("\n");
		}
	}

	if (DEBUG)
		printf("Going to create dataframe now\n");

	// Get rid of the file extension
	table_name = strdup(name);
	temp = strrchr(table_name, '.');
	if (temp != NULL)
		*temp = '\0';

	for (int column = 0; column < column_count; column++)
		csv_types[column] = get_type(csv_table[0][column]);

	df = dataframe_create(table_name, csv_header, csv_types, row_count, column_count);
	if (df == FALSE)
		return NULL;

	if (DEBUG)
		printf("Finished creating dataframe\n");

	// load_dataframe(df)
	this_df = (dataframe *)df;
	for (int column = 0; column < column_count; column++) {
		// Load a column
		dyn_array_p = this_df->column[column];
		if (dyn_array_p == NULL)
			return NULL;

		for (int row = 0; row < row_count; row++) 
			string_array[row] = csv_table[row][column];

		if (load_dyn_array(dyn_array_p, string_array) == FALSE)
			return NULL;
	}

	return df;
}

dataframe_ptr
dataframe_create(char *df_name, char ** csv_header, uint8_t *csv_types, uint32_t row_count, uint32_t column_count)
{
int array_type;
char *array_name;
dataframe *df;

	df = (dataframe *) malloc (sizeof(dataframe));
	if (df == NULL)
		return NULL;

	strcpy(df->name, df_name);
	printf("Creating table %s\n", df_name);

	df->row_count = row_count;
	df->column_count = column_count;

	for (int column = 0; column < column_count; column++) {
		array_name = csv_header[column];
		if (array_name == NULL) {
			array_name = (char *) calloc (MAX_NAME_LENGTH, sizeof(char));
			if (array_name == NULL)
				return FALSE;

			sprintf(array_name, "column_%d", column + 1);
		}

		array_type = csv_types[column];
		if (DEBUG)
			printf("Type of column %d is %d\n", column, array_type);
		if (array_type == UNKNOWN_TYPE)
			return FALSE;

		df->column[column] = create_dyn_array(array_name, row_count, array_type);
		if (DEBUG)
			printf("Successfully created column %d: %s \n", column, array_name);
	}

	for (int i = column_count; i < MAX_COLUMNS; i++)
		df->column[i] = NULL;

	printf("Successfully loaded %d rows, each with %d columns\n", row_count, column_count);

	return (dataframe_ptr) df;
}

void
dump_dataframe(dataframe_ptr df_ptr)
{
dyn_array_ptr dyn_array_p;
dataframe *df;
uint32_t row_count;
uint32_t column_count;

	if (df_ptr == NULL)
		return;

	df = (dataframe *) df_ptr;
	row_count = df->row_count;
	column_count = df->column_count;

	for (int column = 0; column < column_count; column++) {
		dyn_array_p = df->column[column];
		printf("dump_dataframe: column = %d\n", column);
		dump_dyn_array(dyn_array_p);
	}
}

char *
dataframe_get_name(dataframe_ptr df_ptr)
{
dataframe *df;

	if (df_ptr == NULL)
		return NULL;

	df = (dataframe *) df_ptr;
	return(df->name);
}

dyn_array_ptr
dataframe_get_column(dataframe_ptr df_ptr, char *column_name)
{
dataframe *df;
dyn_array_ptr dyn_array_p;

	if (df_ptr == NULL)
		return NULL;

	df = (dataframe *) df_ptr;

	// RESOLVE: Do not check, ignore table_name for now

	for (int column = 0; column < df->column_count; column++) {
		dyn_array_p = df->column[column];
		if (strcmp(column_name, dyn_array_get_name(dyn_array_p)) == 0)
			return(dyn_array_p);
	}

	return NULL;
}

