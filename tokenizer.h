#ifndef TOKENIZER_H
#define TOKENIZER_H

enum TokenType {
	TT_SECTION = 0,
	TT_METADATA,
	TT_TITLE = 2,
	TT_CODE,
	TT_IMAGE = 4,
	TT_TEXT,
	TT_HR = 6,
	TT_HEADER,
	TT_LINK = 8,
	TT_PAGE_REF,
	TT_WEB_REF = 10,
};

struct token {
	enum TokenType type;
	char* str;
	int len;
};

struct token_list {
	struct token* arr;
	int len;
	int capacity;
	int page_count;
};

struct token_list tokenize(char* buffer, int buflen);
int get_next(char* buffer, int buflen, int start, char target);

#endif // TOKENIZER_H
