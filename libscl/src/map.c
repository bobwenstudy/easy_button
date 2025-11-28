/*
 * This file is part of the SCL software.
 * The license which this software falls under is as follows:
 *
 * Copyright (C) 2004-2010 Douglas Jerome <douglas@backstep.org>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


/* *****************************************************************************

FILE NAME

	$RCSfile: map.c,v $
	$Revision: 1.6 $
	$Date: 2010/04/14 03:08:14 $

PROGRAM INFORMATION

	Developed by:	SCL project
	Developer:	Douglas Jerome, drj, <douglas@backstep.org>

FILE DESCRIPTION

	Small Container Library: Map Container Implementation

	Many of the map functions depends upon:
	1. SCL_ALLOCATOR() setting allocated memory to binary 0,
	2. SCL_ALLOCATOR() returning NULL if memory allocation fails and
	3. NULL being equal to binary 0.

CHANGE LOG

	04apr10	drj	Added callback context pointer to scl_map_foreach().

	06jun06	drj	Miscellaenous format finessing.

	16dec04	drj	scl_map_insert() returns SCL_DUPKEY for duplicate keys.

	26nov04	drj	Changed scl_map_update() to scl_map_replace().

	30oct04	drj	File generation.

***************************************************************************** */


/* ************************************************************************* */
/*                                                                           */
/*      F e a t u r e   S w i t c h e s                                      */
/*                                                                           */
/* ************************************************************************* */

/*
 * Select these feature by moving them from the `if UNDEF' into the `else'
 * section.
 */
#ifdef	UNDEF
#   define	_BSD_SOURCE	1	/* 4.3+bsd subsystems           */
#   define	_POSIX_SOURCE	1	/* posix.1                      */
#   define	_POSIX_C_SOURCE	199309L	/* posix.1 and posix.4          */
#   define	_POSIX_C_SOURCE	199506L	/* posix.1 and posix.4 and MORE */
#else
#   define	_POSIX_C_SOURCE	200112L	/* posix.1 and posix.4 and MORE */
#   undef	_REENTRANT
#   define	_REENTRANT		/* thread-safe for glibc        */
#endif


/* ************************************************************************* */
/*                                                                           */
/*      I n c l u d e d   F i l e s                                          */
/*                                                                           */
/* ************************************************************************* */

/*
 * OS Specific Header Files
 */
// #ifdef	WIN32
// #   include	"stdafx.h"
// #endif

/*
 * Standard C (ANSI) Header Files
 */
/* (none) */

/*
 * Posix Header Files
 */
#ifdef	_unix
#   include	<unistd.h> /* always first amongst POSIX header files */
#endif

/*
 * Project Specific Header Files
 */
#include	"libscl/SCL_map.h"


/* ************************************************************************* */
/*                                                                           */
/*      C o n s t a n t s                                                    */
/*                                                                           */
/* ************************************************************************* */

#define   PRED(x)   ((x)->pred)
#define   SUCC(x)   ((x)->succ)

#define   COSMOLOGICAL_CONSTANT   (7)


/* ************************************************************************* */
/*                                                                           */
/*      E x t e r n a l   R e f e r e n c e s                                */
/*                                                                           */
/* ************************************************************************* */

/* (none) */


/* ************************************************************************* */
/*                                                                           */
/*      D a t a   T y p e s   a n d   S t r u c t u r e s                    */
/*                                                                           */
/* ************************************************************************* */

typedef struct S_node_t    S_node_t;
typedef struct S_SCL_map_t S_SCL_map_t;

struct S_node_t
   {
   S_node_t*   pred;
   S_node_t*   succ;
   SCL_map_t   map;
   long        index;
   long        key;
   const void* data;
   };

struct S_SCL_map_t
   {
   S_node_t** table;
   size_t     count;
   size_t     size;
   size_t     bmod;
   };


/* ************************************************************************* */
/*                                                                           */
/*      P u b l i c   G l o b a l   V a r i a b l e s                        */
/*                                                                           */
/* ************************************************************************* */

/* (none) */


/* ************************************************************************* */
/*                                                                           */
/*      P r i v a t e   G l o b a l   V a r i a b l e s                      */
/*                                                                           */
/* ************************************************************************* */

/* (none) */


/* ************************************************************************* */
/*                                                                           */
/*      E x e c u t a b l e   C o d e   (Locally Used Functions)             */
/*                                                                           */
/* ************************************************************************* */


/*****************************************************************************
 * Private Function Prototypes
 *****************************************************************************/

static __inline__ long hashfn (size_t a_downshift, long a_key);
static __inline__ void insert (S_node_t** const a_list, S_node_t* const a_node);
static __inline__ S_node_t* find (S_node_t** const a_list, long a_key);
static __inline__ int rebuild (S_SCL_map_t* a_map);


/*****************************************************************************
 * Private Function hashfn
 *****************************************************************************/

static __inline__ long hashfn (size_t a_downshift, long a_key)
   {
   register uint32_t v;

   v = (uint32_t)a_key * (uint32_t)1103515245L;
   v = v >> a_downshift;

   return (long)v;
   }


/*****************************************************************************
 * Private Function insert
 *****************************************************************************/

static __inline__ void insert (S_node_t** const a_list, S_node_t* const a_node)
   {
   register S_node_t* const node = a_node;
   register S_node_t* const head = *a_list;

   if (head != NULL)
      {
      PRED(head) = node;
      SUCC(node) = head;
      }
   else
      {
      SUCC(node) = NULL;
      }
   PRED(node) = NULL;
   *a_list = node;

   return;
   }


/*****************************************************************************
 * Private Function find
 *****************************************************************************/

static __inline__ S_node_t* find (S_node_t** const a_list, long a_key)
   {
   register S_node_t* node = *a_list;

   while (node != NULL)
      {
      if (node->key == a_key) break;
      node = SUCC(node);
      }

   return node;
   }


/*****************************************************************************
 * Private Function rebuild
 *****************************************************************************/

static __inline__ int rebuild (S_SCL_map_t* a_map)
   {
   register S_SCL_map_t* const map = a_map;

   register size_t size = map->size << 2;
   register size_t bmod = map->bmod -= 2;

   register S_node_t* node;
   register S_node_t* next;
   register long index;

   register int i;

   S_node_t** nodes = (S_node_t**)SCL_ALLOCATOR (size * sizeof(S_node_t*));
   if (nodes == NULL) return SCL_NOMEM;

   for (i=map->size-1 ; i>=0 ; i--)
      {
      node = map->table[i];
      while (node != NULL)
         {
         next = SUCC(node);
         index = hashfn (bmod, node->key);
         node->index = index;
         (void)insert (&nodes[index], node);
         node = next;
         }
      }

   SCL_DEALLOCATOR ((void*)map->table);

   map->table = nodes;
   map->size = size;
   map->bmod = bmod;

   return SCL_OK;
   }


/* ************************************************************************* */
/*                                                                           */
/*      E x e c u t a b l e   C o d e   (External Interface Functions)       */
/*                                                                           */
/* ************************************************************************* */


/*****************************************************************************
 * Public Function scl_map_new
 *****************************************************************************/

#define	BITS_IN_LONG	(sizeof(long)*8)
#define	BITS_IN_SIZE	(2)

SCL_map_t (scl_map_new) (void)
   {
   S_SCL_map_t* const map = (S_SCL_map_t* const)SCL_ALLOCATOR (sizeof(S_SCL_map_t));
   S_node_t** const nodes = (S_node_t** const)SCL_ALLOCATOR ((1<<BITS_IN_SIZE)*sizeof(S_node_t*));

   if ((map == NULL) || (nodes == NULL))
      {
      if (map != NULL) SCL_DEALLOCATOR (map);
      if (nodes != NULL) SCL_DEALLOCATOR (nodes);
      return NULL;
      }

   map->table = nodes;
   map->size = 1 << BITS_IN_SIZE;
   map->bmod = BITS_IN_LONG - BITS_IN_SIZE;

   return map;
   }

#undef	BITS_IN_LONG
#undef	BITS_IN_SIZE


/*****************************************************************************
 * Public Function scl_map_del
 *****************************************************************************/

void (scl_map_del) (SCL_map_t a_map)
   {
   S_SCL_map_t* const map = a_map;

   size_t size = map->size;
   S_node_t** table = map->table;

   S_node_t* node1;
   S_node_t* node2;

   while (size-- > 0)
      {
      node1 = *table++;
      while (node1 != NULL)
         {
         node2 = SUCC(node1);
         SCL_DEALLOCATOR ((void*)node1);
         node1 = node2;
         }
      }

   SCL_DEALLOCATOR (map->table);
   SCL_DEALLOCATOR (map);

   return;
   }


/*****************************************************************************
 * Public Function scl_map_erase
 *****************************************************************************/

void (scl_map_erase) (SCL_map_t a_map)
   {
   S_SCL_map_t* const map = a_map;

   size_t size = map->size;
   S_node_t** table = map->table;

   const S_node_t* node1;
   const S_node_t* node2;

   while (size-- > 0)
      {
      node1 = *table;
      while (node1 != NULL)
         {
         node2 = SUCC(node1);
         SCL_DEALLOCATOR ((void*)node1);
         node1 = node2;
         }
      *table++ = NULL;
      }

   map->count = 0;

   return;
   }


/*****************************************************************************
 * Public Function scl_map_size
 *****************************************************************************/

size_t (scl_map_size) (SCL_map_t a_map)
   {
   return a_map->size;
   }


/*****************************************************************************
 * Public Function scl_map_count
 *****************************************************************************/

size_t (scl_map_count) (SCL_map_t a_map)
   {
   return a_map->count;
   }


/*****************************************************************************
 * Public Function scl_map_insert
 *****************************************************************************/

int (scl_map_insert) (SCL_map_t a_map, long a_key, const void* a_data)
   {
   S_SCL_map_t* const map = a_map;
   const long index = hashfn (map->bmod, a_key);
   S_node_t* node;

   node = find (&a_map->table[index], a_key);
   if (node != NULL) return SCL_DUPKEY;

   node = (S_node_t*)SCL_ALLOCATOR (sizeof(S_node_t));
   if (node == NULL) return SCL_NOMEM;

   node->map = map;
   node->index = index;
   node->key = a_key;
   node->data = a_data;

   (void)insert (&map->table[index], node);

   map->count += 1;

   if (map->count > COSMOLOGICAL_CONSTANT * map->size)
      {
      (void)rebuild (map);
      }

   return SCL_OK;
   }


/*****************************************************************************
 * Public Function scl_map_replace
 *****************************************************************************/

int (scl_map_replace) (SCL_map_t a_map, long a_key, const void* a_data)
   {
   S_SCL_map_t* const map = a_map;
   const long index = hashfn (map->bmod, a_key);
   S_node_t* node = find (&map->table[index], a_key);

   if (node != NULL)
      {
      node->data = a_data;
      return SCL_OK;
      }

   node = (S_node_t*)SCL_ALLOCATOR (sizeof(S_node_t));
   if (node == NULL) return SCL_NOMEM;

   node->map = map;
   node->index = index;
   node->key = a_key;
   node->data = a_data;

   (void)insert (&map->table[index], node);

   map->count += 1;

   if (map->count > COSMOLOGICAL_CONSTANT * map->size)
      {
      (void)rebuild (map);
      }

   return SCL_OK;
   }


/*****************************************************************************
 * Public Function scl_map_remove
 *****************************************************************************/

void* (scl_map_remove) (SCL_map_t a_map, long a_key)
   {
   S_SCL_map_t* const map = a_map;
   const long index = hashfn (map->bmod, a_key);
   S_node_t* const node = find (&map->table[index], a_key);

   const void* data;

   if (node == NULL) return NULL;

   if (SUCC(node) != NULL) PRED(SUCC(node)) = PRED(node);
   if (PRED(node) != NULL) SUCC(PRED(node)) = SUCC(node);
   else map->table[index] = SUCC(node);

   data = node->data;

   SCL_DEALLOCATOR ((void*)node);
   map->count -= 1;

   return (void*)data;
   }


/*****************************************************************************
 * Public Function scl_map_access
 *****************************************************************************/

void* (scl_map_access) (SCL_map_t a_map, long a_key)
   {
   const long index = hashfn (a_map->bmod, a_key);
   const S_node_t* const node = find (&a_map->table[index], a_key);
   return node != NULL ? (void*)node->data : NULL;
   }


/*****************************************************************************
 * Public Function scl_map_at
 *****************************************************************************/

SCL_iterator_t (scl_map_at) (SCL_map_t a_map, long a_key)
   {
   const long index = hashfn (a_map->bmod, a_key);
   return (SCL_iterator_t) find (&a_map->table[index], a_key);
   }


/*****************************************************************************
 * Public Function scl_map_begin
 *****************************************************************************/

SCL_iterator_t (scl_map_begin) (SCL_map_t a_map)
   {
   const S_node_t* node = NULL;

   size_t index = a_map->size;
   const S_node_t** table = (const S_node_t**)a_map->table;

   while ((node == NULL) && (index-- > 0))
      {
      node = *table++;
      }

   return (SCL_iterator_t)node;
   }


/*****************************************************************************
 * Public Function scl_map_end
 *****************************************************************************/

SCL_iterator_t (scl_map_end) (SCL_map_t a_map)
   {
   const S_node_t* node = NULL;

   size_t index = a_map->size;
   const S_node_t** table = (const S_node_t**)&a_map->table[index];

   while ((node == NULL) && (index-- > 0))
      {
      node = *--table;
      }
   if (node != NULL) while (SUCC(node) != NULL) node = SUCC(node);

   return (SCL_iterator_t)node;
   }


/*****************************************************************************
 * Public Function scl_map_next
 *****************************************************************************/

SCL_iterator_t (scl_map_next) (SCL_iterator_t a_iterator)
   {
   const S_node_t* node = a_iterator;

   size_t index = node->index;
   const size_t size = node->map->size;
   const S_node_t** table = (const S_node_t**)&node->map->table[index];

   node = SUCC(node);
   while ((node == NULL) && (++index < size))
      {
      node = *++table;
      }

   return (SCL_iterator_t)node;
   }


/*****************************************************************************
 * Public Function scl_map_prev
 *****************************************************************************/

SCL_iterator_t (scl_map_prev) (SCL_iterator_t a_iterator)
   {
   const S_node_t* node = a_iterator;

   size_t index = node->index;
   const S_node_t** table = (const S_node_t**)&node->map->table[index];

   node = PRED(node);
   if (node != NULL) return (SCL_iterator_t)node;
   while ((node == NULL) && (index-- > 0))
      {
      node = *--table;
      }
   if (node != NULL) while (SUCC(node) != NULL) node = SUCC(node);

   return (SCL_iterator_t)node;
   }


/*****************************************************************************
 * Public Function scl_map_data_set
 *****************************************************************************/

void (scl_map_data_set) (SCL_iterator_t a_iterator, const void* a_data)
   {
   ((S_node_t*)a_iterator)->data = a_data;
   }


/*****************************************************************************
 * Public Function scl_map_data_get
 *****************************************************************************/

void* (scl_map_data_get) (SCL_iterator_t a_iterator)
   {
   return (void*)((S_node_t*)a_iterator)->data;
   }


/*****************************************************************************
 * Public Function scl_map_foreach
 *****************************************************************************/

void (scl_map_foreach) (SCL_map_t a_map, SCL_cbfn_t a_func, void* a_context)
   {
   const S_node_t* node;

   const size_t size = a_map->size;
   const S_node_t** table = (const S_node_t**)a_map->table;

   size_t i;
   int stat;

   for (i=0 ; i<size ; i++)
      {
      node = *table++;
      while (node != NULL)
         {
         stat = (*a_func) (NULL, node->key, (void*)node->data, a_context);
         if (stat != SCL_NOTFOUND) return;
         node = SUCC(node);
         }
      }

   return;
   }


/* end of file */
