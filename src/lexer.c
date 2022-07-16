#include "rim.h"

lexer_t lexer;

void init_lexer(const char *source)
{
	lexer.start = source;
	lexer.current = source;
	lexer.line = 1;
}

bool isAlpha(char c)
{
	return (c >= 'a' && c <= 'z') ||
		   (c >= 'A' && c <= 'Z') ||
		   c == '_';
}

bool isDigit(char c)
{
	return c >= '0' && c <= '9';
}

bool isAtEnd()
{
	return *lexer.current == '\0';
}

char advance()
{
	lexer.current++;
	return lexer.current[-1];
}

char peek()
{
	return *lexer.current;
}

char peekNext()
{
	if (isAtEnd())
		return '\0';
	return lexer.current[1];
}

bool lexer_match(char expected)
{
	if (isAtEnd())
		return false;
	if (*lexer.current != expected)
		return false;
	lexer.current++;
	return true;
}

token_t makeToken(token_type type)
{
	token_t token;
	token.type = type;
	token.start = lexer.start;
	token.length = (int)(lexer.current - lexer.start);
	token.line = lexer.line;
	return token;
}

token_t errorToken(const char *message)
{
	token_t token;
	token.type = TOKEN_ERROR;
	token.start = message;
	token.length = (int)strlen(message);
	token.line = lexer.line;
	return token;
}

void skipWhitespace()
{
	for (;;)
	{
		char c = peek();
		switch (c)
		{
		case ' ':
		case '\r':
		case '\t':
			advance();
			break;

		case '\n':
			lexer.line++;
			advance();
			break;

		case '/':
			if (peekNext() == '/')
			{

				while (peek() != '\n' && !isAtEnd())
					advance();
			}
			else
			{
				return;
			}
			break;

		default:
			return;
		}
	}
}

token_type checkKeyword(int start, int length,
						const char *rest, token_type type)
{
	if (lexer.current - lexer.start == start + length &&
		memcmp(lexer.start + start, rest, length) == 0)
	{
		return type;
	}

	return TOKEN_IDENTIFIER;
}

token_type identifierType()
{

	switch (lexer.start[0])
	{
	case 'a':
		return checkKeyword(1, 2, "nd", TOKEN_AND);
	case 'c':
		return checkKeyword(1, 4, "lass", TOKEN_CLASS);
	case 'e':
		return checkKeyword(1, 3, "lse", TOKEN_ELSE);

	case 'f':
		if (lexer.current - lexer.start > 1)
		{
			switch (lexer.start[1])
			{
			case 'a':
				return checkKeyword(2, 3, "lse", TOKEN_FALSE);
			case 'n':
				return TOKEN_FN;
			case 'o':
				return checkKeyword(2, 1, "r", TOKEN_FOR);
			}
		}
		break;

	case 'i':
		return checkKeyword(1, 1, "f", TOKEN_IF);
	case 'l':
		return checkKeyword(1, 2, "et", TOKEN_LET);
	case 'n':
		return checkKeyword(1, 2, "il", TOKEN_NIL);
	case 'o':
		return checkKeyword(1, 1, "r", TOKEN_OR);
	case 'r':
		return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
	case 's':
		return checkKeyword(1, 4, "uper", TOKEN_SUPER);

	case 't':
		if (lexer.current - lexer.start > 1)
		{
			switch (lexer.start[1])
			{
			case 'h':
				return checkKeyword(2, 2, "is", TOKEN_THIS);
			case 'r':
				return checkKeyword(2, 2, "ue", TOKEN_TRUE);
			}
		}
		break;

	case 'w':
		return checkKeyword(1, 4, "hile", TOKEN_WHILE);
	}

	return TOKEN_IDENTIFIER;
}

token_t identifier()
{
	while (isAlpha(peek()) || isDigit(peek()))
		advance();
	return makeToken(identifierType());
}

token_t number()
{
	while (isDigit(peek()))
		advance();

	if (peek() == '.' && isDigit(peekNext()))
	{

		advance();

		while (isDigit(peek()))
			advance();
	}

	return makeToken(TOKEN_NUMBER);
}

token_t string()
{
	while (peek() != '"' && !isAtEnd())
	{
		if (peek() == '\n')
			lexer.line++;
		advance();
	}

	if (isAtEnd())
		return errorToken("Unterminated string.");

	advance();
	return makeToken(TOKEN_STRING);
}

token_t scan_token()
{

	skipWhitespace();

	lexer.start = lexer.current;

	if (isAtEnd())
		return makeToken(TOKEN_EOF);

	char c = advance();

	if (isAlpha(c))
		return identifier();

	if (isDigit(c))
		return number();

	switch (c)
	{
	case '(':
		return makeToken(TOKEN_LEFT_PAREN);
	case ')':
		return makeToken(TOKEN_RIGHT_PAREN);
	case '{':
		return makeToken(TOKEN_LEFT_BRACE);
	case '}':
		return makeToken(TOKEN_RIGHT_BRACE);
	case ';':
		return makeToken(TOKEN_SEMICOLON);
	case ':':
		return makeToken(TOKEN_COLON);
	case ',':
		return makeToken(TOKEN_COMMA);
	case '.':
		return makeToken(TOKEN_DOT);
	case '-':
		return makeToken(TOKEN_MINUS);
	case '+':
		return makeToken(TOKEN_PLUS);
	case '/':
		return makeToken(TOKEN_SLASH);
	case '*':
		return makeToken(TOKEN_STAR);

	case '!':
		return makeToken(
			lexer_match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
	case '=':
		return makeToken(
			lexer_match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
	case '<':
		return makeToken(
			lexer_match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
	case '>':
		return makeToken(
			lexer_match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);

	case '"':
		return string();
	}

	return errorToken("Unexpected character.");
}
