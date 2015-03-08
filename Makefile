all: lql.c
	cc -g lql.c -o lql

clean:
	rm lql
