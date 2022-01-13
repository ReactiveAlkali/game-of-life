#include "CellularAutomaton.h"
#include <stdio.h>
#include <stdbool.h>

int INIT1[3][3] = {
  {0,0,0},
  {0,0,0},
  {0,0,0}
};

int EXPECTED1[3][3] = {
  {0,0,0},
  {0,0,0},
  {0,0,0}
};

int INIT2[3][3] = {
  {0,0,1},
  {0,1,1},
  {0,0,0}
};

int EXPECTED2[3][3] = {
  {0,1,1},
  {0,1,1},
  {0,0,0}
};

int INIT3[3][3] = {
  {0,1,1},
  {0,0,0},
  {1,0,0}
};

int EXPECTED3[3][3] = {
  {0,0,0},
  {0,1,0},
  {0,0,0}
};

int INIT4[3][3] = {
  {0,1,0},
  {1,1,1},
  {0,1,0}
};

int EXPECTED4[3][3] = {
  {1,1,1},
  {1,0,1},
  {1,1,1}
};

void
print_state (int **state)
{
  for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
        {
          printf("%d", state[i][j]);
        }
      printf("\n");
    }
}

bool
test_state (int **init_state, int **expected_state, int height, int width)
{
  static int test_num = 1;
  bool success        = true;
  Automaton *init     = automaton_create(game_of_life, height, width, init_state);
  
  automaton_update_state(init);

  // check whether the states match
  for (int y = -height / 2; y < height / 2 && success; y++)
    {
      for (int x = -width / 2; x < width / 2 && success; x++)
        {
          init_state[y+height/2][x+width/2] = automaton_get_state(init, y, x);
          success = init_state[y+height/2][x+width/2] == expected_state[y+height/2][x+width/2];
        }
    }

  if (success)
    printf("PASSED %d\n", test_num);
  else
    {
      printf("FAILED %d\n", test_num);
      printf("Expected:\n");
      print_state(expected_state);
      printf("Actual:\n");
      print_state(init_state);
    }

  automaton_destroy(init);
  test_num++;

  return success;
}

int
main ()
{  
  // TEST 1: Dead cells stay dead
  int *init_state1[3] = { INIT1[0], INIT1[1], INIT1[2] };
  int *expected_state1[3] = { EXPECTED1[0], EXPECTED1[1], EXPECTED1[2] };
  test_state(init_state1, expected_state1, 3, 3);

  // TEST 2: Dead cells with 3 neighbours become alive and live cells with two
  // neighbours stay alive
  int *init_state2[3] = { INIT2[0], INIT2[1], INIT2[2] };
  int *expected_state2[3] = { EXPECTED2[0], EXPECTED2[1], EXPECTED2[2] };
  test_state(init_state2, expected_state2, 3, 3);

  // TEST 3: Live cells with 0 or 1 live neighbours becomes dead
  int *init_state3[3] = { INIT3[0], INIT3[1], INIT3[2] };
  int *expected_state3[3] = { EXPECTED3[0], EXPECTED3[1], EXPECTED3[2] };
  test_state(init_state3, expected_state3, 3, 3);  

  // TEST 4: Live cells with > 3 neighbours die, live cells with 3 neighbours live
  int *init_state4[3] = { INIT4[0], INIT4[1], INIT4[2] };
  int *expected_state4[3] = { EXPECTED4[0], EXPECTED4[1], EXPECTED4[2] };
  test_state(init_state4, expected_state4, 3, 3);
  
  return 0;
}
