/*-
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2018 Lassi Kortela
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

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

#define TMIN 20
#define TMAX 77
#define HMAX 32

static unsigned char h[HMAX];
static char t[TMAX + 1];
static size_t hlen;
static size_t tlen;

static void
emith(void)
{
	size_t i;

	printf("$");
	for (i = 0; i < hlen; i++)
		printf("%02x", (unsigned int)h[i]);
	printf("\n");
	hlen = 0;
}

static void
pushh(int ch)
{
	if (hlen >= HMAX)
		emith();
	h[hlen++] = ch;
}

static void
emitt(int endl)
{
	char *white;
	char *p;

	white = t + tlen;
	while ((white > t) && isspace(white[-1]))
		white--;
	if (white == t + tlen) {
		*white = 0;
		if (endl == '\n') {
			printf(":%s\n", t);
		} else {
			// assert null endl
			printf(">%s\n", t);
		}
		tlen = 0;
	} else {
		printf(">%s\n", t);
		for (p = white; p < t + tlen; p++)
			pushh(*p);
		*white = 0;
		tlen = 0;
		if (endl)
			pushh(endl);
		emith();
	}
}

static void
emit_file_contents(const char *filename)
{
	FILE *input;
	int ch;
	size_t i;

	hlen = tlen = 0;
	if (!(input = fopen(filename, "rb"))) {
		fprintf(stderr, "cannot open file %s", filename);
		exit(1);
	}
	while ((ch = fgetc(input)) != EOF) {
		if ((ch == '\n') && !hlen) {
			emitt(ch);
		} else if (isprint(ch) || (ch == '\t')) {
			t[tlen++] = ch;
			if (hlen && (tlen >= TMIN))
				emith();
			if (tlen >= TMAX)
				emitt(0);
		} else {
			if (tlen && !hlen)
				emitt(0);
			for (i = 0; i < tlen; i++)
				pushh(t[i]);
			tlen = 0;
			pushh(ch);
		}
	}
	if (hlen)
		emith();
	if (tlen)
		emitt(0);
	if (ferror(input)) {
		fprintf(stderr, "cannot read from file %s", filename);
		exit(1);
	}
	fclose(input);
}

static void
assert_safe_filename(const char *filename)
{
	(void)filename;
}

static void
write_common_attributes(struct stat *st)
{
	static char buf[128];
	time_t tim;
	struct passwd *pw;
	struct group *gr;

	tim = st->st_mtime;
	strftime(buf, sizeof(buf), "%FT%TZ", gmtime(&tim));
	printf("time %s\n", buf);
	if ((pw = getpwuid(st->st_uid))) {
		printf("uname %s\n", pw->pw_name);
	} else {
		printf("uid %lu\n", (unsigned long)st->st_uid);
	}
	if ((gr = getgrgid(st->st_gid))) {
		printf("gname %s\n", gr->gr_name);
	} else {
		printf("gid %lu\n", (unsigned long)st->st_gid);
	}
	printf("mode %lu\n", (unsigned long)st->st_mode);
}

static void
write_link_attribute(const char *filename)
{
	static char buf[128];
	ssize_t len;

	if ((len = readlink(filename, buf, sizeof(buf))) == (ssize_t)-1) {
		fprintf(stderr, "cannot readlink %s", filename);
		exit(1);
	}
	buf[(size_t)len] = 0;
	assert_safe_filename(buf);
	printf("link %s\n", buf);
}

static void
dofile(const char *filename)
{
	static struct stat st;

	assert_safe_filename(filename);
	if (lstat(filename, &st) == -1) {
		fprintf(stderr, "cannot stat file %s", filename);
		exit(1);
	}
	printf("entry %s\n", filename);
	write_common_attributes(&st);
	switch (st.st_mode & S_IFMT) {
	case S_IFREG:
		printf("type file\n");
		printf("size %llu\n", (unsigned long long)st.st_size);
		emit_file_contents(filename);
		break;
	case S_IFDIR:
		printf("type dir\n");
		break;
	case S_IFLNK:
		printf("type link\n");
		write_link_attribute(filename);
		break;
	default:
		fprintf(stderr, "cannot archive directory entry %s", filename);
		exit(1);
		break;
	}
	printf("end entry\n");
}

int
main(int argc, char **argv)
{
	int i;

	printf("format textarc@2018\n");
	for (i = 0; i < argc; i++) {
		dofile(argv[i]);
	}
	return 0;
}
