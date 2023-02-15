/*
MIT License

Copyright (c) 2023 Ethan Werner

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


// bin.h - flat file, fixed block size, time series database


// TODO: bin_read_columns()

#ifndef BIN_H
#define BIN_H


#include <stdio.h>
#include <stdint.h>
#include <stddef.h>


typedef struct
{
	uint64_t length;
	uint64_t block_size;
} bin_meta_t;


FILE * bin_init( char const *, uint64_t );
FILE * bin_open( char const *path );
void bin_close( FILE * );

void bin_read( FILE *, int64_t, void *, uint64_t );
//void bin_read_columns( FILE *, int64_t, void *, uint64_t );
void bin_write( FILE *, int64_t, void *, uint64_t );
void bin_insert( FILE *, int64_t, void *, uint64_t );
void bin_append( FILE *, void *, uint64_t );

uint64_t bin_length( FILE * );
int64_t bin_search( FILE *, BIN_KEY_TYPE );
uint64_t bin_fuzzy_index( int64_t );


#endif // BIN_H


#ifdef BIN_IMPLEMENTATION // IMPLEMENTATION


#include <stdlib.h>
#include <assert.h>


static void bin_write_length( FILE *, uint64_t );
static void bin_write_block_size( FILE *, uint64_t );
static uint64_t bin_read_block_size( FILE * );


#define index_bytes( i, bs )  \
(                             \
	( i * bs ) + sizeof( bin_meta_t )   \
)


#define member_sizeof( type, member )   \
(                                       \
	sizeof( ( ( type * ) 0 )->member )  \
)


// bin_init()
//
// Creates the desired binary file
//
// path - The path and file name for the desired file
//
// Returns the newly created file

FILE * bin_init( char const *path, uint64_t block_size )
{
	FILE *f = fopen( path, "wb+" );

	if( !f )
	{
		return NULL;
	}

	setvbuf( f, NULL, _IONBF, 0 );

	bin_write_length( f, 0 );
	bin_write_block_size( f, block_size );

	return f;
}


// bin_open()
//
// Opens the desired binary file
//
// path - The path and file name for the desired file
//
// Returns the newly opened file

FILE * bin_open( char const *path )
{
	FILE *f = fopen( path, "rb+" );

	if( !f )
	{
		return NULL;
	}

	setvbuf( f, NULL, _IONBF, 0 );

	return f;
}


// bin_close()
//
// Closes the desired binary file
//
// f - The file descriptor to be closed

void bin_close( FILE *f )
{
	fclose( f );
}


// bin_read()
//
// Read the desired data into the buffer
//
// f - The file pointer
// i - The initial index of the read
// data - The block_t buffer to be read into
// n - The number of elements to read into the buffer

void bin_read( FILE *f, int64_t i, void *data, uint64_t n )
{
	uint64_t bs = bin_read_block_size( f );

	if( i > 0 )
	{
		assert( ( i + n - 1 ) < bin_length( f ) );

		fseek( f, index_bytes( i, bs ), SEEK_SET );
		fread( data, bs, n, f );
	}
	else
	{
		assert( ( -1 * i ) >= 0 );

		fseek( f, index_bytes( ( n + 1 ), bs ), SEEK_SET );
		fread( data, bs, n, f );
	}
}


// bin_write()
//
// Write the buffer into the file
//
// f - The file pointer
// i - The initial index of the write
// data - The block_t buffer to write from
// n - The number of elements to write from the buffer

void bin_write( FILE *f, int64_t i, void *data, uint64_t n )
{
	uint64_t l = bin_length( f );
	uint64_t bs = bin_read_block_size( f );

	assert( i <= ( int64_t ) l );

	fseek( f, index_bytes( i, bs ), SEEK_SET );
	fwrite( data, bs, n, f );

	if( ( i + n ) >= l )
	{
		l += ( i + n - l );
	}

	bin_write_length( f, l );
}


// bin_append()
//
// Appends the given data to the end of the binary file
//
// f - The file to be appended to
// data - The blocks to append to the file
// n - The number of block to be appended

void bin_append( FILE *f, void *data, uint64_t n )
{
	uint64_t l = bin_length( f );
	uint64_t bs = bin_read_block_size( f );

	fseek( f, index_bytes( l, bs ), SEEK_SET );
	fwrite( data, bs, n, f );

	bin_write_length( f, l + n );
}


// bin_insert()
//
// Inserts the given data into the binary file
//
// f - The file to be inserted into
// i - The initial index of the insertion
// data - The blocks to insert into the file
// n - The number of block to be inserted

void bin_insert( FILE *f, int64_t i, void *data, uint64_t n )
{
	uint64_t l = bin_length( f );
	uint64_t bs = bin_read_block_size( f );

	assert( i <= ( int64_t ) l );

	uint64_t tmp_n = ( l - i ) * bs;
	uint8_t tmp[tmp_n];

	fseek( f, index_bytes( i, bs ), SEEK_SET );
	fread( tmp, bs, l - i, f );

	fseek( f, index_bytes( i, bs ), SEEK_SET );
	fwrite( data, bs, n, f );
	fwrite( tmp, 1, tmp_n, f );

	bin_write_length( f, l + n );
}


// bin_length()
//
// Reads the block count number from the given file
//
// f - The file to be read from
//
// Returns the length / number of entries in the file

uint64_t bin_length( FILE *f )
{
	uint64_t n = 0;

	fseek( f, offsetof( bin_meta_t, length ), SEEK_SET );
	fread( &n, member_sizeof( bin_meta_t, length ), 1, f );

	return n;
}


// bin_search()
//
// Search for a given key in the file
//
// f - The file to be searched within
// k - The key to search for
//
// Returns the block_t index of the key. In the case that the key is not found,
// The returned value is -( index + 1 ) where index is that of the first element
// greater than k or bin_length().

int64_t bin_search( FILE *f, BIN_KEY_TYPE k )
{
	uint64_t origin = ftell( f );

	int64_t l = 0;
	int64_t r = bin_length( f ) - 1;
	int64_t m;

	uint64_t bs = bin_read_block_size( f );

	BIN_KEY_TYPE key;

	while( l <= r )
	{
		m = ( l + r ) / 2;
		fseek( f, index_bytes( m, bs ), SEEK_SET );
		fread( &key, sizeof( k ), 1, f );

		if( key < k )
			l = m + 1;

		else if( key > k )
			r = m - 1;

		else
			break;
	}

	fseek( f, origin, SEEK_SET );

	if( key != k )
		return -( l + 1 );

	return m;
}


// bin_fuzzy_index()
//
// If one does a binary search on the file and is returned a negative error,
// it may be useful to retrueve the index which it would be at.
//
// i - The number to map to the correct index
//
// The index remapped the binary file. If the index is already positive, then
// no operation occurrs.

uint64_t bin_fuzzy_index( int64_t i )
{
	return ( i < 0 ) ? -( i + 1 ) : i;
}


/////////////////////////////////////////////////////////////////////
// PRIVATE
/////////////////////////////////////////////////////////////////////


// bin_write_length()
//
// Writes a length to the given file
//
// f - The file to be written to
// count - The length to be written to the file

static void bin_write_length( FILE *f, uint64_t length )
{
	fseek( f, offsetof( bin_meta_t, length ), SEEK_SET );
	fwrite( &length, member_sizeof( bin_meta_t, length ), 1, f );
}


// bin_write_block_size()
//
// Writes a block size to the given file
//
// f - The file to be written to
// count - The block size to be written to the file

static void bin_write_block_size( FILE *f, uint64_t block_size )
{
	fseek( f, offsetof( bin_meta_t, block_size ), SEEK_SET );
	fwrite( &block_size, member_sizeof( bin_meta_t, block_size ), 1, f );
}


// bin_read_block_size()
//
// Reads the block size from the given file
//
// f - The file to be read from
//
// Returns the block size for the entries in the file

static uint64_t bin_read_block_size( FILE *f )
{
	uint64_t n = 0;

	fseek( f, offsetof( bin_meta_t, block_size ), SEEK_SET );
	fread( &n, member_sizeof( bin_meta_t, block_size ), 1, f );

	return n;
}


#endif
