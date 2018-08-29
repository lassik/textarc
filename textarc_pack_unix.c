#include <sys/types.h>

#include <sys/stat.h>

#include <ctype.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "textarc.h"

static struct textarc_entry e;
static struct stat st;

static char *
xstrdup(const char *s)
{
	char *d;
	size_t n;

	n = strlen(s) + 1;
	if (!(d = malloc(n))) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}
	memcpy(d, s, n);
	return d;
}

static void
unpack_unix_filetime(time_t filetime)
{
	struct tm *tm;

	tm = gmtime(&filetime);
	e.year = tm->tm_year + 1900;
	e.month = tm->tm_mon + 1;
	e.day = tm->tm_mday;
	e.hour = tm->tm_hour;
	e.minute = tm->tm_min;
	e.second = tm->tm_sec;
}

static char *
get_symlink_target(const char *filename)
{
	static char buf[128];
	ssize_t len;

	if ((len = readlink(filename, buf, sizeof(buf) - 1)) == (ssize_t)-1) {
		fprintf(stderr, "cannot readlink %s", filename);
		exit(1);
	}
	buf[(size_t)len] = 0;
	return xstrdup(buf);
}

static void
write_unix_entry(const char *filename)
{
	struct passwd *pw;
	struct group *gr;

	memset(&e, 0, sizeof(e));
	if (lstat(filename, &st) == -1) {
		fprintf(stderr, "cannot stat file %s", filename);
		exit(1);
	}
	e.filename = filename;
	unpack_unix_filetime(st.st_mtime);
	e.mode = st.st_mode & ~S_IFMT;
	e.uid = st.st_uid;
	e.gid = st.st_gid;
	if ((pw = getpwuid(st.st_uid))) {
		e.uname = xstrdup(pw->pw_name);
	}
	if ((gr = getgrgid(st.st_gid))) {
		e.gname = xstrdup(gr->gr_name);
	}
	switch (st.st_mode & S_IFMT) {
	case S_IFREG:
		e.type = "file";
		break;
	case S_IFDIR:
		e.type = "dir";
		break;
	case S_IFLNK:
		e.type = "link";
		e.link = get_symlink_target(filename);
		break;
	default:
		fprintf(stderr, "cannot archive directory entry %s",
		        filename);
		exit(1);
		break;
	}
	write_entry(&e);
	free(e.link);
	free(e.uname);
	free(e.gname);
}

int
main(int argc, char **argv)
{
	int i;

	for (i = 1; i < argc; i++) {
		write_unix_entry(argv[i]);
	}
	return 0;
}
