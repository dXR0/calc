#include "./lex.h"

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
		} else if (t_i->t == NUMBER) {
			float a = atof(t_i->v);
			result = a;
			++i;
			--token_count;
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
		} else if (t_i->t == POW) {
			++i;
			--token_count;
			while (token_count > 0 && ((t_i = tokens[i])->t) == NUMBER) {
				float a = atof(t_i->v);
				if (a == 0) {
					result = 1;
				} else {
					float step = result;
					for (int j = 1; j < a; ++j) {
						result *= step;
					}
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
	printer(tokens, *token_count); // NOTE: for debugging
	exit_code = calc(tokens, *token_count, &res);
	freemy(tokens, *token_count);
	if (repl) {
		goto loop;
	}
	free(token_count);
	return exit_code;
}