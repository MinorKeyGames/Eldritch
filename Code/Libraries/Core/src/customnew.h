#ifndef CUSTOMNEW_H
#define CUSTOMNEW_H

#include <stddef.h>	// Required for BUILD_LINUX, at least.
#include <new>		// Required for BUILD_LINUX, at least.

#define USE_ALLOCATOR ( 1 && !BUILD_MAC )   // Early allocations through Cocoa framework makes allocator fail on Mac, or something.

class Allocator;

void* operator new( const size_t Size, Allocator& Alloc );
void* operator new( const size_t Size, Allocator& Alloc, const uint Alignment );
void* operator new[]( const size_t Size, Allocator& Alloc );
void* operator new[]( const size_t Size, Allocator& Alloc, const uint Alignment );

// These forms aren't actually used.
// They are just here to fix warnings about there not being deletes to match news.
void operator delete( void* const pObj, Allocator& Alloc );
void operator delete( void* const pObj, Allocator& Alloc, const uint Alignment );
void operator delete[]( void* const pObj, Allocator& Alloc );
void operator delete[]( void* const pObj, Allocator& Alloc, const uint Alignment );

#if USE_ALLOCATOR

#if BUILD_WINDOWS
#define NEW_THROW
#define DELETE_THROW
#else
#define NEW_THROW		throw( std::bad_alloc )
#define DELETE_THROW	throw()
#endif

// Override global new/delete
void* operator new( size_t Size ) NEW_THROW;		// Allocates from the default allocator
void* operator new[]( size_t Size ) NEW_THROW;		// Allocates from the default allocator
void operator delete( void* pObj ) DELETE_THROW;
void operator delete[]( void* pObj ) DELETE_THROW;

void* operator new( size_t Size, uint Alignment );			// Allocates from the default allocator
void* operator new[]( size_t Size, uint Alignment );		// Allocates from the default allocator
void operator delete( void* pObj, uint Alignment );			// Never needs to be called, just in case the compiler wants it automatically
void operator delete[]( void* pObj, uint Alignment );		// Never needs to be called, just in case the compiler wants it automatically

#endif // USE_ALLOCATOR

#endif // CUSTOMNEW_H