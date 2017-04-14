/*
 * Copyright (c) 2017, Michel Martens <mail at soveran dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "linenoise.h"
#include "sds.h"

#define HINT_COLOR 33
#define OUTPUT_FMT "\x1b[33m= %g\x1b[0m\n"

double stack[0xFF];
double hole = 0;

int top = 0;

sds result;

void push(double value) {
	stack[top++] = value;
}

double pop() {
	if (top == 0) {
		return 0;
	}

	return stack[--top];
}

void eval(const char *input) {
	int i, argc;
	double a, b;

	sds *argv = sdssplitargs(input, &argc);

	top = 0;

	for (i = 0; i < argc; i++) {
		if (isdigit(argv[i][0])) {
			push(strtod(argv[i], NULL));
		} else if (!strcmp(argv[i], "+")) {
			a = pop();
			b = pop();
			push(a + b);
		} else if (!strcmp(argv[i], "-")) {
			a = pop();
			b = pop();
			push(b - a);
		} else if (!strcmp(argv[i], "*")) {
			a = pop();
			b = pop();
			push(b * a);
		} else if (!strcmp(argv[i], "/")) {
			a = pop();
			b = pop();
			push(b / a);
		} else if (!strcmp(argv[i], "_")) {
			push(hole);
		} else if (!strcasecmp(argv[i], "swap")) {
			a = pop();
			b = pop();
			push(a);
			push(b);
		} else if (!strcasecmp(argv[i], "dup")) {
			a = pop();
			push(a);
			push(a);
		}
	}

	sdsclear(result);
	sdsfreesplitres(argv, argc);

	for (i = 0; i < top; i++) {
		result = sdscatprintf(result, " %g", stack[i]);
	}
}

char *hints(const char *input, int *color, int *bold) {
	*color = HINT_COLOR;
	eval(input);
	return result;
}

int main(int argc, char **argv) {
	char *line;

	result = sdsempty();

	linenoiseSetHintsCallback(hints);

	while((line = linenoise("> ")) != NULL) {
		if (line[0] != '\0') {
			hole = stack[top-1];
			printf(OUTPUT_FMT, hole);
		}

		sdsclear(result);
		free(line);
	}

	sdsfree(result);

	return 0;
}
