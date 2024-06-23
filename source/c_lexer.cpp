#include "main.h"
#include <ctype.h> 
#include <stdbool.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <cassert>

#define MAX_LENGTH 100 

bool isDelimiter(char chr) {
    return (chr == ' ' || chr == '+' || chr == '-'
            || chr == '*' || chr == '/' || chr == ','
            || chr == ';' || chr == '%' || chr == '>'
            || chr == '<' || chr == '=' || chr == '('
            || chr == ')' || chr == '[' || chr == ']'
            || chr == '{' || chr == '}' || chr == '\t'
			|| chr == '\n'); 
} 

bool isOperator(char chr) {
    return (chr == '+' || chr == '-' || chr == '*'
            || chr == '/' || chr == '>' || chr == '<'
            || chr == '='); 
} 

bool isValidIdentifier(char* str) {
    return (str[0] != '0' && str[0] != '1' && str[0] != '2'
            && str[0] != '3' && str[0] != '4'
            && str[0] != '5' && str[0] != '6'
            && str[0] != '7' && str[0] != '8'
            && str[0] != '9' && !isDelimiter(str[0])); 
} 

bool isKeyword(char* str) { 
    const char* keywords[] 
        = { "auto",     "break",    "case",     "char", 
            "const",    "continue", "default",  "do", 
            "double",   "else",     "enum",     "extern", 
            "float",    "for",      "goto",     "if", 
            "int",      "long",     "register", "return", 
            "short",    "signed",   "sizeof",   "static", 
            "struct",   "switch",   "typedef",  "union", 
            "unsigned", "void",     "volatile", "while" }; 
    for (int i = 0; 
         i < sizeof(keywords) / sizeof(keywords[0]); i++) { 
        if (strcmp(str, keywords[i]) == 0) { 
            return true; 
        } 
    } 
    return false; 
} 

bool isInteger(char* str) {
    if (str == NULL || *str == '\0') { 
        return false; 
    } 
    int i = 0; 
    while (isdigit(str[i])) { 
        i++;
    } 
    return str[i] == '\0';
} 

char* getSubstring(char* str, int start, int end) {
    int length = strlen(str); 
    int subLength = end - start + 1; 
    char* subStr 
        = (char*)malloc((subLength + 1) * sizeof(char)); 
    strncpy(subStr, str + start, subLength); 
    subStr[subLength] = '\0'; 
    return subStr; 
} 

int getContext(char tok) {
	static int context = 0;

	switch (tok) {
		case ('\''): {
			break;
		}
		case ('\"'): {
			break;
		}
		case ('{'): {
			break;
		}
		case ('}'): {
			break;
		}
		case ('['): {
			break;
		}
		case (']'): {
			break;
		}
		case ('('): {
			break;
		}
		case (')'): {
			break;
		}
		default:break;
	}
	return (context);
}

typedef struct s_lexical_analyzer {
	int end;
	Color color;
} t_lexical_analyzer;

std::vector<t_lexical_analyzer> lexicalAnalyzer(char* input, const size_t len) {
	std::vector<t_lexical_analyzer> ret;
    int left = 0, right = 0; 

    while (right <= len && left <= right) { 
        if (!isDelimiter(input[right])) 
            right++; 
  
        if (isDelimiter(input[right]) && left == right) { 
            if (isOperator(input[right])) {
                printf("Token: Operator, Value: %c\n", input[right]);
				ret.push_back((t_lexical_analyzer){right, RED});
			}
            right++; 
            left = right; 
        } 
        else if (isDelimiter(input[right]) && left != right 
                 || (right == len && left != right)) { 
            char* subStr 
                = getSubstring(input, left, right - 1); 
  
            if (isKeyword(subStr)) {
                printf("Token: Keyword, Value: %s\n", subStr);
				ret.push_back((t_lexical_analyzer){right, GREEN});
			}
  
            else if (isInteger(subStr)) {
                printf("Token: Integer, Value: %s\n", subStr);
				ret.push_back((t_lexical_analyzer){right, PURPLE});
			}
            else if (isValidIdentifier(subStr) 
                     && !isDelimiter(input[right - 1])) {
                printf("Token: Identifier, Value: %s\n", subStr); 
				ret.push_back((t_lexical_analyzer){right, WHITE});
			}
            else if (!isValidIdentifier(subStr) 
                     && !isDelimiter(input[right - 1])) {
                printf("Token: Unidentified, Value: %s\n", subStr);
				ret.push_back((t_lexical_analyzer){right, WHITE});
			}
            left = right; 
        } 
    } 
    return (ret); 
}

void c_lexer() {

}
  
void getTextColor(t_file_header *file, const int start, const int end) {
	char *data;
	int len = end - start;
	if (end > file->codepoint_size) {
		len = file->codepoint_size - start;
	}
	std::vector<int> span;
	for (int y = 0; y < file->glyphs.size(); y++) {
		for (auto line : file->glyphs[y]) {
			span.push_back(line->codepoint);
		}
	}
	UnloadCodepoints(file->codepoints);

	file->codepoint_size = span.size();
	file->codepoints = (int *)MemAlloc(sizeof(int) * file->codepoint_size);
	assert(file->codepoints);
	for (int i = 0; i < span.size(); i++) {
		file->codepoints[i] = span[i];
	}
	data = LoadUTF8(file->codepoints + start, len);
	auto token = lexicalAnalyzer(data, strlen(data));
	UnloadUTF8(data);
	int i = 0;
	int k = 0;
	for (int y = 0; y < file->glyphs.size(); y++) {
		for (auto at : file->glyphs[y]) {
			at->fg = token[k].color;
			if (i++ == token[k].end) {
				k++;
			}
		}
	}
	token.clear();
}


