# include "main.h"
#include <cassert>
# define GLSL_VERSION 330
# include <raylib.h>
# define RAYGUI_IMPLEMENTATION
# include <raygui.h>
# include <raymath.h>
# include <stdlib.h>
# include <stdio.h>
# include <haven_logic.h>
# include <haven_file.hpp>

struct context {
	int cursor_pos;
	int font_size;
	t_file_header file;
} ctx;

std::list<t_glyph*> createLine(char *data, int *status, Vector2 *pos) {
	static int iter = 0;
	int x = 0;
	std::list<t_glyph*> lst;

	while (data[iter]) {
		if (pos->x < x) {
			pos->x = x;
		}
		if (data[iter] == '\n') {
			pos->y++;
			x = 0;
			iter++;
			*status = iter;
			return (lst);
		}
		t_glyph * tmp = new t_glyph;
		assert(tmp);
		tmp->c = data[iter];
		tmp->fg = WHITE;
		lst.push_back(tmp);
		iter++;
		x++;
	}
	*status = -1;
	iter = 0;
	return (lst);
}

Vector2 VecFileToGlyph(const char *filepath) {
	Vector2 pos;
	int status = 0;

	char *data = readFile(filepath);
	while (status > -1) {
		ctx.file.glyphs.push_back(createLine(data, &status, &pos));
		ctx.file.size = status;
	}
	free(data);
	return (pos);
};

t_glyph *createGlyph(const char c, Color fg) {
	t_glyph *glyph = new t_glyph;
	assert(glyph);
	glyph->c = c;
	glyph->fg = fg;
	return (glyph);
}

t_node_file loadWorkspace(void){
	FilePathList workspace_files;
	workspace_files = LoadDirectoryFilesEx(GetWorkingDirectory(), NULL, true);
	int depth = 1;
	t_node_file root;

	root.files = LoadDirectoryFiles(GetWorkingDirectory());
	root.depth = 0;
	root.path = GetWorkingDirectory();

	t_node_file *current_node;
	current_node = &root;
	for (int i = 0; i < current_node->files.count; i++) {
		t_node_file child;
		child.path = current_node->files.paths[i];
		child.files = LoadDirectoryFiles(child.path);
		child.depth = depth;
		if (DirectoryExists(current_node->files.paths[i])) {
			child.isDirectory = true;
		} else {
			child.isDirectory = false;
		}
		current_node->child.insert( {child.path, child});
	}
	for (int y = 0; y < root.files.count; y++) {
		current_node = &root.child[root.files.paths[y]];
		for (int i = 0; i < current_node->files.count; i++) {
			t_node_file child;
			child.path = current_node->files.paths[i];
			child.files = LoadDirectoryFiles(child.path);
			child.depth = depth;
			if (DirectoryExists(current_node->files.paths[i])) {
				child.isDirectory = true;
			} else {
				child.isDirectory = false;
			}
			current_node->child.insert( {child.path, child});
		}
	}
	return (root);
}

void ControlBar() {
	static int status_bar_show = 0;

	GuiDrawRectangle((Rectangle){0, 0, (float)GetScreenWidth(), 20}, 1, GREEN, BLACK);
	if (GuiButton((Rectangle){0, 0, 60 , 20}, "file")) {
		status_bar_show = show_file;
	}
	if (GuiButton((Rectangle){60, 0, 60 , 20}, "edit")) {
		status_bar_show = show_edit;
	}
	if (GuiButton((Rectangle){120, 0, 60 , 20}, "run")) {
		status_bar_show = show_run;
	}
	/*
		file
		edit
		run
		dock?
		help?
	*/
	switch (status_bar_show) {
		case(show_file): {
			//new_file
			//save_file
			//save_files

			break;
		}
		case(show_edit): {
			break;
		}
		case(show_run): {
			break;
		}
		default:break;
	}
}

void saveTheFile(t_file_header file) {
	std::string stream;

	for (int y = 0; y < file.dim.y; y++) {
		auto &span = file.glyphs[y];
		for (auto it : span) {
			stream += it->c;
		}
		stream += '\n';
	}
	writeFile(ctx.file.name.c_str(), stream.c_str(), stream.size());
	stream.clear();
}

void TextEditor(const Rectangle bound) {
	static Vector2 scroll = {};
	static Rectangle view = {};
	static Vector2 cursor = {};
	static int start_line = 0;
	static bool is_saved = true;

	if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_S) && !is_saved) {
		saveTheFile(ctx.file);
		is_saved = true;
	}

	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
		Vector2 mouse_pos = GetMousePosition();
		cursor.x = floor((mouse_pos.x - 40 - bound.x - scroll.x) / (ctx.font_size));
		cursor.y = floor((mouse_pos.y - bound.y - 30 - scroll.y) / (ctx.font_size * 1.5));
		cursor.y = clamp(cursor.y, 0, ctx.file.dim.y - 1);
		cursor.x = clamp(cursor.x, 0, ctx.file.glyphs[cursor.y].size());
	}
	if (IsKeyDown(KEY_LEFT)) {
		if (cursor.x > 0) {
			cursor.x--;
		} else if (cursor.y > 0){
			cursor.y--;
			cursor.x = ctx.file.glyphs[cursor.y].size();
		}
	}
	if (IsKeyDown(KEY_RIGHT)) {
		if (cursor.x < ctx.file.glyphs[cursor.y].size()) {
			cursor.x++;
		} else if (cursor.y < ctx.file.dim.y - 1) {
			cursor.y++;
			cursor.x = 0;
		}
	}
	if (IsKeyDown(KEY_UP) && cursor.y > 0) {
		cursor.y--;
	}
	if (IsKeyDown(KEY_DOWN) && cursor.y < ctx.file.dim.y - 1) {
		cursor.y++;
	}

	auto insert_place = ctx.file.glyphs[cursor.y].begin();
	std::advance(insert_place, cursor.x);

	if (IsKeyPressed(KEY_BACKSPACE)) {
		if (cursor.x) {
			cursor.x--;
			insert_place = ctx.file.glyphs[cursor.y].erase(--insert_place);
			ctx.file.size--;
			if (is_saved)
				is_saved = false;
		} else if (cursor.y >= 1 && ctx.file.glyphs[cursor.y - 1].empty()) {
			ctx.file.glyphs.erase(ctx.file.glyphs.begin() + cursor.y - 1);
			ctx.file.dim.y--;
			cursor.y--;
			cursor.x = 0;
			if (is_saved)
				is_saved = false;
		} else if (cursor.y >= 1) {
			ctx.file.dim.y--;
			cursor.x = ctx.file.glyphs[cursor.y - 1].size();
			cursor.y--;
			ctx.file.glyphs[cursor.y].splice(ctx.file.glyphs[cursor.y].end(), ctx.file.glyphs[cursor.y + 1]);
			ctx.file.glyphs.erase(ctx.file.glyphs.begin() + cursor.y + 1);
			if (is_saved)
				is_saved = false;
		}
	}
	if (IsKeyDown(KEY_ENTER)) {
		std::list<t_glyph *> lst = {};
		lst.splice(lst.begin(), ctx.file.glyphs[cursor.y], insert_place++, ctx.file.glyphs[cursor.y].end());
		cursor.y++;
		ctx.file.glyphs.insert(ctx.file.glyphs.begin() + cursor.y, lst);
		cursor.x = 0;
		ctx.file.size++;
		ctx.file.dim.y++;
		insert_place = ctx.file.glyphs[cursor.y].begin();
		if (is_saved)
			is_saved = false;
	}
	if (IsKeyDown(KEY_TAB)) {
		ctx.file.glyphs[cursor.y].emplace(insert_place++, createGlyph('\t', WHITE));
		cursor.x++;
		ctx.file.size++;
		if (is_saved)
			is_saved = false;
	}
	
	int key = GetCharPressed();
	while (key) {
		if ((key >= 32) && (key <= 125)) {
			ctx.file.glyphs[cursor.y].emplace(insert_place++, createGlyph((char)key, WHITE));
			cursor.x++;
			ctx.file.size++;
			if (is_saved)
				is_saved = false;
		}
		key = GetCharPressed();
	}

	GuiScrollPanel(bound, ctx.file.name.c_str(), (Rectangle){0, 0, (ctx.file.dim.x + 5) * ctx.font_size + 30, (float)30+(ctx.file.dim.y * (float)(ctx.font_size * 1.5))}, &scroll, &view);

	BeginScissorMode(view.x, view. y, view. width, view. height);


	start_line = 0 - scroll.y / (ctx.font_size * 1.5);
	GuiDrawRectangle((Rectangle){bound.x + 1, view.y + 1, 30, view.height - 1}, 1, WHITE, BLACK);
	for (int y = start_line; y < start_line + 200 && y < ctx.file.dim.y; y++) {
		std::list<t_glyph *> &line = ctx.file.glyphs[y];
		if (!line.empty()) {
			int x = 0;
			for (auto tmp : line) {
				char character[2] = {tmp->c, '\0'};
				DrawText(character, 40 + bound.x + scroll.x + x * ctx.font_size, bound.y + 30 + scroll.y + y * (ctx.font_size * 1.5), ctx.font_size, tmp->fg);
				x++;
			}
		}
		DrawText(TextFormat(" %5i ", y + 1), bound.x, bound.y + 30 + scroll.y + y * (ctx.font_size * 1.5), ctx.font_size, WHITE);
	}
	DrawRectangleLines(40 + bound.x + scroll.x + cursor.x * ctx.font_size, bound.y + 30 + scroll.y + cursor.y * (ctx.font_size * 1.5) + ctx.font_size * 0.5, ctx.font_size, ctx.font_size * 0.5, RED);
	EndScissorMode();
	if (is_saved) {
		DrawCircle(bound.width + bound.x - 7, bound.height + bound.y - 9, 4, BLUE);
	} else {
		DrawCircle(bound.width + bound.x - 7, bound.height + bound.y - 9, 4, RED);
	}
}

void LanguageServer(const Rectangle bound) {
	static Vector2 scroll;
	static Rectangle view;

	GuiScrollPanel(bound, "lsp:", (Rectangle){}, &scroll, &view);
	BeginScissorMode(view.x, view.y, view.width, view.height);
	EndScissorMode();
}

void StackViewer(const Rectangle bound) {
	static Vector2 scroll;
	static Rectangle view;

	GuiScrollPanel(bound, "stack:", (Rectangle){}, &scroll, &view);
	BeginScissorMode(view.x, view.y, view.width, view.height);
	EndScissorMode();
}

void Performance(const Rectangle bound) {
	static Vector2 scroll;
	static Rectangle view;
	static float value = 0;

	if (value < 100)
		value++;
	GuiScrollPanel(bound, "performance:", bound, &scroll, &view);
	BeginScissorMode(view.x, view.y, view.width, view.height);
		GuiProgressBar({bound.x, bound.y + 40, 100, 20},	NULL, NULL, &value, 0, 100);
	EndScissorMode();
}


void FileExplorer(t_node_file *workspace_files, const Rectangle bound) {	
	static Vector2 scroll;
	static Rectangle view;

	int max_size;
	for (int i = 0; i < workspace_files->files.count;i++ && max_size++) {
		max_size += workspace_files->child[workspace_files->files.paths[i]].files.count;
	}

	GuiScrollPanel(bound, "Workspace", (Rectangle){0, 0, 180, (float)50 + workspace_files->files.count + max_size * 20}, &scroll, &view);
	BeginScissorMode(view.x, view.y, view.width, view.height);
		for (int i = 0; i < workspace_files->files.count; i++) {
			if (workspace_files->child[workspace_files->files.paths[i]].isDirectory) {
				DrawText(GetFileName(workspace_files->child[workspace_files->files.paths[i]].path), 20 + scroll.x, 50 + 20 * i * workspace_files->child[workspace_files->files.paths[i]].files.count + scroll.y, ctx.font_size, RAYWHITE);
			} else {
				DrawText(GetFileName(workspace_files->child[workspace_files->files.paths[i]].path), 20 + scroll.x, 50 + 20 * i * workspace_files->child[workspace_files->files.paths[i]].files.count + scroll.y, ctx.font_size, WHITE);
			}
			t_node_file *node;
			node = &workspace_files->child[workspace_files->files.paths[i]];
			for (int k = 0; k < node->files.count; k++) {
				if (node->child[node->files.paths[k]].isDirectory) {
					DrawText(GetFileName(node->child[node->files.paths[k]].path), 40 + scroll.x, 50 + 20 * i + k + scroll.y, ctx.font_size, RAYWHITE);
				} else {
					DrawText(GetFileName(node->child[node->files.paths[k]].path), 40 + scroll.x, 50 + 20 * i + k + scroll.y, ctx.font_size, WHITE);
				}
			}
		}
	EndScissorMode();
}

int View(t_node_file *workspace_files){
	static float sep1 = 160;
	static float sep2 = 300;
	static float sep3 = 300;
	static float sep4 = 160;
	//static Rectangle file_bound; = (Rectangle){0, 20, 200, 300 - 20};
	//static Rectangle text_bound;
	//static Rectangle lsp_bound;
	//static Rectangle stack_bound;
	//static Rectangle pref_bound;
	float height, width;


	height = GetScreenHeight();
	width = GetScreenWidth();

	if (IsWindowResized()) {
		sep2 = GetScreenHeight() - 160;
	}

	BeginDrawing();
	ClearBackground(BLACK);
		FileExplorer(workspace_files, (Rectangle){0, 20, sep1, sep2 - 20});
	
		TextEditor((Rectangle){sep1, 20, width - sep1 - sep4, sep2 - 20});
	
		LanguageServer((Rectangle){0, sep2, sep3, height - sep2});
		StackViewer((Rectangle){sep3, sep2, width - sep3 - sep4, height - sep2});
		Performance((Rectangle){width - sep4, 20, width - sep4, height - 20});
		ControlBar();
	EndDrawing();
	return (1);
}

int main(void) {
	//t_file_header openfile;
	int monitor_count = 0;
	int step = 0;
	t_node_file workspace;
	ctx.font_size = 8;

	InitWindow(200, 200, "HavenIde");

	monitor_count = GetMonitorCount();

	t_monitor *displays = (t_monitor *)malloc(monitor_count * sizeof(t_monitor));
	if (!displays) {
		printf("ABORT()\n");
		abort();
	}
	for (int i = 0; i < monitor_count; i++) {
		displays[i].refresh_rate = GetMonitorRefreshRate(i);
		displays[i].width = GetMonitorWidth(i);
		displays[i].height = GetMonitorHeight(i);
		displays[i].id = i;
		printf("refresh rate: %i, width: %i, height: %i, id: %i\n", displays[i].refresh_rate, displays[i].width, displays[i].height, i);
	}

	GuiLoadStyle(TextFormat("%s/../include/styles/terminal/style_terminal.rgs", GetApplicationDirectory()));

	ctx.file.dim = VecFileToGlyph("ylt.txt");
	ctx.file.name = "ylt.txt";
	printf("data size: %llu", ctx.file.size);

	workspace = loadWorkspace();

	SetTargetFPS(30);
	//EnableEventWaiting();
	while (!WindowShouldClose()) {
		switch (step) {
			case (start): {
				BeginDrawing();
					ClearBackground(BLACK);
					for (int i = 0; i < monitor_count; i++) {
						if (GuiButton((Rectangle){20 , (float)20 + 20 * i, 160, 16}, \
							TextFormat("%iHz %ix%i id: %i", \
								displays[i].refresh_rate, displays[i].width, displays[i].height, displays[i].id))) {
							SetWindowMonitor(displays[i].id);
							SetWindowMaxSize(displays[i].width, displays[i].height);
							SetWindowMinSize(720, 480);
							SetWindowSize(720, 480);
							SetWindowState(FLAG_WINDOW_RESIZABLE);
							const Vector2 pos = Vector2Add(GetMonitorPosition(displays[i].id), (Vector2){(float)((displays[i].width - 720) * 0.5), (float)((displays[i].height - 480) * 0.5)});
							SetWindowPosition(pos.x, pos.y);
							step = stdview;
							free (displays);
							displays = 0x00;
							break;
						}
					}
				EndDrawing();
				break;
			}
			case (stdview): {
				step = View(&workspace);
				break;
			}
			default:break;
		}
	}
	CloseWindow();
	return (0);
}
