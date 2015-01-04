#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "xmalloc.h"

#define BLACK 0
#define RED 1

/* Struct that holds info about a dynamic memory region. Set up as a red-black tree
 * address - the memory address of this region (used for lookups)
 * freed - flag that indicates if this memory region was freed or not
 * left - child node with smaller address than parent
 * right - child node with greater address than parent
 */
typedef struct malloc_info
{
	void *address;
	size_t size;
	int freed;
	int color;
	struct malloc_info *left;
	struct malloc_info *right;
} malloc_info;

static void add_info(void *ptr, size_t sz);
static malloc_info *search(void *address);
static void insert(malloc_info *info_to_add);
static malloc_info *insert_helper(malloc_info *current, malloc_info *info_to_add);
static void print_xmalloc_info(void);
static void print_info(malloc_info *info);
static int is_red(malloc_info *info);
static malloc_info *rotate_left(malloc_info *info);
static malloc_info *rotate_right(malloc_info *info);
static void flip_colors(malloc_info *info);

static malloc_info *root = NULL;

/* Wrapper function for malloc that does error checking,
 * as well as adding the information about the malloc'd region to a list.
 * params:
 * bytes - the number of bytes to malloc
 * returns a pointer to the malloc'd region, or calls exit if malloc fails
 */
void *xmalloc(size_t bytes)
{
	if (NULL == root)
	{
		atexit(&print_xmalloc_info);
	}
	void *new_region = malloc(bytes);
	if (NULL == new_region)
	{
		fprintf(stderr, "%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	/* On the off chance that malloc returns an address that was previous freed
	 * first search for it in the list
	 */
	malloc_info *info = search(new_region);
	if (info != NULL)
	{
		info->freed = 0;
		info->size = bytes;
		return new_region;
	}
	add_info(new_region, bytes);
	return new_region;
}

/* Wrapper function for free that does error checking.
 * Detects double frees or frees in the middle of a currently allocated region.
 * Prints an error message when any of these errors are detected.
 * params:
 * ptr - the pointer to the malloc'd region to free
 */
void xfree(void *ptr)
{
	malloc_info *info = search(ptr);

	if (NULL == info)
	{
		fprintf(stderr, "Attempt to free a non-dynamically allocated region detected\n");
	}
	else
	{
		/* If it's not the same address then search returned the predecessor.
		 * This means an attempt was made to free in the middle of a malloc region.
		 */
		if (info->address != ptr)
		{
			fprintf(stderr, "Attempt to free in the middle of a malloc region at address %p detected\n", ptr);
		}
		else
		{
			if (0 == info->freed)
			{
				free(info->address);
				info->freed = 1;
			}
			else
			{
				fprintf(stderr, "Attempt to double free address %p detected\n", ptr);
			}
		}
	}
}

/* Adds info about a malloc'd region to the tree.
 * params:
 * ptr - the pointer to the newly malloc'd region
 * sz - the size of the malloc'd region
 * Calls exit if malloc fails.
 */
static void add_info(void *ptr, size_t sz)
{
	malloc_info *info_to_add = malloc(sizeof(malloc_info));
	if (NULL == info_to_add)
	{
		fprintf(stderr, "%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	info_to_add->address = ptr;
	info_to_add->size = sz;
	info_to_add->freed = 0;

	info_to_add->color = BLACK;
	info_to_add->left = NULL;
	info_to_add->right = NULL;
	
	insert(info_to_add);
}

/* Prints any memory leaks that were detected, and frees the offending regions.
 * Called the the end of the program, so the malloc_info tree is freed as well.
 */
static void print_xmalloc_info(void)
{
	print_info(root);
}

/* Does a post order traversal to free any addresses that are leaking memory.
 * Also frees the tree.
 * params:
 * info - current node in the tree the function is at.
 */
static void print_info(malloc_info *info)
{
	if (NULL != info)
	{
		print_info(info->left);
		print_info(info->right);

		if (0 == info->freed)
		{
			fprintf(stderr, "%u byte leak detected at memory address %p\n", (unsigned) info->size, info->address);
			free(info->address);
		}
		free(info);
	}
}

/* Searches the tree to find the node associated with the address.
 * Returns either the node associated with the address, or the predecessor of
 * the node if the address doesn't exist in the tree but is within the range
 * of the predecessor's memory region. This is to check freeing in the middle of
 * a region that was xmalloc'd.
 * params:
 * address - the address to search for in the tree
 */
static malloc_info *search(void *address)
{
	malloc_info *cursor = root;
	while (NULL != cursor)
	{
		if (address == cursor->address)
		{
			return cursor;
		}
		else if (address < cursor->address)
		{
			cursor = cursor->left;
		}
		else if (address > cursor->address)
		{
			if (address < cursor->address + cursor->size)
			{
				return cursor;
			}
			cursor = cursor->right;
		}
	}
	return NULL;
}

/* Inserts an info node into the tree by calling insert_helper
 * This is to handle insert being recursive and the root being
 * NULL at the start. Simplifies the call to insert as well.
 * info_to_add - the node to insert
 */
static void insert(malloc_info *info_to_add)
{
	root = insert_helper(root, info_to_add);
	root->color = BLACK;
}

/* Recursively inserts a node into the tree. Also does rotations
 * and color flips while recursing down and going back up the tree
 * in order to keep the tree balanced.
 * params:
 * current - the current node in the tree the function is at
 * info_to_add - the node to insert
 */
static malloc_info *insert_helper(malloc_info *current, malloc_info *info_to_add)
{
	if (NULL == current)
	{
		return info_to_add;
	}

	if (is_red(current->left) && is_red(current->right))
	{
		flip_colors(current);
	}

	if (info_to_add->address < current->address)
	{
		current->left = insert_helper(current->left, info_to_add);
	}
	else if (info_to_add->address > current->address)
	{
		current->right = insert_helper(current->right, info_to_add);
	}

	if (is_red(current->right) && !is_red(current->left))
	{
		current = rotate_left(current);
	}
	if (is_red(current->left) && is_red(current->left->left))
	{
		current = rotate_right(current);
	}

	return current;
}

/* Helper function that checks if the link to this node is red
 * params:
 * info - the node whose color is being checked
 */
static int is_red(malloc_info *info)
{
	if (NULL == info)
		return 0;
	return info->color;
}

/* Rotates a node to the left
 * params:
 * info - the node to be rotated
 */
static malloc_info *rotate_left(malloc_info *info)
{
	malloc_info *x = info->right;
	info->right = x->left;
	x->left = info;
	x->color = info->color;
	info->color = RED;
	return x;
}

/* Rotates a node to the right
 * info - the node to be rotated
 */
static malloc_info *rotate_right(malloc_info *info)
{
	malloc_info *x = info->left;
	info->left = x->right;
	x->right = info;
	x->color = info->color;
	info->color = RED;
	return x;
}

/* Inverts the color of a parent node and its children.
 * This is basically splitting a 4-node and passing the middle node up
 * params:
 * info - the node to be color flipped with its children
 */
static void flip_colors(malloc_info *info)
{
	info->color = !info->color;
	info->left->color = !info->left->color;
	info->right->color = !info->right->color;
}
