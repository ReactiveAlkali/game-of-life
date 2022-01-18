/**
 * @file CellularAutomaton.h
 * @author Aaron Nedelec
 * @date 9 Jan 2022
 * @brief Interface for a cellular automaton
 * 
 * This header defines the interface for interacting with a simple 2D cellular
 * automaton.  All the defined cellular automaton are similar in function to 
 * Conway's Game of Life.
 */

#ifndef CELLULAR_AUTOMATON_H
#define CELLULAR_AUTOMATON_H

#include <stdbool.h>

typedef enum AUTOMATON_TYPE
  {
    game_of_life,
    seeds,
    greenberg_hastings,
    highlife,
    day_and_night,
    brians_brain
  } Automaton_Type;

typedef struct AUTOMATON Automaton;

/**
 * @brief Creates a new cellular automaton
 *
 * Creates a cellular automaton of the given type with a given board size.  The
 * automaton is always initialized to a dead state.
 * @param type The type of automaton that is being created.
 * @param height Height of the automaton's grid.
 * @param width Width of the automaton's grid.
 * @return Returns a pointer to the newly created automaton.
 */
Automaton *
automaton_create (Automaton_Type type, int height, int width);

/**
 * @brief Destroys the given automaton.
 *
 * Frees all allocated resources used by the given automaton.
 * @param automaton The cellular automaton to be destroyed.
 */
void
automaton_destroy (Automaton *automaton);

/**
 * @brief Updates the state of the given automaton
 *
 * Updates the state of the given automaton.
 * @param automaton The cellular automaton to be updated.
 * @return Returns whether the state update was successful.
 */ 
bool
automaton_update_state (Automaton *automaton);

/**
 * @brief Retrieve the state at the given location.
 *
 * Retrieves the state of the cell at the given location.  Depending on the 
 * automaton this will return 0, 1, or 2.
 * @param automaton The automaton to retrieve a state from.
 * @param y The y coordinate of the requested cell.
 * @param x The x coordinate of the requested cell.
 * @return Returns the state of the requested cell.
 */
int
automaton_get_state (Automaton *automaton, int y, int x);

int
automaton_get_width (Automaton *automaton);

int
automaton_get_height (Automaton *automaton);

/**
 * @brief Sets the boundaries of the given automaton.
 *
 * The height and width of an automaton define what range of cells will be 
 * checked during the next state update.
 * @param automaton The automaton to set the boundaries for
 * @param height The new height of the automaton.
 * @param width The new width of the given automaton.
 */
void
automaton_set_border (Automaton *automaton, int height, int width);

/**
 * @brief Sets the automaton to a random state.
 *
 * Randomizes the state of the given automaton's board.
 * @param automaton The automaton to randomize the state of.
 * @return Returns whether the state was successfully randomized.
 */
bool
automaton_random_state (Automaton *automaton);

/**
 * @brief Sets the board state of the given automaton.
 *
 * Sets the given automaton to the given state.  The automaton's border will 
 * be resized if the given state is larger that the current board size.
 * @param automaton The automaton to set the board state of.
 * @param height The height of the given state.
 * @param width The width of the given state.
 * @param state A 2D array containing the board state.
 * @return Returns whether the state was successfully set.
 */
bool
automaton_set_board_state (Automaton *automaton, int height, int width, int **state);

/**
 * @brief Sets the automaton's type
 *
 * Sets the given automaton to whatever the given type is
 * @param automaton The automaton to set the type of.
 * @param type The type to set the automaton to.
 */
void
automaton_set_type (Automaton *automaton, Automaton_Type type);

#endif
