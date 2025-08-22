#ifndef PARSER_H
#define PARSER_H

#define STRING_FMT "%.*s"
#define STRING_ARGS(str) str.len, str.start

typedef struct {
	char* start;
	int len;
} string;

struct text_element {
	int level; // uses 0 for <p>
	int target_type; // 0 for page ref, 1 for web ref
	string text;
	string target; // used for links, len == 0 when unsed
};

struct page {
	string title;
	string section_name;
	struct text_element* text_elements;
	int text_el_c;
};

struct website {
	struct {
		string title;
		string author;
		string description;
	} metadata;
	struct page navbar;
	struct page* pages;
	string title;
	int page_c;
};

struct token;
struct website parse(struct token* tokens, int token_count, int page_count);

#endif // PARSER_H
