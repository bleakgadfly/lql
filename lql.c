#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <pwd.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_RATING_LENGTH 3 
#define MAX_AGE_LENGTH 2
#define WORKDIR ".lql"

int 
make_dir(char *dirname) 
{
	struct stat st = {0};
	if(-1 == stat(dirname, &st)) 
		mkdir(dirname, 0700);
	else 
		return -1;
	
	return 0;
}

void
new_lq() 
{
	char distillery[CHAR_MAX];
	int age;
	double rating;

	printf("Distillery: ");
	scanf("%s", distillery);

	printf("Age: ");
	scanf("%d", &age);

	printf("Rating: ");
	scanf("%lf", &rating);
}

int
init()
{
	struct passwd *pw = getpwuid(getuid());
	const char *homedir = pw->pw_dir;
	size_t homedir_len = strlen(homedir);
	size_t workdir_len = homedir_len + strlen(WORKDIR);
	char workdir[workdir_len];
	strcat(workdir, homedir);
	strcat(workdir, "/");
	strcat(workdir, WORKDIR);

	int made_dir = make_dir(workdir);
	assert(made_dir == 0);
}

int
main(int argc, char **argv) 
{
	init();

	int opterr = 0;
	int c;

	while(-1 != (c = getopt(argc, argv, "n:"))) {
		switch(c) {
			case 'n':
				new_lq();
				break;
			default:
				break;
		}
	}
	

	return 0;
}
