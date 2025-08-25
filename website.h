#ifndef WEBSITE_H
#define WEBSITE_H

/* makes headers 1 size smaller than they should be
	 purely cosmetic, TODO move to the config file */
#define HEADER_SIZE_OFFSET 1

#define STR_PRINT "%.*s"
#define STR_FMT(s) s.len, s.str

typedef struct {
	char* str;
	int len;
} string;

struct text_element {
	string str;
	string target;
	// < 0 for code, 0 for <p>, > 0 for headers
	int level;
	int is_page_ref; // 1 for true
};

struct image_element {
	string src;
	string alt_text;
};

struct element {
	int is_text; // 1 for true
	union {
		struct text_element text;
		struct image_element image;
	} as;
};

struct page {
	struct {
		string title; // the tab title
		union {
			string author;
			string name; // the name shown at the top of the page
		};
		union {
			string description;
			string section; // the section in the .port file
		};
	} metadata;

	struct element* elements;
	int el_cap;
	int el_len;
};

struct website {
	int failed;
	struct page* pages;
	int pa_len;
};

struct token_list;
struct website process_tokens(struct token_list* tokens, int page_count);

#endif // WEBSITE_H
