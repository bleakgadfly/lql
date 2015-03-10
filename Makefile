all: lql.c
	cc -g -Wall lql.c -o lql

clean:
	rm lql
