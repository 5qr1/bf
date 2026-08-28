/*bf: build files
 (c) 5qr1 WTFPL 2026 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

/* todo: dont use globals here :- ( */
static int Argc;
static char **Argv;

#define LENGTH(x) sizeof(x)/sizeof(x[0])
#define BUFSIZE 1024
static void eprint(int err, const char *fmt, ...);
static void *emalloc(void *in, size_t n);
static char *apsprintf(char *in, const char *fmt, ...);
static char *lfiletobuf(FILE *in);
static char *process(char **ret, char *st, char *en);
static int script(char **ret, char *st, char *en);
static int variable(char **ret, char *st, char *en);

int
main(int argc, char **argv) {
	Argc = argc;
	Argv = argv;

	char *buf = lfiletobuf(stdin);
	process(NULL, buf, &buf[strlen(buf)]);

	free(buf);
	exit(0);
}

static void
eprint(int err, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	exit(err);
}

static void
*emalloc(void *in, size_t n) {
	void *out;
	if(n <= 0)
		n = 1;
	if(in)
		out = realloc(in, n);
	else
		out = malloc(n);
	if(!out)
		eprint(EXIT_FAILURE, "emalloc() failed\n");

	return out;
}

/* this sucks a bit */
static char
*apsprintf(char *in, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	int l = vsnprintf(NULL, 0, fmt, ap), pl = strlen(in);
	va_end(ap);

	if(!l)
		return in;

	in = emalloc(in, pl + l + 1);
	va_start(ap, fmt);
	vsnprintf(in + pl, l + 1, fmt, ap);
	va_end(ap);

	return in;
}

static char
*lfiletobuf(FILE *in) {
	if(!in)
		return NULL;
	
	char *buf = emalloc(NULL, BUFSIZE);
	size_t s, len = 0, bsize = 2 * BUFSIZE;
	while((s = fread(buf + len, 1, BUFSIZE, in))) {
		len += s;
		if(BUFSIZE + len + 1 > bsize) {
			bsize += BUFSIZE;
			buf = emalloc(buf, bsize);
		}
	}
	buf[len] = '\0';

	return buf;
}

static char
*process(char **ret, char *st, char *en) {
	for(char *p = st; p < en; p++) {
		if(p[0] != '%') {
			if(!ret)
				fputc(p[0], stdout);
			else
				*ret = apsprintf(*ret, "%c", p[0]);
			continue;
		}

		int ch = 0;

		if(p + 1 < en && p[1] == '[')
			ch = script(ret, p, en);
		else if(p + 1 < en)
			ch = variable(ret, p, en);

		if(!ch)
			continue;
		else
			p += ch - 1;
	}

	if(ret)
		return *ret;
	else
		return 0;
}

static int
script(char **ret, char *st, char *en) {
	if(!st || !en || st == en)
		return 0;

	char *p = st + 2;
	for(;p < en && p[0] != ']'; p++);
	if(p >= en)
		return (p - st) + 1;

	char *buf = emalloc(NULL, p - (st + 2) + 1), *pbuf = emalloc(NULL, 1);
	memcpy(buf, st + 2, p - (st + 2));
	buf[p - (st + 2)] = '\0';
	pbuf[0] = '\0';

	pbuf = process(&pbuf, buf, &buf[strlen(buf)]);
	FILE *sh;
	if(!(sh = popen(pbuf, "r")))
		eprint(EXIT_FAILURE, "popen failed: %s", buf);
	
	char *res = lfiletobuf(sh);
	if(!ret)
		printf("%s", res);
	else
		*ret = apsprintf(*ret, "%s", res);
	
	free(buf);
	free(pbuf);
	return (p - st) + 1;
}

static int
variable(char **ret, char *st, char *en) {
	if(!st || !en || st == en)
		return 0;
	
	char *p = st + 1;
	for(;p < en && p[0] != ' ' && p[0] != '\n'; p++);

	char *buf = emalloc(NULL, p - (st + 1) + 1);
	memcpy(buf, st + 1, p - (st + 1));
	buf[p - (st + 1)] = '\0';
	
	char *b = buf;
	for(;b < &buf[strlen(buf)] && !(isdigit(b[0])); b++);
	if(!(isdigit(b[0])))
		return p - st;
	
	int n = atoi(buf);
	if(n > Argc - 1)
		return p - st;

	if(ret)
		*ret = apsprintf(*ret, "%s", Argv[n]);
	else
		printf("%s", Argv[n]);

	free(buf);
	return p - st;
}
