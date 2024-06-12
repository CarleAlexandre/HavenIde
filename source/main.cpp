# include "main.h"
#include <cassert>
# define GLSL_VERSION 330
# include <raylib.h>
# define RAYGUI_IMPLEMENTATION
# include <raygui.h>
# include <raymath.h>
# include <stdlib.h>
# include <haven_logic.h>
# include <stb/stb_c_lexer.h>
# include <sstream>
# include <haven_thread.hpp>

struct context {
	int current_file = 0;
	Rectangle terminal_bound;
	Rectangle texteditor_bound;
	vi_mod mode;
	t_terminal term;
	Font font;
} ctx;

t_glyph *createGlyph(char c, Color fg, Color bg) {
	t_glyph *glyph;

	glyph = (t_glyph *)malloc(sizeof(t_glyph));
	glyph->c = c;
	glyph->fg = fg;
	glyph->bg = bg;
	return (glyph);
}

std::list<t_glyph *> loadGlyphLine(char *data, float *x, int *count) {
	std::list<t_glyph *> lst;
	int i = 0;

	for (; data[i] && data[i] != '\n';i++) {
		lst.push_back(createGlyph(data[i], WHITE, BLACK));
		if (i > *x) *x = i;
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

	char *span = data;
	for (int i = 0; span[i]; i++) {
		new_file->glyphs.push_back(loadGlyphLine(&span[i], &new_file->dim.x, &i));
		new_file->dim.y++;
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

t_workspace loadWorkspace(const char *workspace_filepath){
	t_workspace workspace;
	std::unordered_map<std::string, workspace_token_e> dictionnary = {
		{"paths", path_tok},
		{"theme", theme_tok},
		{"font", font_tok},
		{"fontsize", fontsize_tok},
		{"history_size", history_size_tok},
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
			default:break;
		}
	}
	for (auto file : workspace.paths) {
		auto tmp = loadFileRW(file.c_str());
		assert(tmp);
		workspace.files.push_back(tmp);
	}
	free(data);
	return (workspace);
}

int ControlBar(t_workspace *workspace) {
	static int status_bar_show = 0;
	int scwidth;

	scwidth = GetScreenWidth();

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

	int width = 0;
	for (int x = 0; x < workspace->files.size(); x++) {
		if (x == ctx.current_file) {
			GuiButton({180 + (float)(x * 20), 0, 120, 20}, workspace->files[x]->name);
			width = 100;
		} else {
			if (GuiButton({180 + (float)(x * 20) + width, 0, 20, 20}, TextFormat("%i", x))) ctx.current_file = x;
		}
	}

	if (GuiButton({(float)scwidth - 20, 0, 20, 20}, "X")) return(-1);
	if (GuiButton({(float)scwidth - 40, 0, 20, 20}, "||")) IsWindowMaximized() ? RestoreWindow() : MaximizeWindow();
	if (GuiButton({(float)scwidth - 60, 0, 20, 20}, ".")) IsWindowMinimized() ? RestoreWindow() : MinimizeWindow();
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
			//load_file
			//save_file
			//edit_workspace
			//

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
	return (1);
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
	SaveFileText(file.name, (char *)stream.c_str());
	stream.clear();
}

void TextEditor(const Rectangle bound, t_workspace *workspace) {
	static Vector2 scroll = {};
	static Rectangle view = {};
	static int start_line = 0;

	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), bound)) {
		Vector2 mouse_pos = GetMousePosition();
		workspace->files[ctx.current_file]->cursor.x = floor((mouse_pos.x - 40 - ctx.texteditor_bound.x - scroll.x) / (workspace->fontsize));
		workspace->files[ctx.current_file]->cursor.y = floor((mouse_pos.y - bound.y - 30 - scroll.y) / (workspace->fontsize * 1.5));
		workspace->files[ctx.current_file]->cursor.y = clamp(workspace->files[ctx.current_file]->cursor.y, 0, workspace->files[ctx.current_file]->dim.y - 1);
		workspace->files[ctx.current_file]->cursor.x = clamp(workspace->files[ctx.current_file]->cursor.x, 0, workspace->files[ctx.current_file]->glyphs[workspace->files[ctx.current_file]->cursor.y].size());
	}

	GuiScrollPanel(bound, workspace->files[ctx.current_file]->name, (Rectangle){0, 0, (workspace->files[ctx.current_file]->dim.x + 5) * (float)(workspace->fontsize * 0.5) + 60, (float)30+(workspace->files[ctx.current_file]->dim.y * (float)(workspace->fontsize * 1.5))}, &scroll, &view);
	start_line = 0 - scroll.y / (workspace->fontsize * 1.5);
	BeginScissorMode(view.x, view. y, view. width, view. height);
	GuiDrawRectangle((Rectangle){bound.x + 1, view.y + 1, 50, view.height - 1}, 1, WHITE, BLACK);
	for (int y = start_line; y < start_line + 200 && y < workspace->files[ctx.current_file]->dim.y; y++) {
		std::list<t_glyph *> &line = workspace->files[ctx.current_file]->glyphs[y];
		if (!line.empty()) {
			int x = 0;
			for (auto tmp : line) {
				char character[2] = {tmp->c, '\0'};
				DrawTextEx(ctx.font, character, {60 + bound.x + scroll.x + x * (float)(workspace->fontsize * 0.5), bound.y + 30 + scroll.y + y * (float)(workspace->fontsize * 1.5)}, workspace->fontsize, 0, tmp->fg);
				x++;
			}
		}
		DrawTextEx(ctx.font, TextFormat(" %5i ", y + 1), {bound.x, bound.y + 30 + scroll.y + y * (float)(workspace->fontsize * 1.5)}, workspace->fontsize, 0, WHITE);
	}
	DrawRectangleLines(60 + bound.x + scroll.x + workspace->files[ctx.current_file]->cursor.x * (workspace->fontsize * 0.5), bound.y + 30 + scroll.y + workspace->files[ctx.current_file]->cursor.y * (workspace->fontsize * 1.5), workspace->fontsize * 0.5, workspace->fontsize, GREEN);
	EndScissorMode();
	if (workspace->files[ctx.current_file]->is_saved) {
		DrawCircle(bound.width + bound.x - 7, bound.height + bound.y - 9, 4, BLUE);
	} else {
		DrawCircle(bound.width + bound.x - 7, bound.height + bound.y - 9, 4, RED);
	}
}

void TerminalOut(t_workspace *workspace, const Rectangle bound) {	
	static Vector2 scroll;
	static Rectangle view;

	int maxX = 0;
	for (auto str : ctx.term.fOut) {
		if (maxX < str.size()) {
			maxX = str.size();
		}
	}
	GuiScrollPanel(bound, "Terminal:", (Rectangle){0, 0, (float)maxX * (workspace->fontsize) + 20, (float)(ctx.term.fOut.size() * (workspace->fontsize * 1.5) + 40)}, &scroll, &view);
	BeginScissorMode(view.x, view.y, view.width, view.height);
	int y = 0;
	for (auto str : ctx.term.fOut) {
		DrawTextEx(ctx.font, str.c_str(), {bound.x + scroll.x + 10, bound.y + y * (float)(workspace->fontsize * 1.5) + scroll.y + 40}, workspace->fontsize, 0, GREEN);
		y++;
	}
	EndScissorMode();
}

void editorInput(t_workspace *workspace) {
	if (CheckCollisionPointRec(GetMousePosition(), ctx.texteditor_bound)) {
		auto insert_place = workspace->files[ctx.current_file]->glyphs[workspace->files[ctx.current_file]->cursor.y].begin();
		std::advance(insert_place, workspace->files[ctx.current_file]->cursor.x);
		if (IsKeyDown(KEY_LEFT_CONTROL)){
			if (IsKeyPressed(KEY_S) && !workspace->files[ctx.current_file]->is_saved) {
				saveTheFile(*workspace->files[ctx.current_file]);
				workspace->files[ctx.current_file]->is_saved = true;
			}
			if ((IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_H)) && ctx.current_file) {
				ctx.current_file--;
			}
			if ((IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_L)) && ctx.current_file < workspace->files.size() - 1) {
				ctx.current_file++;
			}
			if (IsKeyPressed(KEY_P)) {
				ctx.mode = normal;
				ctx.term.open = true;
			}
		} else {
			if (IsKeyPressed(KEY_LEFT) || (IsKeyPressed(KEY_H) && ctx.mode == normal && !ctx.term.open)) {
				if (workspace->files[ctx.current_file]->cursor.x > 0) {
					workspace->files[ctx.current_file]->cursor.x--;
				} else if (workspace->files[ctx.current_file]->cursor.y > 0){
					workspace->files[ctx.current_file]->cursor.y--;
					workspace->files[ctx.current_file]->cursor.x = workspace->files[ctx.current_file]->glyphs[workspace->files[ctx.current_file]->cursor.y].size();
				}
			}
			if (IsKeyPressed(KEY_RIGHT) || (IsKeyPressed(KEY_L) && ctx.mode == normal && !ctx.term.open)) {
				if (workspace->files[ctx.current_file]->cursor.x < workspace->files[ctx.current_file]->glyphs[workspace->files[ctx.current_file]->cursor.y].size()) {
					workspace->files[ctx.current_file]->cursor.x++;
				} else if (workspace->files[ctx.current_file]->cursor.y < workspace->files[ctx.current_file]->dim.y - 1) {
					workspace->files[ctx.current_file]->cursor.y++;
					workspace->files[ctx.current_file]->cursor.x = 0;
				}
			}
			if ((IsKeyPressed(KEY_UP) || (IsKeyPressed(KEY_J) && ctx.mode == normal && !ctx.term.open)) && workspace->files[ctx.current_file]->cursor.y > 0) {
				workspace->files[ctx.current_file]->cursor.y--;
				workspace->files[ctx.current_file]->cursor.x = clamp(workspace->files[ctx.current_file]->cursor.x, 0, workspace->files[ctx.current_file]->glyphs[workspace->files[ctx.current_file]->cursor.y].size());
			}
			if ((IsKeyPressed(KEY_DOWN) || (IsKeyPressed(KEY_K) && ctx.mode == normal && !ctx.term.open)) && workspace->files[ctx.current_file]->cursor.y < workspace->files[ctx.current_file]->dim.y - 1) {
				workspace->files[ctx.current_file]->cursor.y++;
				workspace->files[ctx.current_file]->cursor.x = clamp(workspace->files[ctx.current_file]->cursor.x, 0, workspace->files[ctx.current_file]->glyphs[workspace->files[ctx.current_file]->cursor.y].size());
			}
			if (ctx.mode == normal) {
				if (IsKeyPressed(KEY_I) && !ctx.term.open) {
					ctx.mode = insert;
					GetKeyPressed();
				}
			}
			if (ctx.mode == insert) {
				if (IsKeyPressed(KEY_ESCAPE))
					ctx.mode = normal;
			}

			if (ctx.mode == insert) {
				if (IsKeyPressed(KEY_BACKSPACE)) {
					if (workspace->files[ctx.current_file]->cursor.x) {
						workspace->files[ctx.current_file]->cursor.x--;
						insert_place = workspace->files[ctx.current_file]->glyphs[workspace->files[ctx.current_file]->cursor.y].erase(--insert_place);
						if (workspace->files[ctx.current_file]->is_saved)
							workspace->files[ctx.current_file]->is_saved = false;
					} else if (workspace->files[ctx.current_file]->cursor.y >= 1 && workspace->files[ctx.current_file]->glyphs[workspace->files[ctx.current_file]->cursor.y - 1].empty()) {
						workspace->files[ctx.current_file]->glyphs.erase(workspace->files[ctx.current_file]->glyphs.begin() + workspace->files[ctx.current_file]->cursor.y - 1);
						workspace->files[ctx.current_file]->dim.y--;
						workspace->files[ctx.current_file]->cursor.y--;
						workspace->files[ctx.current_file]->cursor.x = 0;
						if (workspace->files[ctx.current_file]->is_saved)
							workspace->files[ctx.current_file]->is_saved = false;
					} else if (workspace->files[ctx.current_file]->cursor.y >= 1) {
						workspace->files[ctx.current_file]->dim.y--;
						workspace->files[ctx.current_file]->cursor.x = workspace->files[ctx.current_file]->glyphs[workspace->files[ctx.current_file]->cursor.y - 1].size();
						workspace->files[ctx.current_file]->cursor.y--;
						workspace->files[ctx.current_file]->glyphs[workspace->files[ctx.current_file]->cursor.y].splice(workspace->files[ctx.current_file]->glyphs[workspace->files[ctx.current_file]->cursor.y].end(), workspace->files[ctx.current_file]->glyphs[workspace->files[ctx.current_file]->cursor.y + 1]);
						workspace->files[ctx.current_file]->glyphs.erase(workspace->files[ctx.current_file]->glyphs.begin() + workspace->files[ctx.current_file]->cursor.y + 1);
						if (workspace->files[ctx.current_file]->is_saved)
							workspace->files[ctx.current_file]->is_saved = false;
					}
				}
				if (IsKeyPressed(KEY_ENTER)) {
					std::list<t_glyph *> lst = {};
					lst.splice(lst.begin(), workspace->files[ctx.current_file]->glyphs[workspace->files[ctx.current_file]->cursor.y], insert_place++, workspace->files[ctx.current_file]->glyphs[workspace->files[ctx.current_file]->cursor.y].end());
					workspace->files[ctx.current_file]->cursor.y++;
					workspace->files[ctx.current_file]->glyphs.insert(workspace->files[ctx.current_file]->glyphs.begin() + workspace->files[ctx.current_file]->cursor.y, lst);
					workspace->files[ctx.current_file]->cursor.x = 0;
					workspace->files[ctx.current_file]->dim.y++;
					insert_place = workspace->files[ctx.current_file]->glyphs[workspace->files[ctx.current_file]->cursor.y].begin();
					if (workspace->files[ctx.current_file]->is_saved)
						workspace->files[ctx.current_file]->is_saved = false;
				}
				if (IsKeyPressed(KEY_TAB)) {
					workspace->files[ctx.current_file]->glyphs[workspace->files[ctx.current_file]->cursor.y].emplace(insert_place++, createGlyph('\t', WHITE, BLACK));
					workspace->files[ctx.current_file]->cursor.x++;
					if (workspace->files[ctx.current_file]->is_saved)
						workspace->files[ctx.current_file]->is_saved = false;
				}
				int key = GetCharPressed();
				while (key) {
					if ((key >= 32) && (key <= 125)) {
						workspace->files[ctx.current_file]->glyphs[workspace->files[ctx.current_file]->cursor.y].emplace(insert_place++, createGlyph((char)key, WHITE, BLACK));
						workspace->files[ctx.current_file]->cursor.x++;
						if (workspace->files[ctx.current_file]->is_saved)
							workspace->files[ctx.current_file]->is_saved = false;
					}
					key = GetCharPressed();
				}
			}
		}
	}
}

void TerminalIn(t_workspace *workspace, const Rectangle bound) {
	if (GuiTextBox(bound, ctx.term.in, 100, true)) {
		ctx.term.open = false;
		if (execCmd(ctx.term.in, ctx.term.out, 4096)) {
			std::string str;
			for (;!ctx.term.out.empty();) {
				if (ctx.term.out.front() == '\n') {
					ctx.term.fOut.push_back(str);
					str.clear();
					ctx.term.out.pop();
					if (ctx.term.out.empty()) break;
				}
				str += ctx.term.out.front();
				ctx.term.out.pop();
			}
			memset(ctx.term.in, 0, 100);
			return;
		}
		ctx.term.fOut.push_back(std::string(TextFormat("Unknow Command: %s", ctx.term.in)));
	}
}

int View(t_workspace *workspace){
	static float sep1 = 400;
	float height, width;
	int ret = 1;

	height = GetScreenHeight();
	width = GetScreenWidth();

	editorInput(workspace);

	ctx.terminal_bound = (Rectangle){0, 20, sep1, height - 20};
	ctx.texteditor_bound = (Rectangle){sep1, 20, width - sep1, height - 20};

	BeginDrawing();
	ClearBackground(BLACK);
		TerminalOut(workspace, ctx.terminal_bound);
		TextEditor(ctx.texteditor_bound, workspace);
		if (ctx.term.open == true) {
			TerminalIn(workspace, {sep1, 20, width - sep1 - 30, 20});
		}
		ret = ControlBar(workspace);
	EndDrawing();
	return (ret);
}

int main(int ac, char **av) {
	int monitor_count = 0;
	int step = 0;
	t_workspace workspace;
	std::atomic_bool join_sync;

	join_sync.store(false);

	startThreadPool(&join_sync);

	InitWindow(400, 400, "HavenIde");
	SetExitKey(0);

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

	if (ac <= 1) {
		workspace = loadWorkspace("default.workspace");
	} else {
		workspace = loadWorkspace(av[1]);
	}

	GuiLoadStyle(TextFormat("include/styles/%s/style_%s.rgs", workspace.theme.c_str(), workspace.theme.c_str()));

	ctx.font = LoadFont(TextFormat("assets/font/%s.ttf", workspace.font.c_str()));
	GuiSetFont(ctx.font);

	SetTargetFPS(120);

	bool shouldClose = false;
	while (!shouldClose) {
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
							SetWindowSize(1600, 1000);
							SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_UNDECORATED);
							SetWindowPosition(0, 0);
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
			case (close): {
				shouldClose = true;
				break;
			}
			default:break;
		}
		if (WindowShouldClose()) {
			shouldClose = true;
		}
	}
	UnloadFont(ctx.font);
	CloseWindow();
	endThreadPool(&join_sync);
	return (0);
}
