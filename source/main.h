# include <stddef.h>
# include <stdint.h>
# include <stdbool.h>
# include <sstream>
# include <vector>
# include <raylib.h>
# include <unordered_map>
# include <linked_list.h>
# include <list>

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

typedef struct s_node_file {
	FilePathList files;
	const char *path;
	int depth;
	bool isDirectory = false;
	std::unordered_map<const char *, s_node_file> child;
} t_node_file;

typedef struct s_glyph {
	char c;
	Color fg;
	Color bg;
} t_glyph;

typedef struct s_file_header{
	size_t size;
	Vector2 dim;
	std::vector<std::list<t_glyph*>> glyphs;
	std::string name;
}t_file_header;
