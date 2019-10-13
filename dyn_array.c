#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include "common.h"
#include "dyn_array.h"

// gcc -std=gnu99 -c dyn_array.c

#define DEBUG 0

typedef struct {
char name[MAX_NAME_LENGTH];
uint8_t array_type;
uint32_t array_length;
value max;
value min;
uint8_t *element_array;
struct dyn_array *next;
} dyn_array;


dyn_array_ptr
create_dyn_array(char *array_name, uint32_t array_length, uint8_t array_type)
{
dyn_array *dyn_array_p;

	dyn_array_p = (dyn_array *) calloc (array_length, sizeof(dyn_array));
	if (dyn_array_p == NULL)
		return NULL;

	if (array_name != NULL)
		strncpy(dyn_array_p->name, array_name, MAX_NAME_LENGTH);
	else
		dyn_array_p->name[0] = '\0'; 

	dyn_array_p->array_type = array_type;
	dyn_array_p->array_length = array_length;
	dyn_array_p->element_array = NULL;
	dyn_array_p->next = NULL;

	return (dyn_array_ptr) dyn_array_p;
}

// RESOLVE: Date and Money to be implemented
// RESOLVE: NULL value to be implemented

boolean
load_dyn_array(dyn_array_ptr dynamic_array_ptr, char **input_string_p)
{
uint8_t array_type;
uint32_t array_length;
int32_t this_int32;
int32_t *p_int32;
float this_float;
float *p_float;
char *this_string;
char **p_string;
dyn_array *dyn_array_p;

	if (dynamic_array_ptr == NULL)
		return 0;

	dyn_array_p = (dyn_array *) dynamic_array_ptr;
 
	array_type = dyn_array_p->array_type;
	array_length = dyn_array_p->array_length;

	printf("load_dyn_array: array_type = %d\n", array_type);

	if (array_type == INTEGER_TYPE) {
		p_int32 = (void *) calloc (array_length, sizeof(int32_t));
		if (p_int32 == NULL)
			return FALSE;
		dyn_array_p->element_array = (void *) p_int32;

		for (int i = 0; i < dyn_array_p->array_length; i++) {
			sscanf(input_string_p[i], "%d", &this_int32);
			p_int32[i] = this_int32;
		}
	} else if (array_type == FLOAT_TYPE) {
		p_float = (void *) calloc (array_length, sizeof(float));
		if (p_float == NULL)
			return FALSE;
		dyn_array_p->element_array = (void *) p_float;

		for (int i = 0; i < dyn_array_p->array_length; i++) {
			sscanf(input_string_p[i], "%f", &this_float);
			p_float[i] = this_float;
		}
	} else if (array_type == STRING_TYPE) {
		p_string = (void *) calloc (array_length, sizeof(char *));
		if (p_string == NULL)
			return FALSE;
		dyn_array_p->element_array = (void *) p_string;

		for (int i = 0; i < dyn_array_p->array_length; i++) {
			p_string[i] = strdup(input_string_p[i]);
		}
	}

	return TRUE;
}

void
dump_dyn_array(dyn_array_ptr dynamic_array_ptr)
{
int32_t *ptr_int32;
float *ptr_float;
char **string;
dyn_array *dyn_array_p;

	if (dynamic_array_ptr == NULL)
		return;

	dyn_array_p = (dyn_array *) dynamic_array_ptr;

	if (dyn_array_p->array_type == STRING_TYPE) {
		string = (char **) dyn_array_p->element_array;
		for (int i = 0; i < dyn_array_p->array_length; i++) {
			printf("%s\n", string[i]);
		}
	} else if (dyn_array_p->array_type == FLOAT_TYPE) {
		ptr_float = (float *) dyn_array_p->element_array;
		for (int i = 0; i < dyn_array_p->array_length; i++) {
			printf("%f\n", ptr_float[i]);
		}
	} else if (dyn_array_p->array_type == INTEGER_TYPE) {
		ptr_int32 = (int32_t *) dyn_array_p->element_array;
		for (int i = 0; i < dyn_array_p->array_length; i++) {
			printf("%d\n", ptr_int32[i]);
		}
	}
}

char *
dyn_array_get_name(dyn_array_ptr dynamic_array_ptr)
{
dyn_array *dyn_array_p;

	if (dynamic_array_ptr == NULL)
		return NULL;

	dyn_array_p = (dyn_array *) dynamic_array_ptr;
    return (dyn_array_p->name);
}

uint32_t
count_dyn_array(dyn_array_ptr dynmic_array_ptr, value search_val, uint8_t op)
{
int64_t int_val;
int64_t *int_arr;
float float_val;
float *float_arr;
uint32_t array_length;
uint32_t count;
dyn_array *dyn_array_p;

	if (dynmic_array_ptr == NULL) {
		printf("count_dyn_array: dynmic_array_ptr = NULL\n");
		return 0;
	}

	dyn_array_p = (dyn_array *) dynmic_array_ptr;

	if (dyn_array_p->array_type != search_val.data_type) {
		printf("count_dyn_array: dynmic_array_ptr = %d, search_val.data_type = %d\n", dyn_array_p->array_type, search_val.data_type);
		return 0;
	}

	array_length = dyn_array_p->array_length;
	count = 0;

	printf("count_dyn_array: array_length = %d\n", array_length);

	// RESOLVE: Need to loop till end of linked list
	if (dyn_array_p->array_type == INTEGER_TYPE) {
		int_val = search_val.data.int_data;
		int_arr = (int64_t *)dyn_array_p->element_array;

       	switch(op){
           case OP_EQ: 
				for (int i = 0; i < array_length; i++)
					if (int_arr[i] == int_val)
						count++;
				break;
           case OP_NEQ:
				for (int i = 0; i < array_length; i++)
					if (int_arr[i] != int_val)
						count++;
				break;
           case OP_GT: 
				for (int i = 0; i < array_length; i++)
					if (int_arr[i] > int_val)
						count++;
				break;
           case OP_GTE: 
				for (int i = 0; i < array_length; i++)
					if (int_arr[i] >= int_val)
						count++;
				break;
           case OP_LT: 
				for (int i = 0; i < array_length; i++)
					if (int_arr[i] < int_val)
						count++;
				break;
           case OP_LTE: 
				for (int i = 0; i < array_length; i++)
					if (int_arr[i] <= int_val)
						count++;
				break;
           default:
               return(-1);
       	}

	} else if (dyn_array_p->array_type == FLOAT_TYPE) {
		float_val = search_val.data.float_data;
		float_arr = (float *)dyn_array_p->element_array;

		printf("count_dyn_array: float_val = %f\n", float_val);
		printf("count_dyn_array: float_arr = 0x%X\n", float_arr);

		switch(op){
           	case OP_EQ: 
				for (int i = 0; i < array_length; i++)
					if (float_arr[i] == float_val)
						count++;
				break;
           case OP_NEQ:
				for (int i = 0; i < array_length; i++)
					if (float_arr[i] != float_val)
						count++;
				break;
           case OP_GT: 
				for (int i = 0; i < array_length; i++)
					if (float_arr[i] > float_val)
						count++;
				break;
           case OP_GTE: 
				for (int i = 0; i < array_length; i++)
					if (float_arr[i] >= float_val)
						count++;
				break;
           case OP_LT: 
				for (int i = 0; i < array_length; i++)
					if (float_arr[i] < float_val) {
						count++;
						printf("%f\n", float_arr[i]);
					}
				break;
           case OP_LTE: 
				for (int i = 0; i < array_length; i++)
					if (float_arr[i] <= float_val)
						count++;
				break;
           default:
               return(-1);
		}
	}

	return count;
}

