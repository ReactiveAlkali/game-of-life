#include "CellularAutomaton.h"
#include "PointSet.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

struct AUTOMATON
{
  int height;
  int width;
  Automaton_Type type;
  Point_Set *board_state;
};

/*
 * NEIGHBOURHOOD CHECKS
 *
 * Counts how many cells in the given state are located in the neighbourhood
 * surrounding the given location.
 */

static int
check_von_neumann_neighbourhood (Automaton *automaton, int y, int x, int state)
{
  assert(automaton);
  assert(automaton->board_state);

  return (automaton_get_state(automaton, y - 1, x) == state)
    + (automaton_get_state(automaton, y + 1, x) == state)
    + (automaton_get_state(automaton, y, x - 1) == state)
    + (automaton_get_state(automaton, y, x + 1) == state);
}

// Counts how many cells of the given state are in the Moore Neighbourhood
// of the given location.
static int
check_moore_neighbourhood (Automaton *automaton, int y, int x, int state)
{
  int count = 0;

  for (int i = y - 1; i <= y + 1; i++)
    {
      for (int j = x - 1; j <= x + 1; j++)
        {
          // Don't check the current location
          if ((i != y || j != x)
              && state == automaton_get_state(automaton, i, j))
            count++;
        }
    }

  return count;
}

static Point_Set *
random_state (int height, int width, int num_states)
{
  Point_Set *new_state = point_set_create(sizeof(int), free);
  if (!new_state)
    goto done;

  for (int y = -height / 2; y < height / 2 + height; y++)
    {
      for (int x = -width / 2; x < width / 2 + width; x++)
        {
          int state = rand() % num_states;
          if (state)
            point_set_insert(new_state, y, x, &state);
        }
    }

 done:
  return new_state;
}

/*
 * STATE UPDATE FUNCTIONS
 *
 * The following functions update the state of the given cellular automaton.  
 * Each function implementing a different set of rules corresponding to the 
 * different cellular automaton implemented here.
 */

static int
next_state_seeds (Automaton *automaton, int y, int x)
{
  int next_state      = 0;
  int current_state   = automaton_get_state(automaton, y, x);
  int live_neighbours = check_moore_neighbourhood(automaton, y, x, 1);
  
  if (!current_state && live_neighbours == 2)
    next_state = 1;

  return next_state;
}

static int
next_state_life (Automaton *automaton, int y, int x)
{
  int next_state      = 0;
  int current_state   = automaton_get_state(automaton, y, x);
  int live_neighbours = check_moore_neighbourhood(automaton, y, x, 1);

  if ((current_state && (live_neighbours == 2 || live_neighbours == 3))
      || (!current_state && live_neighbours == 3))
    next_state = 1;

  return next_state;
}

static int
next_state_highlife (Automaton *automaton, int y, int x)
{
  int next_state      = 0;
  int current_state   = automaton_get_state(automaton, y, x);
  int live_neighbours = check_moore_neighbourhood(automaton, y, x, 1);

  // Born with 3 or 6 neighbours; survives with 2 or 3 neighbours
  if ((!current_state && (live_neighbours == 3 || live_neighbours == 6))
      || (current_state && (live_neighbours == 2 || live_neighbours == 3)))
    next_state = 1;

  return next_state;
}

static int
next_state_greenberg_hastings (Automaton *automaton, int y, int x)
{
  int next_state      = 0;
  int current_state   = automaton_get_state(automaton, y, x);
  int live_neighbours = check_von_neumann_neighbourhood(automaton, y, x, 1);

  if (current_state == 1)
    next_state = 2;
  else if (current_state == 2)
    next_state = 0;
  else if (live_neighbours > 0)
    next_state = 1;

  return next_state;
}

static int
next_state_brians_brain (Automaton *automaton, int y, int x)
{
  int next_state    = 0;
  int current_state = automaton_get_state(automaton, y, x);
  int on_neighbours = check_moore_neighbourhood(automaton, y, x, 1);

  if (current_state == 1)
    next_state = 2;
  else if (current_state == 2)
    next_state = 0;
  else if (on_neighbours == 2)
    next_state = 1;

  return next_state;
}

static int
next_state_day_and_night (Automaton *automaton, int y, int x)
{
  int next_state      = 0;
  int current_state   = automaton_get_state(automaton, y, x);
  int live_neighbours = check_moore_neighbourhood(automaton, y, x, 1);

  /* B3678/S34678 */
  if ((current_state && (live_neighbours == 3 || live_neighbours == 4
                         || live_neighbours == 6 || live_neighbours == 7
                         || live_neighbours == 8))
      || (!current_state && (live_neighbours == 3 || live_neighbours == 6
                             || live_neighbours == 7 || live_neighbours == 8)))
    next_state = 1;

  return next_state;
}

static Point_Set *
next_board_state (Automaton *automaton)
{
  Point_Set *next_state = point_set_create(sizeof(int), free);
  if (!next_state)
    goto done;

  for (int y = -automaton->height / 2;
       y < automaton->height - automaton->height / 2; y++)
    {
      for (int x = -automaton->width / 2;
           x < automaton->width - automaton->width / 2; x++)
        {
          int cell_state = 0;

          // Get the next state of the cell according to the automaton's type
          switch (automaton->type)
            {
            case seeds:
              cell_state = next_state_seeds(automaton, y, x);
              break;
            case game_of_life:
              cell_state = next_state_life(automaton, y, x);
              break;
            case highlife:
              cell_state = next_state_highlife(automaton, y, x);
              break;
            case greenberg_hastings:
              cell_state = next_state_greenberg_hastings(automaton, y, x);
              break;
            case brians_brain:
              cell_state = next_state_brians_brain(automaton, y, x);
              break;
            case day_and_night:
              cell_state = next_state_day_and_night(automaton, y, x);
              break;
            }
          
          // Any point not in the point set is assumed to be zero
          if (cell_state)
            point_set_insert(next_state, y, x, &cell_state);
        }
    }

 done:
  return next_state;
}

/*
 * CONSTRUCTION AND DESTRUCTION
 */

static Automaton *
automaton_new_struct (Automaton_Type type, int height, int width)
{
  Automaton *new_automaton = malloc(sizeof(Automaton));
  if (!new_automaton)
    goto done;

  // initialize the automaton's members
  new_automaton->height = height;
  new_automaton->width  = width;
  new_automaton->type   = type;

 done:
  return new_automaton;
}

Automaton *
automaton_create (Automaton_Type type, int height, int width)
{
  Automaton *new_automaton = automaton_new_struct(type, height, width);
  if (!new_automaton)
    goto done;

  new_automaton->board_state = point_set_create(sizeof(int), free);
  if (!new_automaton->board_state)
    {
      free(new_automaton);
      new_automaton = NULL;
    }

 done:
  return new_automaton;
}

void
automaton_destroy (Automaton *automaton)
{
  point_set_destroy(automaton->board_state);
  free(automaton);
}

bool
automaton_update_state (Automaton *automaton)
{
  assert(automaton);
  assert(automaton->board_state);
  
  bool success = true;
  Point_Set *next_state;

  next_state = next_board_state(automaton);
  if (!next_state)
    {
      success = false;
      goto done;
    }

  // set the automaton's state to the next one
  point_set_destroy(automaton->board_state);
  automaton->board_state = next_state;

 done:
  return success;
}

/*
 * GETTERS
 */

int
automaton_get_width (Automaton *automaton)
{
  return automaton->width;
}

int
automaton_get_height (Automaton *automaton)
{
  return automaton->height;
}

int
automaton_get_state (Automaton *automaton, int y, int x)
{
  int state = 0;

  int *state_ptr = point_set_search(automaton->board_state, y, x);
  if (state_ptr)
    state = *state_ptr;

  return state;
}

/*
 * SETTERS
 */

bool
automaton_random_state (Automaton *automaton)
{
  assert(automaton);
  assert(automaton->board_state);

  bool success = false;
  Point_Set *new_state;

  switch (automaton->type)
    {
    case game_of_life:
    case seeds:
    case highlife:
    case day_and_night:
      new_state = random_state(automaton->height, automaton->width, 2);
      break;
    case greenberg_hastings:
    case brians_brain:
      new_state = random_state(automaton->height, automaton->width, 1);
    }
  if (!new_state)
    goto done;

  free(automaton->board_state);
  automaton->board_state = new_state;
  success = true;

 done:
  return success;
}

void
automaton_set_border (Automaton *automaton, int height, int width)
{
  automaton->height = height;
  automaton->width = width;
}
