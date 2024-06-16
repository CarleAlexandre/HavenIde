#include "main.h"
#include <cassert>
#define GLSL_VERSION 330
#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <raymath.h>
#include <stdlib.h>
#include <haven_logic.h>
#include <stb/stb_c_lexer.h>
#include <haven_thread.hpp>
#include <stb/stb_truetype.h>
#include <stb/stb_textedit.h>

t_context ctx;

std::unordered_map<vi_mod, const char *> vi_mods_dictionnary {
	{normal, "normal"},
	{insert, "insert"},
	{visual, "visual"},
	{visual_block, "visual_block"},
	{insert_block, "insert_block"},
};

void saveTheFile(t_file_header *file) {
	file->is_saved = true;
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
	auto data = LoadUTF8(file->codepoints, file->codepoint_size);
	SaveFileText(file->name, data);
	UnloadUTF8(data);
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
		ctx.setting_open = true;
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
			GuiButton({0, 20, 60, 20}, "+ file");
			GuiButton({0, 40, 60, 20}, "- file");
			if (GuiButton({0, 60, 60, 20}, "save")) saveTheFile(workspace->files[ctx.current_file]);
			if (GuiButton({0, 80, 60, 20}, "save all")) {
				for (int i = 0; i < workspace->files.size(); i++) {
					saveTheFile(workspace->files[i]);
				}
			}
			if (GuiButton({0, 100, 60, 20}, "close")) return(-1);

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

void TextEditor(const Rectangle bound, t_workspace *workspace) {
	static Vector2 scroll = {};
	static Rectangle view = {};
	static int start_line = 0;
	static double time = 0;
	static float incr = 0;
	t_cursor *cursor;
	
	cursor = &workspace->files[ctx.current_file]->cursor;

	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), bound)) {
		Vector2 mouse_pos = GetMousePosition();
		cursor->pos.x = 0;
		cursor->pos.y = floor((mouse_pos.y - bound.y - 30 - scroll.y) / (workspace->fontsize * 1.5));
		cursor->pos.y = clamp(cursor->pos.y, 0, workspace->files[ctx.current_file]->glyphs.size() - 1);
	}
	start_line = 0 - scroll.y / (workspace->fontsize * 1.5);
	int max_line = start_line + (int)floor(bound.height / (workspace->fontsize * 1.5)) + 1;
	int max_width = 0;
	for (int i = start_line; i < max_line && i <  workspace->files[ctx.current_file]->glyphs.size(); i++){
		auto tmp = workspace->files[ctx.current_file]->glyphs[i].size();
		if (tmp > max_width) max_width = tmp;
	}
	GuiScrollPanel(bound, workspace->files[ctx.current_file]->name, (Rectangle){0, 0, (max_width + 2) * (float)(workspace->fontsize * 0.5) + 60, (float)30+(workspace->files[ctx.current_file]->glyphs.size() * (float)(workspace->fontsize * 1.5))}, &scroll, &view);
	BeginScissorMode(view.x, view. y, view. width, view. height);
	int sort = 0;
	for (int y = start_line; y < max_line && y < workspace->files[ctx.current_file]->glyphs.size(); y++) {
		std::list<t_glyph *> &line = workspace->files[ctx.current_file]->glyphs[y];
		int x = 0;
		int glyphx = 0;
		int position_x = 0;
		if (!line.empty()) {
			for (auto tmp : line) {
				Vector2 pos = {60 + bound.x + scroll.x + glyphx * (float)(workspace->fontsize * 0.5), bound.y + 30 + scroll.y + y * (float)(workspace->fontsize * 1.5)};
				if (cursor->pos.y == y && cursor->pos.x == x) {
					sort = position_x;
				}
				switch (tmp->codepoint) {
					case ('\n'): {
						position_x ++;
						break;
					}
					case ('\t'): {
						int mod = tmp->spacing - glyphx % tmp->spacing;
						for (int space = 0; space < mod; ++space) {
							pos.x = (60 + bound.x + scroll.x + glyphx * (float)(workspace->fontsize * 0.5));
							DrawTextCodepoint(ctx.font, 32, pos, workspace->fontsize, tmp->fg);
							glyphx++;
						}
						position_x += mod + 1;
						break;
					}
					case ('\0'): {
						break;
					}
					default: {
						DrawTextCodepoint(ctx.font, tmp->codepoint, pos, workspace->fontsize, tmp->fg);
						position_x ++;
						break;
					}
				}
				glyphx++;
				x++;
			}
			if (cursor->pos.y == y && cursor->pos.x == x) {
				sort = position_x;
			}
		}
	}
	cursor->render_pos.x = 60 + bound.x + scroll.x + sort * (workspace->fontsize * 0.5);
	cursor->render_pos.y = bound.y + 30 + scroll.y + cursor->pos.y * (workspace->fontsize * 1.5);
	GuiDrawRectangle((Rectangle){bound.x + 1, view.y + 1, 50, view.height - 1}, 1, WHITE, BLACK);
	for (int y = start_line; y < max_line && y < workspace->files[ctx.current_file]->glyphs.size(); y++) {
		DrawTextEx(ctx.font, TextFormat(" %5i ", y + 1), {bound.x, bound.y + 30 + scroll.y + y * (float)(workspace->fontsize * 1.5)}, workspace->fontsize, 0, WHITE);
	}
	if (!GetKeyPressed()) {
		time += GetFrameTime();
		if (cursor->alpha >= 1 ) incr = -0.02;
		if (cursor->alpha <= 0) incr = 0.02;
		if (time >= 0.02) {
			cursor->alpha += incr;
			time = 0;
		}
	} else {
		cursor->alpha = 1;
		time = -1;
	}
	switch (workspace->cursor_style.style) {
		case (box_cursor): {
			DrawRectangleLines(cursor->render_pos.x, cursor->render_pos.y, workspace->fontsize * 0.5, workspace->fontsize, ColorAlpha(workspace->cursor_style.color, cursor->alpha));
			break;
		}
		case (half_box_cursor): {
			DrawRectangleLines(cursor->render_pos.x, cursor->render_pos.y + workspace->fontsize * 0.5, workspace->fontsize * 0.5, workspace->fontsize * 0.5, ColorAlpha(workspace->cursor_style.color, cursor->alpha));
			break;
		}
		case (underscore_cursor): {
			DrawRectangle(cursor->render_pos.x, cursor->render_pos.y + workspace->fontsize, workspace->fontsize * 0.5, 2, ColorAlpha(workspace->cursor_style.color, cursor->alpha));
			break;
		}
		case (pipe_cursor): {
			DrawLine(cursor->render_pos.x, cursor->render_pos.y, cursor->render_pos.x, cursor->render_pos.y + workspace->fontsize, ColorAlpha(workspace->cursor_style.color, cursor->alpha));
			break;
		}
		default:break;
	};
	EndScissorMode();
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

void editorInput(t_workspace *workspace, const double delta_time) {
	static double time = 0;
	if (CheckCollisionPointRec(GetMousePosition(), ctx.texteditor_bound)) {
		t_file_header *current = workspace->files[ctx.current_file];
		auto insert_place = current->glyphs[current->cursor.pos.y].begin();
		std::advance(insert_place, current->cursor.pos.x);
		if (IsKeyDown(KEY_LEFT_CONTROL)){
			if (IsKeyPressed(KEY_S) && !current->is_saved) {
				saveTheFile(current);
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
			if (IsKeyDown(KEY_LEFT) || (IsKeyDown(KEY_H) && ctx.mode == normal && !ctx.term.open)) {
				time += delta_time;
				if (time >= INPUT_TIME * 0.5) {
					if (current->cursor.pos.x > 0) {
						current->cursor.pos.x--;
					} else if (current->cursor.pos.y > 0){
						current->cursor.pos.y--;
						current->cursor.pos.x = current->glyphs[current->cursor.pos.y].size() - 1;
					}
					time = 0;
				}
			}
			if (IsKeyDown(KEY_RIGHT) || (IsKeyDown(KEY_L) && ctx.mode == normal && !ctx.term.open)) {
				time += delta_time;
				if (time >= INPUT_TIME * 0.5) {
					if (current->cursor.pos.x < current->glyphs[current->cursor.pos.y].size() - 1) {
						current->cursor.pos.x++;
					} else if (current->cursor.pos.y < current->glyphs.size() - 1) {
						current->cursor.pos.y++;
						current->cursor.pos.x = 0;
					}
					time = 0;
				}
			}
			if ((IsKeyDown(KEY_UP) || (IsKeyDown(KEY_J) && ctx.mode == normal && !ctx.term.open)) && current->cursor.pos.y > 0) {
				time += delta_time;
				if (time >= INPUT_TIME* 0.5) {
					current->cursor.pos.y--;
					current->cursor.pos.x = clamp(current->cursor.pos.x, 0, current->glyphs[current->cursor.pos.y].size() - 1);
					time = 0;
				}
			}
			if ((IsKeyDown(KEY_DOWN) || (IsKeyDown(KEY_K) && ctx.mode == normal && !ctx.term.open)) && current->cursor.pos.y < current->glyphs.size() - 1) {
				time += delta_time;
				if (time >= INPUT_TIME* 0.5) {
					current->cursor.pos.y++;
					current->cursor.pos.x = clamp(current->cursor.pos.x, 0, current->glyphs[current->cursor.pos.y].size() - 1);
					time = 0;
				}
			}
			if (ctx.mode == normal) {
				if (IsKeyPressed(KEY_I) && !ctx.term.open) {
					ctx.mode = insert;
					return;
				}
			}
			if (ctx.mode == insert) {
				if (IsKeyPressed(KEY_ESCAPE))
					ctx.mode = normal;
			}

			if (ctx.mode == insert) {
				if (IsKeyDown(KEY_BACKSPACE)) {
					time += delta_time;
					if (time >= INPUT_TIME) {
						if (current->cursor.pos.x) {
							current->cursor.pos.x--;
							insert_place = current->glyphs[current->cursor.pos.y].erase(--insert_place);
							if (current->is_saved)
								current->is_saved = false;
						} else if (current->cursor.pos.y >= 1 && current->glyphs[current->cursor.pos.y - 1].empty()) {
							current->glyphs.erase(current->glyphs.begin() + current->cursor.pos.y - 1);
							current->cursor.pos.y--;
							current->cursor.pos.x = 0;
							if (current->is_saved)
								current->is_saved = false;
						} else if (current->cursor.pos.y >= 1) {
							current->cursor.pos.x = current->glyphs[current->cursor.pos.y - 1].size();
							current->cursor.pos.y--;
							current->glyphs[current->cursor.pos.y].splice(current->glyphs[current->cursor.pos.y].end(), current->glyphs[current->cursor.pos.y + 1]);
							current->glyphs.erase(current->glyphs.begin() + current->cursor.pos.y + 1);
							if (current->is_saved)
								current->is_saved = false;
						}
						time = 0;
					}
				}
				if (IsKeyDown(KEY_ENTER)) {
					time += delta_time;
					if (time >= INPUT_TIME) {
						std::list<t_glyph *> lst = {};
						lst.splice(lst.cbegin(), current->glyphs[current->cursor.pos.y], current->glyphs[current->cursor.pos.y].cbegin(), insert_place);
						lst.emplace_back(createGlyph('\n', WHITE));
						current->glyphs.insert(current->glyphs.cbegin() + current->cursor.pos.y, lst);
						current->cursor.pos.y++;
						current->cursor.pos.x = 0;
						insert_place = current->glyphs[current->cursor.pos.y].begin();
						if (current->is_saved)
							current->is_saved = false;
						time = 0;
					}
				}
				if (IsKeyDown(KEY_TAB)) {
					time += delta_time;
					if (time >= INPUT_TIME) {
						current->glyphs[current->cursor.pos.y].emplace(insert_place++, createGlyph('\t', WHITE));
						current->cursor.pos.x++;
						if (current->is_saved)
							current->is_saved = false;
						time = 0;
					}
				}
				if (IsKeyPressed(KEY_BACKSPACE)) {
					time = 0;
					if (current->cursor.pos.x) {
						current->cursor.pos.x--;
						insert_place = current->glyphs[current->cursor.pos.y].erase(--insert_place);
						if (current->is_saved)
							current->is_saved = false;
					} else if (current->cursor.pos.y >= 1 && current->glyphs[current->cursor.pos.y - 1].empty()) {
						current->glyphs.erase(current->glyphs.begin() + current->cursor.pos.y - 1);
						current->cursor.pos.y--;
						current->cursor.pos.x = 0;
						if (current->is_saved)
							current->is_saved = false;
					} else if (current->cursor.pos.y >= 1) {
						current->cursor.pos.x = current->glyphs[current->cursor.pos.y - 1].size();
						current->cursor.pos.y--;
						current->glyphs[current->cursor.pos.y].splice(current->glyphs[current->cursor.pos.y].end(), current->glyphs[current->cursor.pos.y + 1]);
						current->glyphs.erase(current->glyphs.begin() + current->cursor.pos.y + 1);
						if (current->is_saved)
							current->is_saved = false;
					}
				}
				if (IsKeyPressed(KEY_ENTER)) {
					time = 0;
					std::list<t_glyph *> lst = {};
					lst.splice(lst.cbegin(), current->glyphs[current->cursor.pos.y], current->glyphs[current->cursor.pos.y].cbegin(), insert_place);
					lst.emplace_back(createGlyph('\n', WHITE));
					current->glyphs.insert(current->glyphs.cbegin() + current->cursor.pos.y, lst);
					current->cursor.pos.y++;
					current->cursor.pos.x = 0;
					insert_place = current->glyphs[current->cursor.pos.y].begin();
					if (current->is_saved)
						current->is_saved = false;
				}
				if (IsKeyPressed(KEY_TAB)) {
					time = 0;
					current->glyphs[current->cursor.pos.y].emplace(insert_place++, createGlyph('\t', WHITE));
					current->cursor.pos.x++;
					if (current->is_saved)
						current->is_saved = false;
				}
				int key = GetCharPressed();
				while (key) {
					if ((key >= 32) && (key <= 125)) {
						current->glyphs[current->cursor.pos.y].emplace(insert_place++, createGlyph(key, WHITE));
						current->cursor.pos.x++;
						if (current->is_saved)
							current->is_saved = false;
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
			for (;;) {
				if (ctx.term.out.front() == '\r' || ctx.term.out.front() == '\n') {
					ctx.term.fOut.push_back(str);
					str.clear();
					while (!ctx.term.out.empty() && (ctx.term.out.front() == '\n' || ctx.term.out.front() == '\r')) {
						ctx.term.out.pop();
					}
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

//void updateTextColor(t_file_header *file) {
//	switch (extension_dictionnary[GetFileExtension(file->name)]) {
//		case (c_ext):{
//			stb_lexer lexer;
//			for (int i = 0; i < file->glyphs.size(); i++) {
//				stb_c_lexer_init(&lexer, file->glyphs[i], const char *input_stream_end, char *string_store, int store_length);
//				stb_c_lexer_get_token(stb_lexer *lexer);
//			}
//			break;
//		}
//		case (cpp_ext):{break;}
//		case (markdown_ext):{break;}
//		default:break;
//	}
//}
void ContextBar(const Rectangle bound, t_workspace *workspace) {
	auto cursor = workspace->files[ctx.current_file]->cursor;
	bool saved =  workspace->files[ctx.current_file]->is_saved ? true : false;
	const char *save = saved ? "saved" : "not saved";
	int width = GetTextWidth(save);
	switch (ctx.mode) {
		case (normal): {
			GuiDrawRectangle(bound, 2, GREEN, ColorAlpha(GREEN, 0.1));
			DrawTextEx(ctx.font, TextFormat("x:%.0f,y:%.0f | size: %i | normal", cursor.pos.x, cursor.pos.y, workspace->fontsize), {bound.x + 5, bound.y + 3}, 14, 0, GREEN);
			DrawTextEx(ctx.font, save, {bound.x + bound.width - (5 + width), bound.y + 3}, 14, 0, saved ? GREEN : RED);
			break;
		}
		case (insert): {
			GuiDrawRectangle(bound, 2, BLUE, ColorAlpha(GREEN, 0.1));
			DrawTextEx(ctx.font, TextFormat("x:%.0f,y:%.0f | size: %i | insert", cursor.pos.x, cursor.pos.y, workspace->fontsize), {bound.x + 5, bound.y + 3}, 14, 0, BLUE);
			DrawTextEx(ctx.font, save, {bound.x + bound.width - (5 + width), bound.y + 3}, 14, 0, saved ? GREEN : RED);
			break;
		}
		case (visual): {
			GuiDrawRectangle(bound, 2, YELLOW, ColorAlpha(GREEN, 0.1));
			DrawTextEx(ctx.font, TextFormat("x:%.0f,y:%.0f | size: %i | visual", cursor.pos.x, cursor.pos.y, workspace->fontsize), {bound.x + 5, bound.y + 3}, 14, 0, YELLOW);
			DrawTextEx(ctx.font, save, {bound.x + bound.width - (5 + width), bound.y + 3}, 14, 0, saved ? GREEN : RED);
			break;
		}
		case (insert_block): {
			GuiDrawRectangle(bound, 2, DARKBLUE, ColorAlpha(GREEN, 0.1));
			DrawTextEx(ctx.font, TextFormat("x:%.0f,y:%.0f | size: %i | insert_block", cursor.pos.x, cursor.pos.y, workspace->fontsize), {bound.x + 5, bound.y + 3}, 14, 0, DARKBLUE);
			DrawTextEx(ctx.font, save, {bound.x + bound.width - (5 + width), bound.y + 3}, 14, 0, saved ? GREEN : RED);
			break;
		}
		case (visual_block): {
			GuiDrawRectangle(bound, 2, BROWN, ColorAlpha(GREEN, 0.1));
			DrawTextEx(ctx.font, TextFormat("x:%.0f,y:%.0f | size: %i | visual_block", cursor.pos.x, cursor.pos.y, workspace->fontsize), {bound.x + 5, bound.y + 3}, 14, 0, BROWN);
			DrawTextEx(ctx.font, save, {bound.x + bound.width - (5 + width), bound.y + 3}, 14, 0, saved ? GREEN : RED);
			break;
		}
		default:break;
	}
}

int View(t_workspace *workspace){
	static float sep1 = 400;
	float height, width;
	int ret = 1;
	//static double parse_time = 0;

	double delta_time = GetFrameTime();

	//parse_time += delta_time;

	height = GetScreenHeight();
	width = GetScreenWidth();

	editorInput(workspace, delta_time);
	//if (parse_time >= 5) {
	//	updateTextColor(workspace->files[ctx.current_file]);
	//}

	ctx.terminal_bound = (Rectangle){0, 20, sep1, height - 20};
	ctx.texteditor_bound = (Rectangle){sep1, 20, width - sep1, height - 40};

	BeginDrawing();
	ClearBackground(BLACK);
		TerminalOut(workspace, ctx.terminal_bound);
		TextEditor(ctx.texteditor_bound, workspace);
		if (ctx.term.open == true) {
			TerminalIn(workspace, {sep1, 20, width - sep1 - 30, 20});
		}
		ret = ControlBar(workspace);
		if (ctx.setting_open) {
			renderSetting(workspace, &ctx.setting_open);
		}
		ContextBar({ctx.terminal_bound.width, ctx.texteditor_bound.height + 20, ctx.texteditor_bound.width, 20}, workspace);
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
	const char * appdir = GetApplicationDirectory();
	workspace = loadWorkspace(TextFormat("%s/workspace/default.workspace", appdir));
	GuiLoadStyle(TextFormat("%s/assets/styles/style_%s.rgs", appdir, workspace.theme.c_str()));
	ctx.font = LoadFontEx(TextFormat("%s/assets/font/%s.ttf", appdir, workspace.font.c_str()), 32, NULL, INT16_MAX);
	SetTextureFilter(ctx.font.texture, TEXTURE_FILTER_TRILINEAR);
	GuiSetFont(ctx.font);
	SetTargetFPS(120);
	bool shouldClose = false;
	while (!shouldClose) {
		switch (step) {
			case (start): {
				BeginDrawing();
					ClearBackground(BLACK);
					for (int i = 0; i < monitor_count; i++) {
						if (GuiButton((Rectangle){50 , (float)20 + 32 * i, 300, 30}, \
							TextFormat("%iHz %ix%i id: %i", \
								displays[i].refresh_rate, displays[i].width, displays[i].height, displays[i].id))) {
							SetWindowMonitor(displays[i].id);
							SetWindowMaxSize(displays[i].width, displays[i].height);
							SetWindowMinSize(720, 480);
							SetWindowSize(1600, 1000);
							SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_UNDECORATED);
							Vector2 pos = GetMonitorPosition(i);
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
