#ifndef SHS_2605_VECTOR_H
#define SHS_2605_VECTOR_H

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "CPPAlign.h"

#define VECTOR_DEFAULT_SIZE 16
#define VECTOR_EXPAND_CAP 2048

void * aligned_malloc ( size_t alignment, size_t size );

template <class T>
class Vector
{
public:
	
	Vector ( int32_t InitSize = - 1 )
	{
		
		if ( InitSize <= 0 )
			InitSize = VECTOR_DEFAULT_SIZE;
		
		if ( alignof ( T ) <= 8 )
		{
		
			Data = reinterpret_cast <T *> ( malloc ( sizeof ( T ) * InitSize ) );
		
			if ( Data == NULL )
				throw "NULL allocation.";
		
		}
		else
		{

			Data = reinterpret_cast <T *> ( aligned_malloc ( alignof ( T ), sizeof ( T ) * InitSize ) );

			if ( Data == NULL )
				throw "NULL allocation.";

		}

		Size = InitSize;
		
		Length = 0;
		
	};
	
	~Vector ()
	{
		
		free ( Data );

		Data = 0;
		
	};
	
	T & operator[] ( unsigned int x )
	{
		
		if ( x >= Length )
			throw "Error: element out of bounds.";

		return Data [ x ];
		
	};

	void FillTo ( uint32_t index, T fillValue )
	{

		expandFor ( index );

		for ( uint32_t i = Length; i < index + 1; i ++ )
			Data [ i ] = fillValue;

	};
	
	void Push ( T x )
	{
		
		expandFor ( Length + 1 );
		
		Data [ Length ] = x;
		Length ++;
		
	};
	
	T Pop ()
	{
		
		Length --;
		return Data [ Length ];
		
	};
	
	T Peek ()
	{
		
		return Data [ Length - 1 ];
		
	};
	
	void Remove ( uint32_t start, uint32_t length )
	{
		
		if ( start >= Length )
			return;
		if ( start + length > Length )
			length = Length - start;
		
		for ( unsigned int i = start; i < Length - length; i ++ )
			Data [ i ] = Data [ i + length ];
		
		Length -= length;
		
	};
	
	int32_t IndexOf ( T x, uint32_t start = 0 )
	{
		
		for (; start < Length; start ++ )
			if ( Data [ start ] == x )
				return start;
		
		return -1;
		
	};
	
	uint32_t GetLength ()
	{
		
		return Length;
		
	}
	
private:
	
	void expandFor ( uint32_t postlength )
	{
		
		if ( postlength <= Size )
			return;
		
		uint32_t newSize;
		
		if ( Size * 2 >= postlength )
			newSize = Size * 2;
		else
			newSize = postlength + Size;
		
		Size = newSize;

		if ( alignof ( T ) <= 8 )
		{

			Data = reinterpret_cast <T *> ( realloc ( Data, sizeof ( T ) * Size ) );
			
			if ( Data == NULL )
				throw "NULL reallocation.";
		
		}
		else
		{

			void * NewData = aligned_malloc ( alignof ( T ), sizeof ( T ) * Size );
			
			if ( NewData == NULL )
				throw "NULL reallocation.";

			memcpy ( NewData, reinterpret_cast <void *> ( Data ), sizeof ( T ) * Length );

			free ( Data );
			Data = reinterpret_cast <T *> ( NewData );	

		}

	};
	
	T * Data;
	uint32_t Size;
	uint32_t Length;
	
};

#endif
