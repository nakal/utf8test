
#include <stdio.h>
#include <stdlib.h>

static void dump(unsigned char seq_bytes);

int main(int argc, char *argv[])
{
	unsigned char seq_len;

	if (argc != 1) {
		fprintf(stderr, "Syntax:\n\t%s\n", argv[0]);
		exit(1);
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

	/*	sequence start values */
	const unsigned int min[4] = {
		0x0u, 0x80u, 0x800u, 0x10000u
	};

	/*	sequence end values */
	const unsigned int max[4] = {
		0x7fu, 0x7ffu, 0xffffu, 0x10ffffu
	};

	/*	the current character to output */
	unsigned int cur;

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

	/*	ok, let's output them all */
	for (cur = min[seq_bytes - 1]; cur <= max[seq_bytes - 1]; cur++) {

		unsigned int c = cur;
		unsigned char d, b[6];

		/*	print the unicode notation */
		printf("U+%04X\t", c);

		/*
		 *	Filter out some characters and output manually.
		 *	The default branch is for the most characters.
		 */
		switch (c) {
		case 0x08:
			printf("<BSP>");
			b[0] = 8;
			break;
		case '\t':
			printf("<TAB>");
			b[0] = 9;
			break;
		case '\n':
			printf("<RET>");
			b[0] = 10;
			break;
		case 0x20:
			printf("<SPC>");
			b[0] = 0x20;
			break;
		default:
			for (d = seq_bytes - 1; d > 0; d--) {
				b[d] = seq | (c & mask);
				c >>= size;
			}

			b[0] = seq0 | (c & mask0);
			fwrite(b, seq_bytes, 1, stdout);
			break;
		}
		putchar('\t');

		/*	also output the hex bytes */
		for (d = 0; d < seq_bytes; d++)
			printf("%02x%s", b[d], d < seq_bytes - 1 ? " " : "");

		/*	finish the line */
		putchar('\n');
	}
}
