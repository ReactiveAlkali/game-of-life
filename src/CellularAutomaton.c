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
  //printf("%d y=%d x=%d\n", count, y, x);

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

static Point_Set *
next_board_state_seeds (Automaton *automaton)
{
  assert(automaton);
  assert(automaton->board_state);
  assert(automaton->type == seeds);

  int live_state        = 1;
  Point_Set *next_state = point_set_create(sizeof(int), free);
  if (!next_state)
    goto done;

  for (int y = -automaton->height / 2;
       y < automaton->height - automaton->height / 2; y++)
    {
      for (int x = -automaton->width / 2;
           x < automaton->width - automaton->width / 2; x++)
        {
          int current_state   = automaton_get_state(automaton, y, x);
          int live_neighbours = check_moore_neighbourhood(automaton, y, x,
                                                          live_state);
          // Cell is born if it's dead and has two neighbours, all others die
          if (!current_state && live_neighbours == 2)
            point_set_insert(next_state, y, x, &live_state);
        }
    }

 done:
  return next_state;
}

static Point_Set *
next_board_state_life (Automaton *automaton)
{
  int live_state        = 1;
  Point_Set *next_state = point_set_create(sizeof(int), free);
  if (!next_state)
    goto done;

  // state coordinates are centred on (0,0)
  for (int y = -automaton->height / 2; y <= automaton->height / 2; y++)
    {
      for (int x = -automaton->width / 2; x <= automaton->width / 2; x++)
        {
          int state           = automaton_get_state(automaton, y, x);
          int live_neighbours = check_moore_neighbourhood(automaton, y, x, 1);
          
          if ((state && (live_neighbours == 2 || live_neighbours == 3))
              || (!state && live_neighbours == 3))
            point_set_insert(next_state, y, x, &live_state);
        }
    }

 done:
  return next_state;
}

static Point_Set *
next_board_state_wa_tor (Automaton *automaton)
{
  assert(automaton);
  assert(automaton->board_state);
  assert(automaton->type == wa_tor);

  int state_fish        = 1;
  int state_shark       = 2;
  Point_Set *next_state = point_set_create(sizeof(int), free);
}

// Creates an automaton struct ptr
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
automaton_create (Automaton_Type type, int height, int width, int **init_state)
{
  Automaton *new_automaton = automaton_new_struct(type, height, width);
  if (!new_automaton)
    goto done;

  new_automaton->board_state = point_set_create(sizeof(int), free);
  if (!new_automaton->board_state)
    {
      free(new_automaton);
      new_automaton = NULL;
      goto done;
    }

  // use the given initial state or random if NULL
  if (init_state)
    {
      for (int i = 0; i < height; i++)
        {
          for (int j = 0; j < width; j++)
            {
              int state = init_state[i][j];
              if (state)
                {
                  point_set_insert(new_automaton->board_state, i - height / 2,
                                   j - width / 2, &state);
                }
            }
        }
    }
  else
    {
      // randomize the automaton's state
      switch (type)
        {
        case game_of_life:
        case seeds:
          new_automaton->board_state = random_state(height, width, 2);
          break;
        default:
          new_automaton->board_state = NULL;
        }
      if (!new_automaton->board_state)
        {
          free(new_automaton);
          new_automaton = NULL;
        }
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
  bool success = true;
  Point_Set *next_state;

  switch (automaton->type)
    {
    case game_of_life:
      next_state = next_board_state_life(automaton);
      break;
    case seeds:
      next_state = next_board_state_seeds(automaton);
      break;
    default:
      next_state = NULL;
    }
  // Update failed
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

int
automaton_get_state (Automaton *automaton, int y, int x)
{
  int state = 0;

  int *state_ptr = point_set_search(automaton->board_state, y, x);
  if (state_ptr)
    state = *state_ptr;

  return state;
}

void
automaton_set_border (Automaton *automaton, int height, int width)
{
  automaton->height = height;
  automaton->width = width;
}
