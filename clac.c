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
#include <errno.h>
#include <math.h>
#include "linenoise.h"
#include "sds.h"

#define HINT_COLOR 33
#define CONFIG_MAX 1024
#define WORDS_FILE "clac/words"
#define OUTPUT_FMT "\x1b[33m= %g\x1b[0m\n"

static double stack[0xFF];
static double hole = 0;
static int top = 0;
static sds result;

typedef struct node {
	sds word;
	sds meaning;
	struct node *next;
} node;

static node *head = NULL;

static node *find(sds word) {
	node *curr = head;

	while (curr != NULL) {
		if (!strcasecmp(word, curr->word)) {
			return curr;
		}

		curr = curr->next;
	}

	return NULL;
}

static void add(sds word, sds meaning) {
	node *curr = find(word);

	if (curr != NULL) {
		fprintf(stderr, "Duplicate definition of \"%s\" in words file\n", word);
		curr->meaning = meaning;
		return;
	}

	curr = (node *) malloc(sizeof(node));

	if (curr == NULL) {
		fprintf(stderr, "Not enough memory to load words\n");
		exit(1);
	}

	curr->word = word;
	curr->meaning = meaning;
	curr->next = head;
	head = curr;
}

static void cleanup() {
	node *curr;

	while (head != NULL) {
		curr = head;
		head = curr->next;

		sdsfree(curr->word);
		sdsfree(curr->meaning);

		free(curr);
	}
}

static void push(double value) {
	stack[top++] = value;
}

static double pop() {
	if (top == 0) {
		return 0;
	}

	return stack[--top];
}

static int parse(sds input) {
	int argc;
	sds *argv = sdssplitargs(input, &argc);

	if (argc == 0) {
		sdsfreesplitres(argv, argc);
		return 0;
	}

	if (argc != 2) {
		sdsfreesplitres(argv, argc);
		fprintf(stderr, "Incorrect definition: %s\n", input);
		return 1;
	}

	add(argv[0], argv[1]);

	return 0;
}

static void load(sds filename) {
	FILE *fp;

	if ((fp = fopen(filename, "r")) == NULL) {
		if (errno == ENOENT) {
			return;
		}

		fprintf(stderr, "Can't open file %s\n", filename);
		sdsfree(filename);
		exit(1);
	}

	char buf[CONFIG_MAX+1];
	int linecount, i;

	sds *lines;
	sds content = sdsempty();

	while(fgets(buf, CONFIG_MAX+1, fp) != NULL) {
		content = sdscat(content, buf);
	}

	fclose(fp);

	lines = sdssplitlen(content, strlen(content), "\n", 1, &linecount);

	for (i = 0; i < linecount; i++) {
		lines[i] = sdstrim(lines[i], " \t\r\n");

		if (parse(lines[i]) != 0) {
			sdsfreesplitres(lines, linecount);

			fprintf(stderr, "(%s:%d)\n", filename, i+1);
			exit(1);
		}
	}

	sdsfreesplitres(lines, linecount);
	sdsfree(content);
}

static void eval(const char *input);

static void process(sds word) {
	double a, b;
	char *z;
	node *n;

	if (!strcmp(word, "+")) {
		a = pop();
		b = pop();
		push(a + b);
	} else if (!strcmp(word, "-")) {
		a = pop();
		b = pop();
		push(b - a);
	} else if (!strcmp(word, "*")) {
		a = pop();
		b = pop();
		push(b * a);
	} else if (!strcmp(word, "/")) {
		a = pop();
		b = pop();
		push(b / a);
	} else if (!strcmp(word, "%")) {
		a = pop();
		b = pop();
		push(b - a * floor(b / a));
	} else if (!strcmp(word, "^")) {
		a = pop();
		b = pop();
		push(pow(b, a));
	} else if (!strcmp(word, "_")) {
		push(hole);
	} else if (!strcasecmp(word, "ceil")) {
		a = pop();
		push(ceil(a));
	} else if (!strcasecmp(word, "floor")) {
		a = pop();
		push(floor(a));
	} else if (!strcasecmp(word, "round")) {
		a = pop();
		push(round(a));
	} else if (!strcasecmp(word, "swap")) {
		a = pop();
		b = pop();
		push(a);
		push(b);
	} else if (!strcasecmp(word, "dup")) {
		a = pop();
		push(a);
		push(a);
 	} else if ((n = find(word)) != NULL) {
 		eval(n->meaning);
	} else {
		a = strtod(word, &z);

		if (*z == '\0') {
			push(a);
		}
	}
}

static void eval(const char *input) {
	int i, argc;

	sds *argv = sdssplitargs(input, &argc);

	for (i = 0; i < argc; i++) {
		process(argv[i]);
	}

	sdsfreesplitres(argv, argc);
}

static char *hints(const char *input, int *color, int *bold) {
	int i;

	top = 0;
	eval(input);
	sdsclear(result);

	for (i = 0; i < top; i++) {
		result = sdscatprintf(result, " %g", stack[i]);
	}

	*color = HINT_COLOR;

	return result;
}

static sds buildpath(const char *fmt, const char *dir) {
	return sdscatfmt(sdsempty(), fmt, dir, WORDS_FILE);
}

static void config() {
	result = sdsempty();

	sds filename = NULL;

	if (getenv("CLAC_WORDS") != NULL) {
		filename = sdsnew(getenv("CLAC_WORDS"));
	} else if (getenv("XDG_CONFIG_HOME") != NULL) {
		filename = buildpath("%s/%s", getenv("XDG_CONFIG_HOME"));
	} else if (getenv("HOME") != NULL) {
		filename = buildpath("%s/.config/%s", getenv("HOME"));
	}

	if (filename) {
		load(filename);
		sdsfree(filename);
	}
}

int main(int argc, char **argv) {
	char *line;

	config();

	if (argc == 2) {
		eval(argv[1]);

		while (top > 0) {
			printf("%g\n", pop());
		}

		exit(0);
	}

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
	cleanup();

	return 0;
}
