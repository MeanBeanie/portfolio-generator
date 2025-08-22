#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

const char* css_file;

int main(int argc, char* argv[]){
	if(argc < 2){ fprintf(stderr, "Usage: text2web <infile>\n"); return 1; }

	FILE* infile = fopen(argv[1], "r");
	if(infile == NULL){
		fprintf(stderr, "Failed to open file at '%s': %s\n", argv[1], strerror(errno));
		return 1;
	}

	fseek(infile, 0, SEEK_END);
	size_t in_size = ftell(infile);
	char buffer[in_size];
	fseek(infile, 0, SEEK_SET);
	if(fread(buffer, sizeof(char), in_size, infile) != in_size){
		fprintf(stderr, "Failed to read all data from file\n");
		fclose(infile);
		return 1;
	}
	fclose(infile);

	int page_count = 0;
	int token_count = 0;
	struct token* tokens = lex(buffer, in_size, &token_count, &page_count);
	if(tokens == NULL || page_count == 0 || token_count == 0){ goto end; }
	printf("--- Lexing ---\nFinished tokenizing the input file with %d tokens (%d pages)\n", token_count, page_count);
	int text_lit_count = 0;
	for(int i = 0; i < token_count; i++){
		if(tokens[i].type == TT_TEXT_LITERAL){ text_lit_count++; }
	}
	printf("Merged text literals, went from %d to ", text_lit_count);
	post_lex(tokens, token_count);
	text_lit_count = 0;
	for(int i = 0; i < token_count; i++){
		if(tokens[i].type == TT_TEXT_LITERAL){ text_lit_count++; }
	}
	printf("%d\n--- END Lexing ---\n", text_lit_count);

	struct website site = parse(tokens, token_count, page_count);
	printf("--- Parsing ---\nCreated site %.*s, with %d pages\n", site.metadata.title.len, site.metadata.title.start, site.page_c);
	printf(
		"--- Metadata ---\n> Title: "STRING_FMT"\n> Author: "STRING_FMT"\n> Desc: "STRING_FMT"\n--- END Metadata ---\n",
		STRING_ARGS(site.metadata.title),
		STRING_ARGS(site.metadata.author),
		STRING_ARGS(site.metadata.description)
	);
	for(int i = 0; i < site.page_c; i++){
		printf("- Page %d: \"%.*s\": %d text elements\n", i, site.pages[i].title.len, site.pages[i].title.start, site.pages[i].text_el_c);
	}
	printf("--- END Parsing ---\n");

	char filepath[128];
	for(int i = 0; i < site.page_c; i++){
		memset(filepath, 0, 128);
		strncpy(filepath, site.pages[i].title.start, site.pages[i].title.len);
		strcat(filepath, ".html");
		filepath[127] = '\0';
		FILE* file = fopen(filepath, "w");
		if(file == NULL){ fprintf(stderr, "Failed to open file '%s': %s\n", filepath, strerror(errno)); continue; }
		fprintf(file, "<!DOCTYPE html>\n<html>\n");

		fprintf(
			file,
			"\t<head>\n"
			"\t\t<meta charset=\"UTF-8\">\n"
			"\t\t<title>"STRING_FMT"</title>\n"
			"\t\t<meta name=\"author\" content=\""STRING_FMT"\">\n"
			"\t\t<meta name=\"description\" content=\""STRING_FMT"\">\n"
			"\t\t<link rel=\"stylesheet\" href=\"style.css\">\n"
			"\t</head>\n",
			STRING_ARGS(site.metadata.title),
			STRING_ARGS(site.metadata.author),
			STRING_ARGS(site.metadata.description)
		);

		fprintf(
			file,
			"\t<body>\n"
			"\t<div class=\"text_element_box\">\n"
			"\t\t<h1 class=\"page_title\">"STRING_FMT"</h1>\n\t\t<hr>\n",
			STRING_ARGS(site.pages[i].title)
		);

		for(int j = 0; j < site.pages[i].text_el_c; j++){
			struct text_element* el = &site.pages[i].text_elements[j];
			if(el->target.len == 0){
				if(el->level == 0){
					fprintf(
						file,
						"\t\t<p class=\"text_element_p\">"STRING_FMT"</p>\n",
						STRING_ARGS(el->text)
					);
				}
				else{
					fprintf(
						file,
						"\t\t<h%d class=\"text_element_h\">"STRING_FMT"</h%d>\n",
						el->level, STRING_ARGS(el->text), el->level
					);
				}
			}
			else{
				fprintf(
					file,
					"\t\t<a class=\"text_element_p\" href=\""STRING_FMT".html\">"STRING_FMT"</a>\n"
					"\t\t<p></p>\n",
					STRING_ARGS(el->target), STRING_ARGS(el->text)
				);
			}
		}

		fprintf(file, "\t</div>\n\t</body>\n</html>");
		fclose(file);
	}

	FILE* file = fopen("style.css", "w");
	fwrite(css_file, sizeof(char), strlen(css_file), file);
	fclose(file);

end:
	free(tokens);
	return 0;
}

const char* css_file = 
"body {\n"
"\tbackground-color: #202020;\n"
"\tcolor: #ddd;\n"
"}\n"
".page_title {\n"
"\ttext-align: center;\n"
"}\n"
".text_element_p {\n"
"\tmargin-left: 1%;\n"
"\twidth: 98%;\n"
"\toverflow-wrap: break-word;\n"
"}\n"
".text_element_h {\n"
"\ttext-align: center;\n"
"}\n"
".text_element_box {\n"
"\tmargin-left: 24%;\n"
"\twidth: 52%;\n"
"\theight: 100%;\n"
"\tborder-right: 1px solid red;\n"
"\tborder-left: 1px solid red;\n"
"}\n"
"\0";
