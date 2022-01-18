#include "CellularAutomaton.h"
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#define MENU_WIDTH 25
#define MENU_HEIGHT 10

/* Indices of this array should match the automaton type enum */
static char *automaton_choices[] = {
  "Conway's Game of Life",
  "Seeds",
  "Greenberg-Hastings",
  "Highlife",
  "Day and Night",
  "Brian's Brain"
};
static const int num_automaton_choices = 6;

static char *state_choices[] = {
  "Random State",
  "Load From File",
  "Custom State"
};
static const int num_state_choices = 3;

/* A structure for storing the current state of our menu */
struct Menu
{
  bool is_automaton_menu;
  bool is_open;
  int curr_choice;
  int num_choices;
} menu;

void
update_menu (int key)
{
  switch (key)
    {
    case KEY_UP:
      --menu.curr_choice;
      if (menu.curr_choice < 0)
        menu.curr_choice = menu.num_choices - 1;
      break;
    case KEY_DOWN:
      ++menu.curr_choice;
      if (menu.curr_choice >= menu.num_choices)
        menu.curr_choice = 0;
      break;
    case 10:
      menu.is_automaton_menu = !menu.is_automaton_menu;
      menu.curr_choice       = 0;
      menu.num_choices       = (menu.is_automaton_menu ? num_automaton_choices
                                : num_state_choices);
      /* Close the menu after we've choosen a starting state */
      if (menu.is_automaton_menu)
        menu.is_open = false;
    }
}

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
              wattrset(win, (state == 1) ? A_NORMAL : A_DIM);
              mvwaddch(win, row, col, '#');
            }
        }
    }
  wrefresh(win);
}

void
print_menu (WINDOW *win)
{
  int x = 2;
  int y = 2;
  wclear(win);
  box(win, 0, 0);
  for (int i = 0; i < menu.num_choices; ++i)
    {
      if (menu.curr_choice == i)
        {
          wattron(win, A_REVERSE);
          mvwprintw(win, y, x, "%s", menu.is_automaton_menu
                    ? automaton_choices[i] : state_choices[i]);
          wattroff(win, A_REVERSE);
        }
      else
        mvwprintw(win, y, x, "%s", menu.is_automaton_menu ? automaton_choices[i]
                  : state_choices[i]);
      ++y;
    }
  wrefresh(win);
}

int
main ()
{
  WINDOW *life_win;
  WINDOW *menu_win;

  // initialize menu struct
  menu.curr_choice       = 0;
  menu.is_automaton_menu = true;
  menu.is_open           = true;
  menu.num_choices       = num_automaton_choices;
  
  // initialization
  srand(time(NULL));
  initscr();
  cbreak();
  curs_set(0);
  keypad(stdscr, true);
  timeout(-1);
  
  printw("F1 to exit   F2 to toggle menu");
  refresh();

  Automaton *life = automaton_create(game_of_life, (LINES - 1) * 2, COLS * 2);
  
  life_win = newwin(LINES - 1, COLS, 1, 0);
  menu_win = newwin(MENU_HEIGHT, MENU_WIDTH, LINES / 2 - MENU_HEIGHT / 2,
                    COLS / 2 - MENU_WIDTH / 2);

  int key;
  do
    {
      switch (key)
        {
        case KEY_F(2): /* Toggle the menu on/off */
          menu.is_open = !menu.is_open;
          if (menu.is_open)
            timeout(-1);
          else
            timeout(1000);
          break;
        case '\n':
          if (menu.is_automaton_menu)
            automaton_set_type(life, menu.curr_choice);
          else
            {
              switch (menu.curr_choice)
                {
                case 0:
                  automaton_random_state(life);
                  break;
                }
              timeout(1000);
            }
          break;
        }

      if (menu.is_open)
        {
          update_menu(key);
          print_menu(menu_win);
        }  

      // The menu may have just been closed
      if (!menu.is_open)
        {
          render_automaton(life_win, life);
          automaton_update_state(life);
        }
    }
  while ((key = getch()) != KEY_F(1));
  
  automaton_destroy(life);
  endwin();
  return 0;
}
