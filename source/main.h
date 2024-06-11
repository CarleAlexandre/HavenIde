# include <stddef.h>
# include <stdint.h>
# include <stdbool.h>
# include <sstream>
# include <vector>
# include <raylib.h>
# include <unordered_map>
# include <linked_list.h>
# include <list>
# include <haven_parser.hpp>

typedef struct s_monitor {
	int refresh_rate, height, width, id;
}t_monitor;

typedef enum {
	start = 0,
	stdview = 1,
} step_e;

typedef enum {
	show_none = 0,
	show_file = 1,
	show_edit = 2,
	show_run = 3,
} status_bar_e;

typedef enum {
	normal = 0,
	insert = 1,
	visual = 2,
	visual_block = 3,
} vi_mod;

typedef enum {
	path_tok = 1,
	theme_tok = 2,
	font_tok = 3,
	fontsize_tok = 4,
}workspace_token_e;

typedef struct s_glyph {
	char c;
	Color fg;
	Color bg;
} t_glyph;

typedef struct s_file_header{
	Vector2 dim;
	std::vector<std::list<t_glyph*>> glyphs;
	std::string name;
	std::string raw;
}t_file_header;

typedef struct s_workspace {
	std::vector<std::string> paths;
	std::vector<t_file_header *> files;
	std::string theme;
	std::string font;
	u32 fontsize;
} t_workspace;
