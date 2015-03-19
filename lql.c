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

#include "config.def.h"

#define MAX_RATING_LENGTH 3 
#define MAX_AGE_LENGTH 2
#define WORKDIR "/.lql"
#define NULL_TERM_LEN 1

static char* get_workdir(void);
static char* get_distillery_path(char*, size_t);

typedef struct {
	char distillery[CHAR_MAX];
	signed int age;
	double rating;
} whisky;

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

static void
print_distilleries(void)
{

    char* workdir = get_workdir(); 
    struct dirent **dirlist, *filelist;
    int i, offset = 2, n = scandir(workdir, &dirlist, 0, alphasort);

    if(n <= 0)
        printf("No distilleries found in %s\n", workdir);
    else {
        for(i = offset; i < n; i++) {
            if(DT_DIR == dirlist[i]->d_type) {
                char *dirname = dirlist[i]->d_name;
                char *distillery_path = get_distillery_path(dirname, strlen(dirname));
                DIR *dir = opendir(distillery_path);
                int reviews = 0;

                while((filelist = readdir(dir)) != NULL) {
                    if(DT_REG == filelist->d_type) {
                        reviews++;
                    }
                }

                printf("[%d] %s (%d)\n", i - 1, dirname, reviews);
                free(distillery_path);
            }
        }
    }

    free(workdir);
}

static char* 
get_workdir()
{
	struct passwd *pw = getpwuid(getuid());
	const char *homedir = pw->pw_dir;
	size_t workdir_len = strlen(homedir) + strlen(WORKDIR);
	char *workdir = malloc(workdir_len + NULL_TERM_LEN);
	memcpy(workdir, homedir, strlen(homedir));
	memcpy(workdir + strlen(homedir), WORKDIR, strlen(WORKDIR));
	memcpy(workdir + strlen(homedir) + strlen(WORKDIR), "\0", 1);

	return workdir;
}

static char*
get_distillery_path(char *dist_name, size_t dist_len) 
{
	size_t br_len = strlen("/"); 

	char *workdir = get_workdir();
	size_t distdir_len = strlen(workdir) + br_len  + dist_len;
	char *dest = malloc(distdir_len + NULL_TERM_LEN);

    size_t offset = NULL;
    memcpy(dest, workdir, strlen(workdir));
    memcpy(dest + (offset += strlen(workdir)), "/", strlen("/"));
    memcpy(dest + (offset += strlen("/")), dist_name, dist_len);
    memcpy(dest + (offset += dist_len), "\0", NULL_TERM_LEN);

    free(workdir);
    return dest;
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
	
	return nextNum;
}

static void
create_lq(whisky *lq) 
{
	FILE *f;
	long next_num; 
	char *fname = NULL; 
	size_t br_len = strlen("/"); 

	char *workdir = get_workdir();
	size_t offset = NULL, distdir_len = sizeof(workdir) + br_len  + sizeof(lq->distillery);
	char *distdir = malloc(distdir_len + NULL_TERM_LEN);

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
	memcpy(fname + (offset += strlen(distdir)), "/", br_len);
	memcpy(fname + (offset += br_len), buf, strlen(buf) + NULL_TERM_LEN); 

	f = fopen(fname, "w");
	if(NULL == f) {
		printf("Error opening file %s\n", fname);
		exit(1);
	}

	fprintf(f, "%s\n", lq->distillery);
	fprintf(f, "%d\n", lq->age); 
	fprintf(f, "%f\n", lq->rating);

	free(fname);
	free(workdir); // Remove this once workdir is modified in own function
	free(distdir);
}

static void
new_lq_from_interact(void) 
{
	whisky lq;

	printf("Distillery: ");
	scanf("%s", lq.distillery); 

	printf("Age: ");
	scanf("%d", &lq.age);

	printf("Rating: ");
	scanf("%lf", &lq.rating);

	create_lq(&lq);
}

static void
new_lq_from_stdin(void)
{
}

static void
init(void)
{
	char *workdir = get_workdir();	
	make_dir(workdir);
	free(workdir);
}

int
main(int argc, char **argv) 
{
	init();

	int c;
	while(-1 != (c = getopt(argc, argv, "np"))) {
		switch(c) {
			case 'n':
				new_lq_from_interact();
				break;
            case 'p':
                print_distilleries();
                break;
			default:
				new_lq_from_stdin();
				break;
		}
	}
	

	return 0;
}
