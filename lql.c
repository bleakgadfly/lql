#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <pwd.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#define MAX_RATING_LENGTH 3 
#define MAX_AGE_LENGTH 2
#define WORKDIR "/.lql"
#define NULL_TERM_LEN 1

typedef struct {
	char distillery[CHAR_MAX];
	signed int age;
	double rating;
} entity;

static int 
make_dir(char *dirname) 
{
	struct stat st = {0};
	if(-1 == stat(dirname, &st)) 
		mkdir(dirname, 0700);
	
	return 0;
}

static int 
numsort(const struct dirent **file1, const struct dirent **file2)
{
	const char *a = (*file1)->d_name;
	const char *b = (*file2)->d_name;
	char *ptr;
	long num_a = strtol(a, &ptr, 10);
	long num_b = strtol(b, &ptr, 10);
	return num_a > num_b ? 1 : (num_a == num_b ? 0 : -1);
}

/*
 * Make this void and do not return a malloc'd value
 */
static char* 
get_workdir()
{
	struct passwd *pw = getpwuid(getuid());
	const char *homedir = pw->pw_dir;
	size_t workdir_len = strlen(homedir) + strlen(WORKDIR);
	char *workdir = malloc(workdir_len + NULL_TERM_LEN);
	strcat(workdir, homedir);
	strcat(workdir, WORKDIR);
	strcat(workdir, "\0");

	return workdir;
}

static long
next_entry_num(const char *distdir)
{
	long nextNum;
	struct dirent **namelist;
	int i, n = scandir(distdir, &namelist, 0, numsort);

	if(n <= 0) 
		nextNum = 0;
	else {
		for(i = 0; i < n; i++) {
			if(DT_REG == namelist[i]->d_type) {
				char *ptr, *fname = namelist[i]->d_name;	
				nextNum = strtol(fname, &ptr, 10);
				nextNum++;
			}
		}
	}
	
	//free(dir);
	return nextNum;
}

static void
create_lq(entity *lq) 
{
	FILE *f;
	long next_num; 
	char *workdir, *distdir, *fnum, *fname = NULL; 
	size_t next_entry_len, distdir_len, br_len = strlen("/"); 

	workdir = get_workdir();
	distdir_len = sizeof(workdir) + br_len  + sizeof(lq->distillery);
	distdir = malloc(distdir_len + NULL_TERM_LEN);

	strcat(distdir, workdir);
	strcat(distdir, "/");
	strcat(distdir, lq->distillery);

	make_dir(distdir);
	next_num = next_entry_num(distdir);

	const int n = snprintf(NULL, 0, "%ld", next_num);
	assert(n > 0);
	char buf[n+1];
	int c = snprintf(buf, n+1, "%ld", next_num);
	assert(buf[n] == '\0');
	assert(c == n);

	fname = malloc(strlen(distdir) + br_len + strlen(buf) + NULL_TERM_LEN);
	memcpy(fname, distdir, strlen(distdir));
	memcpy(fname + strlen(distdir), "/", br_len);
	memcpy(fname + strlen(distdir) + br_len, buf, strlen(buf) + NULL_TERM_LEN); 

	printf("Opening %s\n", fname);
	f = fopen(fname, "w");
	if(NULL == f) {
		printf("Error opening file %s\n", fname);
		exit(1);
	}

	fprintf(f, "%s\n", lq->distillery);
	fprintf(f, "%d\n", lq->age); 
	fprintf(f, "%f\n", lq->rating);

	free(fname);
	free(workdir);
	free(distdir);
}

static void
new_lq_from_interact() 
{
	entity lq;

	printf("Distillery: ");
	scanf("%s", lq.distillery); // Replace spaces with underscore

	printf("Age: ");
	scanf("%d", &lq.age);

	printf("Rating: ");
	scanf("%lf", &lq.rating);

	create_lq(&lq);
}

static void
new_lq_from_stdin()
{
}

static int
init()
{
	char *workdir = get_workdir();	
	make_dir(workdir);
	free(workdir);
}

int
main(int argc, char **argv) 
{
	init();

	int opterr = 0;
	int c;

	while(-1 != (c = getopt(argc, argv, "n"))) {
		switch(c) {
			case 'n':
				new_lq_from_interact();
				break;
			default:
				new_lq_from_stdin();
				break;
		}
	}
	

	return 0;
}
