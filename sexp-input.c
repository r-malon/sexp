#include "sexp.h"

/**************************************/
/* CHARACTER ROUTINES AND DEFINITIONS */
/**************************************/

char upper[256];		/* upper[c] is upper case version of c */
char decdigit[256];		/* decdigit[c] is nonzero if c is a dec digit */
char decvalue[256];		/* decvalue[c] is value of c as dec digit */
char hexdigit[256];		/* hexdigit[c] is nonzero if c is a hex digit */
char hexvalue[256];		/* hexvalue[c] is value of c as a hex digit */
char base64digit[256];	/* base64char[c] is nonzero if c is base64 digit */
char base64value[256];	/* base64value[c] is value of c as base64 digit */
char tokenchar[256];	/* tokenchar[c] is true if c can be in a token */
char alpha[256];		/* alpha[c] is true if c is alphabetic /A-Za-z/ */

/* initializeCharacterTables()
 * Initializes all of the above arrays
 */
void
initializeCharacterTables()
{
	int i;
	for (i = 0; i < 256; i++)
		upper[i] = i;
	for (i = 'a'; i <= 'z'; i++)
		upper[i] = i - 'a' + 'A';
	for (i = 0; i <= 255; i++)
		alpha[i] = decdigit[i] = base64digit[i] = false;
	for (i = '0'; i <= '9'; i++) {
		base64digit[i] = hexdigit[i] = decdigit[i] = true;
		decvalue[i] = hexvalue[i] = i - '0';
		base64value[i] = (i - '0') + 52;
	}
	for (i = 'a'; i <= 'f'; i++) {
		hexdigit[i] = hexdigit[upper[i]] = true;
		hexvalue[i] = hexvalue[upper[i]] = i - 'a' + 10;
	}
	for (i = 'a'; i <= 'z'; i++) {
		base64digit[i] = base64digit[upper[i]] = true;
		alpha[i] = alpha[upper[i]] = true;
		base64value[i] = i - 'a' + 26;
		base64value[upper[i]] = i - 'a';
	}
	base64digit['+'] = base64digit['/'] = true;
	base64value['+'] = 62;
	base64value['/'] = 63;
	base64value['='] = 0;
	for (i = 0; i < 255; i++)
		tokenchar[i] = false;
	for (i = 'a'; i <= 'z'; i++)
		tokenchar[i] = tokenchar[upper[i]] = true;
	for (i = '0'; i <= '9'; i++)
		tokenchar[i] = true;
	tokenchar['-'] = true;
	tokenchar['.'] = true;
	tokenchar['/'] = true;
	tokenchar['_'] = true;
	tokenchar[':'] = true;
	tokenchar['*'] = true;
	tokenchar['+'] = true;
	tokenchar['='] = true;
}

/* isBase64Digit(c)
 * returns true if c is a Base64 digit A-Za-Z0-9+/
 */
int
isBase64Digit(int c)
{
	return (c >= 0 && c <= 255) && base64digit[c];
}

/* isTokenChar(c)
 * Returns true if c is allowed in a token
 */
int
isTokenChar(int c)
{
	return (c >= 0 && c <= 255) && tokenchar[c];
}

/**********************/
/* SEXP INPUT STREAMS */
/**********************/

/* changeInputByteSize(is,newByteSize)
 */
void
changeInputByteSize(sexpInputStream *is, int newByteSize)
{
	is->byteSize = newByteSize;
	is->nBits = 0;
	is->bits = 0;
}

/* getChar(is)
 * This is one possible character input routine for an input stream.
 * (This version uses the standard input stream.)
 * getChar places next 8-bit character into is->nextChar.
 * It also updates the count of number of 8-bit characters read.
 * The value EOF is obtained when no more input is available.  
 * This code handles 4/6/8-bit channels.
 */
void
getChar(sexpInputStream *is)
{
	int c;
	if (is->nextChar == EOF) {
		is->byteSize = 8;
		return;
	}
	while ((c = is->nextChar = fgetc(is->inputFile)) != EOF) {
		/* End of region reached; return terminating character, after
			checking for unused bits */
		if ((is->byteSize == 6 && (c == '|' || c == '}'))
			|| (is->byteSize == 4 && (c == '#')))
		{
			if (is->nBits > 0 && (((1 << is->nBits) - 1) & is->bits) != 0)
				warn("%d-bit region ended with %d unused bits left-over",
					is->byteSize, is->nBits);
			changeInputByteSize(is, 8);
			return;
		/* ignore whitespace in hex and Base64 regions */
		} else if (is->byteSize != 8 && isspace(c));
		/* ignore equals sign in Base64 regions */
		else if (is->byteSize == 6 && c == '=');
		else if (is->byteSize == 8) {
			is->count++;
			return;
		} else if (is->byteSize < 8) {
			is->bits = is->bits << is->byteSize;
			is->nBits += is->byteSize;
			if (is->byteSize == 6 && isBase64Digit(c))
				is->bits = is->bits | base64value[c];
			else if (is->byteSize == 4 && isxdigit(c))
				is->bits = is->bits | hexvalue[c];
			else
				err(1, "character %c found in %d-bit coding region",
					(int) is->nextChar, is->byteSize);
			if (is->nBits >= 8) {
				is->nextChar = (is->bits >> (is->nBits - 8)) & 0xFF;
				is->nBits -= 8;
				is->count++;
				return;
			}
		}
	}
}

/* newSexpInputStream()
 * Creates and initializes a new sexpInputStream object.
 * (Prefixes stream with one blank and initializes it,
 *  so that it reads from standard input.)
 */
sexpInputStream *
newSexpInputStream()
{
	sexpInputStream *is;
	is = malloc(sizeof (sexpInputStream));
	is->nextChar = ' ';
	is->getChar = getChar;
	is->count = -1;
	is->byteSize = 8;
	is->bits = 0;
	is->nBits = 0;
	is->inputFile = stdin;
	return is;
}

/*****************************************/
/* INPUT (SCANNING AND PARSING) ROUTINES */
/*****************************************/

/* skipWhiteSpace(is)
 * Skip over any whitespace on the given sexpInputStream.
 */
void
skipWhiteSpace(sexpInputStream *is)
{
	while (isspace(is->nextChar))
		is->getChar(is);
}

/* skipChar(is, c)
 * Skip the following input character on input stream is, if it is
 * equal to the character c. If it is not equal, then an error occurs.
 */
void
skipChar(sexpInputStream *is, int c)
{
	if (is->nextChar == c)
		is->getChar(is);
	else
		err(1, "character %x (hex) found where %c (char) expected",
			(int) is->nextChar, (int) c);
}

/* scanToken(is, ss)
 * Scan one or more characters into simple string ss as a token.
 */
void
scanToken(sexpInputStream *is, sexpSimpleString *ss)
{
	skipWhiteSpace(is);
	while (isTokenChar(is->nextChar)) {
		appendCharToSimpleString(is->nextChar, ss);
		is->getChar(is);
	}
	return;
}

/* scanToEOF(is)
 * Scan one or more characters (until EOF reached)
 * Return an object that is just that string
 */
sexpObject *
scanToEOF(sexpInputStream *is)
{
	sexpSimpleString *ss = newSimpleString();
	sexpString *s = newSexpString();
	setSexpStringString(s, ss);
	skipWhiteSpace(is);
	while (is->nextChar != EOF) {
		appendCharToSimpleString(is->nextChar, ss);
		is->getChar(is);
	}
	return (sexpObject *) s;
}

/* scanDecimal(is)
 * Returns long integer that is value of decimal number
 */
unsigned long int
scanDecimal(sexpInputStream *is)
{
	unsigned long int value = 0L;
	int i = 0;
	while (isdigit(is->nextChar)) {
		value = value * 10 + decvalue[is->nextChar];
		is->getChar(is);
		if (i++ > 8)
			err(1, "Decimal number %d... too long.", (int) value);
	}
	return value;
}

/* scanVerbatimString(is, ss, length)
 * Reads verbatim string of given length into simple string ss.
 */
void
scanVerbatimString(sexpInputStream *is, sexpSimpleString *ss, long int length)
{
	long int i = 0L;
	skipWhiteSpace(is);
	skipChar(is, ':');
	if (length == -1L)	/* no length was specified */
		err(1, "%s", "Verbatim string had no declared length.");
	for (i = 0; i < length; i++) {
		appendCharToSimpleString(is->nextChar, ss);
		is->getChar(is);
	}
	return;
}

/* scanQuotedString(is, ss, length)
 * Reads quoted string of given length into simple string ss.
 * Handles ordinary C escapes. 
 * If of indefinite length, length is -1.
 */
void
scanQuotedString(sexpInputStream *is, sexpSimpleString *ss, long int length)
{
	int c;
	skipChar(is, '"');
	while (length == -1 || simpleStringLength(ss) <= length) {
		if (is->nextChar == '\"') {
			if (length == -1 || (simpleStringLength(ss) == length)) {
				skipChar(is, '\"');
				return;
			} else
				err(1, "Quoted string ended too early. Declared length was %d",
					(int) length);
		} else if (is->nextChar == '\\') {	/* handle C escape sequence */
			is->getChar(is);
			c = is->nextChar;
			if (c == 'b')
				appendCharToSimpleString('\b', ss);
			else if (c == 't')
				appendCharToSimpleString('\t', ss);
			else if (c == 'v')
				appendCharToSimpleString('\v', ss);
			else if (c == 'n')
				appendCharToSimpleString('\n', ss);
			else if (c == 'f')
				appendCharToSimpleString('\f', ss);
			else if (c == 'r')
				appendCharToSimpleString('\r', ss);
			else if (c == '\"')
				appendCharToSimpleString('\"', ss);
			else if (c == '\'')
				appendCharToSimpleString('\'', ss);
			else if (c == '\\')
				appendCharToSimpleString('\\', ss);
			else if (c >= '0' && c <= '7') {	/* octal number */
				int j, val;
				val = 0;
				for (j = 0; j < 3; j++) {
					if (c >= '0' && c <= '7') {
						val = (val << 3) | (c - '0');
						if (j < 2) {
							is->getChar(is);
							c = is->nextChar;
						}
					} else
						err(1, "Octal character \\%o... too short.", val);
				}
				if (val > 255)
					err(1, "Octal character \\%o... too big.", val);
				appendCharToSimpleString(val, ss);
			} else if (c == 'x') {	/* hexadecimal number */
				int j, val;
				val = 0;
				is->getChar(is);
				c = is->nextChar;
				for (j = 0; j < 2; j++) {
					if (isxdigit(c)) {
						val = (val << 4) | hexvalue[c];
						if (j < 1) {
							is->getChar(is);
							c = is->nextChar;
						}
					} else
						err(1, "Hex character \\x%x... too short.", val);
				}
				appendCharToSimpleString(val, ss);
			} else if (c == '\n') {	/* ignore backslash line feed */
				/* also ignore following carriage-return if present */
				is->getChar(is);
				if (is->nextChar != '\r')
					goto gotnextchar;
			} else if (c == '\r') {	/* ignore backslash carriage-return */
				/* also ignore following linefeed if present */
				is->getChar(is);
				if (is->nextChar != '\n')
					goto gotnextchar;
			} else
				warn("Escape character \\%c... unknown.", c);
		}	/* end of handling escape sequence */
		else
			appendCharToSimpleString(is->nextChar, ss);
		is->getChar(is);
	  gotnextchar:;
	}	/* end of main while loop */
	return;
}

/* scanHexString(is, ss, length)
 * Reads hexadecimal string into simple string ss.
 * String is of given length result, or length = -1 if indefinite length.
 */
void
scanHexString(sexpInputStream *is, sexpSimpleString *ss, long int length)
{
	changeInputByteSize(is, 4);
	skipChar(is, '#');
	while (is->nextChar != EOF && (is->nextChar != '#' || is->byteSize == 4)) {
		appendCharToSimpleString(is->nextChar, ss);
		is->getChar(is);
	}
	skipChar(is, '#');
	if (simpleStringLength(ss) != length && length >= 0)
		warn("Hex string has length %d different than declared length %d",
			(int) simpleStringLength(ss), (int) length);
}

/* scanBase64String(is, ss, length)
 * Reads base64 string into simple string ss.
 * String is of given length result, or length = -1 if indefinite length.
 */
void
scanBase64String(sexpInputStream *is, sexpSimpleString *ss, long int length)
{
	changeInputByteSize(is, 6);
	skipChar(is, '|');
	while (is->nextChar != EOF && (is->nextChar != '|' || is->byteSize == 6)) {
		appendCharToSimpleString(is->nextChar, ss);
		is->getChar(is);
	}
	skipChar(is, '|');
	if (simpleStringLength(ss) != length && length >= 0)
		warn("Base64 string has length %d different than declared length %d",
			(int) simpleStringLength(ss), (int) length);
}

/* scanSimpleString(is)
 * Reads and returns a simple string from the input stream.
 * Determines type of simple string from the initial character, and
 * dispatches to appropriate routine based on that. 
 */
sexpSimpleString *
scanSimpleString(sexpInputStream *is)
{
	long int length;
	sexpSimpleString *ss;
	ss = newSimpleString();
	skipWhiteSpace(is);
	/* Note that it is important in the following code to test for token-ness
	 * before checking the other cases, so that a token may begin with ":",
	 * which would otherwise be treated as a verbatim string missing a length.
	 */
	if (isTokenChar(is->nextChar) && !isdigit(is->nextChar))
		scanToken(is, ss);
	else if (isdigit(is->nextChar)
			 || is->nextChar == '\"'
			 || is->nextChar == '#'
			 || is->nextChar == '|'
			 || is->nextChar == ':') {
		if (isdigit(is->nextChar))
			length = scanDecimal(is);
		else
			length = -1L;
		if (is->nextChar == '\"')
			scanQuotedString(is, ss, length);
		else if (is->nextChar == '#')
			scanHexString(is, ss, length);
		else if (is->nextChar == '|')
			scanBase64String(is, ss, length);
		else if (is->nextChar == ':')
			scanVerbatimString(is, ss, length);
	} else
		err(1, "illegal character at position %d: %d (decimal)",
			is->count, is->nextChar);
	if (simpleStringLength(ss) == 0)
		warn("%s", "Simple string has zero length.");
	return ss;
}

/* scanString(is)
 * Reads and returns a string [presentationhint]string from input stream.
 */
sexpString *
scanString(sexpInputStream *is)
{
	sexpString *s;
	sexpSimpleString *ss;
	s = newSexpString();
	/* scan presentation hint */
	if (is->nextChar == '[') {
		skipChar(is, '[');
		ss = scanSimpleString(is);
		setSexpStringPresentationHint(s, ss);
		skipWhiteSpace(is);
		skipChar(is, ']');
		skipWhiteSpace(is);
	}
	ss = scanSimpleString(is);
	setSexpStringString(s, ss);
	closeSexpString(s);
	return s;
}

/* scanList(is)
 * Read and return a sexpList from the input stream.
 */
sexpList *
scanList(sexpInputStream *is)
{
	sexpList *list;
	sexpObject *object;
	skipChar(is, '(');
	skipWhiteSpace(is);
	list = newSexpList();
	if (is->nextChar == ')') {
	/* err(1, "List () with no contents is illegal."); */
		;	/* OK */
	} else {
		object = scanObject(is);
		sexpAddSexpListObject(list, object);
	}
	while (true) {
		skipWhiteSpace(is);
		if (is->nextChar == ')') {
			/* We just grabbed last element of list */
			skipChar(is, ')');
			closeSexpList(list);
			return list;
		} else {
			object = scanObject(is);
			sexpAddSexpListObject(list, object);
		}
	}
}

/* scanObject(is)
 * Reads and returns a sexpObject from the given input stream.
 */
sexpObject *
scanObject(sexpInputStream *is)
{
	sexpObject *object;
	skipWhiteSpace(is);
	if (is->nextChar == '{') {
		changeInputByteSize(is, 6);	/* order of this statement and next is */
		skipChar(is, '{');			/* Important! */
		object = scanObject(is);
		skipChar(is, '}');
		return object;
	} else {
		if (is->nextChar == '(')
			object = (sexpObject *) scanList(is);
		else
			object = (sexpObject *) scanString(is);
		return object;
	}
}
