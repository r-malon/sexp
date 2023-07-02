#include <stdio.h>
#include <stdlib.h>
#include "sexp.h"

/******************/
/* ERROR MESSAGES */
/******************/

/* ErrorMessage(level, msg, c1, c2)
 * Prints error message on stderr (msg is a format string)
 * c1 and c2 are (optional) integer parameters for the message.
 * Terminates iff level==ERROR, otherwise returns.
 */
void
ErrorMessage(int level, char *msg, int c1, int c2)
{
	fflush(stdout);
	if (level == WARNING)
		fprintf(stderr, "Warning: ");
	else if (level == ERROR)
		fprintf(stderr, "Error: ");
	fprintf(stderr, msg, c1, c2);
	if (level == ERROR)
		exit(1);
}

/**********************/
/* STORAGE ALLOCATION */
/**********************/

/* initializeMemory()
 * Take care of memory initialization 
 */
void
initializeMemory() {} /* nothing in this implementation -- use malloc */

/* sexpAlloc(n)
 * Allocates n bytes of storage. 
 * Terminates execution if no memory available.
 */
char *
sexpAlloc(int n)
{
	char *c = malloc((unsigned int) n);
	if (c == NULL)
		ErrorMessage(ERROR, "Error in sexpAlloc: out of memory!", 0, 0);
	return c;
}

/***********************************/
/* SEXP SIMPLE STRING MANIPULATION */
/***********************************/

/* newSimpleString()
 * Creates and initializes new sexpSimpleString object.
 * Allocates 16-character buffer to hold string.
 */
sexpSimpleString *
newSimpleString()
{
	sexpSimpleString *ss;
	ss = (sexpSimpleString *) sexpAlloc(sizeof (sexpSimpleString));
	ss->length = 0;
	ss->allocatedLength = 16;
	ss->string = (uint8_t *) sexpAlloc(16);
	return ss;
}

/* simpleStringLength(ss)
 * Returns length of simple string 
 */
long int
simpleStringLength(sexpSimpleString *ss)
{
	return ss->length;
}

/* simpleStringString(ss)
 * Returns pointer to character array of simple string 
 */
uint8_t *
simpleStringString(sexpSimpleString *ss)
{
	return ss->string;
}

/* reallocateSimpleString(ss)
 * Changes space allocated to ss.
 * Space allocated is set to roughly 3/2 the current string length, plus 16.
 */
sexpSimpleString *
reallocateSimpleString(sexpSimpleString *ss)
{
	int newsize, i;
	uint8_t *newstring;
	if (ss == NULL)
		ss = newSimpleString();
	if (ss->string == NULL)
		ss->string = (uint8_t *) sexpAlloc(16);
	else {
		newsize = 16 + 3 * (ss->length) / 2;
		newstring = (uint8_t *) sexpAlloc(newsize);
		for (i = 0; i < ss->length; i++)
			newstring[i] = ss->string[i];
		/* Zero string before freeing, as it may be sensitive */
		for (i = 0; i < ss->allocatedLength; i++)
			ss->string[i] = 0;
		free(ss->string);
		ss->string = newstring;
		ss->allocatedLength = newsize;
	}
	return ss;
}

/* appendCharToSimpleString(c,ss)
 * Appends the character c to the end of simple string ss.
 * Reallocates storage assigned to s if necessary to make room for c.
 */
void
appendCharToSimpleString(int c, sexpSimpleString *ss)
{
	if (ss == NULL)
		ss = newSimpleString();
	if (ss->string == NULL || ss->length == ss->allocatedLength)
		ss = reallocateSimpleString(ss);
	ss->string[ss->length] = (uint8_t) (c & 0xFF);
	ss->length++;
}

/****************************/
/* SEXP STRING MANIPULATION */
/****************************/

/* newSexpString()
 * Creates and initializes a new sexpString object.
 * Both the presentation hint and the string are initialized to NULL.
 */
sexpString *
newSexpString()
{
	sexpString *s;
	s = (sexpString *) sexpAlloc(sizeof (sexpString));
	s->type = SEXP_STRING;
	s->presentationHint = NULL;
	s->string = NULL;
	return s;
}

/* sexpStringPresentationHint()
 * Returns presentation hint field of the string 
 */
sexpSimpleString *
sexpStringPresentationHint(sexpString *s)
{
	return s->presentationHint;
}

/* setSexpStringPresentationHint()
 * Assigns the presentation hint field of the string
 */
void
setSexpStringPresentationHint(sexpString *s, sexpSimpleString *ss)
{
	s->presentationHint = ss;
}

/* setSexpStringString()
 * Assigns the string field of the string
 */
void
setSexpStringString(sexpString *s, sexpSimpleString *ss)
{
	s->string = ss;
}

/* sexpStringString()
 * Returns the string field of the string
 */
sexpSimpleString *
sexpStringString(sexpString *s)
{
	return s->string;
}

/* closeSexpString()
 * Finish up string computations after created 
 */
void
closeSexpString(sexpString *s) { (void)s; } /* do nothing in this implementation */

/**************************/
/* SEXP LIST MANIPULATION */
/**************************/

/* newSexpList()
 * Creates and initializes a new sexpList object.
 * Both the first and rest fields are initialized to NULL, which is
 * SEXP's representation of an empty list.
 */
sexpList *
newSexpList()
{
	sexpList *list;
	list = (sexpList *) sexpAlloc(sizeof (sexpList));
	list->type = SEXP_LIST;
	list->first = NULL;
	list->rest = NULL;
	return list;
}

/* sexpAddSexpListObject()
 * Add object to end of list
 */
void
sexpAddSexpListObject(sexpList *list, sexpObject *object)
{
	if (list->first == NULL)
		list->first = object;
	else {
		while (list->rest != NULL)
			list = list->rest;
		list->rest = newSexpList();
		list = list->rest;
		list->first = object;
	}
}

/* closeSexpList()
 * Finish off a list that has just been input
 */
void
closeSexpList(sexpList *list) { (void)list; } /* nothing in this implementation */

/* Iteration on lists.
 * To accomodate different list representations, we introduce the
 * notion of an "iterator".
*/

/* sexpListIter()
 * return the iterator for going over a list 
 */
sexpIter *
sexpListIter(sexpList *list)
{
	return (sexpIter *) list;
}

/* sexpIterNext()
 * advance iterator to next element of list, or else return null
 */
sexpIter *
sexpIterNext(sexpIter *iter)
{
	if (iter == NULL)
		return NULL;
	return (sexpIter *) ((sexpList *) iter)->rest;
}

/* sexpIterObject()
 * return object corresponding to current state of iterator
 */
sexpObject *
sexpIterObject(sexpIter *iter)
{
	if (iter == NULL)
		return NULL;
	return ((sexpList *) iter)->first;
}

/****************************/
/* SEXP OBJECT MANIPULATION */
/****************************/

int
isObjectString(sexpObject *object)
{
	if (((sexpString *) object)->type == SEXP_STRING)
		return true;
	return false;
}

int
isObjectList(sexpObject *object)
{
	if (((sexpList *) object)->type == SEXP_LIST)
		return true;
	return false;
}
