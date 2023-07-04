#include "sexp.h"

int
main(int argc, char **argv)
{
	char *c; int i;
	bool swa = true, swb = true, swc = true, swp = true, sws = false, 
		swx = true, swl = false;
	sexpObject *object;
	sexpInputStream *is;
	sexpOutputStream *os;
	initializeCharacterTables();
	initializeMemory();
	is = newSexpInputStream();
	os = newSexpOutputStream();

	/* process switches */
	if (argc > 1)
		swa = swb = swc = swp = sws = swx = swl = false;

	for (i = 1; i < argc; i++) {
		c = argv[i];
		if (*c != '-') {
			fprintf(stderr, "Unrecognized switch %s\n", c);
			exit(1);
		}
		c++;
		if (*c == 'a')			/* advanced output */
			swa = true;
		else if (*c == 'b')		/* Base64 output */
			swb = true;
		else if (*c == 'c')		/* canonical output */
			swc = true;
		else if (*c == 'i') {	/* input file */
			if (i + 1 < argc)
				i++;
			is->inputFile = fopen(argv[i], "r");
			if (is->inputFile == NULL)
				err(1, "%s", "Can't open input file.");
		} else if (*c == 'l')	/* suppress linefeeds after output */
			swl = true;
		else if (*c == 'o') {	/* output file */
			if (i + 1 < argc)
				i++;
			os->outputFile = fopen(argv[i], "w");
			if (os->outputFile == NULL)
				err(1, "%s", "Can't open output file.");
		} else if (*c == 'p')	/* prompt for input */
			swp = true;
		else if (*c == 's')		/* treat input as one big string */
			sws = true;
		else if (*c == 'w') {	/* set output width */
			if (i + 1 < argc)
				i++;
			os->maxcolumn = atoi(argv[i]);
		} else if (*c == 'x')	/* execute repeatedly */
			swx = true;
		else {
			fprintf(stderr, "Unrecognized switch: %s\n", argv[i]);
			exit(1);
		}
	}
	if (swa == false && swb == false && swc == false)
		swc = true;		/* must have some output format! */

	/* main loop */
	if (swp)
		is->nextChar = -2;	/* this is not EOF */
	else
		is->getChar(is);

	while (is->nextChar != EOF) {
		if (swp) {
			fprintf(stderr, "Input: ");
			fflush(stdout);
		}

		changeInputByteSize(is, 8);
		if (is->nextChar == -2)
			is->getChar(is);

		skipWhiteSpace(is);
		if (is->nextChar == EOF)
			break;

		if (sws)
			object = scanToEOF(is);
		else
			object = scanObject(is);

		if (swc) {
			if (swp) {
				fprintf(stderr, "Canonical output: ");
				fflush(stdout);
				os->newLine(os, ADVANCED);
			}
			canonicalPrintObject(os, object);
			if (!swl) {
				putchar('\n');
				fflush(stdout);
			}
		}

		if (swb) {
			if (swp) {
				fprintf(stderr, "Base64 (of canonical) output: ");
				fflush(stdout);
				os->newLine(os, ADVANCED);
			}
			base64PrintWholeObject(os, object);
			if (!swl) {
				putchar('\n');
				fflush(stdout);
			}
		}

		if (swa) {
			if (swp) {
				fprintf(stderr, "Advanced transport output: ");
				fflush(stdout);
				os->newLine(os, ADVANCED);
			}
			advancedPrintObject(os, object);
			if (!swl) {
				putchar('\n');
				fflush(stdout);
			}
		}

		if (!swx)
			break;

		if (!swp)
			skipWhiteSpace(is);
		else if (!swl) {
			putchar('\n');
			fflush(stdout);
		}
	}

	return 0;
}
