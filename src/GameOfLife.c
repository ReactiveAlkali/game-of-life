#include "CellularAutomaton.h"
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#define MENU_WIDTH 25
#define MENU_HEIGHT 10

static char controls_msg[] = "F1 Exit   F2 Toggle Menu   ";
static char input_controls[] = "ARROWS Move   SPACE Cycle State   ENTER Start Automaton";

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
}

void
get_user_state (WINDOW *life_win, Automaton *automaton, int key)
{
  int height, width;
  int y, x;
  getmaxyx(life_win, height, width);
  getyx(life_win, y, x);
  
  switch (key)
    {
    case KEY_UP:
      if (y > 0)
        wmove(life_win, --y, x);
      break;
    case KEY_DOWN:
      if (y < height - 1)
        wmove(life_win, ++y, x);
      break;
    case KEY_RIGHT:
      if (x < width - 1)
        wmove(life_win, y, ++x);
      break;
    case KEY_LEFT:
      if (x > 0)
        wmove(life_win, y, --x);
      break;
    case ' ':
      automaton_cycle_state(automaton, y - height / 2, x - width / 2);
    }
  render_automaton(life_win, automaton);
  wmove(life_win, y, x);
}

void
print_basic_controls ()
{
  clear();
  printw(controls_msg);
  refresh();
}

int
main ()
{
  WINDOW *life_win;
  WINDOW *menu_win;
  bool get_input = false;

  // initialize menu struct
  menu.curr_choice       = 0;
  menu.is_automaton_menu = true;
  menu.is_open           = true;
  menu.num_choices       = num_automaton_choices;
  
  // initialization curses
  srand(time(NULL));
  initscr();
  cbreak();
  curs_set(0);
  keypad(stdscr, true);
  timeout(-1);

  /* user controls */
  printw(controls_msg);
  refresh();

  Automaton *life = automaton_create(game_of_life, (LINES - 1) * 2, COLS * 2);

  /* Initialize our windows */
  life_win = newwin(LINES - 1, COLS, 1, 0);
  menu_win = newwin(MENU_HEIGHT, MENU_WIDTH, LINES / 2 - MENU_HEIGHT / 2,
                    COLS / 2 - MENU_WIDTH / 2);
  
  /* Used a do while so that the menu gets displayed before asking for input */
  int key;
  do
    { 
      switch (key)
        {
        case KEY_F(2): /* Toggle the menu on/off */
          menu.is_open = !menu.is_open;
          if (menu.is_open)
            {
              timeout(-1);
              curs_set(0);
            }
          else
            {
              timeout(1000);
              if (get_input)
                curs_set(1);
            }
          break;
        case '\n':
          /* Handle whatever the current choice on the menu is */
          if (menu.is_open)
            {
              get_input = false;
              if (menu.is_automaton_menu)
                automaton_set_type(life, menu.curr_choice);
              else
                {
                  switch (menu.curr_choice)
                    {
                    case 0:/* generate a soup */
                      automaton_random_state(life);
                      timeout(1000);
                      print_basic_controls();
                      break;
                    case 1:/* load from a text file */
                      print_basic_controls();
                      break;
                    case 2:/* user input state */
                      automaton_dead_state(life);
                      printw("%s%s", controls_msg, input_controls);
                      refresh();
                      curs_set(1);
                      wmove(life_win, LINES / 2, COLS / 2);
                      get_input = true;
                      break;
                    }
                  update_menu(key);
                }
            }
          else if (get_input)
            {
              get_input = false;
              curs_set(0);
              timeout(1000);
              print_basic_controls();
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
          if (!get_input)
            {
              automaton_update_state(life);
              render_automaton(life_win, life);
            }
          else
            get_user_state(life_win, life, key);
          wrefresh(life_win);
        }
    }
  while ((key = getch()) != KEY_F(1));
  
  automaton_destroy(life);
  endwin();
  return 0;
}
