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
# include <haven_file.hpp>
# include <iostream>

struct context {
	int cursor_pos;
	int font_size;
	t_file_header file;
} ctx;

Vector2 FileToGlyph(t_node **list, const char *filepath) {
	Vector2 pos = {0};
	Vector2 dim;

	char *data = readFile(filepath);

	for(int i = 0; data[i]; i++) {
		t_glyph *glyph = 0x00;
		glyph = (t_glyph *)malloc(sizeof(t_glyph));
		if (!glyph) {
			perror("couldn't allocat memory");
			abort();
		}
		glyph->c = data[i];
		glyph->pos = pos;
		if (data[i] == '\n') {
			pos.y++;
			pos.x = 0;
		} else {
			pos.x++;
		}
		if (dim.x < pos.x) {
			dim.x = pos.x;
		}
		if (dim.y < pos.y) {
			dim.y = pos.y;
		}
		addNodeBack(list, createNode(glyph));
	}
	free(data);
	return (dim);
};

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

void TextEditor(const Rectangle bound) {
	static int cursor_pos = 0;
	static Vector2 scroll = {};
	static Rectangle view = {};
	int i = 0;

	//if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
	//	cursor_pos = GetMousePosition();
	//}
	//if \n then draw number of line after
	//count number of line
	// get cursor with row/ column idx and if no text exit, then go to last


	//GuiScrollPanel(bound, file->name, (Rectangle){0, 0, 100, 100}, &scroll, &view);
	GuiScrollPanel(bound, "text editor:", (Rectangle){0, 0, (ctx.file.dim.x + 5) * ctx.font_size + 30, (float)(30 + (ctx.file.dim.y + 2) * ctx.font_size * 1.5)}, &scroll, &view);
	BeginScissorMode(view.x, view. y, view. width, view. height);
	t_node *tmp = *ctx.file.list;
	DrawText(TextFormat(" %5i ", i + 1), bound.x, bound.y + 30 + ctx.font_size * i * 1.5 + scroll.y, ctx.	font_size, WHITE);
	while (((t_glyph*)tmp->data)->c && tmp->next) {
		if (((t_glyph*)tmp->data)->c == '\n') {
			i++;
		}
		char character[2] = {((t_glyph*)tmp->data)->c, '\0'};
		DrawText(character, 40 + bound.x + scroll.x + ((t_glyph*)tmp->data)->pos.x * ctx.font_size, bound.y + 30 + scroll.y + ((t_glyph*)tmp->data)->pos.y * (ctx.font_size * 1.5), ctx.font_size, BLUE);
		tmp = tmp->next;
	}
	GuiDrawRectangle((Rectangle){bound.x + 1, view.y + 1, 30, view.height - 1}, 1, WHITE, BLACK);
	for (int k = 0; k <= i; k++) {
		DrawText(TextFormat(" %5i ", k + 1), bound.x, bound.y + 30 + ctx.font_size * k * 1.5 + scroll.y, ctx.	font_size, WHITE);
	}
	DrawText(TextFormat(" %5i ", i + 2), bound.x, bound.y + 30 + ctx.font_size * (i + 1) * 1.5 + scroll.y, ctx.	font_size, WHITE);
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

	std::fstream file("Makefile");

	if (!file.is_open()) {
		perror("could not load file");
		abort();
	}

	ctx.file.list = (t_node **)malloc(sizeof(t_node *));
	*ctx.file.list = 0x00;
	std::cout << ctx.file.list << "; " << ctx.file.list << ";\n";
	ctx.file.dim = FileToGlyph(ctx.file.list, "Makefile");

	workspace = loadWorkspace();

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
				step = View(&workspace);
				break;
			}
			default:break;
		}
	}
	CloseWindow();
	return (0);
}
