#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define SIZE 256

char *shift(int *argc, char ***argv) {
	if (*argc <= 0) {
		// fputs("[ERROR]: not enough arguments provided\n", stderr);
		return NULL;
	}
	char *result = **argv;
	(*argc) -= 1;
	(*argv) += 1;
	return result;
}

typedef enum {
	NUMBER = -1,
	UNKNOWN = 0,
	//
	PLUS = '+',
	MINUS = '-',
	MUL = 'x',
	DIV = '/',
	// POWER = '^',
	// SQRT = '',
	CLEAR = 'c',
} TOKEN_NAME;

typedef struct {
	TOKEN_NAME t;
	char *v;
} Token;

void to_string(Token *token) {
	switch (token->t) {
		case NUMBER:
			printf("NUMBER(%s)\n",token->v);
			break;
		default:
			printf("SYMBOL(%s)\n", token->v);
	}
}

void printer(Token **tokens, size_t n) {
	for (int i=0;i<n;i++) {
		to_string(tokens[i]);
	}
}

void freemy(Token **tokens, size_t n) {
	for (int i=0; i<n; ++i) {
		free(tokens[i]->v);
		free(tokens[i]);
	}
	free(tokens);
}

Token **lex(char *buf, size_t size, size_t *token_count) {
	if (size == 0) {
		return 0;
	}

	Token **tokens = calloc(size, sizeof(Token *));
	int str_size = SIZE;
	int tokens_size = 0;
	for (int i=0; i < size; ++i) {
		Token *new = calloc(1, sizeof(Token));
		new->t = UNKNOWN;
		char *val = calloc(str_size, sizeof(char));
		char b_i = buf[i];
		if (b_i >= '0' && b_i <= '9') {
			new->t = NUMBER;
			int dot_count = 0;
			int j = 0;
			do {
				if (b_i == '.') {
					char b_ip1;
					if (i+1 < size && !((b_ip1 = buf[i+1]) >= '0' && b_ip1 <= '9')) {
						break;
					}
					if (dot_count) {
						new->t = UNKNOWN;
					}
					++dot_count;
				}
				val[j] = b_i;
				++i;
				++j;
			} while (((b_i = buf[i]) >= '0' && b_i <= '9' || b_i == '.') && i < size);
			--i;
			val = realloc(val, j);			
		} else if (b_i == PLUS || b_i == MINUS ||
			b_i == MUL || b_i == DIV || 
			b_i == CLEAR) {
			new->t = b_i;
			val[0] = b_i;
			val = realloc(val, 1);
		}
		if (new->t != UNKNOWN) {
			new->v = val;
			tokens[tokens_size] = new;
			++tokens_size;
		}
	}
	(*token_count) = tokens_size;
	// printer(tokens, tokens_size);
	// freemy(tokens, tokens_size);
	return tokens;
}

Token **s_isfifo(size_t *token_count) { // piped in
	char *buf = calloc(SIZE, sizeof(char));
	char c;
	size_t i = 0;
	size_t size = SIZE;
	while ((c = fgetc(stdin)) != EOF) {
		buf[i] = c;
		++i;
		if (i >= size) {
			size *= 2;
			buf = realloc(buf, size);
		}
	}
	Token **tokens = lex(buf, i, token_count);
	free(buf);
	return tokens;
}

void my_memset(char *buf, int c, size_t n) {
	for (int i=0; i<n; ++i) {
		buf[i] = c;
	}
}

Token **s_ischr(size_t *token_count) { // REPL
	char *buf = calloc(SIZE, sizeof(char));
	size_t size = SIZE;
	char c;
	
	size_t i = 0;
	while ((c = fgetc(stdin)) != '\n' && c != EOF) {
		buf[i] = c;
		++i;
		if (i >= size) {
			size *= 2;
			buf = realloc(buf, size);
		}
	}
	Token **tokens = lex(buf, i, token_count);
	free(buf);
	return tokens;
}

Token **s_isreg(size_t *token_count) { // file directed as stdin, eg ./a.out < file
	fseek(stdin, 0, SEEK_END);
	int size = ftell(stdin);
	rewind(stdin);
	char *buf = calloc(size, sizeof(char));
	fread(buf, sizeof(char), size, stdin);
	Token **tokens = lex(buf, size, token_count);
	free(buf);
	return tokens;
}

Token **w_args(int argc, char **argv, size_t *token_count) {
	char *buf = calloc(SIZE, sizeof(char));
	char *arg;
	size_t i = 0;
	size_t size = SIZE;
	while (arg = shift(&argc, &argv)) {
		while ( *arg != '\0') {
			buf[i] = *arg;
			++i;
			arg += 1;
			if (i+1 >= size) {
				size *= 2;
				buf = realloc(buf, size);
			}
		}
		buf[i] = ' ';
		++i;		
	}
	Token **tokens = lex(buf, i, token_count);
	free(buf);
	return tokens;
}

int calc(Token **tokens, size_t token_count, float *res) {
	int i = 0;
	float result = (*res);
	while (token_count > 0) {
		Token *t_i = tokens[i];
		if (t_i->t == CLEAR) {
			++i;
			--token_count;
			result = 0;
			(*res) = 0;
		} else if (t_i->t == PLUS) {
			++i;
			--token_count;
			while (token_count > 0 && ((t_i = tokens[i])->t) == NUMBER) {
				float a = atof(t_i->v);
				result += a;
				++i;
				--token_count;
			}
		} else if (t_i->t == MINUS) {
			++i;
			--token_count;
			while (token_count > 0 && ((t_i = tokens[i])->t) == NUMBER) {
				float a = atof(t_i->v);
				result -= a;
				++i;
				--token_count;
			}
		} else if (t_i->t == MUL) {
			if (result == 0) {
				result = 1;
			}
			++i;
			--token_count;
			while (token_count > 0 && ((t_i = tokens[i])->t) == NUMBER) {
				float a = atof(t_i->v);
				result *= a;
				++i;
				--token_count;
			}
		} else if (t_i->t == DIV) {
			++i;
			--token_count;
			while (token_count > 0 && ((t_i = tokens[i])->t) == NUMBER) {
				float a = atof(t_i->v);
				if (a == 0) {
					fputs("[WARNING]: division by 0; zeroing the result\n", stdout);
					result = 0;
				} else {
					result /= a;
				}
				++i;
				--token_count;
			}
		} else {
			fputs("[ERROR]: undefined action: ", stderr);
			fputs(tokens[i]->v, stderr);
			fputs("\n", stderr);
			return 1;
		}
	}
	(*res) = result;
	printf("= %g\n", result);
	return 0;
}

int main(int argc, char **argv) {
	char *prog_name = shift(&argc, &argv); // shift program name
	Token **tokens;
	size_t *token_count = calloc(1, sizeof(size_t *));
	int repl = 0;
	int exit_code = 0;
	float res = 0;
	if (argc > 0) {
		tokens = w_args(argc, argv, token_count);
	} else {
		struct stat stats;
		fstat(fileno(stdin), &stats);
		int stats_mode = stats.st_mode;
		if (S_ISFIFO(stats_mode)) {
			tokens = s_isfifo(token_count);
		} else if (S_ISCHR(stats_mode)) {
			repl = 1;
			while (1) {
				tokens  = s_ischr(token_count);
				goto calc;
loop:
			}		
		} else if (S_ISREG(stats_mode)) {
			tokens = s_isreg(token_count);
		} else {
			fputs("[ERROR]: unsupported filetype\n", stderr);
			return 1;
		}
	}

calc:
	printer(tokens, *token_count);
	exit_code = calc(tokens, *token_count, &res);
	freemy(tokens, *token_count);
	if (repl) {
		goto loop;
	}
	free(token_count);
	return exit_code;
}