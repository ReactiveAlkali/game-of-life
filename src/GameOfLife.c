#include "CellularAutomaton.h"
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#define MENU_WIDTH 25
#define MENU_HEIGHT 10
#define NUM_AUTOMATONS 6
#define STATE_CHOICES 3

/* Indices of this array should match the automaton type enum */
char *automaton_choices[] = {
  "Conway's Game of Life",
  "Seeds",
  "Greenberg-Hastings",
  "Highlife",
  "Day and Night",
  "Brian's Brain"
};

char *state_choices[] = {
  "Random State",
  "Load From File",
  "Custom State"
};

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

void
print_menu (WINDOW *win, int highlight, int num_choices, bool state_menu)
{
  int x = 2;
  int y = 2;
  wclear(win);
  box(win, 0, 0);
  for (int i = 0; i < num_choices; ++i)
    {
      if (highlight == i)
        {
          wattron(win, A_REVERSE);
          mvwprintw(win, y, x, "%s", state_menu ? state_choices[i] :
                    automaton_choices[i]);
          wattroff(win, A_REVERSE);
        }
      else
        mvwprintw(win, y, x, "%s", state_menu ? state_choices[i] :
                  automaton_choices[i]);
      ++y;
    }
  wrefresh(win);
}

int
main ()
{
  WINDOW *life_win;
  WINDOW *menu_win;
  bool menu_open  = true;
  bool state_menu = false;
  int num_choices = NUM_AUTOMATONS;
  int highlight   = 0;
  int choice      = -1;
  
  // initialization
  srand(time(NULL));
  initscr();
  cbreak();
  curs_set(0);
  keypad(stdscr, true);
  timeout(0);
  
  printw("F1 to exit   F2 to toggle menu");
  refresh();

  Automaton *life = automaton_create(game_of_life, (LINES - 1) * 2, COLS * 2);
  automaton_random_state(life);
  
  life_win = newwin(LINES - 1, COLS, 1, 0);
  menu_win = newwin(MENU_HEIGHT, MENU_WIDTH, LINES / 2 - MENU_HEIGHT / 2,
                    COLS / 2 - MENU_WIDTH / 2);

  int key;
  while ((key = getch()) != KEY_F(1))
    {
      switch (key)
        {
        case KEY_F(2):
          menu_open   = !menu_open;
          num_choices = NUM_AUTOMATONS;
          state_menu  = false;
          choice      = -1;
          break;
        case KEY_UP:
          if (highlight == 0)
            highlight = num_choices - 1;
          else
            --highlight;
          break;
        case KEY_DOWN:
          if (highlight == num_choices - 1)
            highlight = 0;
          else
            ++highlight;
          break;
        case '\n':
          choice      = highlight;
          highlight   = 0;
          state_menu  = !state_menu;
          num_choices = state_menu ? STATE_CHOICES : NUM_AUTOMATONS;
          break;
        }

      if (menu_open)
        {
          print_menu(menu_win, highlight, num_choices, state_menu);
          timeout(-1);
          if (choice > 0)
            {
              if (state_menu)
                {
                  automaton_destroy(life);
                  life = automaton_create(choice, LINES * 2, COLS * 2);
                }
            }
        }
      else
        {
          render_automaton(life_win, life);
          automaton_update_state(life);
          timeout(1000);
        }
    }
  
  automaton_destroy(life);
  endwin();
  return 0;
}
