
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <string.h>

#define UNICODE_FIRST_CHAR 0x0u
#define UNICODE_LAST_CHAR 0x10ffffu

/*	sequence start values */
static const unsigned int utf8_min[4] = {
	UNICODE_FIRST_CHAR, 0x80u, 0x800u, 0x10000u
};

/*	sequence end values */
static const unsigned int utf8_max[4] = {
	0x7fu, 0x7ffu, 0xffffu, UNICODE_LAST_CHAR
};

/*	 unicode symbol output range */
static unsigned int unicode_start = UNICODE_FIRST_CHAR;
static unsigned int unicode_end = UNICODE_LAST_CHAR;

/*	delimiter (option) */
static int delimiter = '\t';

/*	output format (option) */
#define FORMAT_MAXLEN 16
static char format[FORMAT_MAXLEN + 1] = "uch";

static void dump(unsigned char seq_bytes);
static void parse_options(int argc, char *argv[]);
static int parse_unicode_notation(const char *s, const char opt);
static void usage(const char *progname);

static void show_utf8_notation(unsigned int c);
static void show_utf8_char(unsigned int c, const unsigned char *b,
    unsigned char len);
static void show_utf8_bytes(unsigned char b[], unsigned char len);

int main(int argc, char *argv[])
{
	unsigned char seq_len;

	/*	locale string */
	const char *locale;

	parse_options(argc, argv);

	/*	check locale */
	setlocale(LC_CTYPE, "");
	locale = setlocale(LC_CTYPE, NULL);

	if (locale == NULL)
		fprintf(stderr, "setlocale returned NULL.\n");
	else {
		char *dot;

		dot = strrchr(locale, '.');
		if (dot == NULL)
			fprintf(stderr, "setlocale returned '%s', which does "
			    "not specify encoding.\n", locale);
		else {
			if (strcmp(++dot, "UTF-8") != 0)
				fprintf(stderr, "setlocale returned encoding "
				    "'%s', which is not 'UTF-8'.\n", dot);
		}
	}

	/*
	 *	First short (1 byte) sequences are dumped, then the longer
	 *	ones. UTF-8 has a 4 bytes maximum.
	 */
	for (seq_len = 1; seq_len <= 4; seq_len++)
		dump(seq_len);

	exit(0);
}

/*
 *	Show all characters of length seq_bytes.
 */
static void dump(unsigned char seq_bytes)
{
	/*	for first byte, we need to calculate */
	unsigned char seq0;
	unsigned char size0;
	unsigned char mask0;

	/*	the following bytes look all same */
	const unsigned char size = 6;
	const unsigned char seq = 0x80;
	const unsigned char mask = 0x3f;

	/*	the current character to output */
	unsigned int cur;
	size_t format_len;

	/*	length of the output format specifier */
	format_len = strlen(format);

	/*	calculate properties of the first byte */
	switch (seq_bytes) {
	case 1:
		seq0 = 0;
		size0 = 7;
		break;
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		size0 = 7 - seq_bytes;
		seq0 = (((unsigned char)1 << seq_bytes)
		    - 1) << (size0 + 1);
		break;
	default:
		return;
	}

	mask0 = ((unsigned char)1 << size0) - 1;

	/*	determine the first char to output */
	cur = utf8_min[seq_bytes - 1];
	if (unicode_start > cur)
		cur = unicode_start;

	/*	ok, let's output them all */
	for (; cur <= utf8_max[seq_bytes - 1] && cur <= unicode_end;cur++) {

		unsigned int c = cur;
		unsigned char d, b[6];
		size_t fp;

		for (d = seq_bytes - 1; d > 0; d--) {
			b[d] = seq | (c & mask);
			c >>= size;
		}
		b[0] = seq0 | (c & mask0);

		for (fp = 0; fp < format_len; fp++) {
			switch (format[fp]) {
			case 'c':
				show_utf8_char(cur, b, seq_bytes);
				break;
			case 'h':
				show_utf8_bytes(b, seq_bytes);
				break;
			case 'u':
				show_utf8_notation(cur);
				break;
			default:
				break;
			}

			if (fp < format_len - 1)
				putchar(delimiter);
		}

		/*	finish the line */
		putchar('\n');
	}
}

/*	print the unicode notation */
static void show_utf8_notation(unsigned int c)
{
	printf("U+%04X", c);
}

/*	print the hex bytes */
static void show_utf8_bytes(unsigned char b[], unsigned char len)
{
	unsigned char d;

	for (d = 0; d < len; d++)
		printf("%02x%s", b[d], d < len - 1 ?
		    " " : "");
}

static void show_utf8_char(unsigned int c, const unsigned char *b,
    unsigned char len)
{
	/*
	 *	Filter out some characters and output manually.
	 *	The default branch is for the most characters.
	 */
	switch (c) {
	case 0x08:
		printf("<BSP>");
		break;
	case '\t':
		printf("<TAB>");
		break;
	case '\n':
		printf("<RET>");
		break;
	case ' ':
		printf("<SPC>");
		break;
	default:
		fwrite(b, len, 1, stdout);
		break;
	}
}

static void parse_options(int argc, char *argv[])
{
	char ch;

	while ((ch = getopt(argc, argv, "d:e:f:s:")) != -1) {

		int n;

		switch (ch) {
		case 'd':
			delimiter = optarg[0];
			break;
		case 'e':
		case 's':
			n = parse_unicode_notation(optarg, ch);
			if (n < 0)
				usage(argv[0]);
			if (ch == 's')
				unicode_start = (unsigned int)n;
			else
				unicode_end = (unsigned int)n;
			break;
		case 'f':
			if (strlen(optarg) > 0)
				*stpncpy(format, optarg, FORMAT_MAXLEN) = 0;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}
}

static int parse_unicode_notation(const char *s, const char opt)
{
	char *endp;
	unsigned long res;

	if (((s[0] == 'U' || s[0] == 'u') && s[1] == '+')
	    || (s[0] == '0' && s[1] == 'x'))
		s += 2;

	if (*s == 0)
		goto parse_error;

	res = strtoul(s, &endp, 16);
	if (*endp != 0)
		goto parse_error;

	if (res > UNICODE_LAST_CHAR)
		goto parse_error;

	return (int)res;

parse_error:
	fprintf(stderr, "Invalid unicode value for -%c\n", opt);
	return -1;
}

static void usage(const char *progname)
{
	fprintf(stderr, "Syntax:\n\t%s [ options ]\n\nOptions:\n", progname);
	fprintf(stderr,"\t-s code\tfirst character to output (hex, accepts"
	    " prefix \"U+\")\n");
	fprintf(stderr,"\t-e code\tlast character to output (hex, accepts"
	    " prefix \"U+\")\n");
	fprintf(stderr,"\t-f fmt\tformat (default \"uch\"; max length: %u)\n",
	    FORMAT_MAXLEN);
	fprintf(stderr,"\t\th\tshow hex byte sequence\n");
	fprintf(stderr,"\t\tc\tshow UTF-8 character\n");
	fprintf(stderr,"\t\tu\tshow unicode notation (\"U+xxxx\")\n");
	fprintf(stderr,"\t-d char\tdelimiter within one character"
	    " (default '\\t')\n");
	exit(1);
}
