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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crc.h"
#include "textarc.h"

#define TMIN 20
#define TMAX 77
#define HMAX 32

static unsigned char h[HMAX];
static char t[TMAX + 1];
static size_t hlen;
static size_t tlen;

static void
check(int rv)
{
	if (rv < 0) {
		fprintf(stderr, "cannot write output file\n");
		exit(1);
	}
}

static void
emith(void)
{
	size_t i;

	check(printf("$"));
	for (i = 0; i < hlen; i++)
		check(printf("%02x", (unsigned int)h[i]));
	check(printf("\n"));
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
			check(printf(":%s\n", t));
		} else {
			// assert null endl
			check(printf(">%s\n", t));
		}
		tlen = 0;
	} else {
		check(printf(">%s\n", t));
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
	unsigned int crc;
	unsigned long size;

	hlen = tlen = 0;
	crc = 0;
	size = 0;
	if (!(input = fopen(filename, "rb"))) {
		fprintf(stderr, "cannot open file %s", filename);
		exit(1);
	}
	while ((ch = fgetc(input)) != EOF) {
		crc = crc32_byte(crc, ch);
		size++;
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
	crc = crc32_done(crc, size);
	check(printf("size %lu\n", size));
	check(printf("cksum %u\n", crc));
}

static void
assert_safe_basename(const char *s)
{
	(void)s;
}

static void
assert_safe_filename(const char *filename)
{
	(void)filename;
}

static void
write_name(const char *name, const char *value)
{
	if (!value)
		return;
	assert_safe_basename(value);
	check(printf("%s %s\n", name, value));
}

static void
write_ulong(const char *name, unsigned long value)
{
	if (value == (unsigned long)-1)
		return;
	check(printf("%s %lu\n", name, value));
}

void
write_entry(struct textarc_entry *e)
{
	assert_safe_filename(e->filename);
	check(printf("entry %s\n", e->filename));
	check(printf("time %04d-%02d-%02dT%02d:%02d:%02dZ\n", e->year, e->month,
	        e->day, e->hour, e->minute, e->second));
	write_ulong("mode", e->mode);
	write_name("uname", e->uname);
	write_ulong("uid", e->uid);
	write_name("gname", e->gname);
	write_ulong("gid", e->gid);
	write_name("type", e->type);
	if (!strcmp(e->type, "file")) {
		emit_file_contents(e->filename);
	} else if (!strcmp(e->type, "link")) {
		assert_safe_filename(e->link);
		check(printf("link %s\n", e->link));
	}
	check(printf("end entry\n"));
}
