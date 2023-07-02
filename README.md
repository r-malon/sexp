# SEXP---(S-expressions)
---
(from http://people.csail.mit.edu/rivest/sexp.html)
---

S-expressions are a data structure for representing complex data. They are a variation on LISP S-expressions. (Lisp was invented by John McCarthy).

We have adapted S-expressions for use in SDSI and SPKI. The work reported here is, however, application-independent. (That is, we will use S-expressions in the SPKI/SDSI work, but have developed it and presented it in an application-independent manner, so that others may most easily consider adopting it for other applications.)

SDSI has been developed by Professors Ronald L. Rivest and Butler Lampson of MIT's Laboratory for Computer Science, members of LCS's Cryptography and Information Security research group.

SDSI research is supported by DARPA contract DABT63-96-C-0018, "Security for Distributed Computer Systems".

SPKI has been developed by Carl Ellison and others in the IETF SPKI working group.

We are currently merging the designs to form SPKI/SDSI 2.0. This design will use S-expressions as the basic data structure.

## References and Documentation

[SEXP IETF draft](https://datatracker.ietf.org/doc/draft-rivest-sexp/)

The following code reads and parses S-expressions. It also prints out the S-expressions in a variety of formats. (Note that most applications will not need anything but the simple canonical and transport formats; the code here is considerably more complex because it also supports the advanced format, both for input and for output.) The "sexp-main" program also contains some documentation on running the program. (Or just type "sexp -h" to get the help print-out.) The code is available under the MIT License.

- [sexp.h](sexp.h)
- [sexp-basic.c](sexp-basic.c)
- [sexp-input.c](sexp-input.c)
- [sexp-output.c](sexp-output.c)
- [sexp-main.c](sexp-main.c)

Here are some sample inputs and outputs (warning: while these look like SDSI/SPKI files, they are only approximations).

- [canonical](sexp-sample-c)
- [transport](sexp-sample-b)
- [advanced](sexp-sample-a)

The program 'sexp' reads, parses, and prints out S-expressions.
INPUT:
stdin is read by default, but this can be changed:
  -i filename      -- takes input from file instead.
  -p               -- prompts user for console input
Input is normally parsed, but this can be changed:
  -s               -- treat input up to EOF as a single string
CONTROL LOOP:
The main routine typically reads one S-expression, prints it out again, 
and stops.  This may be modified:
  -x               -- execute main loop repeatedly until EOF
OUTPUT:
Output is normally written to stdout, but this can be changed:
  -o filename      -- Write output to file instead
The output format is normally canonical, but this can be changed:
  -a               -- Write output in advanced transport format
  -b               -- Write output in Base64 output format
  -c               -- Write output in canonical format
  -l               -- suppress linefeeds after output
More than one output format can be requested at once.
There is normally a line-width of 75 on output, but:
  -w width         -- changes line width to specified width.
                      (0 implies no line-width constraint)
The default switches are: -p -a -b -c -x
Typical usage: cat certificate-file | sexp -a -x
