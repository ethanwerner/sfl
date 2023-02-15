#ifndef POOL_H
#define POOL_H


#include <stdint.h>


typedef struct pool_t
{
	uint32_t chunk_n;  // Chunks per Block
	uint32_t chunk_s;  // Chunk size in bytes
	void *blocks;      // Blocks
	void *free;        // Free chunk
} pool_t;

pool_t * pool_init( uint32_t, uint32_t );
void pool_print( pool_t * pool );
void pool_fini( pool_t * );
void * pool_malloc( pool_t * );
void pool_free( pool_t *, void * );

#endif // POOL_H


#ifdef POOL_IMPLEMENTATION // IMPLEMENTATION


#include <stdlib.h>
#include <stdio.h>


#define BLOCKS_INITIAL 1


static void * block_init( uint32_t, uint32_t, void * );
static void block_set_next( void *, void * );
static void * block_get_chunks( void * );
static int block_contains( void *, void *, uint32_t, uint32_t );


// pool_init()
//
// Initializes a memory pool and returns a pointer to the memory pool
//
// chunk_s - The size of a chunk in bytes
// chunk_n - The number of chunks per block

pool_t * pool_init( uint32_t chunk_n, uint32_t chunk_s )
{
	pool_t *pool = ( pool_t * ) malloc( sizeof( pool_t ) );

	// Set size and count
	pool->chunk_n = chunk_n;
	pool->chunk_s = chunk_s;
	
	// Create block header and free chunk pointer
	//  Will eventually be placed into the pool	
	void *blocks = block_init( chunk_n, chunk_s, NULL );
	void *free = block_get_chunks( blocks );

	// More than one block on startup
	if( BLOCKS_INITIAL > 1 )
	{
		uintptr_t *head;
		for( int i = 1; i < BLOCKS_INITIAL; i++ )
		{
			head = ( uintptr_t * ) block_init( chunk_n, chunk_s, free );			
			block_set_next( head, blocks );

			free = block_get_chunks( head );
			blocks = head;
		}
	}

	pool->blocks = blocks;
	pool->free = free;

	return pool;
}


// pool_fini()
//
// Frees a memory pool and its corresponding blocks
//
// pool - A pointer to the pool object

void pool_fini( pool_t *pool )
{
	uintptr_t *block = pool->blocks;
	uintptr_t *tmp;
	
	// Fill chunk linked list
	while( block )
	{
		tmp = ( uintptr_t * ) *block;
		free( block );
		block = tmp;
	}

	free( pool );
}


// pool_malloc()
//
// Malloc replacement returning blocks the size of chunk_s taking the chunk 
//  from the free linked list and increments the free pool element.
//
// pool - A pointer to the pool object

void * pool_malloc( pool_t *pool )
{
	uintptr_t *free = pool->free;
	pool->free = ( void * ) *free;
	
	return free;
}


// pool_free()
//
// Malloc replacement returning blocks the size of chunk_s
//
// pool - A pointer to the pool object

void pool_free( pool_t *pool, void *p )
{
	// Convert the pointer into a usable type
	uintptr_t *free = ( uintptr_t * ) p;
	
	// Set the value at the address to the current front of the free list
	*free = ( uintptr_t ) pool->free;

	// Set the pool's free head to the newly freed pointer
	pool->free = ( void * ) free;
}


// block_init()
//
// Initializes the block with the space to fit a pointer to the next block and
//  a set of initialized chunks ordered in a free list
//
// chunk_s - The size of a chunk in bytes
// chunk_n - The number of chunks per block
// chunk_l - The pointer held in the last chunk

static void * block_init( uint32_t chunk_n, uint32_t chunk_s, void *chunk_l )
{
	// Allocate memory for the chunks and a pool_block_t wrapper
	void *raw = malloc( sizeof( uintptr_t ) + ( chunk_n * chunk_s ) );
	
	// Temporary pointer used to initialize chunks and block header
	uintptr_t *p;
	
	// Block header used to point to the next block
	p = ( uintptr_t * ) raw;
	*p = 0;
	p++;

	// Fill chunk linked list
	for( int i = 0; i < chunk_n - 1; i++ )
	{
		*p = ( uintptr_t ) ( ( uint8_t * ) p + chunk_s );
		p = ( uintptr_t * ) *p;
	}

	*p = ( uintptr_t ) chunk_l;

	return raw;
}


// block_set_next()
//
// Adds a pointer to next in the first 8 bytes of block_c 
//
// block_c - The block being added to 
// block_n - The block being added

static void block_set_next( void *block_c, void *block_n )
{
	uintptr_t *p = ( uintptr_t * ) block_c;
	*p = ( uintptr_t ) block_n;
}


// block_get_chunks()
//
// Returns the pointer to the beginning of the chunk section
//
// block - The pointer to the beginning of the block

static void * block_get_chunks( void *block )
{
	return ( ( void * ) ( ( uintptr_t ) block + sizeof( uintptr_t ) ) );
}


// block_contains()
//
// Returns an int corresponding to whether or not a block contains a chunk
//
// p - The specific chunk in question
// block - The pointer to the beginning of the block
// chunk_s - The size of a chunk in bytes
// chunk_n - The number of chunks per block

int block_contains( void *p, void *block, uint32_t chunk_s, uint32_t chunk_n )
{
	uintptr_t *chunks = ( uintptr_t * ) block_get_chunks( block );
	uintptr_t *pv = ( uintptr_t * ) p;

	return ( chunks <= pv ) && ( pv < ( chunks + chunk_s * chunk_n ) );
}


#endif // POOL_IMPLEMENTATION
