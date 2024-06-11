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
# include <stb/stb_c_lexer.h>
# include <sstream>

struct context {
	t_workspace workspace;
	u32 current_file = 0;
} ctx;

//need to find why it segfault sometime
t_file_header *loadFileRW(const char *filepath) {
	assert(filepath);
	t_file_header *new_file = (t_file_header *)malloc(sizeof(t_file_header));
	assert(new_file);
	new_file->name = filepath;
	new_file->raw = readFile(new_file->name.c_str());
	assert(!new_file->raw.empty());

	std::stringstream stream(new_file->raw);
	std::string line = {};
	while(std::getline(stream, line)) {
		std::list<t_glyph *> lst = {};
		for (auto c : line) {
			t_glyph *glyph = (t_glyph*)malloc(sizeof(t_glyph));
			assert(glyph);
			glyph->c = c;
			glyph->fg = WHITE;
			glyph->bg = BLACK;
			lst.push_back(glyph);
		}
		t_glyph *glyph = (t_glyph*)malloc(sizeof(t_glyph));
		if (lst.size() > new_file->dim.x) {
			new_file->dim.x = new_file->glyphs.size();
		}
		new_file->glyphs.push_back(lst);
		new_file->dim.y++;
	}
	return (new_file);
};

t_glyph *createGlyph(const char c, Color fg) {
	t_glyph *glyph = new t_glyph;
	assert(glyph);
	glyph->c = c;
	glyph->fg = fg;
	return (glyph);
}

void splitPath(std::string &from, std::vector<std::string> &paths) {
	std::string span;
	std::stringstream stream(from.c_str());

	while (std::getline(stream, span)) {
		paths.push_back(span);
	}
}

t_workspace loadWorkspace(const char *workspace_filepath){
	t_workspace workspace;
	std::unordered_map<std::string, workspace_token_e> dictionnary = {
		{"paths", path_tok},
		{"theme", theme_tok},
		{"font", font_tok},
		{"fontsize", fontsize_tok},
	};

	std::string data = readFile(workspace_filepath);

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
			default:break;
		}
	}
	for (auto file : workspace.paths) {
		assert(!file.empty());
		workspace.files.push_back(loadFileRW(file.c_str()));
	}
	return (workspace);
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
	writeFile(file.name.c_str(), stream.c_str(), stream.size());
	file.raw.clear();
	file.raw = stream;
}

void TextEditor(const Rectangle bound, t_workspace *workspace) {
	static Vector2 scroll = {};
	static Rectangle view = {};
	static Vector2 cursor = {};
	static int start_line = 0;
	static bool is_saved = true;

	if (CheckCollisionPointRec(GetMousePosition(), bound)) {
		if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_S) && !is_saved) {
			saveTheFile(*workspace->files[ctx.current_file]);
			is_saved = true;
		}

		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
			Vector2 mouse_pos = GetMousePosition();
			cursor.x = floor((mouse_pos.x - 40 - bound.x - scroll.x) / (workspace->fontsize));
			cursor.y = floor((mouse_pos.y - bound.y - 30 - scroll.y) / (workspace->fontsize * 1.5));
			cursor.y = clamp(cursor.y, 0, workspace->files[ctx.current_file]->dim.y - 1);
			cursor.x = clamp(cursor.x, 0, workspace->files[ctx.current_file]->glyphs[cursor.y].size());
		}
		if (IsKeyDown(KEY_LEFT)) {
			if (cursor.x > 0) {
				cursor.x--;
			} else if (cursor.y > 0){
				cursor.y--;
				cursor.x = workspace->files[ctx.current_file]->glyphs[cursor.y].size();
			}
		}
		if (IsKeyDown(KEY_RIGHT)) {
			if (cursor.x < workspace->files[ctx.current_file]->glyphs[cursor.y].size()) {
				cursor.x++;
			} else if (cursor.y < workspace->files[ctx.current_file]->dim.y - 1) {
				cursor.y++;
				cursor.x = 0;
			}
		}
		if (IsKeyDown(KEY_UP) && cursor.y > 0) {
			cursor.y--;
		}
		if (IsKeyDown(KEY_DOWN) && cursor.y < workspace->files[ctx.current_file]->dim.y - 1) {
			cursor.y++;
		}

		auto insert_place = workspace->files[ctx.current_file]->glyphs[cursor.y].begin();
		std::advance(insert_place, cursor.x);

		if (IsKeyPressed(KEY_BACKSPACE)) {
			if (cursor.x) {
				cursor.x--;
				insert_place = workspace->files[ctx.current_file]->glyphs[cursor.y].erase(--insert_place);
				if (is_saved)
					is_saved = false;
			} else if (cursor.y >= 1 && workspace->files[ctx.current_file]->glyphs[cursor.y - 1].empty()) {
				workspace->files[ctx.current_file]->glyphs.erase(workspace->files[ctx.current_file]->glyphs.begin() + cursor.y - 1);
				workspace->files[ctx.current_file]->dim.y--;
				cursor.y--;
				cursor.x = 0;
				if (is_saved)
					is_saved = false;
			} else if (cursor.y >= 1) {
				workspace->files[ctx.current_file]->dim.y--;
				cursor.x = workspace->files[ctx.current_file]->glyphs[cursor.y - 1].size();
				cursor.y--;
				workspace->files[ctx.current_file]->glyphs[cursor.y].splice(workspace->files[ctx.current_file]->glyphs[cursor.y].end(), workspace->files[ctx.current_file]->glyphs[cursor.y + 1]);
				workspace->files[ctx.current_file]->glyphs.erase(workspace->files[ctx.current_file]->glyphs.begin() + cursor.y + 1);
				if (is_saved)
					is_saved = false;
			}
		}
		if (IsKeyDown(KEY_ENTER)) {
			std::list<t_glyph *> lst = {};
			lst.splice(lst.begin(), workspace->files[ctx.current_file]->glyphs[cursor.y], insert_place++, workspace->files[ctx.current_file]->glyphs[cursor.y].end());
			cursor.y++;
			workspace->files[ctx.current_file]->glyphs.insert(workspace->files[ctx.current_file]->glyphs.begin() + cursor.y, lst);
			cursor.x = 0;
			workspace->files[ctx.current_file]->dim.y++;
			insert_place = workspace->files[ctx.current_file]->glyphs[cursor.y].begin();
			if (is_saved)
				is_saved = false;
		}
		if (IsKeyDown(KEY_TAB)) {
			workspace->files[ctx.current_file]->glyphs[cursor.y].emplace(insert_place++, createGlyph('\t', WHITE));
			cursor.x++;
			if (is_saved)
				is_saved = false;
		}
		
		int key = GetCharPressed();
		while (key) {
			if ((key >= 32) && (key <= 125)) {
				workspace->files[ctx.current_file]->glyphs[cursor.y].emplace(insert_place++, createGlyph((char)key, WHITE));
				cursor.x++;
				if (is_saved)
					is_saved = false;
			}
			key = GetCharPressed();
		}
	}

	GuiScrollPanel(bound, workspace->files[ctx.current_file]->name.c_str(), (Rectangle){0, 0, (workspace->files[ctx.current_file]->dim.x + 5) * workspace->fontsize + 30, (float)30+(workspace->files[ctx.current_file]->dim.y * (float)(workspace->fontsize * 1.5))}, &scroll, &view);

	BeginScissorMode(view.x, view. y, view. width, view. height);


	start_line = 0 - scroll.y / (workspace->fontsize * 1.5);
	GuiDrawRectangle((Rectangle){bound.x + 1, view.y + 1, 30, view.height - 1}, 1, WHITE, BLACK);
	for (int y = start_line; y < start_line + 200 && y < workspace->files[ctx.current_file]->dim.y; y++) {
		std::list<t_glyph *> &line = workspace->files[ctx.current_file]->glyphs[y];
		if (!line.empty()) {
			int x = 0;
			for (auto tmp : line) {
				char character[2] = {tmp->c, '\0'};
				DrawText(character, 40 + bound.x + scroll.x + x * workspace->fontsize, bound.y + 30 + scroll.y + y * (workspace->fontsize * 1.5), workspace->fontsize, tmp->fg);
				x++;
			}
		}
		DrawText(TextFormat(" %5i ", y + 1), bound.x, bound.y + 30 + scroll.y + y * (workspace->fontsize * 1.5), workspace->fontsize, WHITE);
	}
	DrawRectangleLines(40 + bound.x + scroll.x + cursor.x * workspace->fontsize, bound.y + 30 + scroll.y + cursor.y * (workspace->fontsize * 1.5) + workspace->fontsize * 0.5, workspace->fontsize, workspace->fontsize * 0.5, RED);
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

	GuiScrollPanel(bound, "stdErr:", (Rectangle){}, &scroll, &view);
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


void TerminalOut(t_workspace *workspace, const Rectangle bound) {	
	static Vector2 scroll;
	static Rectangle view;

	GuiScrollPanel(bound, "Terminal:", (Rectangle){0, 0, 180, (float)50 + workspace->paths.size() * (float)(workspace->fontsize * 1.5)}, &scroll, &view);
	BeginScissorMode(view.x, view.y, view.width, view.height);
	EndScissorMode();
}

int View(t_workspace *workspace){
	static float sep1 = 200;
	static float sep2 = 300;
	static float sep3 = 300;
	float height, width;

	height = GetScreenHeight();
	width = GetScreenWidth();

	if (IsWindowResized()) {
		sep2 = GetScreenHeight() - 160;
	}

	BeginDrawing();
	ClearBackground(BLACK);
		TerminalOut(workspace, (Rectangle){0, 20, sep1, sep2 - 20});
	
		TextEditor((Rectangle){sep1, 20, width - sep1, sep2 - 20}, workspace);
	
		LanguageServer((Rectangle){0, sep2, sep3, height - sep2});
		StackViewer((Rectangle){sep3, sep2, width - sep3, height - sep2});
		ControlBar();
	EndDrawing();
	return (1);
}

int main(int ac, char **av) {
	//t_file_header openfile;
	int monitor_count = 0;
	int step = 0;
	t_workspace workspace;

	InitWindow(400, 400, "HavenIde");

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

	if (ac <= 1) {
		workspace = loadWorkspace("default.workspace");
	} else {
		workspace = loadWorkspace(av[1]);
	}

	SetTargetFPS(240);
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
							SetWindowSize(720 * 0.5, 480 * 0.5);
							SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_UNDECORATED);
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
