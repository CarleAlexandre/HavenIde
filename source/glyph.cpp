#include "main.h"
#include <cassert>
#include <string.h>

t_glyph *createGlyph(int data, Color fg) {
	t_glyph *glyph;

	glyph = (t_glyph *)malloc(sizeof(t_glyph));
	assert(glyph);
	glyph->codepoint = data;
	glyph->fg = fg;
	if (glyph->codepoint == '\t') {
		glyph->spacing = 4;
		return (glyph);
	}
	glyph->spacing = 1;
	return (glyph);
}

std::list<t_glyph *> loadGlyphLine(int *data, int *count, int max_size) {
	std::list<t_glyph *> lst;
	int i = 0;

	for (; i + *count < max_size;i++) {
		lst.push_back(createGlyph(data[i], WHITE));
		if (data[i] == '\n')break;
	}
	*count += i;
	return (lst);
}

t_file_header *loadFileRW(const char *filepath) {
	int count = 0;
	assert(filepath);
	t_file_header *new_file = 0x00;
	new_file = (t_file_header *)malloc(sizeof(t_file_header));
	assert(new_file);
	memset(new_file, 0, sizeof(t_file_header));
	new_file->name = strdup(filepath);
	new_file->is_saved = true;
	new_file->cursor = {};
	char *data = LoadFileText(filepath);
	if (!data){
		new_file->glyphs.push_back(std::list<t_glyph *>(0x00));
		return(new_file);
	}
	new_file->codepoints = LoadCodepoints(data, &new_file->codepoint_size);

	int *span = new_file->codepoints;
	for (int i = 0; i < new_file->codepoint_size; i++) {
		new_file->glyphs.push_back(loadGlyphLine(&new_file->codepoints[i], &i, new_file->codepoint_size));
	}
	if (new_file->glyphs.back().back()->codepoint == '\n') {
		new_file->glyphs.push_back(std::list<t_glyph *>({createGlyph('\0', WHITE)}));
	} else {
		new_file->glyphs.back().push_back(createGlyph('\0', WHITE));
	}
	free(data);
	return (new_file);
};

void splitPath(std::string &from, std::vector<std::string> &paths) {
	std::string span;
	std::stringstream stream(from.c_str());

	while (std::getline(stream, span)) {
		paths.push_back(span);
	}
}