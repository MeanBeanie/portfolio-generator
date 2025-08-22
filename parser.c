#include "parser.h"
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define cur_page res.pages[pagenr]
#define cur_elements res.pages[pagenr].text_elements
#define cur_count res.pages[pagenr].text_el_c

#define ensure_size(count,arr)\
	if(count >= text_element_cap){\
		text_element_cap *= 2;\
		arr = realloc(arr, sizeof(struct text_element)*text_element_cap);\
	}


struct website parse(struct token* tokens, int token_count, int page_count){
	struct website res = {0};
	res.pages = malloc(sizeof(struct page)*page_count);
	res.page_c = page_count;
	res.navbar.text_elements = malloc(sizeof(struct text_element)*8);

	int pagenr = -1;

	int text_element_cap = 8;

	for(int i = 0; i < token_count; i++){
		switch(tokens[i].type){
			case TT_METADATA:
			{
				int title_len = find_next(tokens[i].start, tokens[i].len, 2, '|');
				res.metadata.title = (string){ .start = tokens[i].start+2, .len = title_len-1 };
				int author_len = find_next(tokens[i].start, tokens[i].len, 4+title_len, '|');
				res.metadata.author = (string){ .start = tokens[i].start+4+title_len, .len = author_len-1 };
				int desc_len = find_next(tokens[i].start, tokens[i].len, 6+title_len+author_len, '\n');
				res.metadata.description = (string){ .start = tokens[i].start+6+title_len+author_len, .len = desc_len };
				break;
			}
			case TT_SITE_TITLE:
			{
				res.title = (string){
					.start = tokens[i].start+2,
					.len = tokens[i].len-2
				};
				break;
			}
			case TT_PAGE_TITLE:
			{
				int len = find_next(tokens[i].start, tokens[i].len, 2, '\n');
				cur_page.title = (string){
					.start = tokens[i].start+2,
					.len = len
				};
				break;
			}
			case TT_SECTION:
			{
				pagenr++;
				text_element_cap = 8;
				cur_page.section_name = (string){ .start = tokens[i].start+1, .len = tokens[i].len-1 };
				cur_elements = malloc(sizeof(struct text_element)*text_element_cap);
				break;
			}
			case TT_HEADER:
			{
				int level = 0;
				for(int off = 0; off < tokens[i].len; off++){
					if(tokens[i].start[off] != '#'){ break; }
					level++;
				}
				ensure_size(cur_count, cur_elements);
				cur_elements[cur_count++] = (struct text_element){
					.level = level,
					.target = {0},
					.text = (string){
						.start = tokens[i].start+level+1,
						.len = tokens[i].len-level-1
					}
				};
				break;
			}
			case TT_TEXT_LITERAL:
			{
				if(pagenr < 0){
					ensure_size(res.navbar.text_el_c, res.navbar.text_elements);
					res.navbar.text_elements[res.navbar.text_el_c++] = (struct text_element){
						.level = 0,
						.target = {0},
						.text = (string){
							.start = tokens[i].start,
							.len = tokens[i].len
						}
					};
				}
				else{
					ensure_size(cur_count, cur_elements);
					cur_elements[cur_count++] = (struct text_element){
						.level = 0,
						.target = {0},
						.text = (string){
							.start = tokens[i].start,
							.len = tokens[i].len
						}
					};
				}
				break;
			}
			case TT_LINK:
			{
				if(i+1 >= token_count || (i+1 < token_count && (tokens[i+1].type != TT_SECTION_REF && tokens[i+1].type != TT_WEBSITE_REF))){
					fprintf(stderr, "Hey so uh, your link has no target, its not me its you, skipping this link\n");
					break;
				}
				int type = tokens[i+1].type == TT_SECTION_REF ? 0 : 1;
				if(i-1 < 0){
					fprintf(stderr, "Uh oh! This link has no text behind it, website description being used instead");
					if(pagenr < 0){
						ensure_size(res.navbar.text_el_c, res.navbar.text_elements);
						res.navbar.text_elements[res.navbar.text_el_c++] = (struct text_element){
							.level = 0,
							.target = {0},
							.text = (string){
								.start = res.metadata.description.start,
								.len = res.metadata.description.len
							}
						};
					}
					else{
						ensure_size(cur_count, cur_elements);
						cur_elements[cur_count++] = (struct text_element){
							.level = 0,
							.target = {0},
							.text = (string){
								.start = res.metadata.description.start,
								.len = res.metadata.description.len
							}
						};
					}
				}

				if(pagenr < 0){
					res.navbar.text_elements[res.navbar.text_el_c-1].target = (string){
						.start = tokens[i+1].start+1,
						.len = tokens[i+1].len-1
					};
					res.navbar.text_elements[res.navbar.text_el_c].target_type = type;
				}
				else{
					cur_elements[cur_count-1].target = (string){
						.start = tokens[i+1].start+1,
						.len = tokens[i+1].len-1
					};
					cur_elements[cur_count-1].target_type = type;
				}
				break;
			}
			case TT_WEBSITE_REF: case TT_SECTION_REF: // its handled in the TT_LINK
			case TT_IGNORE: break; // ignoring the TT_IGNORE? how wild
		}
	}

	int passed = 0;
	for(int i = 0; i < page_count; i++){
		if(strncmp("index", res.pages[i].section_name.start, 5) == 0){
			passed = 1;
		}
	}
	if(passed == 0){
		fprintf(stderr, "No index page found, this may be an issue\n");
	}

	return res;
}
