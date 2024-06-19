#include "main.h"
#include <cassert>
#include <stdlib.h>

typedef enum {
	path_tok = 1,
	theme_tok = 2,
	font_tok = 3,
	fontsize_tok = 4,
	history_size_tok = 5,
	cursor_style = 6,
	cursor_color = 7,
	cursor_smooth = 8,
}workspace_token_e;

t_workspace loadWorkspace(const char *workspace_filepath){
	t_workspace workspace;
	std::unordered_map<std::string, workspace_token_e> dictionnary = {
		{"paths", path_tok},
		{"theme", theme_tok},
		{"font", font_tok},
		{"fontsize", fontsize_tok},
		{"history_size", history_size_tok},
		{"cursor_style", cursor_style},
		{"cursor_color", cursor_color},
		{"cursor_smooth", cursor_smooth},
	};

	char *data = LoadFileText(workspace_filepath);

	std::vector<t_token> tok = tokenizer(data, ",", 1, dictionnary);

	for (auto token: tok) {
		switch (token.identifier) {
			case (path_tok): {
				splitPath(token.value, workspace.paths);
				break;
			}
			case (theme_tok): {
				workspace.theme = token.value;
				break;
			}
			case (font_tok): {
				workspace.font = token.value;
				break;
			}
			case (fontsize_tok): {
				workspace.fontsize = atoi(token.value.c_str());
				break;
			}
			case (history_size_tok): {
				workspace.history_size = atoi(token.value.c_str());
				break;
			}
			case (cursor_color): {
				int count;
				const char **tmp = TextSplit(token.value.c_str(), '.', &count);
				assert(count == 4);
				workspace.cursor_style.color = {
					(unsigned char)atoi(tmp[0]),
					(unsigned char)atoi(tmp[1]),
					(unsigned char)atoi(tmp[2]),
					(unsigned char)atoi(tmp[3])
				};
				break;
			}
			case (cursor_style): {
				workspace.cursor_style.style = (style_cursor_e)atoi(token.value.c_str());
				break;
			}
			case (cursor_smooth): {
				workspace.cursor_style.smooth = token.value == "true" ? true : false;
				break;
			}
			default:break;
		}
	}
	for (auto file : workspace.paths) {
		auto tmp = loadFileRW(file.c_str());
		assert(tmp);
		tmp->cursor.alpha = 1;
		tmp->cursor.pos = {0, 0};
		tmp->cursor.render_pos = {0, 0};
		tmp->cursor.idx = 0;
		workspace.files.push_back(tmp);
	}
	free(data);
	return (workspace);
}