#ifndef _OWN_STL_DECL_H_
#define _OWN_STL_DECL_H_

#include <string>
#include <vector>
#include <list>

#include "Engine/Memory/Mem.h"

namespace aya {
template<typename T>
class Allocator {
public:
	typedef T value_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T &reference;
	typedef const T& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	// bind MyAllocator to the U type
	template <class U>
	struct rebind {
		typedef Allocator<U> other;
	};


	Allocator() throw( ){}
	Allocator( const Allocator& ) throw( ){}
	template <class U> Allocator( const Allocator<U>& ) throw( ){}
	~Allocator() throw( ){}

	// Allocate memory
	pointer allocate( size_type num, void *hint = 0 )
	{
		(void)hint;
		return (pointer)usg::mem::Alloc( usg::MEMTYPE_STANDARD, usg::ALLOC_OBJECT, num * sizeof( T ), 4 );
	}
	// Initialize memory allocated
	void construct( pointer p, const T& value )
	{
		// force to construct
		new( (void*)p ) T( value );
	}

	// Deallocate memory
	void deallocate( pointer p, size_type num )
	{
		(void)num;

		usg::mem::Free( usg::MEMTYPE_STANDARD, (void*)p );
	}
	// Destruct memory initialized
	void destroy( pointer p )
	{
		p;
		p->~T();
	}

	pointer address( reference value ) const { return &value; }
	const_pointer address( const_reference value ) const { return &value; }

	// Return max numbers of allocatable
	size_type max_size() const throw( )
	{
		return std::numeric_limits<size_t>::max() / sizeof( T );
	}
};

template <typename T, typename U>
bool operator == ( const Allocator<T>& lhs, Allocator<U> const & rhs ) {
	return true;
}
template <typename T, typename U>
bool operator != ( const Allocator<T>& lhs, Allocator<U> const & rhs ) {
	return false;
}

typedef std::basic_string< char, std::char_traits<char>, Allocator<char> > string;
typedef std::vector< aya::string, aya::Allocator<aya::string> > StringVector;
}

#endif // _OWN_STL_DECL_H_
