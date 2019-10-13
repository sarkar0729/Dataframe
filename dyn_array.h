typedef struct dyn_array *dyn_array_ptr;

dyn_array_ptr create_dyn_array(char *array_name, uint32_t array_length, uint8_t array_type);
boolean load_dyn_array(dyn_array_ptr dynamic_array_ptr, char **input_string_p);
void dump_dyn_array(dyn_array_ptr dynamic_array_ptr);
uint32_t count_dyn_array(dyn_array_ptr dynmic_array_ptr, value search_val, uint8_t op);
char * dyn_array_get_name(dyn_array_ptr dynamic_array_ptr);
