#ifndef LEXER_H
#define LEXER_H

int find_next(char* buf, int buflen, int start, char target);

enum TokenType {
	TT_METADATA,
	TT_SECTION,
	TT_SECTION_REF,
	TT_HEADER,
	TT_TEXT_LITERAL,
	TT_LINK,
	TT_IGNORE, // used in the parsing stage
};

struct token {
	enum TokenType type;
	char* start; // points to the source buffer
	int len;
};

// allocates space for tokens, so dont before hand
struct token* lex(char* buffer, int buflen, int* t_size, int* page_count);

void post_lex(struct token* tokens, int count);

#endif // LEXER_H
