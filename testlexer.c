#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"

static const char *token_type_to_string(TokenType type)
{
	switch (type) {
		case TOKEN_TYPE_NULL:
			return "TOKEN_TYPE_NULL";
		case OP_TOKEN:
			return "OP_TOKEN";
		case REG_TOKEN:
			return "REG_TOKEN";
		case NUM_TOKEN:
			return "NUM_TOKEN";
		case IMM_TOKEN:
			return "IMM_TOKEN";
		case COMMA_TOKEN:
			return "COMMA_TOKEN";
		case PAREN_TOKEN:
			return "PAREN_TOKEN";
		case CPAREN_TOKEN:
			return "CPAREN_TOKEN";
		case SPACE_TOKEN:
			return "SPACE_TOKEN";
		case TOKEN_TYPE_END:
			return "TOKEN_TYPE_END";
		default:
			return "UNKNOWN_TOKEN_TYPE";
	}
}

int main()
{
	FILE *fp = NULL;
	char str[1024];
	int line = 1;
	Token *tokens =
	(Token *)malloc(sizeof(Token) * MAX_TOKEN);
repl:
	memset(tokens, 0, sizeof(Token) * MAX_TOKEN);
	printf(">");
	if (!fgets(str, sizeof(str), stdin)) {
        printf("\n");
    	return 0;
	}
	if (strcasecmp(str, "exit\n") == 0)
		return 0;
	str[strlen(str)-1] = '\0';

	int len = lexer(str, tokens, line);
	parse(tokens, len);
	if (len > 0) {
		line++;
		for (size_t i = 0; i < len; i++)
			printf("%d,%d: %s (%s)\n", tokens[i].lineNum, tokens[i].colNum,
				tokens[i].lexeme, token_type_to_string(tokens[i].type));
	}
	goto repl;
}
