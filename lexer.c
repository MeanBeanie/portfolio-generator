#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>

// uses '\1' for whitespace cause realistically, who is searching for a '\1'
#define C_WHITESPACE '\1'
#define last_char(c) ((i > 0 && buffer[i-1] == c) || i == 0)

#define ensure_size \
	if((*t_size) >= t_cap){\
		t_cap *= 2;\
		tokens = realloc(tokens, sizeof(struct token)*t_cap);\
	}
#define new_token(t,s,l) \
	tokens[(*t_size)++] = (struct token){\
		.type = t,\
		.start = s,\
		.len = l,\
	}

// returns how many characters exist between the start and the next target char
int find_next(char* buf, int buflen, int start, char target){
	if(target == '\1'){
		for(int i = start+1; i < buflen; i++){
			if(buf[i] == ' '
			|| buf[i] == '\n'
			|| buf[i] == '\t'
			|| buf[i] == '\r'){
				return i-start;
			}
		}
	}
	else{
		for(int i = start+1; i < buflen; i++){
			if(buf[i] == target){ return i-start; }
		}
	}
	
	return -1;
}

struct token* lex(char* buffer, int buflen, int* t_size, int* page_count){
	int t_cap = 8;
	struct token* tokens = malloc(sizeof(struct token)*t_cap);

	for(int i = 0; i < buflen; i++){
		if(last_char('\n') && buffer[i] == '\n'){ continue; }
		else if(last_char('\n') && buffer[i] == '['){
			ensure_size;
			int len = find_next(buffer, buflen, i, ']');
			if(len < 0){
				fprintf(stderr, "Unclosed '[' found\n");
				goto lexing_failed;
			}
			new_token(TT_SECTION, buffer+i, len);
			(*page_count)++;
			i += len;
		}
		else if(buffer[i] == '('){
			ensure_size;
			int len = find_next(buffer, buflen, i, ')');
			if(len < 0){
				fprintf(stderr, "Unclosed '(' found\n");
				goto lexing_failed;
			}
			new_token(TT_SECTION_REF, buffer+i, len);
			i += len;
		}
		else if(buffer[i] == '>'){
			ensure_size;
			new_token(TT_LINK, buffer+i, 1);
		}
		else if(last_char('\n') && buffer[i] == '#'){
			ensure_size;
			int len = find_next(buffer, buflen, i, '\n');
			if(len < 0){
				fprintf(stderr, "Expected newline after header\n");
				goto lexing_failed;
			}
			new_token(TT_HEADER, buffer+i, len);
			i += len;
		}
		else if(last_char('\n') && buffer[i] == '%'){
			ensure_size;
			int len = find_next(buffer, buflen, i, '\n');
			if(len < 0){
				fprintf(stderr, "Expected newline after metadata\n");
				goto lexing_failed;
			}
			new_token(TT_METADATA, buffer+i, len+1);
			i += len;
		}
		else if(last_char(' ') || last_char('\n') || last_char('\t') || last_char('\r')){
			// probably text, who knows
			ensure_size;
			int len = find_next(buffer, buflen, i, C_WHITESPACE);
			if(len < 0){
				fprintf(stderr, "Somehow you managed no whitespace(like not even a newline) after text\n");
				goto lexing_failed;
			}
			new_token(TT_TEXT_LITERAL, buffer+i, len);
			i += len;
		}
	}

	return tokens;
	// only reached when using goto
lexing_failed:
	free(tokens);
	return NULL;
}

void post_lex(struct token* tokens, int count){
	for(int i = 0; i < count-1; i++){
		for(int j = i+1; j < count; j++){
			if(tokens[j].type != TT_TEXT_LITERAL){ break; }
			if(tokens[i].type == TT_TEXT_LITERAL && tokens[j].type == TT_TEXT_LITERAL){
// Forces text objects to not connect over a newline
				int failed = 0;
				for(int k = 0; k < tokens[i].len+tokens[j].len+1; k++){
					if(tokens[i].start[k] == '\n'){
						failed = 1;
						break;
					}
				}
				if(failed == 1){ continue; }
				tokens[i].len += tokens[j].len+1;
				tokens[j].type = TT_IGNORE;
			}
		}
	}
}
