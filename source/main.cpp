# include "main.h"
# define GLSL_VERSION 330
# include <raylib.h>
# define RAYGUI_IMPLEMENTATION
# include <raygui.h>
# include <raymath.h>
# include <stdlib.h>
# include <stdio.h>
# include <haven_logic.h>
# include <fstream>

struct context {
	int cursor_pos;
	int font_size;
	t_file_header file;
} ctx;

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

void TextEditor(t_file_header *file, const Rectangle bound) {
	static int cursor_pos = 0;
	static Vector2 scroll = {};
	static Rectangle view = {};
	int counter = 0;

	//if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
	//	cursor_pos = GetMousePosition();
	//}

	for (int i = 0; i < ctx.file.buffer.str().size(); i++) {
		if (ctx.file.buffer.str()[i] == '\n') {
			counter++;
		}
	}
	//if \n then draw number of line after
	//count number of line
	// get cursor with row/ column idx and if no text exit, then go to last


	//GuiScrollPanel(bound, file->name, (Rectangle){0, 0, 100, 100}, &scroll, &view);
	GuiScrollPanel(bound, "text editor:", (Rectangle){0, 0, 1000, 10000}, &scroll, &view);
	BeginScissorMode(view.x, view. y, view. width, view. height);
		for (int i = 0; i <= counter; i++) {
			DrawText(TextFormat(" %5i ", i + 1), bound.x, bound.y + 20 + ctx.font_size * i * 1.5 + scroll.y, ctx.font_size, WHITE);
		}
		DrawText(ctx.file.buffer.str().c_str(), 40 + bound.x + scroll.x, bound.y + 20 + scroll.y, ctx.font_size, BLUE);
	EndScissorMode();
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


void FileExplorer(FilePathList *workspace_files, const Rectangle bound) {	
	static Vector2 scroll;
	static Rectangle view;

	GuiScrollPanel(bound, "Workspace", (Rectangle){0, 0, 180, (float)50 + workspace_files->count * 20}, &scroll, &view);
	BeginScissorMode(view.x, view.y, view.width, view.height);
		for (int i = 0; i < workspace_files->count; i++) {
			DrawText(GetFileName(workspace_files->paths[i]), 20 + scroll.x, 50 + 20 * i + scroll.y, ctx.font_size, WHITE);
		}
	EndScissorMode();
}

int View(FilePathList *workspace_files){
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
		FileExplorer(workspace_files, (Rectangle){0, 20, sep1, sep2});
		TextEditor(NULL, (Rectangle){sep1, 20, width - sep1 - sep4, sep2});
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
	FilePathList workspace_files;
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
	workspace_files = LoadDirectoryFiles(GetWorkingDirectory());

	GuiLoadStyle(TextFormat("%s/../include/styles/terminal/style_terminal.rgs", GetApplicationDirectory()));

	std::fstream file("Makefile");

	if (!file.is_open()) {
		perror("could not load file");
		abort();
	}

	ctx.file.buffer << file.rdbuf();
	file.close();
	ctx.file.size = ctx.file.buffer.str().size();
	printf("%s\n", ctx.file.buffer.str().c_str());

	SetTargetFPS(30);
	EnableEventWaiting();
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
				step = View(&workspace_files);
				break;
			}
			default:break;
		}
	}
	UnloadDirectoryFiles(workspace_files);
	CloseWindow();
	return (0);
}
