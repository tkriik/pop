#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VM_STACK_MAX 8

enum term_type {
	/* Data */
	TERM_INTEGER = 1,

	/* Code */
	TERM_DOT,
	TERM_STACK,

	TERM_PLUS
};

struct term {
	enum term_type type;
	union {
		long i;
	};
};

void term_print(struct term term, int newline)
{
	switch (term.type) {
	case TERM_INTEGER:
		printf("%ld", term.i);
		break;

	case TERM_DOT:
		printf("builtin<.>");
		break;

	case TERM_STACK:
		printf("builtin<.s>");
		break;
	}

	if (newline)
		putchar('\n');
}

struct env {
	char *word;
	struct term term;
	struct env *tail;
};

struct env *env_define(struct env *tail, const char *word, struct term term)
{
	struct env *head = malloc(sizeof(*head));
	if (head == NULL)
		err(1, "malloc");

	head->word = calloc(1, strlen(word) + 1);
	if (head->word == NULL)
		err(1, "malloc");
	memcpy(head->word, word, strlen(word));

	head->term = term;
	head->tail = tail;

	return head;
}

int env_find(struct env *env, const char *word, struct term *termp)
{
	assert(word != NULL);
	assert(termp != NULL);

	if (env == NULL)
		return 0;

	if (strcmp(env->word, word) == 0) {
		*termp = env->term;
		return 1;
	}

	return env_find(env->tail, word, termp);
}

struct vm {
	struct term stack[VM_STACK_MAX];
	size_t stack_size;

	struct env *env;
};

void vm_init(struct vm *vm)
{
	assert(vm != NULL);

	memset(vm->stack, 0, sizeof(vm->stack));
	vm->stack_size = 0;

	vm->env = NULL;

	struct term dot = { .type = TERM_DOT };
	vm->env = env_define(vm->env, ".", dot);

	struct term stack = { .type = TERM_STACK };
	vm->env = env_define(vm->env, ".s", stack);

	struct term plus = { .type = TERM_PLUS };
	vm->env = env_define(vm->env, "+", plus);
}

void vm_push_integer(struct vm *vm, long i)
{
	assert(vm != NULL);

	if (vm->stack_size == VM_STACK_MAX)
		errx(1, "stack overflow");

	struct term term = {
		.type = TERM_INTEGER,
		.i = i
	};

	vm->stack[vm->stack_size++] = term;
}

struct term vm_pop(struct vm *vm)
{
	assert(vm != NULL);

	if (vm->stack_size == 0)
		errx(1, "stack underflow");

	return vm->stack[--vm->stack_size];
}

long vm_pop_integer(struct vm *vm)
{
	struct term term = vm_pop(vm);
	if (term.type != TERM_INTEGER)
		errx(1, "expected integer");

	return term.i;
}

const char *read_word(void)
{
	static char word[64];
	size_t len = 0;
	memset(word, '\0', sizeof(word));

	enum {
		SEEK,
		WORD
	} state = SEEK;

	int ch;
	while ((ch = getchar()) != EOF) {
		assert(len < sizeof(word));

		if (len == sizeof(word) - 1)
			break;

		char c = (char)ch;

		switch (state) {
		case SEEK:
			/* SEEK -> SEEK */
			if (isspace(c))
				continue;

			/* SEEK -> WORD */
			state = WORD;
			word[len++] = c;
			break;

		case WORD:
			/* WORD -> RET */
			if (isspace(c))
				return word;

			word[len++] = c;
			break;
		}
	}

	if (len == 0)
		return NULL;

	return word;
}

int parse_integer(const char *word, long *i)
{
	assert(word != NULL);
	assert(i != NULL);

	errno = 0;
	char *endptr = NULL;
	*i = strtol(word, &endptr, 10);
	if ((errno == ERANGE && (*i == LONG_MAX || *i == LONG_MIN))
		|| (errno != 0 && *i == 0)
		|| (*endptr != '\0'))
		return -1;

	return 0;
}

void vm_run(struct vm *vm)
{
	assert(vm != NULL);

	const char *word;
	while ((word = read_word()) != NULL) {
		long l;
		if (parse_integer(word, &l) == 0) {
			vm_push_integer(vm, l);
			continue;
		}

		struct term term;
		if (!env_find(vm->env, word, &term)) {
			warnx("undefined word: %s", word);
			continue;
		}

		struct term res;
		long a;
		long b;

		switch (term.type) {
		case TERM_DOT:
			res = vm_pop(vm);
			term_print(res, 1);
			break;

		case TERM_STACK:
			printf("<%zu>", vm->stack_size);
			for (size_t i = 0; i < vm->stack_size; i++) {
				putchar(' ');
				term_print(vm->stack[i], 0);
			}
			putchar('\n');
			break;

		case TERM_PLUS:
			a = vm_pop_integer(vm);
			b = vm_pop_integer(vm);
			vm_push_integer(vm, a + b);
			break;
		}
	}
}

int main(void)
{
	struct vm vm;

	vm_init(&vm);
	vm_run(&vm);

	return EXIT_SUCCESS;
}
