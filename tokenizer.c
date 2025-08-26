#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define next(target) get_next(buffer, buflen, i+1, target)
int get_next(char* buffer, int buflen, int start, char target){
	for(int i = start; i < buflen; i++){
		if(buffer[i] == target){ return i; }
	}
	return -1;
}

#define ensure_len\
	if(res.len >= res.capacity){\
		res.capacity *= 2;\
		res.arr = realloc(res.arr, res.capacity*sizeof(struct token));\
	}

#define fail_if(cond,msg)\
	if((cond)){\
		fprintf(stderr, "\n[line %d] "msg, line);\
		goto failure;\
	}

#define cur_elem res.arr[res.len]

struct token_list tokenize(char* buffer, int buflen){
	struct token_list res = (struct token_list){
		.arr = malloc(sizeof(struct token)*8),
		.len = 0,
		.capacity = 8,
		.page_count = 0
	};

	int line = 1;

	char last_c = '\n';
	for(int i = 0; i < buflen; i++){
		ensure_len;
		char c = buffer[i];
		if(c == '\n'){
			line++;
		}

		switch(c){
			case '"':
			{
				int end = next('"');
				fail_if(end < 0, "Unclosed '\"' found\n");

				cur_elem = (struct token){
					.type = TT_TEXT,
					.len = end-i,
					.str = malloc(sizeof(char)*(end-i+1))
				};
				strncpy(cur_elem.str, buffer+i, cur_elem.len);
				cur_elem.str[cur_elem.len] = '\0';

				i = end;
				res.len++;
				break;
			}
			case '>':
			{
				// doesn't factor into the str, just ensures that a reference follows
				int end = next('(');
				if(end < 0){ end = next('{'); }
				fail_if(end < 0, "Link expects a page reference or a website reference afterwards\n");

				cur_elem = (struct token){
					.type = TT_LINK,
					.len = 1,
					.str = malloc(2*sizeof(char))
				};
				cur_elem.str[0] = '>';
				cur_elem.str[1] = '\0';

				i += cur_elem.len;
				res.len++;
				break;
			}
			case '(':
			{
				int end = next(')');
				fail_if(end < 0, "Unclosed '(' found\n");

				cur_elem = (struct token){
					.type = TT_PAGE_REF,
					.len = end-i,
					.str = malloc(sizeof(char)*(end-i+1))
				};
				strncpy(cur_elem.str, buffer+i, cur_elem.len);
				cur_elem.str[cur_elem.len] = '\0';

				i += cur_elem.len;
				res.len++;
				break;
			}
			case '{':
			{
				int end = next('}');
				fail_if(end < 0, "Unclosed '{' found\n");

				cur_elem = (struct token){
					.type = TT_WEB_REF,
					.len = end-i,
					.str = malloc(sizeof(char)*(end-i+1))
				};
				strncpy(cur_elem.str, buffer+i, cur_elem.len);
				cur_elem.str[cur_elem.len] = '\0';

				i += cur_elem.len;
				res.len++;
				break;
			}
			case '`':
			{
				int end = next('`');
				fail_if(end < 0, "Unclosed '`' found\n");

				cur_elem = (struct token){
					.type = TT_CODE,
					.len = end-i,
					.str = malloc(sizeof(char)*(end-i+1))
				};
				strncpy(cur_elem.str, buffer+i, cur_elem.len);
				cur_elem.str[cur_elem.len] = '\0';

				i += cur_elem.len;
				res.len++;
				break;
			}
			default: break;
		}
		if(last_c == '\n'){
			switch(c){
				case '-':
				{
					int end = next('-');
					fail_if(end < 0, "Expected closing '-' for <hr>\n");

					cur_elem = (struct token){
						.type = TT_HR,
						.len = end-i,
						.str = malloc(sizeof(char)*(end-i+1))
					};
					strncpy(cur_elem.str, buffer+i, cur_elem.len);
					cur_elem.str[cur_elem.len] = '\0';

					i += cur_elem.len;
					res.len++;
					break;
				}
				case '.':
				{
					int end = next('\n');
					fail_if(end < 0, "Expected newline after include\n");

					char filepath[128] = {0};
					strncpy(filepath, buffer+i+1, end-i-1);

					FILE* file = fopen(filepath, "r");
					fail_if(file == NULL, "Failed to open included file\n");
					fseek(file, 0, SEEK_END);
					int fsize = ftell(file);
					buffer = realloc(buffer, buflen+fsize+1);
					fail_if(buffer == NULL, "Failed to get enough space to include file\n");
					fseek(file, 0, SEEK_SET);

					for(int j = end+1; j < buflen; j++){
						buffer[j+fsize] = buffer[j];
					}

					fread(buffer+end+1, sizeof(char), fsize, file);
					fclose(file);

					buflen += fsize;
					buffer[buflen] = '\0';
					break;
				}
				case '$':
				{
					int end = next('\n');
					fail_if(end < 0, "Expected newline after image\n");

					cur_elem = (struct token){
						.type = TT_IMAGE,
						.len = end-i,
						.str = malloc(sizeof(char)*(end-i+1))
					};
					strncpy(cur_elem.str, buffer+i, cur_elem.len);
					cur_elem.str[cur_elem.len] = '\0';

					i += cur_elem.len;
					res.len++;
					break;
				}
				case '[':
				{
					int end = next(']');
					fail_if(end < 0, "Unclosed '[' found\n");

					cur_elem = (struct token){
						.type = TT_SECTION,
						.len = end-i,
						.str = malloc(sizeof(char)*(end-i+1))
					};
					strncpy(cur_elem.str, buffer+i, cur_elem.len);
					cur_elem.str[cur_elem.len] = '\0';
					res.page_count++;

					i = end;
					res.len++;
					break;
				}
				case '~':
				{
					int end = next('\n');
					fail_if(end < 0, "Title expects a newline afterwards\n");

					cur_elem = (struct token){
						.type = TT_TITLE,
						.len = end-i,
						.str = malloc(sizeof(char)*(end-i+1))
					};
					strncpy(cur_elem.str, buffer+i, cur_elem.len);
					cur_elem.str[cur_elem.len] = '\0';

					i += cur_elem.len;
					res.len++;
					break;
				}
				case '%':
				{
					int end = next('\n');
					fail_if(end < 0, "Website metadata expects a newline afterwards\n");

					cur_elem = (struct token){
						.type = TT_METADATA,
						.len = end-i,
						.str = malloc(sizeof(char)*(end-i+1))
					};
					strncpy(cur_elem.str, buffer+i, cur_elem.len);
					cur_elem.str[cur_elem.len] = '\0';

					i += cur_elem.len;
					res.len++;
					break;
				}
				case '#':
				{
					// this is mostly done to make sure it has a text element after it
					int end = next('"');
					fail_if(end < 0, "Header expects a text element after it\n");

					cur_elem = (struct token){
						.type = TT_HEADER,
						.len = end-i,
						.str = malloc(sizeof(char)*(end-i+1))
					};
					strncpy(cur_elem.str, buffer+i, cur_elem.len);
					cur_elem.str[cur_elem.len] = '\0';

					i += cur_elem.len-1;
					res.len++;
					break;
				}
				default: break;
			}
		}

		last_c = buffer[i];
	}

	return res;
failure:
	free(res.arr);
	res.arr = NULL;
	return res;
}
