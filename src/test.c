#include <ncurses.h>
#include <menu.h>
#include <stdlib.h>
#include <string.h>
#include <form.h>

int
main()
{
	WINDOW* input_win;
	WINDOW* dialog_win;
	WINDOW* dialog_pad;
	char input_str[150];

	initscr();
	cbreak();
	keypad(stdscr, TRUE);
	refresh();

	dialog_win = newwin(LINES - 3, COLS - 1, 0, 0);
	box(dialog_win, 0, 0);
	wrefresh(dialog_win);

	input_win = newwin(3, COLS - 1, LINES - 3, 0);
	box(input_win, 0, 0);
	wrefresh(input_win);

	dialog_pad = newpad(LINES + 5, COLS - 2);
	prefresh(dialog_pad, 0, 0, 1, 1, 10, 30);

	int y = LINES - 6;
	int yy = 0;
	while (1) {
		mvwgetstr(input_win, 1, 1, input_str);
		if (strcmp(input_str, "q") == 0)
			break;
		werase(input_win);
		box(input_win, 0, 0);
		wrefresh(input_win);

		werase(dialog_win);
		box(dialog_win, 0, 0);
		wrefresh(dialog_win);

		wprintw(dialog_pad, "%s\n", input_str);
		prefresh(dialog_pad, yy, 0, y, 1, LINES - 5, COLS - 3);
		if (y  == 1) {
			y++;
			yy++;
		} else {
			y--;
		}
	}

	delwin(input_win);
	delwin(dialog_win);
	delwin(dialog_pad);
	endwin();
	return 0;
}
