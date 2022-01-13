#include "PointSet.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct RB_TREE_NODE RB_Tree_Node;
struct RB_TREE_NODE
{
  bool is_red; /* true if red, false if black */
  RB_Tree_Node *left_child;
  RB_Tree_Node *right_child;
  RB_Tree_Node *parent;
  int x, y;
  void *data;
};

struct POINT_SET
{
  RB_Tree_Node *root;
  size_t data_size;
  void (*free_fn)(void *);
};

// sentinel for the leaves of the tree
static RB_Tree_Node tree_null = { .is_red = false };

/*
 * The point set is implemented as a red-black tree.  The following functions 
 * are for performing operations on said red-black tree.  These functions are 
 * based on pseudocode found in Introduction to Algorithms, 3rd Edition.
 */

// Perform a left rotation about the given node
static void
left_rotate (Point_Set *point_set, RB_Tree_Node *x)
{
  assert(x->right_child != &tree_null);
  assert(point_set->root->parent == &tree_null);

  // y starts out as x's right child
  RB_Tree_Node *y = x->right_child;

  // turn y's left subtree into x's right subtree
  x->right_child = y->left_child;
  if (y->left_child != &tree_null)
    y->left_child->parent = x;

  // link x's parent to y
  y->parent = x->parent;
  if (x->parent == &tree_null)
    point_set->root = y;
  else if (x == x->parent->left_child)
    x->parent->left_child = y;
  else
    x->parent->right_child = y;

  // put x to y's left
  y->left_child = x;
  x->parent = y;
}

static void
right_rotate (Point_Set *point_set, RB_Tree_Node *x)
{
  assert(x->left_child != &tree_null);
  assert(point_set->root->parent == &tree_null);

  RB_Tree_Node *y = x->left_child;

  // turn y's right subtree into x's left subtree
  x->left_child = y->right_child;
  if (y->right_child != &tree_null)
    y->right_child->parent = x;

  // link x's parent to y
  y->parent = x->parent;
  if (x->parent == &tree_null)
    point_set->root = y;
  else if (x == x->parent->right_child)
    x->parent->right_child = y;
  else
    x->parent->left_child = y;

  // put x to y's right
  y->right_child = x;
  x->parent = y;
}

static void
rb_insert_fixup (Point_Set *point_set, RB_Tree_Node *z)
{
  while (z->parent->is_red)
    {
      if (z->parent == z->parent->parent->left_child)
        {
          RB_Tree_Node *y = z->parent->parent->right_child;
          if (y->is_red)
            {
              z->parent->is_red = false;
              y->is_red = false;
              z->parent->parent->is_red = true;
              z = z->parent->parent;
            }
          else
            {
              if (z == z->parent->right_child)
                {
                  z = z->parent;
                  left_rotate(point_set, z);
                }
              z->parent->is_red = false;
              z->parent->parent->is_red = true;
              right_rotate(point_set, z->parent->parent);
            }
        }
      else
        {
          RB_Tree_Node *y = z->parent->parent->left_child;
          if (y->is_red)
            {
              z->parent->is_red = false;
              y->is_red = false;
              z->parent->parent->is_red = true;
              z = z->parent->parent;
            }
          else
            {
              if (z == z->parent->left_child)
                {
                  z = z->parent;
                  right_rotate(point_set, z);
                }
              z->parent->is_red = false;
              z->parent->parent->is_red = true;
              left_rotate(point_set, z->parent->parent);
            }
        }
    }
  point_set->root->is_red = false;
}

/*
 * Returns false if we find a node with the same xy.  No duplicates are allowed
 * so the given node does not get inserted into the tree.
 */
static bool
rb_insert (Point_Set *point_set, RB_Tree_Node *z)
{
  assert(point_set);
  assert(z);
  
  RB_Tree_Node *y = &tree_null;
  RB_Tree_Node *x = point_set->root;

  // search the tree for where to insert z, exit if duplicate is found
  while (x != &tree_null)
    {
      y = x;
      if (z->x == x->x && z->y == x->y)
        return false;
      else if (z->x < x->x || (z->x == x->x && z->y < x->y))
        x = x->left_child;
      else
        x = x->right_child;
    }

  // insert the node into the tree
  z->parent = y;
  if (y == &tree_null)
    point_set->root = z;
  else if (z->x < y->x || (z->x == y->x && z->y < y->y))
    y->left_child = z;
  else
    y->right_child = z;

  // set up the newly inserted node
  z->left_child = &tree_null;
  z->right_child = &tree_null;
  z->is_red = true;
  rb_insert_fixup(point_set, z);

  return true;
}

// replace one subtree with another
static void
rb_transplant (Point_Set *point_set, RB_Tree_Node *u, RB_Tree_Node *v)
{
  assert(point_set);
  assert(u);
  assert(v);
  
  if (u->parent == &tree_null)
    point_set->root = v;
  else if (u == u->parent->left_child)
    u->parent->left_child = v;
  else
    u->parent->right_child = v;
  v->parent = u->parent;
}

static RB_Tree_Node *
tree_minimum (RB_Tree_Node *x)
{
  while (x->left_child != &tree_null)
    x = x->left_child;
  return x;
}

static void
rb_delete_fixup (Point_Set *point_set, RB_Tree_Node *x)
{
  while (x != point_set->root && !x->is_red)
    {
      if (x == x->parent->left_child)
        {
          RB_Tree_Node *w = x->parent->right_child;
          if (w->is_red)
            {
              w->is_red = false;
              x->parent->is_red = true;
              left_rotate(point_set, x->parent);
              w = x->parent->right_child;
            }
          if (!w->left_child->is_red && !w->right_child->is_red)
            {
              w->is_red = true;
              x = x->parent;
            }
          else
            {
              if (!w->right_child->is_red)
                {
                  w->left_child->is_red = false;
                  w->is_red = true;
                  right_rotate(point_set, w);
                  w = x->parent->right_child;
                }
              w->is_red = x->parent->is_red;
              x->parent->is_red = false;
              w->right_child->is_red = false;
              left_rotate(point_set, x->parent);
              x = point_set->root;
            }
        }
      else
        {
          RB_Tree_Node *w = x->parent->left_child;
          if (w->is_red)
            {
              w->is_red = false;
              x->parent->is_red = true;
              right_rotate(point_set, x->parent);
              w = x->parent->left_child;
            }
          if (!w->right_child->is_red && !w->left_child->is_red)
            {
              w->is_red = true;
              x = x->parent;
            }
          else
            {
              if (!w->left_child->is_red)
                {
                  w->right_child->is_red = false;
                  w->is_red = true;
                  left_rotate(point_set, w);
                  w = x->parent->left_child;
                }
              w->is_red = x->parent->is_red;
              x->parent->is_red = false;
              w->left_child->is_red = false;
              right_rotate(point_set, x->parent);
              x = point_set->root;
            }
        }
    }
  x->is_red = false;
}

static void
rb_delete (Point_Set *point_set, RB_Tree_Node *z)
{
  RB_Tree_Node *x;
  RB_Tree_Node *y = z;
  bool y_init_red = y->is_red;

  if (z->left_child == &tree_null)
    {
      x = z->right_child;
      rb_transplant(point_set, z, z->right_child);
    }
  else if (z->right_child == &tree_null)
    {
      x = z->left_child;
      rb_transplant(point_set, z, z->left_child);
    }
  else
    {
      y = tree_minimum(z->right_child);
      y_init_red = y->is_red;
      x = y->right_child;
      if (y->parent == z)
        x->parent = y;
      else
        {
          rb_transplant(point_set, y, y->right_child);
          y->right_child = z->right_child;
          y->right_child->parent = y;
        }
      rb_transplant(point_set, z, y);
      y->left_child = z->left_child;
      y->left_child->parent = y;
      y->is_red = z->is_red;
    }
  if (!y_init_red)
    rb_delete_fixup(point_set, x);

  // free the node's resources
  point_set->free_fn(z->data);
  free(z);
}

static RB_Tree_Node *
tree_search (RB_Tree_Node *curr, int x, int y)
{
  while (curr != &tree_null && (x != curr->x || y != curr->y))
    {
      if (x < curr->x || (x == curr->x && y < curr->y))
        curr = curr->left_child;
      else
        curr = curr->right_child;
    }
  return curr;
}

/*
 * Definitions for the interface functions found in the header
 */

Point_Set *
point_set_create (size_t data_size, void (*free_fn)(void *))
{
  assert(free_fn);
  assert(data_size > 0);
  
  Point_Set *new_point_set = malloc(sizeof(Point_Set));
  
  if (new_point_set)
    {
      new_point_set->root = &tree_null;
      new_point_set->data_size = data_size;
      new_point_set->free_fn = free_fn;
    }
  
  return new_point_set;
}

void
point_set_destroy (Point_Set *point_set)
{
  // we keep deleting the root until no nodes are left
  while (point_set->root != &tree_null)
    rb_delete(point_set, point_set->root);

  free(point_set);
}

bool
point_set_insert (Point_Set *point_set, int x, int y, const void *data)
{
  assert(point_set != NULL);
  assert(data != NULL);
  
  bool ret = false;
  RB_Tree_Node *new_node;

  // allocate memory for the new node exit if NULL
  new_node = malloc(sizeof(RB_Tree_Node));
  if (!new_node)
    goto done;

  new_node->x = x;
  new_node->y = y;

  new_node->data = malloc(point_set->data_size);
  if (!new_node->data)
    {
      free(new_node);
      goto done;
    }
  memcpy(new_node->data, data, point_set->data_size);

  ret = rb_insert(point_set, new_node);
  if (!ret)
    {
      point_set->free_fn(new_node->data);
      free(new_node);
    }

 done: 
  return ret;
}

bool
point_set_delete (Point_Set *point_set, int x, int y)
{
  assert(point_set);
  
  bool success = false;
  RB_Tree_Node *to_delete = tree_search(point_set->root, x, y);

  if (to_delete != &tree_null)
    {
      rb_delete(point_set, to_delete);
      success = true;
    }
  
  assert(tree_search(point_set->root, x, y) == &tree_null);
  return success;
}

void *
point_set_search (Point_Set *point_set, int x, int y)
{
  assert(point_set);
  
  void *ret = NULL;
  RB_Tree_Node *node = tree_search(point_set->root, x, y);

  if (node != &tree_null)
    ret = node->data;
  
  return ret;
}
/*
int
main ()
{
  Point_Set *points = point_set_create(sizeof(int), free);
  int num = 53;
  bool success;

  for (int x = -50; x < 50; x++)
    {
      for (int y = -50; y < 50; y++)
        {
          assert(!point_set_search(points, x, y));
        }
    }
  

  for (int x = -50; x < 50; x++)
    {
      for (int y = -50; y < 50; y++)
        {
          success = point_set_insert(points, x, y, &num);
          assert(success);
          int *num_ptr = point_set_search(points, x, y);
          assert(num_ptr);
          assert(*num_ptr == 53);
        }
    }
  printf("Test 1 passed\n");
  
  for (int x = -50; x < 50; x++)
    {
      for (int y = -50; y < 50; y++)
        {
          success = point_set_insert(points, x, y, &num);
          assert(!success);
          assert(point_set_search(points, x, y));
        }
    }
  printf("Test 2 passed\n");

  for (int x = -50; x < 0; x++)
    {
      for (int y = 0; y < 50; y++)
        {
          success = point_set_delete(points, x, y);
          assert(success);
          assert(point_set_search(points, x, y) == NULL);
        }
    }
  printf("Test 3 passed\n");

  for (int x = -50; x < 0; x++)
    {
      for (int y = 0; y < 50; y++)
        {
          success = point_set_delete(points, x, y);
          assert(!success);
          assert(!point_set_search(points, x, y));
        }
    }
  printf("Test 4 passed\n");

  point_set_destroy(points);

  return 0;
}
*/
