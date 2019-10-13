typedef struct dataframe *dataframe_ptr;

dataframe_ptr create_dataframe(char *name);
dataframe_ptr dataframe_load_from_csv(char *name);
boolean load_dataframe(dataframe_ptr df_ptr);
char *dataframe_get_name(dataframe_ptr df_ptr);
dyn_array_ptr dataframe_get_column(dataframe_ptr df_ptr, char *column_name);
void dump_dataframe(dataframe_ptr df_ptr);

