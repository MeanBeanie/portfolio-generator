#include "website.h"
#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define cur_page res.pages[res.pa_len]
#define cur_elem cur_page.elements[cur_page.el_len]
#define last_elem cur_page.elements[cur_page.el_len-1]

#define ensure_elements_size\
	if(cur_page.el_len >= cur_page.el_cap){\
		cur_page.el_cap *= 2;\
		cur_page.elements = realloc(cur_page.elements, cur_page.el_cap*sizeof(struct element));\
	}

struct website process_tokens(struct token_list* tokens, int page_count){
	struct website res = {0};
	res.pages = calloc(page_count+1, sizeof(struct page));
	res.pa_len = 0;

	cur_page.elements = calloc(8, sizeof(struct element));
	cur_page.el_cap = 8;
	cur_page.el_len = 0;

	int held_level = 0;

	for(int i = 0; i < tokens->len; i++){
		ensure_elements_size;

		switch(tokens->arr[i].type){
			case TT_SECTION:
			{
				res.pa_len++;
				cur_page.elements = calloc(8, sizeof(struct element));
				cur_page.el_cap = 8;
				cur_page.el_len = 0;

				cur_page.metadata.section = (string){
					.str = tokens->arr[i].str+1,
					.len = tokens->arr[i].len-1
				};
				break;
			}
			case TT_METADATA:
			{
				if(res.pa_len == 0){
					int title_end = get_next(tokens->arr[i].str, tokens->arr[i].len, 2, '|');
					int author_end = get_next(tokens->arr[i].str, tokens->arr[i].len, 2+title_end, '|');

					cur_page.metadata.title       = (string){ .str = tokens->arr[i].str+2,            .len = title_end-2                     };
					cur_page.metadata.author      = (string){ .str = tokens->arr[i].str+2+title_end,  .len = author_end-title_end-2          };
					cur_page.metadata.description = (string){ .str = tokens->arr[i].str+2+author_end, .len = tokens->arr[i].len-author_end-2 };
				}
				else{
					cur_page.metadata.title = (string){ .str = tokens->arr[i].str+2, .len = tokens->arr[i].len-2 };
				}
				break;
			}
			case TT_TITLE:
			{
				if(res.pa_len != 0){
					cur_page.metadata.name = (string){ .str = tokens->arr[i].str+2, .len = tokens->arr[i].len-2 };
				}
				break;
			}
			case TT_HEADER:
			{
				for(int j = 0; j < tokens->arr[i].len; j++){
					if(tokens->arr[i].str[j] == '#'){ held_level++; }
					else{ break; }
				}
				break;
			}
			case TT_TEXT:
			{
				cur_elem = (struct element){
					.is_text = 1,
					.as.text = (struct text_element){
						.is_page_ref = -1,
						.level = held_level,
						.str = (string){ .str = tokens->arr[i].str+1, .len = tokens->arr[i].len-1 }
					}
				};

				held_level = 0;

				cur_page.el_len++;
				break;
			}
			case TT_LINK:
			{
				// NOTE kinda assumes no images before the link but oh well
				if(cur_page.el_len <= 0){
					// if there is no element before the link, create one with the dummy text: ">"
					cur_elem = (struct element){
						.is_text = 1,
						.as.text = (struct text_element){
							.level = 0,
							.str = (string){ .str = tokens->arr[i].str, .len = tokens->arr[i].len }
						}
					};

					cur_page.el_len++;
				}
				if(i+1 >= tokens->len || (i+1 < tokens->len && (tokens->arr[i+1].type != TT_PAGE_REF && tokens->arr[i+1].type != TT_WEB_REF))){
					fprintf(stderr, "no link exists for text: "STR_PRINT"\n| next type: %d\n", STR_FMT(last_elem.as.text.str), tokens->arr[i+1].type);
					// if no target exists after the link, link it to whatever the current page is
					last_elem.as.text.is_page_ref = 1;
					last_elem.as.text.target = cur_page.metadata.section;
				}
				else{
					last_elem.as.text.is_page_ref = (tokens->arr[i+1].type == TT_PAGE_REF);
					last_elem.as.text.target = (string){
						.str = tokens->arr[i+1].str+1,
						.len = tokens->arr[i+1].len-1
					};
				}

				break;
			}
			case TT_CODE:
			{
				cur_elem = (struct element){
					.is_text = 1,
					.as.text = (struct text_element){
						.is_page_ref = -1,
						.level = -1,
						.str = (string){
							.str = tokens->arr[i].str+1,
							.len = tokens->arr[i].len-1
						}
					}
				};

				cur_page.el_len++;
				break;
			}
			case TT_IMAGE:
			{
				int src_end = get_next(tokens->arr[i].str, tokens->arr[i].len, 1, '|');

				cur_elem = (struct element){
					.is_text = 0,
					.as.image = (struct image_element){
						.src = (string){
							.str = tokens->arr[i].str+1,
							.len = src_end-1
						},
						.alt_text = (string){
							.str = tokens->arr[i].str+src_end+2,
							.len = tokens->arr[i].len-src_end-2
						}
					}
				};

				cur_page.el_len++;
				break;
			}
			case TT_PAGE_REF: case TT_WEB_REF:
			{ break; } // gets handled by TT_LINK
		}
	}

	res.pa_len++;

	return res;
}
