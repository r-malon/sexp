#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <err.h>

#ifndef SEXP_H
#define SEXP_H

#define DEFAULTLINELENGTH 75

/* PRINTING MODES */
enum Mode {
	CANONICAL=1,	/* Standard for hashing and tranmission */
	BASE64,			/* Base64 version of canonical */
	ADVANCED		/* Pretty-printed */
};

/* TYPES OF OBJECTS */
enum ObjectType {
	SEXP_STRING=1,
	SEXP_LIST
};

typedef struct sexpSimpleString {
	long int length;
	long int allocatedLength;
	uint8_t *string;
} sexpSimpleString;

typedef struct sexpString {
	enum ObjectType type;
	sexpSimpleString *presentationHint;
	sexpSimpleString *string;
} sexpString;

/* If first is NULL, then rest must also be NULL; this is empty list */
typedef struct sexpList {
	enum ObjectType type;
	union sexpObject *first;
	struct sexpList *rest;
} sexpList;

/* Allows a pointer to something of either type */
typedef union sexpObject {
	sexpString string;
	sexpList list;
} sexpObject;

/* an "iterator" for going over lists */
/* In this implementation, it is the same as a list */
typedef sexpList sexpIter;

typedef struct sexpInputStream {
	int nextChar;		/* character currently being scanned */
	int byteSize;		/* 4 or 6 or 8 == currently scanning mode */
	int bits;			/* Bits waiting to be used */
	int nBits;			/* number of such bits waiting to be used */
	void (*getChar)();
	int count;			/* number of 8-bit characters output by getChar */
	FILE *inputFile;	/* where to get input, if not stdin */
} sexpInputStream;

typedef struct sexpOutputStream {
	long int column;		/* column where next character will go */
	long int maxcolumn;		/* max usable column, or -1 if no maximum */
	long int indent;		/* current indentation level (starts at 0) */
	void (*putChar)();		/* output a character */
	void (*newLine)();		/* go to next line (and indent) */
	int byteSize;			/* 4 or 6 or 8 depending on output mode */
	int bits;				/* bits waiting to go out */
	int nBits;				/* number of bits waiting to go out */
	long int base64Count;	/* number of hex or base64 chars printed in this region */
	enum Mode mode;
	FILE *outputFile;		/* where to put output, if not stdout */
} sexpOutputStream;

/* Function prototypes */

/* sexp-basic */
void initializeMemory();
sexpSimpleString *newSimpleString();
long int simpleStringLength();
uint8_t *simpleStringString();
sexpSimpleString *reallocateSimpleString();
void appendCharToSimpleString();
sexpString *newSexpString();
sexpSimpleString *sexpStringPresentationHint();
sexpSimpleString *sexpStringString();
void setSexpStringPresentationHint();
void setSexpStringString();
void closeSexpString();
sexpList *newSexpList();
void sexpAddSexpListObject();
void closeSexpList();
sexpIter *sexpListIter();
sexpIter *sexpIterNext();
sexpObject *sexpIterObject();
int isObjectString();
int isObjectList();

/* sexp-input */
void initializeCharacterTables();
int isWhiteSpace();
int isDecDigit();
int isHexDigit();
int isBase64Digit();
int isTokenChar();
int isAlpha();
void changeInputByteSize();
void getChar();
sexpInputStream *newSexpInputStream();
void skipWhiteSpace();
void skipChar();
void scanToken();
sexpObject *scanToEOF();
unsigned long int scanDecimal();
void scanVerbatimString();
void scanQuotedString();
void scanHexString();
void scanBase64String();
sexpSimpleString *scanSimpleString();
sexpString *scanString();
sexpList *scanList();
sexpObject *scanObject();

/* sexp-output */
void putChar();
void varPutChar();
void changeOutputByteSize();
void flushOutput();
void newLine();
sexpOutputStream *newSexpOutputStream();
void printDecimal();
void canonicalPrintVerbatimSimpleString();
void canonicalPrintString();
void canonicalPrintList();
void canonicalPrintObject();
void base64PrintWholeObject();
int canPrintAsToken();
int significantNibbles();
void advancedPrintTokenSimpleString();
int advancedLengthSimpleStringToken();
void advancedPrintVerbatimSimpleString();
int advancedLengthSimpleStringVerbatim();
void advancedPrintBase64SimpleString();
void advancedPrintHexSimpleString();
int canPrintAsQuotedString();
void advancedPrintQuotedStringSimpleString();
void advancedPrintSimpleString();
void advancedPrintString();
int advancedLengthSimpleStringBase64();
int advancedLengthSimpleString();
int advancedLengthString();
int advancedLengthList();
void advancedPrintList();
void advancedPrintObject();

#endif /* SEXP_H */
