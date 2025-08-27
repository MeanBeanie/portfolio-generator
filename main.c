#include "tokenizer.h"
#include "website.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define new_p(el_ref,prefix,suffix)\
	if(el_ref as.text.is_page_ref < 0){\
		fprintf(file, "\t\t<p class=\"text_element_p\">"STR_PRINT"</p>\n",\
		STR_FMT(el_ref as.text.str));\
	}\
	else if(el_ref as.text.is_page_ref == 0){\
		fprintf(file, "\t\t<a class=\"text_element_p\" href=\""STR_PRINT"\">"prefix STR_PRINT suffix"</a>\n",\
		STR_FMT(el_ref as.text.target), STR_FMT(el_ref as.text.str));\
	}\
	else{\
		fprintf(file, "\t\t<a class=\"text_element_p\" href=\""STR_PRINT".html\">"prefix STR_PRINT suffix"</a>\n",\
		STR_FMT(el_ref as.text.target), STR_FMT(el_ref as.text.str));\
	}

#define new_h(el_ref,prefix,suffix)\
	if(el_ref as.text.is_page_ref < 0){\
		fprintf(file, "\t\t<h%d class=\"text_element_h\">"STR_PRINT"</h%d>\n",\
		el_ref as.text.level + HEADER_SIZE_OFFSET, STR_FMT(el_ref as.text.str), el_ref as.text.level + HEADER_SIZE_OFFSET);\
	}\
	else if(el_ref as.text.is_page_ref == 0){\
		fprintf(file, "\t\t<a class=\"text_element_h\" href=\""STR_PRINT"\"><h%d>"prefix STR_PRINT suffix"</h%d></a>\n",\
			STR_FMT(el_ref as.text.target),\
			el_ref as.text.level + HEADER_SIZE_OFFSET,\
			STR_FMT(el_ref as.text.str),\
			el_ref as.text.level + HEADER_SIZE_OFFSET\
		);\
	}\
	else{\
		fprintf(file, "\t\t<a class=\"text_element_h\" href=\""STR_PRINT".html\"><h%d>"prefix STR_PRINT suffix"</h%d></a>\n",\
			STR_FMT(el_ref as.text.target),\
			el_ref as.text.level + HEADER_SIZE_OFFSET,\
			STR_FMT(el_ref as.text.str),\
			el_ref as.text.level + HEADER_SIZE_OFFSET\
		);\
	}

int main(int argc, char* argv[]){
	if(argc < 2){
		fprintf(stderr, "Usage: portgen <infile>\n");
		return 1;
	}

	FILE* file = fopen(argv[1], "r");
	if(file == NULL){
		perror("Failed to open infile");
		return 1;
	}
	fseek(file, 0, SEEK_END);
	int buflen = ftell(file);
	char* buffer = malloc(sizeof(char)*(buflen+1));
	fseek(file, 0, SEEK_SET);
	fread(buffer, sizeof(char), buflen, file);
	buffer[buflen] = '\0';
	fclose(file);

	struct token_list tokens = tokenize(buffer, buflen);
	if(tokens.arr == NULL){
		fprintf(stderr, "\nFailed to tokenize data\n");
		goto failed_tokenize;
	}

	struct website site = process_tokens(&tokens, tokens.page_count);
	if(site.failed == 1){
		fprintf(stderr, "\nFailed to create website from tokens\n");
		goto failed_processing;
	}

	for(int i = 1; i < site.pa_len; i++){
		char filepath[128] = {0};
		strncpy(filepath, site.pages[i].metadata.section.str, site.pages[i].metadata.section.len);
		strcat(filepath, ".html");
		filepath[127] = '\0';

		FILE* file = fopen(filepath, "w");
		if(file == NULL){
			fprintf(stderr, "Failed to open file: %s\n", filepath);
			break;
		}

		fprintf(
			file,
			"<!DOCTYPE html>\n<html>\n\t<head>\n"
			"\t\t<meta charset=\"UTF-8\">\n"
			"\t\t<meta name=\"author\" content=\""STR_PRINT"\">\n"
			"\t\t<meta name=\"description\" content=\""STR_PRINT"\">\n"
			"\t\t<link rel=\"stylesheet\" href=\"style.css\">\n",
			STR_FMT(site.pages[0].metadata.author),
			STR_FMT(site.pages[0].metadata.description)
		);

		if(site.pages[i].metadata.title.len != 0){
			fprintf(file, "\t\t<title>"STR_PRINT"</title>\n", STR_FMT(site.pages[i].metadata.title));
		}
		else{
			fprintf(file, "\t\t<title>"STR_PRINT"</title>\n", STR_FMT(site.pages[0].metadata.title));
		}

		fprintf(
			file,
			"\t</head>\n\t<body>\n"
			"\t<div class=\"global_box\">\n"
		);
		fprintf(
			file,
			"\t<div style=\"display: inline;\">\n"
			"\t\t<p>"STR_PRINT"</p>"
			"\t\t<nav style=\"text-align:center;\">\n",
			STR_FMT(site.pages[0].metadata.author)
		);
		for(int j = 0; j < site.pages[0].el_len; j++){
			struct element* el = &site.pages[0].elements[j];
			if(el->type){
				new_p(el->, "[", "]");
			}
		}
		fprintf(file, "\t\t</nav>\n\t</div>\n\t\t<hr>\n");
		if(site.pages[i].metadata.name.len != 0){
			fprintf(
				file,
				"\t\t<h2 class=\"page_title\">"STR_PRINT"</h2>\n\t\t<hr>\n",
				STR_FMT(site.pages[i].metadata.name)
			);
		}

		for(int j = 0; j < site.pages[i].el_len; j++){
			struct element* el = &site.pages[i].elements[j];

			switch(el->type){
				case 0: // IMAGE
				{
					fprintf(file, "\t\t<img src=\""STR_PRINT"\" alt=\""STR_PRINT"\">",
					STR_FMT(el->as.image.src), STR_FMT(el->as.image.alt_text));
					break;
				}
				case 1: // TEXT
				{
					if(el->as.text.level == 0){
						new_p(el->, "", "");
					}
					else if(el->as.text.level < 0){
						fprintf(
							file,
							"\t\t<xmp class=\"code\">"STR_PRINT"</xmp>\n",
							STR_FMT(el->as.text.str)
						);
					}
					else{
						new_h(el->, "", "");
					}
					fprintf(file, "\t\t<p></p>\n");
					break;
				}
				case 2: // HR
				{
					fprintf(file, "\t\t<hr class=\"hori-"STR_PRINT"\">\n",
					STR_FMT(el->as.hr));
					break;
				}
			}
		}

		fprintf(file, "\t</div>\n\t</body>\n</html>");
		fclose(file);
	}

	// NOTE: not freeing the strings as they point to the already allocated token strings
	for(int i = 0; i < site.pa_len; i++){
		free(site.pages[i].elements);
	}
	free(site.pages);

failed_processing:
	for(int i = 0; i < tokens.len; i++){
		free(tokens.arr[i].str);
	}
	free(tokens.arr);

failed_tokenize:
	return 0;
}
