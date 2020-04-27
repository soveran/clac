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

/* UI */
#define HINT_COLOR 33
#define NUMBER_FMT "%.15g"
#define OUTPUT_FMT "\x1b[33m= " NUMBER_FMT "\x1b[0m\n"
#define WORDEF_FMT "%s \x1b[33m\"%s\"\x1b[0m\n"

/* Config */
#define BUFFER_MAX 1024
#define WORDS_FILE "clac/words"
#define CAPACITY   0xFF

/* Stack */
#define count(S)   ((S)->top)
#define clear(S)   ((S)->top = 0)
#define isfull(S)  ((S)->top == CAPACITY)
#define isempty(S) ((S)->top == 0)

/* Arithmetic */
#define modulo(A, B) ((A) - (B) * floor((A) / (B)))

typedef struct stack {
	double items[CAPACITY];
	int top;
} stack;

typedef struct node {
	sds word;
	sds meaning;
	struct node *next;
} node;

static stack stacks[] = {{{}, 0}, {{}, 0}};
static stack *s0 = &stacks[0];
static stack *s1 = &stacks[1];
static node *head = NULL;
static node *tail = NULL;
static sds result;
static double hole = 0;

static int isoverflow(stack *s) {
	if (isfull(s)) {
		fprintf(stderr, "\r\nStack is full!\n");
		return 1;
	}

	return 0;
}

static double peek(stack *s) {
	if (isempty(s)) {
		return 0;
	}

	return s->items[s->top-1];
}

static void push(stack *s, double value) {
	if (!isoverflow(s)) {
		s->items[s->top++] = value;
	}
}

static double pop(stack *s) {
	if (isempty(s)) {
		return 0;
	}

	return s->items[--s->top];
}

static void move(stack *s, stack *t, int n) {
	while (!isempty(s) && n > 0) {
		push(t, pop(s));
		n--;
	}
}

static void roll(stack *s, stack *aux, int m, int n) {
	if (m > count(s)) {
		m = count(s);
	}

	if (m < 2) {
		return;
	}

	if (n < 0) {
		n = m - modulo(abs(n), m);
	}

	if (n == 0 || n == m) {
		return;
	}

	m--;

	double a;

	while (n > 0) {
		a = pop(s);
		move(s, aux, m);
		push(s, a);
		move(aux, s, m);
		n--;
	}
}

static double add(stack *s, int n) {
	double a = pop(s);

	while (!isempty(s) && n > 1) {
		a += pop(s);
		n--;
	}

	return a;
}

static node *get(sds word) {
	node *curr = head;

	while (curr != NULL) {
		if (!strcasecmp(word, curr->word)) {
			return curr;
		}

		curr = curr->next;
	}

	return NULL;
}

static void set(sds word, sds meaning) {
	node *curr = get(word);

	if (curr != NULL) {
		fprintf(stderr, "Duplicate definition of \"%s\"\n", word);
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
	curr->next = NULL;
	if (head == NULL) {
		head = curr;
	} else {
		tail->next = curr;
	}
	tail = curr;
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

	set(argv[0], argv[1]);

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

	char buf[BUFFER_MAX+1];
	int linecount, i;

	sds *lines;
	sds content = sdsempty();

	while(fgets(buf, BUFFER_MAX+1, fp) != NULL) {
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

	if (!strcmp(word, "_")) {
		push(s0, hole);
	} else if (!strcmp(word, "+")) {
		if (count(s0) > 1) {
			a = pop(s0);
			b = pop(s0);
			push(s0, a + b);
		}
	} else if (!strcmp(word, "-")) {
		if (count(s0) > 1) {
			a = pop(s0);
			b = pop(s0);
			push(s0, b - a);
		}
	} else if (!strcmp(word, "*")) {
		if (count(s0) > 1) {
			a = pop(s0);
			b = pop(s0);
			push(s0, b * a);
		}
	} else if (!strcmp(word, "/")) {
		if (count(s0) > 1) {
			a = pop(s0);
			b = pop(s0);
			push(s0, b / a);
		}
	} else if (!strcmp(word, "%")) {
		if (count(s0) > 1) {
			a = pop(s0);
			b = pop(s0);
			push(s0, modulo(b, a));
		}
	} else if (!strcmp(word, "^")) {
		if (count(s0) > 1) {
			a = pop(s0);
			b = pop(s0);
			push(s0, pow(b, a));
		}
	} else if (!strcasecmp(word, "sum")) {
		push(s0, add(s0, count(s0)));
	} else if (!strcasecmp(word, "add")) {
		push(s0, add(s0, pop(s0)));
	} else if (!strcasecmp(word, "abs")) {
		if (count(s0) > 0) {
			push(s0, fabs(pop(s0)));
		}
	} else if (!strcasecmp(word, "ceil")) {
		if (count(s0) > 0) {
			push(s0, ceil(pop(s0)));
		}
	} else if (!strcasecmp(word, "floor")) {
		if (count(s0) > 0) {
			push(s0, floor(pop(s0)));
		}
	} else if (!strcasecmp(word, "round")) {
		if (count(s0) > 0) {
			push(s0, round(pop(s0)));
		}
	} else if (!strcasecmp(word, "sin")) {
		if (count(s0) > 0) {
			push(s0, sin(pop(s0)));
		}
	} else if (!strcasecmp(word, "cos")) {
		if (count(s0) > 0) {
			push(s0, cos(pop(s0)));
		}
	} else if (!strcasecmp(word, "tan")) {
		if (count(s0) > 0) {
			push(s0, tan(pop(s0)));
		}
	} else if (!strcasecmp(word, "asin")) {
		if (count(s0) > 0) {
			push(s0, asin(pop(s0)));
		}
	} else if (!strcasecmp(word, "acos")) {
		if (count(s0) > 0) {
			push(s0, acos(pop(s0)));
		}
	} else if (!strcasecmp(word, "atan")) {
		if (count(s0) > 0) {
			push(s0, atan(pop(s0)));
		}
	} else if (!strcasecmp(word, "atan2")) {
		if (count(s0) > 1) {
			a = pop(s0);
			b = pop(s0);
			push(s0, atan2(b, a));
		}
	} else if (!strcasecmp(word, "ln")) {
		if (count(s0) > 0) {
			push(s0, log(pop(s0)));
		}
	} else if (!strcasecmp(word, "log")) {
		if (count(s0) > 0) {
			push(s0, log10(pop(s0)));
		}
	} else if (!strcasecmp(word, "erf")) {
		if (count(s0) > 0) {
			push(s0, erf(pop(s0)));
		}
	} else if (!strcasecmp(word, "!")) {
		if (count(s0) > 0) {
			a = pop(s0);

			if (a == 0) {
				push(s0, 1);
			} else {
				push(s0, a * tgamma(a));
			}
		}
	} else if (!strcasecmp(word, "dup")) {
		if (!isempty(s0)) {
			push(s0, peek(s0));
		}
	} else if (!strcasecmp(word, "roll")) {
		a = pop(s0);
		b = pop(s0);

		roll(s0, s1, b, a);
	} else if (!strcasecmp(word, "swap")) {
		if (count(s0) > 1) {
			a = pop(s0);
			b = pop(s0);

			push(s0, a);
			push(s0, b);
		}
	} else if (!strcasecmp(word, "drop")) {
		pop(s0);
	} else if (!strcasecmp(word, "count")) {
		push(s0, (double) count(s0));
	} else if (!strcasecmp(word, "clear")) {
		clear(s0);
	} else if (!strcasecmp(word, "stash")) {
		move(s0, s1, pop(s0));
	} else if (!strcasecmp(word, "fetch")) {
		move(s1, s0, pop(s0));
	} else if (!strcasecmp(word, ".")) {
		move(s0, s1, 1);
	} else if (!strcasecmp(word, ",")) {
		move(s1, s0, 1);
	} else if (!strcasecmp(word, ":")) {
		move(s0, s1, count(s0));
	} else if (!strcasecmp(word, ";")) {
		move(s1, s0, count(s1));
	} else if ((n = get(word)) != NULL) {
		eval(n->meaning);
	} else {
		a = strtod(word, &z);

		if (*z == '\0') {
			push(s0, a);
		} else if (!isalpha(word[0])) {
			push(s0, NAN);
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

static void completion(const char *input, linenoiseCompletions *lc) {}

static char *hints(const char *input, int *color, int *bold) {
	int i;

	clear(s0);
	clear(s1);

	eval(input);
	sdsclear(result);

	result = sdscat(result, " ");

	for (i = 0; i < count(s0); i++) {
		result = sdscatprintf(result, " " NUMBER_FMT, s0->items[i]);
	}

	if (!isempty(s1)) {
		result = sdscat(result, " ⋮");

		for (i = s1->top-1; i > -1; i--) {
			result = sdscatprintf(result, " " NUMBER_FMT, s1->items[i]);
		}
	}

	*color = HINT_COLOR;

	return result;
}

static sds buildpath(const char *fmt, const char *dir) {
	return sdscatfmt(sdsempty(), fmt, dir, WORDS_FILE);
}

static void config() {
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

	result = sdsempty();

	config();

	if (argc == 2) {
		eval(argv[1]);

		while (count(s0) > 0) {
			printf(NUMBER_FMT "\n", pop(s0));
		}

		exit(0);
	}

	if (argc > 2) {
		fprintf(stderr, "usage: clac [expression]\n");
		exit(1);
	}

	linenoiseSetHintsCallback(hints);
	linenoiseSetCompletionCallback(completion);

	while((line = linenoise("> ")) != NULL) {
		if (!strcmp(line, "words")) {
			node *curr = head;

			while (curr != NULL) {
				printf(WORDEF_FMT, curr->word, curr->meaning);
				curr = curr->next;
			}
		} else if (!strcmp(line, "reload")) {
			cleanup();
			config();
		} else if (!isempty(s0)) {
			hole = peek(s0);
			clear(s0);
			printf(OUTPUT_FMT, hole);
		}

		sdsclear(result);
		linenoiseHistoryAdd(line);
		free(line);
	}

	sdsfree(result);
	cleanup();

	return 0;
}
