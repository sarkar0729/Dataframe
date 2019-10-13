#define MAX_NAME_LENGTH 64

#define UNKNOWN_TYPE 0
#define INTEGER_TYPE 1
#define FLOAT_TYPE 2
#define STRING_TYPE 3

#define OP_GT	1
#define OP_LT	2
#define OP_GTE	3
#define OP_LTE	4
#define OP_EQ	5
#define OP_NEQ	6

typedef enum { FALSE, TRUE } boolean;

union Data {
int64_t int_data;
float float_data;
char *string_data;
};

typedef struct {
union Data data;
uint8_t data_type;
} value;

