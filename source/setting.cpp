#include "main.h"
#include <raygui.h>

static std::unordered_map<int, std::string> theme {
	{0, "ashes"},
	{1, "bluish"},
	{2, "candy"},
	{3, "cherry"},
	{4, "cyber"},
	{5, "dark"},
	{6, "enefete"},
	{7, "jungle"},
	{8, "lavanda"},
	{9, "sunny"},
	{10, "terminal"}
};

extern t_context ctx;

void renderSetting(t_workspace *workspace, bool *active) {
	static int active_theme;
	static bool theme_open = false;
	
	if (GuiWindowBox({0, 20, (float)GetScreenWidth(), (float)GetScreenHeight() - 20}, "setting")) *active ^= *active;
	
	if (GuiButton({0, 60, 20, 20},"-")) workspace->fontsize--;
	DrawRectangle(20, 60, 60, 20, WHITE);
	DrawTextEx(ctx.font, TextFormat("%i", workspace->fontsize), {30, 62}, 16, 0, BLACK);
	if (GuiButton({80, 60, 20, 20},"+")) workspace->fontsize++;

	GuiColorPicker({100, 100, 100, 100}, "cursor color:", &workspace->cursor_style.color);
	DrawTextEx(ctx.font, "Theme:", {0, 40}, workspace->fontsize, 0, WHITE);
	if (GuiDropdownBox({120, 40, 100, 20}, "ashes;bluish;candy;cherry;cyber;dark;enefete;jungle;lavanda;sunny;terminal", &active_theme, theme_open)) {
		theme_open = !theme_open;
		if (!theme_open && workspace->theme != theme[active_theme]) {
			workspace->theme = theme[active_theme];
			GuiLoadStyle(TextFormat("%s/assets/styles/style_%s.rgs", GetApplicationDirectory(), workspace->theme.c_str()));
			GuiSetFont(ctx.font);
		}
	}

/*
theme:terminal,
font:firacode,
fontsize:16,
history_size:100,
cursor_style:3,
cursor_color:0.228.48.255,
*/
}
