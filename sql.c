#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include "common.h"
#include "dyn_array.h"
#include "dataframe.h"
#include "sql.h"

// gcc -std=gnu99 -c sql.c

#define DEBUG 0

#define TOKEN_UNKNOWN	0
#define TOKEN_ID		1
#define TOKEN_NUMBER	2
#define TOKEN_STRING	3
#define TOKEN_KEYWORD	4
#define TOKEN_OP		5
#define TOKEN_END		6

#define SUB_TOKEN_UNKNOWN	0
#define SUB_TOKEN_GT	1
#define SUB_TOKEN_LT	2
#define SUB_TOKEN_GTE	3
#define SUB_TOKEN_LTE	4
#define SUB_TOKEN_EQ	5
#define SUB_TOKEN_NEQ	6

#define SUB_TOKEN_AND	7
#define SUB_TOKEN_OR	8

#define SUB_TOKEN_L_PAREN	9
#define SUB_TOKEN_R_PAREN	10

#define SUB_TOKEN_SELECT	10
#define SUB_TOKEN_FROM		11
#define SUB_TOKEN_WHERE		12

char *operand_stack[256];
int operand_stack_ptr;
char *op_stack[256];
int op_stack_ptr;

void
lex(char *str, int *type, int *sub_type)
{

	if (isalpha(*str)!= 0) {
		if (strcasecmp(str, "AND") == 0) {
			*type = TOKEN_KEYWORD;
			*sub_type = SUB_TOKEN_AND;
		} else if (strcasecmp(str, "OR") == 0) {
			*type = TOKEN_KEYWORD;
			*sub_type = SUB_TOKEN_OR;
		} else {
			*type = TOKEN_ID;
			*sub_type = SUB_TOKEN_UNKNOWN;
		}
	} else if (isdigit(*str)!= 0) {
			*type = TOKEN_NUMBER;
			*sub_type = SUB_TOKEN_UNKNOWN;
	} else if (*str == '\"') {
			*type = TOKEN_STRING;
			*sub_type = SUB_TOKEN_UNKNOWN;
	} else if (*str == '=') {
			*type = TOKEN_OP;
			*sub_type = SUB_TOKEN_EQ;
	} else if (*str == '>') {
			*type = TOKEN_OP;
			*sub_type = SUB_TOKEN_GT;
	} else if (*str == '<') {
			*type = TOKEN_OP;
			*sub_type = SUB_TOKEN_LT;
	} else {
			*type = TOKEN_UNKNOWN;
			*sub_type = SUB_TOKEN_UNKNOWN;
	}
}

boolean
postfix_convert(char **infix_tokens, char **postfix_tokens, int token_count)
{
int token_type;
int token_sub_type;
char *operand_stack[256];
int operand_stack_ptr;
char *op_stack[256];
int op_stack_ptr;
char *op;
char *op1;
char *op2;
char *op_stack_top;
int postfix_token_idx;

	printf("postfix_convert: input = ");
	for (int i = 0; i < token_count; i++)
		printf("%s ", infix_tokens[i]);
	printf("\n");

	operand_stack_ptr = 0;
	op_stack_ptr = 0;
	postfix_token_idx = 0;

	for (int i = 0; i < token_count; i++) {
		lex(infix_tokens[i], &token_type, &token_sub_type);
		switch(token_type){
        	case TOKEN_ID: // Push to operand stack
        	case TOKEN_NUMBER: // Push to operand stack
        	case TOKEN_STRING: // Push to operand stack
				operand_stack[operand_stack_ptr] = infix_tokens[i];
				printf("Pushed to operand stack %s, new stack pointer = %d\n", infix_tokens[i], operand_stack_ptr+1);
				operand_stack_ptr++;
				break;
        	case TOKEN_OP: // Push to op stack
				if ((token_sub_type == SUB_TOKEN_GT) || (token_sub_type == SUB_TOKEN_LT) ||
					(token_sub_type == SUB_TOKEN_GTE) || (token_sub_type == SUB_TOKEN_LTE) ||
					(token_sub_type == SUB_TOKEN_EQ) || (token_sub_type == SUB_TOKEN_NEQ)) {
					if (op_stack_ptr == 0) {
						op_stack[op_stack_ptr] = infix_tokens[i];
						printf("Pushed to op stack %s\n", infix_tokens[i]);
						op_stack_ptr++;
						break;
					} else {
						op_stack_top = op_stack[op_stack_ptr - 1];
						if (strcmp(op_stack_top, "AND") == 0 || strcmp(op_stack_top, "OR") == 0) {
							op_stack[op_stack_ptr] = infix_tokens[i];
							printf("Pushed to op stack %s\n", infix_tokens[i]);
							op_stack_ptr++;
							break;
						} else {
							printf("postfix_convert: Error at line %d\n", __LINE__);
							return FALSE;
						}
					}
				} else {
					printf("postfix_convert: Error at line %d\n", __LINE__);
					return FALSE;
				}
        	case TOKEN_KEYWORD:
				if ((token_sub_type == SUB_TOKEN_AND) || (token_sub_type == SUB_TOKEN_OR)) {
					// Pop two operands and one op
					// Check against stack underflow
					if (op_stack_ptr == 0) {
						printf("postfix_convert: Error at line %d\n", __LINE__);
						return FALSE;
					}

					op_stack_ptr--;
					op = op_stack[op_stack_ptr];

					printf("Pop-ed from op stack %s\n", op);

					operand_stack_ptr--;
					if (operand_stack_ptr == 0) {
						printf("postfix_convert: Error at line %d\n", __LINE__);
						return FALSE;
					}

					op2 = operand_stack[operand_stack_ptr];
					printf("Pop-ed from operand stack %s, new stack pointer = %d\n", op2, operand_stack_ptr);

					operand_stack_ptr--;
					if (operand_stack_ptr < 0) {
						printf("postfix_convert: Error at line %d\n", __LINE__);
						return FALSE;
					}

					op1 = operand_stack[operand_stack_ptr];
					printf("Pop-ed from operand stack %s, new stack pointer = %d\n", op1, operand_stack_ptr);

					postfix_tokens[postfix_token_idx++] = op1;
					postfix_tokens[postfix_token_idx++] = op2;
					postfix_tokens[postfix_token_idx++] = op;

					// Now push the current operand (AND or OR) to the stack
					// However if there is an op in the op_stack and it is of
					// same or higher priority, pop it out first
					// RESOLVE: Do not go past '('

					while (op_stack_ptr > 0) {
						op_stack_ptr--;
						op = op_stack[op_stack_ptr];
						printf("Pop-ed from op stack %s\n", op);
						postfix_tokens[postfix_token_idx++] = op;
					}
					
					op_stack[op_stack_ptr] = infix_tokens[i];
					printf("At line %d, pushed to op stack %s\n", __LINE__, infix_tokens[i]);
					op_stack_ptr++;
					break;

				} else {
					// Key word other than AND or OR
					return(FALSE);
				}
			default: 
				return(FALSE);
		}

	}

	// Reached end of token list, dump the operator and operand stack
	
	while (op_stack_ptr != 0) {
		op_stack_ptr--;
		op = op_stack[op_stack_ptr];

		if (operand_stack_ptr == 0) {
			// Flush the op stack
			postfix_tokens[postfix_token_idx++] = op;
			while (op_stack_ptr != 0) {
				op_stack_ptr--;
				op = op_stack[op_stack_ptr];
				postfix_tokens[postfix_token_idx++] = op;
			}
			break;
		}
		operand_stack_ptr--;
		op2 = operand_stack[operand_stack_ptr];

		if (operand_stack_ptr == 0) {
			printf("postfix_convert: Error at line %d\n", __LINE__);
			return FALSE;
		}
		operand_stack_ptr--;
		op1 = operand_stack[operand_stack_ptr];

		postfix_tokens[postfix_token_idx++] = op1;
		postfix_tokens[postfix_token_idx++] = op2;
		postfix_tokens[postfix_token_idx++] = op;
	}

	printf("Postfix representation: ");

	for (int i = 0; i < postfix_token_idx; i++)
		printf("%s ", postfix_tokens[i]);

	printf("\n");

	return TRUE;

}

void
process_select_statement(char *select_cmd, dataframe_ptr df_ptr)
{
int field_count;
char *dataframe_name;
char *token;
char *cmdsep = ", \t";
char *column_list[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
char *expression_token_list[256];
char *postfix_token_list[256];
int expression_token_count;
char *save_ptr;

	if (1)
		printf("select cmd: %s\n", select_cmd);

	field_count = 0;
	save_ptr = NULL;
	token = strtok_r(select_cmd, cmdsep, &save_ptr);   

	while (token != NULL) {
		if (!strcmp("from", token))
			break;
		if (1)
			printf("%d: %s\n", field_count, token);
		column_list[field_count] = strdup(token);

		field_count++;
		token = strtok_r(NULL, cmdsep, &save_ptr);   
	}

	if (token == NULL) {
		printf("Invalid syntax. Missing 'from' clause\n");
		return;
	}

	if (field_count == 0) {
		printf("Invalid syntax. At least one column to be specified\n");
		return;
	}

	// Get the dataframe name
	token = strtok_r(NULL, cmdsep, &save_ptr);   
	printf("Table: %s\n", token);

	// Now validate the dataframe name and column names
	dataframe_name = dataframe_get_name(df_ptr);
	if (strcmp(token, dataframe_name) != 0) {
		printf("Invalid dataframe name. Loaded dataframe name is %s\n", dataframe_name);
		return;
	}

	for (int i = 0; i < field_count; i++) {
		if (DEBUG)
			printf("Iteration %d:Validating column name %s\n", i, column_list[i]);

		if (dataframe_get_column(df_ptr, column_list[i]) == NULL) {
			printf("Invalid column name %s\n", column_list[i]);
			return;
		}
	}

	// The dataframe name must be followed by 'where' clause
	token = strtok_r(NULL, cmdsep, &save_ptr);   
	if (strcmp("where", token)) {
		printf("Invalid syntax. keyword 'where' expected\n");
		return;
	}

	// Now collect tokens for rest of the statement
	// These tokens form the boolean expression of where clause
	for (int i = 0; i < 256; i++)
		expression_token_list[i] = NULL;

	expression_token_count = 0;
	token = strtok_r(NULL, cmdsep, &save_ptr);   

	while (token != NULL) {
		expression_token_list[expression_token_count] = strdup(token);
		expression_token_count++;
		token = strtok_r(NULL, cmdsep, &save_ptr);   
	}

	for (int i = 0; i < expression_token_count; i++)
		printf("%d: %s\n", i, expression_token_list[i]);

	if (postfix_convert(expression_token_list, postfix_token_list, expression_token_count) == FALSE) {
		printf("Invalid syntax in 'where' clause\n");
		return;
	}

}

boolean
execute_sql_command(char *cmd, dataframe_ptr df_ptr)
{
int field_count;
char *token;
char *save_ptr;
char *cmdsep = ", \t";

	printf("execute_sql_command: %s\n", cmd);

	token = strtok_r(cmd, cmdsep, &save_ptr);
	if (token == NULL)
		return FALSE;

	if (strcmp("select", token) == 0) {
		process_select_statement(save_ptr, df_ptr);
		return TRUE;

	} else if (strcmp("quit", token) == 0){
		return FALSE;
	} else {
		printf("Invalid syntax %s. Type help to know the syntax\n", token);
		return TRUE;
	}

	return FALSE;
}

