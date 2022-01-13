#include "CellularAutomaton.h"
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

void
render_automaton (WINDOW *win, Automaton *automaton)
{
  int height, width;
  getmaxyx(win, height, width);
  wclear(win);

  for (int row = 0; row < height; row++)
    {
      for (int col = 0; col < width; col++)
        {
          int state = automaton_get_state(automaton, row - height / 2,
                                          col - width / 2);
          if (state)
            {
              attrset((state == 1) ? A_NORMAL : A_DIM);
              mvwaddch(win, row, col, '#');
            }
        }
    }

  wrefresh(win);
}

WINDOW *
display_quit_msg (WINDOW *win)
{
  const char *msg = "Press F1 to quit";

  if (!win)
    win = newwin(1, strlen(msg), 0, 0);

  wclear(win);
  waddstr(win, msg);
  wrefresh(win);

  return win;
}
  
  

int
main ()
{
  // initialization
  srand(time(NULL));
  initscr();
  curs_set(0);
  halfdelay(10);
  keypad(stdscr, true);
  
  printw("F1 to exit");
  refresh();

  Automaton *life          = automaton_create(seeds, (LINES - 1) * 2, COLS * 2, NULL);
  WINDOW *automaton_window = newwin(LINES - 1, COLS, 1, 0);

  int key;
  while (life && (key = getch()) != KEY_F(1))
    {
      if (key == KEY_RESIZE)
        {
          delwin(automaton_window);
          automaton_window = newwin(LINES - 1, COLS, 1, 0);
        }
      render_automaton(automaton_window, life);
      automaton_update_state(life);
    }
  
  automaton_destroy(life);
  endwin();
  return 0;
}
