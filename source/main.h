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
# include <mutex>
# include <queue>

typedef struct s_monitor {
	int refresh_rate, height, width, id;
}t_monitor;

typedef enum {
	close = -1,
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
	insert_block = 4,
} vi_mod;

typedef enum {
	box_cursor = 1,
	half_box_cursor = 2,
	underscore_cursor = 3,
	pipe_cursor = 4,
} style_cursor_e;

typedef struct s_glyph {
	int codepoint;
	int spacing;//number of space beffore next char
	Color fg;
} t_glyph;

typedef struct s_cursor {
	int idx;
	Vector2 pos;
	Vector2 render_pos;
	float alpha;
} t_cursor;

typedef struct s_cursor_style {
	Color color;
	style_cursor_e style;
} t_cursor_style;

typedef struct s_file_header{
	t_cursor cursor;
	char *name;
	bool is_saved = true;
	std::vector<std::list<t_glyph*>> glyphs;
	int line_number;
	int *codepoints;
	int codepoint_size;
}t_file_header;

typedef struct s_terminal {
	char in[100];
	std::queue<char> out;
	std::list<std::string> fOut;
	std::mutex mtx;
	bool open = false;
} t_terminal;

typedef struct s_workspace {
	std::vector<std::string> paths;
	std::vector<t_file_header *> files;
	std::string theme;
	std::string font;
	u32 fontsize;
	u32 history_size;
	t_cursor_style cursor_style;
} t_workspace;

typedef enum {
	c_ext = 1,
	cpp_ext = 2,
	markdown_ext = 3,
} extension_enum;

typedef struct s_context {
	int current_file = 0;
	Rectangle terminal_bound;
	Rectangle texteditor_bound;
	vi_mod mode;
	t_terminal term;
	Font font;
	bool setting_open = false;
} t_context;

#define INPUT_TIME 0.075

static std::unordered_map<std::string, int> extension_dictionnary{
	{"c", c_ext},
	{"h", c_ext},
	{"cpp", cpp_ext},
	{"cc", cpp_ext},
	{"hh", cpp_ext},
	{"hpp", cpp_ext},
	{"md", markdown_ext},
};

bool execCmd(char *cmd, std::queue<char> &out, int max_size);
t_glyph *createGlyph(int data, Color fg) ;
std::list<t_glyph *> loadGlyphLine(int *data, float *x, int *count);
t_file_header *loadFileRW(const char *filepath);
void splitPath(std::string &from, std::vector<std::string> &paths);
t_workspace loadWorkspace(const char *workspace_filepath);
void renderSetting(t_workspace *workspace, bool *active);
