CC=gcc
CFLAGS=-std=gnu99 -c

all: csvql

csvql: csvql.o dataframe.o dyn_array.o sql.o
	$(CC) -o csvql csvql.o dataframe.o dyn_array.o sql.o

csvql.o: csvql.c common.h dataframe.h dyn_array.h sql.h
	$(CC) $(CFLAGS) csvql.c

dataframe.o: csvql.c common.h dataframe.h dyn_array.h
	$(CC) $(CFLAGS) dataframe.c

dyn_array.o: dyn_array.c common.h dyn_array.h
	$(CC) $(CFLAGS) dyn_array.c

sql.o: sql.c common.h dataframe.h dyn_array.h sql.h
	$(CC) $(CFLAGS) sql.c

clean:
	rm -f dataframe.o dyn_array.o sql.o

