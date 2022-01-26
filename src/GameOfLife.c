#include "CellularAutomaton.h"
#include "String.h"
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#define MENU_WIDTH 25
#define MENU_HEIGHT 10

static char controls_msg[] = "F1 Exit   F2 Toggle Menu   ";
static char input_controls[] = "ARROWS Move   SPACE Cycle State   ENTER Start Automaton";

static Automaton *life;
static String *input_buffer;

/* State variables of our program */
static bool collecting_input = false;
static bool loading_file     = false;

/* The windows of our program */
static WINDOW *life_win;
static WINDOW *menu_win;
static WINDOW *input_win;

/*
 * FILE IO
 */

void
print_file_prompt ()
{
  wclear(input_win);
  box(input_win, 0, 0);
  wattrset(input_win, A_STANDOUT);
  mvwprintw(input_win, 1, COLS / 2 - 9, "ENTER FILE PATH");
  wattrset(input_win, A_NORMAL);
  mvwprintw(input_win, 2, 2, input_buffer->s);
  wrefresh(input_win);
}

void
print_invalid_file ()
{
  wclear(input_win);
  box(input_win, 0, 0);
  wattrset(input_win, A_STANDOUT);
  mvwprintw(input_win, 1, COLS / 2 - 6, "INVALID FILE");
  wattrset(input_win, A_NORMAL);
  wrefresh(input_win);
}

bool
load_file_state ()
{
  bool success = true;
  int height;
  int width;
  FILE *fp;

  fp = fopen(input_buffer->s, "r");
  if (!fp)
    {
      success = false;
      goto done;
    }

  /* first line of the file should contain the height followed by the width */
  int rv = fscanf(fp, "%d %d\n", &height, &width);
  if (rv != 2)
    {
      success = false;
      goto err;
    }
  
  int c = fgetc(fp);
  for (int line = 0; line < height && c != EOF && success; ++line)
    {
      for (int col = 0; col < width && c != EOF && c != '\n' && success; ++col)
        {
          switch (c)
            {
            case '0':
              success = automaton_set_state(life, line - height / 2,
                                            col - width / 2, 1);
              break;
            case '-':
              success = automaton_set_state(life, line - height / 2,
                                            col - width / 2, 2);
              break;
            }
          c = fgetc(fp);
        }
      c = fgetc(fp);
    }

 err:
  fclose(fp);
 done:
  return success;
}

/*
 * PRINTING TO SCREEN
 */

void
print_basic_controls ()
{
  clear();
  printw(controls_msg);
  refresh();
}

void
render_automaton (Automaton *automaton)
{
  int height, width;
  getmaxyx(life_win, height, width);
  wclear(life_win);

  for (int row = 0; row < height; row++)
    {
      for (int col = 0; col < width; col++)
        {
          int state = automaton_get_state(automaton, row - height / 2,
                                          col - width / 2);
          if (state)
            {
              wattrset(life_win, (state == 1) ? A_NORMAL : A_DIM);
              mvwaddch(life_win, row, col, '#');
            }
        }
    }
}

/*
 * MENU FUNCTIONS
 */

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
      break;
    }
}

void
select_menu_option (int key)
{
  collecting_input = false;
  loading_file     = false;
  
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
          automaton_dead_state(life);
          print_basic_controls();
          loading_file = true;
          print_file_prompt();
          break;
        case 2:/* user input state */
          automaton_dead_state(life);
          printw("%s%s", controls_msg, input_controls);
          refresh();
          curs_set(1);
          wmove(life_win, LINES / 2, COLS / 2);
          collecting_input = true;
          break;
        }
      update_menu(key);
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
  render_automaton(automaton);
  wmove(life_win, y, x);
}

int
main ()
{
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
  noecho();

  /* user controls */
  printw(controls_msg);
  refresh();

  life         = automaton_create(game_of_life, (LINES - 1) * 2, COLS * 2);
  input_buffer = string_create();

  /* Initialize our windows */
  life_win = newwin(LINES - 1, COLS, 1, 0);
  menu_win = newwin(MENU_HEIGHT, MENU_WIDTH, LINES / 2 - MENU_HEIGHT / 2,
                    COLS / 2 - MENU_WIDTH / 2);
  input_win = newwin(4, COLS, LINES / 2, 0);
  
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
              if (collecting_input)
                curs_set(1);
            }
          break;
        case '\n':
          if (menu.is_open)
            select_menu_option(key);
          else if (collecting_input)
            {
              collecting_input = false;
              curs_set(0);
              timeout(1000);
              print_basic_controls();
            }
          else if (loading_file)
            {
              bool rv = load_file_state();
              string_clear(input_buffer);
              if (!rv)
                print_invalid_file();
              else
                {
                  loading_file = false;
                  timeout(1000);
                }
            }
          break;
        }

      /* Get the path of the file from the user */
      if (loading_file && key != '\n')
        {
          if (key == KEY_BACKSPACE)
            string_pop_back(input_buffer);
          else
            string_push_back(input_buffer, key);
          print_file_prompt();
        }
      
      if (menu.is_open)
        {
          update_menu(key);
          print_menu(menu_win);
        }  
      
      // The menu may have just been closed
      if (!menu.is_open && !loading_file)
        {
          if (!collecting_input)
            {
              automaton_update_state(life);
              render_automaton(life);
            }
          else
            get_user_state(life_win, life, key);
          wrefresh(life_win);
        }

      key = getch();
    }
  while (key != KEY_F(1) && key != KEY_RESIZE);
  
  automaton_destroy(life);
  string_destroy(input_buffer);
  endwin();
  return 0;
}
