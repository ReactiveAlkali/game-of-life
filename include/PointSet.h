/**
 * @file PointSet.h
 * @author Aaron Nedelec
 * @date 1 Jan 2021
 * @brief Interface for a set of points.
 *
 * This file defines an interface for a PointSet.  A PointSet is a set of 
 * points where each point has a location and some data value.  All points
 * are identified by their xy coordinates and no duplicates are allowed.
 */

#ifndef _POINT_SET_H_
#define _POINT_SET_H_

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief A struct holding a set of points
 */
typedef struct POINT_SET Point_Set;

/**
 * @brief Create a new point set
 *
 * @param data_size The size of the data that is going to be stored at a point
 * @param free_fn A function pointer used to free a stored data element
 * @return This function returns a pointer to the newly created point set or 
 * NULL if creation failed.
 */
Point_Set *
point_set_create (size_t data_size, void (*free_fn)(void *));

/**
 * @brief Destroy a point set.
 *
 * Destroys the given point set freeing all allocated resources.
 * @param ps The point set to destroy
 */
void
point_set_destroy (Point_Set *point_set);

/**
 * @brief Inserts point into a PointSet
 *
 * Inserts a new point into the given point set.  The data to be stored at that
 * point is copied into the set.  Duplicate points, identified via their 
 * coordinates, are not allowed and will result in the insertion failing.
 * @param point_set The point set to insert data into
 * @param x The x coordinate of the inserted point
 * @param y The y coordinate of the inserted point
 * @param data The data contained at the inserted point
 * @return Whether or not the insertion was successful
 */
bool
point_set_insert (Point_Set *point_set, int x, int y, const void *data);

/**
 * @brief Deletes the given point from the set
 *
 * The point corresponding to the given coordinates will be deleted from the 
 * set should it exist.
 * @param point_set The set of points to delete a point from
 * @param x The x coordinate of the point to delete
 * @param y The y coordinate of the point to delete
 * @return Whether or not the deletion was successful
 */
bool
point_set_delete (Point_Set *point_set, int x, int y);

/**
 * @brief Searches a point set for a point at the given coordinates
 *
 * Searches the given point set for a point at the given coordinates.  Should
 * one exist its data is copied to the supplied data pointer.  Should no such
 * point exist this function returns NULL.
 * @param point_set The point set to search in
 * @param x The x coordinate of the requested point
 * @param y The y coordinate of the requested point
 * @return A pointer to the data stored at the given coordinates or NULL if none
 * exists.
 */
void *
point_set_search (Point_Set *point_set, int x, int y);

#endif
