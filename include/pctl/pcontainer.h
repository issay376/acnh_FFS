//
// pcontainer.h --- common declarations and utilities for Pointer based Container Template Library
//
//	Version 2.0
//
//      Copyright (C) 2003, 2018 Kazunari Saitoh
//
//      This library (Pointer based Container Template Library) is free software:
//      you can redistribute it and/or modify it under the term of the
//      GNU Lesser General Public License as published by the Free Software Foundation,
//      either version 3 of the License, or (at your option) any later version.
//
//      This library is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU Lesser General Public License for more details.
//
//      You should have received a copy of the GNU Lesser General Public License
//      along with this library.  If not, see <http://www.gnu.org/licenses/>.
//
// update log:
//	04/01/2003	Create OMT version 1.1.0
//	10/31/2014	Create PCTL version 1.1.0, C++11 support
//	02/15/2015	marge with common.h
//	02/09/2017	version 1.3.1 version number sync. with conteiner headers, pointer of native array support
//	02/23/2017	version 1.3.11 change c-string support: as char[], not char*
//	03/05/2017	version 1.3.12 rewrite type_traits using C++14
//	02/03/2018	Version 2.0, C++11/14 support completed
//

#ifndef __PCTL_PCONTAINER_H
#define __PCTL_PCONTAINER_H

#ifndef PCTL_NAMESPACE
#define PCTL_NAMESPACE pctl
#endif

#include <algorithm>
#include <cwchar>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <initializer_list>
#include <iterator>
#include <locale>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

// c++14 compatible type transformation traits for c++11
#ifndef __cpp_lib_transformation_trait_aliases
namespace std {
	template <bool C, typename T, typename F> using conditional_t   = typename conditional<C,T,F>::type;
	template <bool C, typename T = void>	  using enable_if_t	= typename enable_if<C,T>::type;
	template <typename T>			  using add_pointer_t   = typename add_pointer<T>::type;
	template <typename T>			  using remove_reference_t = typename remove_reference<T>::type;
	template <typename T>			  using remove_const_t  = typename remove_const<T>::type;
	template <typename T>			  using remove_cv_t	= typename remove_cv<T>::type;
}
#endif

#ifndef NO_NAMESPACE
namespace PCTL_NAMESPACE {
#endif // NO_NAMESPACE

// Common Constants and Memory Management Policy
// ============================================================================
#ifdef __OBJC__
enum Policy { pRefer, pStore, pCopy, pMRC };
#else
enum Policy { pRefer, pStore, pCopy };
#endif

// Error Handling
// ============================================================================
inline void pctl_error( const char* msg, ... )
{
        va_list ap;

        va_start( ap, msg );
        fprintf( stderr, "PCTL Error: " );
        vfprintf( stderr, msg, ap );
        fprintf( stderr, "\n" );
        va_end( ap );

        exit( -1 );
}

// ----------------------------------------------------------------------------
inline void pctl_warning( const char* msg, ... )
{
#ifdef _DEBUG
        va_list ap;

        va_start( ap, msg );
        fprintf( stderr, "PCTL Warning: " );
        vfprintf( stderr, msg, ap );
        fprintf( stderr, "\n" );
        va_end( ap );
#endif
}

// Type Deducers ( use like 'typename const_p<T>::t' )
// ============================================================================
//
//	Note: std::decay cannot use because it will remove constness
//
template <typename T>
struct func_p { typedef std::conditional_t<std::is_function<T>::value, std::add_pointer_t<T>, T> t; };

// Pointer deducer for native array
template <typename T, size_t N = 0> struct value_p		{ typedef typename func_p<T>::t t; };
template <typename V>		    struct value_p<V[]>		{ typedef V* t; };
template <typename V, size_t N>	    struct value_p<V[N]>	{ typedef V* t; };

// Const pointer deducer 
template <typename T, size_t N = 0> struct const_p 		{ typedef typename func_p<T>::t t; };
template <typename V>		    struct const_p<V*> 		{ typedef const V* t; };
template <typename V>		    struct const_p<V[]> 	{ typedef const V* t; };
template <typename V, size_t N>	    struct const_p<V[N]> 	{ typedef const V* t; };

// Const array deducer 
template <typename T, size_t N = 0> struct const_a 		{ typedef typename func_p<T>::t t; };
template <typename V>		    struct const_a<V*> 		{ typedef const V* t; };
template <typename V>		    struct const_a<V[]> 	{ typedef const V t[]; };
template <typename V, size_t N>	    struct const_a<V[N]> 	{ typedef const V t[N]; };

// Policy depending const pointer deducer
template <typename T, Policy P, size_t N = 0> struct ccopy_p	{ typedef typename value_p<T>::t t; };
template <typename V>		struct ccopy_p<V*,pCopy>	{ typedef const V* t; };
template <typename V>		struct ccopy_p<V[],pCopy>	{ typedef const V* t; };
template <typename V, size_t N>	struct ccopy_p<V[N],pCopy>	{ typedef const V* t; };

template <typename T, Policy P, size_t N = 0> struct ccopy_a	{ typedef typename func_p<T>::t t; };
template <typename V>		struct ccopy_a<V*,pCopy>	{ typedef const V* t; };
template <typename V>		struct ccopy_a<V[],pCopy>	{ typedef const V t[]; };
template <typename V, size_t N>	struct ccopy_a<V[N],pCopy>	{ typedef const V t[N]; };

template <typename T, Policy P, size_t N = 0> struct cnref_p	{ typedef typename value_p<T>::t t; };
template <typename V>		struct cnref_p<V*,pStore>	{ typedef const V* t; };
template <typename V>		struct cnref_p<V[],pStore>	{ typedef const V* t; };
template <typename V, size_t N>	struct cnref_p<V[N],pStore>	{ typedef const V* t; };
template <typename V>		struct cnref_p<V*,pCopy>	{ typedef const V* t; };
template <typename V>		struct cnref_p<V[],pCopy>	{ typedef const V* t; };
template <typename V, size_t N>	struct cnref_p<V[N],pCopy>	{ typedef const V* t; };

template <typename T, Policy P, size_t N = 0> struct cnref_a	{ typedef typename func_p<T>::t t; };
template <typename V>		struct cnref_a<V*,pStore>	{ typedef const V* t; };
template <typename V>		struct cnref_a<V[],pStore>	{ typedef const V t[]; };
template <typename V, size_t N>	struct cnref_a<V[N],pStore>	{ typedef const V t[N]; };
template <typename V>		struct cnref_a<V*,pCopy>	{ typedef const V* t; };
template <typename V>		struct cnref_a<V[],pCopy>	{ typedef const V t[]; };
template <typename V, size_t N>	struct cnref_a<V[N],pCopy>	{ typedef const V t[N]; };

// Iterator value deducer
template <typename It> using ivalue_t = typename std::iterator_traits<It>::value_type;

// Pointer Container Checker ( is_pcontainer<T>, is_keyed<T>, is_(pcontainer)<T> )
// ============================================================================
template <typename T, Policy P> 						class plist;
template <typename T, Policy P, typename K, Policy Q, typename H, typename E>	class phash;
template <typename T, Policy P, typename K, Policy Q, typename C>		class pmultimap;
template <typename T, Policy P, typename K, Policy Q, typename C>		class pmap;
template <typename T, Policy P, typename C>					class pmultiset;
template <typename T, Policy P, typename C>					class pset;
template <typename T, Policy P>							class pvector;

template <typename PC>
	struct is_pcontainer : public std::false_type { }; 
template <typename T, Policy P>
	struct is_pcontainer<plist<T,P>> : public std::true_type { };
template <typename T, Policy P, typename K, Policy Q, typename H, typename E> 
	struct is_pcontainer<phash<T,P,K,Q,H,E>> : public std::true_type { };
template <typename T, Policy P, typename K, Policy Q, typename C>
	struct is_pcontainer<pmultimap<T,P,K,Q,C>> : public std::true_type { };
template <typename T, Policy P, typename K, Policy Q, typename C>
	struct is_pcontainer<pmap<T,P,K,Q,C>> : public std::true_type { };
template <typename T, Policy P, typename C>
	struct is_pcontainer<pmultiset<T,P,C>> : public std::true_type { };
template <typename T, Policy P, typename C>
	struct is_pcontainer<pset<T,P,C>> : public std::true_type { };
template <typename T, Policy P>
	struct is_pcontainer<pvector<T,P>> : public std::true_type { };

template <typename PC>
	struct is_keyed : public std::false_type { };
template <typename T, Policy P, typename K, Policy Q, typename H, typename E> 
	struct is_keyed<phash<T,P,K,Q,H,E>> : public std::true_type { };
template <typename T, Policy P, typename K, Policy Q, typename C>
	struct is_keyed<pmultimap<T,P,K,Q,C>> : public std::true_type { };
template <typename T, Policy P, typename K, Policy Q, typename C>
	struct is_keyed<pmap<T,P,K,Q,C>> : public std::true_type { };

template <typename PC>
	struct is_plist : public std::false_type { };
template <typename T, Policy P>
	struct is_plist<plist<T,P>> : public std::true_type { };

template <typename PC>
	struct is_phash : public std::false_type { };
template <typename T, Policy P, typename K, Policy Q, typename H, typename E> 
	struct is_phash<phash<T,P,K,Q,H,E>> : public std::true_type { };

template <typename PC>
	struct is_pmultimap : public std::false_type { };
template <typename T, Policy P, typename K, Policy Q, typename C>
	struct is_pmultimap<pmultimap<T,P,K,Q,C>> : public std::true_type { };

template <typename PC>
	struct is_pmap : public std::false_type { };
template <typename T, Policy P, typename K, Policy Q, typename C>
	struct is_pmap<pmap<T,P,K,Q,C>> : public std::true_type { };

template <typename PC>
	struct is_pmultiset : public std::false_type { };
template <typename T, Policy P, typename C>
	struct is_pmultiset<pmultiset<T,P,C>> : public std::true_type { };

template <typename PC>
	struct is_pset : public std::false_type { };
template <typename T, Policy P, typename C>
	struct is_pset<pset<T,P,C>> : public std::true_type { };

template <typename PC>
	struct is_pvector : public std::false_type { };
template <typename T, Policy P>
	struct is_pvector<pvector<T,P>> : public std::true_type { };

// Common Function Objects (hash_fn, eql_fn, cmp_fn, dup_fn, del_fn)
// ============================================================================

// hash_fn
// ----------------------------------------------------------------------------
template <typename K, size_t N = 0> struct hash_fn;

// ----------------------------------------------------------------------------
template <typename K> struct hash_fn_varray 
{
	size_t operator()( const K* const& key ) const
	{
		size_t	r = 0;
		const K	t = K();

		if ( key ) {
			for ( const K* p = key; *p != t; ++p ) r += ( r << 2 ) + hash_fn<K>()( *p );
		}
		return r;
	}
};

template <typename K> struct hash_fn_id
{
	size_t operator()( K const& key ) const
	{
		return static_cast<size_t>( key );
	}
};

// ----------------------------------------------------------------------------
struct hash_fn_pvoid
{
	size_t operator()( const void* key ) const
	{
		size_t	r = 0;
		size_t	x = reinterpret_cast<size_t>( key );

		while ( 0 < x ) {
			r += x + x + x;
			x >>= 5;
		}
		return r;
	}
};

// ----------------------------------------------------------------------------
template <typename K, size_t N> struct hash_fn
{
	size_t operator()( K const& key ) const
	{
		size_t	r = 0;
		size_t	x = static_cast<size_t>( key );

		while ( 0 < x ) {
			r += x + x + x;
			x >>= 5;
		}
		return r;
	}
};

// partially specialized hash_fns
// ----------------------------------------------------------------------------
template <> struct hash_fn<char>	: public hash_fn_id<char> { };
template <> struct hash_fn<const char>	: public hash_fn_id<char> { };

template <> struct hash_fn<void*>	: public hash_fn_pvoid { };
template <> struct hash_fn<const void*>	: public hash_fn_pvoid { };

template <typename K> struct hash_fn<K*>
{
	size_t operator()( const K* const& key ) const { return key ? hash_fn<K>()( *key ) : 0; }
};
template <typename K>		struct hash_fn<K[]>  : public hash_fn_varray<K>   { };
template <typename K, size_t N> struct hash_fn<K[N]> 
{
	size_t operator()( const K* const& key ) const
	{
		size_t	r = 0;

		if ( key ) {
			for ( size_t i = 0; i < N; ++i ) r += ( r << 2 ) + hash_fn<K>()( key[ i ] );
		}
		return r;
	}
};

// eql_fn
// ----------------------------------------------------------------------------
template <typename K, size_t N = 0> struct eql_fn 
{
	bool operator()( K const& k1, K const& k2 ) const { return k1 == k2; }
};

// eql_fn utilities
// ----------------------------------------------------------------------------
template <typename K> struct eql_fn_varray 
{
	const K		term = K();

	bool operator()( const K* const& k1, const K* const& k2 ) const
	{
		if      ( k1 == nullptr && k2 == nullptr ) return  true;
		else if ( k1 == nullptr || k2 == nullptr ) return false;

		const K*	a = k1;
		const K*	b = k2;

		for ( ; *a != term && *b != term && *a == *b; ++a, ++b ) ;

		if ( *a == term && *b == term )	return  true;
		else 				return false;
	}
};

struct eql_fn_pvoid
{
	bool operator()( const void* const& k1, const void* const& k2 ) const { return k1 == k2; }
};

// partially specialized eql_fns
// ----------------------------------------------------------------------------
template <> struct eql_fn<void*>	   : eql_fn_pvoid { };
template <> struct eql_fn<const void*>     : eql_fn_pvoid { };

template <typename K> struct eql_fn<K*>
{
	bool operator()( const K* const& k1, const K* const& k2 ) const
	{
		if      ( k1 == nullptr && k2 == nullptr )	return  true;
		else if ( k1 == nullptr || k2 == nullptr )	return  false;
		else if ( *k1 == *k2 )				return  true;
		else						return  false;
	}
};
template <typename K>		struct eql_fn<K[]>  : public eql_fn_varray<K>   { };
template <typename K, size_t N> struct eql_fn<K[N]> 
{
	bool operator()( const K* const& k1, const K* const& k2 ) const
	{
		if      ( k1 == nullptr && k2 == nullptr ) return  true;
		else if ( k1 == nullptr || k2 == nullptr ) return false;

		size_t	i;

		for ( i = 0; i < N; ++i ) if ( k1[ i ] != k2[ i ] ) break;
		return i == N;
	}
};

// cmp_fn
// ----------------------------------------------------------------------------
template <typename K, size_t N = 0> struct cmp_fn 
{
	int operator()( K const& k1, K const& k2 ) const
	{
		if	( k1 <  k2 ) return -1;
		else if ( k1 == k2 ) return  0;	
		else		     return  1; 
	}
};

// cmp_fn - utilities
// ----------------------------------------------------------------------------
template <typename K> struct cmp_fn_varray 
{
	const K		term = K();

	int operator()( const K* const& k1, const K* const& k2 ) const
	{
		if ( k1 == nullptr || k2 == nullptr ) {
			if      ( k1 != nullptr )	return  1;
			else if ( k2 != nullptr )	return -1;
			else 				return  0;
		} else {
			const K*	a;
			const K*	b;

			for ( a = k1, b = k2; *a != term && *b != term && *a == *b; ++a, ++b ) ;

			if ( *a == term && *b == term )	return  0;
			else if	( *a == term ) 		return -1;
			else if ( *b == term ) 		return  1;
			else if ( *a < *b )   		return -1;
			else 				return  1;
		}
	}
};

struct cmp_fn_pvoid 
{
	int operator()( const void* const& k1, const void* const& k2 ) const
	{
		if	( k1 <  k2 ) return -1;
		else if ( k1 == k2 ) return  0;	
		else		     return  1; 
	}
};

// partially specialized cmp_fns
// ----------------------------------------------------------------------------
template <> struct cmp_fn<void*>	   : public cmp_fn_pvoid { };
template <> struct cmp_fn<const void*>     : public cmp_fn_pvoid { };

template <typename K> struct cmp_fn<K*> 
{
	int operator()( const K* const& k1, const K* const& k2 ) const
	{
		if ( k1 == nullptr || k2 == nullptr ) {
			if      ( k1 != nullptr )	return  1;
			else if ( k2 != nullptr )	return -1;
			else				return  0;
		} else {
			if	( *k1 == *k2 )		return  0;
			else if ( *k1 <  *k2 )		return -1;
			else				return  1;
		}
	}
};
template <typename K>	   struct cmp_fn<K[]>  : public cmp_fn_varray<K>   { };
template <typename K, size_t N> struct cmp_fn<K[N]> 
{
	int operator()( const K* const& k1, const K* const& k2 ) const
	{
		if ( k1 == nullptr || k2 == nullptr ) {
			if      ( k1 != nullptr )	return  1;
			else if ( k2 != nullptr )	return -1;
			else 				return  0;
		} else {
			int	r = 0;

			for ( size_t i = 0; i < N; ++i ) {
				if ( k1[ i ] < k2[ i ] ) { r = -1; break; }
				if ( k2[ i ] < k1[ i ] ) { r =  1; break; }
			}
			return r;
		}
	}
};

// Note: If you use a special memory manager and the element pointer of container
//	will be created/deleted not by 'new' or 'delete', please make specialized
//	dup_fn/del_fn appropriately. 
//
// dup_fn
// ----------------------------------------------------------------------------
template <typename K, size_t N = 0> struct dup_fn { /* not implemented */ };

template <typename K> struct dup_fn<K*>
{
	K* operator()( K const* const& p ) { return p ? new K( *p ) : nullptr; }
};

template <typename K> struct dup_fn<K[]>
{
	typedef std::remove_const_t<K>	V;

	V* operator()( const V* const& p )
	{
		V*	r;

		if ( p ) {
			unsigned	n = 0;

			for ( const V* q = p; *q != V(); ++q ) ++n;

			r = new V[ n + 1 ];
			for ( unsigned i = 0; i < n; ++i ) r[ i ] = p[ i ];
			r[ n ] = V();
		} else {
			r = nullptr;
		}
		return r;
	}
};

template <typename K, size_t N> struct dup_fn<K[N]>
{
	typedef std::remove_const_t<K>	V;

	V* operator()( const V* const& p )
	{
		V*	r;

		if ( p ) {
			r = new V[ N ];
			for ( unsigned i = 0; i < N; ++i ) r[ i ] = p[ i ];
		} else {
			r = nullptr;
		}
		return r;
	}
};

//
// del_fn
// ----------------------------------------------------------------------------
template <typename K, size_t N = 0> struct del_fn { /* not implemented */ };

template <typename K> struct del_fn<K*>
{
	void operator()( K const* const& p ) { if ( p ) delete p; }
};

template <typename K> struct del_fn<K[]>
{
	void operator()( K const* const& p ) { if ( p ) delete [] p; }
};

template <typename K, size_t N> struct del_fn<K[N]>
{
	void operator()( K const* const& p ) { if ( p ) delete [] p; }
};

// Memory Management Policy Classes
// ============================================================================
template <Policy P, typename T, size_t N = 0> struct policy;

// refer policy
// ----------------------------------------------------------------------------
template <typename T> struct policy<pRefer, T>
{
    private:
	typedef typename value_p<T>::t	value_type;

    public:
	static value_type set( value_type const& d ) { return d; }	// for container element constructor
	static value_type copy( value_type const& d ) { return d; }	// for container copy constructor/assignment etc.
	static value_type del( value_type const& d ) { return d; }	// for container element destructor
	
	// release managed data value_type for pStore/pCopy
	static value_type reldat( value_type const& d ) { return value_type(); }

	// release unused argument for pStore
	static value_type relarg( value_type const& d ) { return value_type(); }

	// utility - policy predicate
	static constexpr Policy		value = pRefer;
	static constexpr bool is_refer() { return true; }
	static constexpr bool is_store() { return false; }
	static constexpr bool is_copy() { return false; }
};

// store policy
// ----------------------------------------------------------------------------
template <typename T> struct store_policy_base
{
    protected:
	typedef typename value_p<T>::t	pointer;
	typedef typename const_p<T>::t	const_pointer;

    public:
	static pointer set( pointer const& d ) { return d; }
	static pointer copy( const_pointer const& p ) { return p ? dup_fn<T>()( p ) : nullptr; }
	static pointer del( pointer p ) { if ( p ) del_fn<T>()( p ); return nullptr; }

	static pointer reldat( pointer const& p ) { return p; }
	static pointer relarg( pointer p ) { if ( p ) del_fn<T>()( p ); return nullptr; }

	// policy predicate
	static constexpr Policy		value = pStore;
	static constexpr bool is_refer() { return false; }
	static constexpr bool is_store() { return true; }
	static constexpr bool is_copy() { return false; }
};
template <typename T> struct policy<pStore, T> { /* only for pointer */ };
template <typename T> struct policy<pStore, T*> : public store_policy_base<T*> { };
template <typename T> struct policy<pStore, T[]> : public store_policy_base<T[]> { }; 
template <typename T, size_t N> struct policy<pStore, T[N]> : public store_policy_base<T[N]> { };

// copy policy
// ----------------------------------------------------------------------------
template <typename T> struct copy_policy_base
{
    protected:
	typedef typename value_p<T>::t	pointer;
	typedef typename const_p<T>::t	const_pointer;

    public:
	static pointer set( const_pointer const& p ) { return p ? dup_fn<T>()( p ) : nullptr; }
	static const_pointer copy( const_pointer const& p ) { return p; }
	static pointer del( pointer p ) { if ( p ) del_fn<T>()( p ); return nullptr; }

	static pointer reldat( pointer const& p ) { return p; }
	static pointer relarg( pointer const& p ) { return nullptr; }

	// policy predicate
	static constexpr Policy		value = pCopy;
	static bool constexpr is_refer() { return false; }
	static bool constexpr is_store() { return false; }
	static bool constexpr is_copy() { return true; }
};
template <typename T> struct policy<pCopy, T> { /* only for pointer */ };
template <typename T> struct policy<pCopy, T*> : public copy_policy_base<T*> { };
template <typename T> struct policy<pCopy, T[]> : public copy_policy_base<T[]> { };
template <typename T, size_t N> struct policy<pCopy, T[N]> : public copy_policy_base<T[N]> { };

#ifdef __OBJC__
//
// Policy for Objective-C++ Manual Reference Counting -- trial implementation -- 2012/06/25
// ----------------------------------------------------------------------------
template <typename T> struct policy<pMRC, T> { /* only for pointer */ };
template <typename T> struct policy<pMRC, T*> 
{
	typedef T*	value_type;

	static T* set( T* p ) { return p ? [p retain] : nil; }
	static T* copy( T* p ) { return p; }
	static T* del( T* p ) { if ( p ) [p release]; return nil; }

	static T* reldat( const T*& p ) { return p; }
	static T* relarg( const T*& p ) { return T(); }
};
#endif

//
// Policy based Pointer Reference for pcontainer element
//
// Objectives:
//	- It keeps referece of an element value of pcontainer by pointer member 'm_p'
//	  or error status ( m_p == nullptr )
//	- Once it has been constructed, the referenced element is never been changed
//	  except the case of bad assignment which m_p will be nullptr (error status)
//	- It can use the referenced element by retrieval operation, including 
//	  operator*, oprator-> for pointer, and operator[]( size_t ) for native array
//	- Assiginment operation to the preferece acts as policy based assignment to
//	  the refered element itself (also act appropriate for pointer/native array)
//	- Cross-policy and const_/non const_ assignment is available
// 	- It can control constness of derefered value ( preference/const_preference )
// 
// Inconvenience:
//	- static_cast<value_type> is required when the preference is directly used
//	  as function argument.
//
// ============================================================================
template <typename T, Policy P>
class preference_base
{
    protected:
	typedef typename value_p<T>::t			  	value_type;
	typedef typename ccopy_p<T,P>::t			ccopy_pointer;

	value_type*		m_p;

    public:
	preference_base() : m_p( nullptr ) { }			// for error return
	preference_base( preference_base const& r ) : m_p( r.m_p ) { }
	preference_base& operator=( preference_base const& ) = delete;
	~preference_base() { };

	// for pcontainer internal use 			
	preference_base( value_type& p ) : m_p( &p ) { }

	// keyword 'explicit' is required avoiding ambiguity with value_type() operator in the case of integral T
	explicit operator bool() const				{ return  this->is_valid(); }

	bool operator==( preference_base<T,P> const& r )	{ return  this->eq( r ); }
	bool operator!=( preference_base<T,P> const& r )	{ return !this->eq( r ); }
	bool operator==( value_type const& p )			{ return  this->eq( p ); }
	bool operator!=( value_type const& p )			{ return !this->eq( p ); }

    protected:
    	// utilities
	value_type& get() const
	{
		if ( m_p == nullptr ) pctl_error( "derefered invalid preference" );
		return *m_p;
	}
	bool is_valid() const { return m_p != nullptr; }
	bool eq( preference_base const& r ) const { return m_p && r.m_p && *m_p == *r.m_p; }
	bool eq( value_type const& p ) const { return m_p && *m_p == p; }

	template <typename U, Policy Q>
	 void assign( preference_base<U,Q> const& r )
	{
		// duplication support for pStore

		if ( r.is_valid()) assign( policy<P,T>::copy( *r.m_p ));
		else {
			pctl_warning( "assignment of invalid preference" );

			if ( m_p ) {
				if ( *m_p ) policy<P,T>::del( *m_p );
				*m_p = typename value_p<T>::t();
				m_p = nullptr;
			}
		}
	}
	void assign( ccopy_pointer const& p )
	{
		// no duplication support for pStore

		if ( m_p ) {
			if ( *m_p != p ) {
				if ( *m_p ) policy<P,T>::del( *m_p );
				*m_p = policy<P,T>::set( p );
			}
		} else {
			pctl_error( "assignment for invalid preference" );
		}
	}

	template <typename U, Policy Q> friend class preference_base;
};

// ------------------------------------------------------------------------------
template <typename T, Policy P, size_t N = 0>
class preference : public preference_base<T,P>
{
	typedef typename preference_base<T,P>::value_type  	value_type;
	typedef typename preference_base<T,P>::ccopy_pointer	ccopy_pointer;

    public:
	preference()						= default;
	preference( preference const& )				= default;
	preference& operator=( preference const& r )		{ this->assign( r ); return *this; }
	~preference()						= default;

	preference( value_type& p )				: preference_base<T,P>( p ) { }
	operator value_type() const				{ return this->get(); }	
	value_type* operator&()					{ return this->m_p; }

	template <typename U, Policy Q, typename = std::enable_if_t<std::is_assignable<T&,U&>::value>>
	 preference& operator=( preference_base<U,Q> const& r )	{ this->assign( r ); return *this; }
	preference& operator=( ccopy_pointer const& p )		{ this->assign( const_cast<value_type const&>( p )); return *this; }
};

template <typename V, Policy P>
class preference<V*,P> : public preference_base<V*,P>
{
	typedef typename preference_base<V*,P>::value_type  	value_type;
	typedef typename preference_base<V*,P>::ccopy_pointer	ccopy_pointer;

    public:
	preference()						= default;
	preference( preference const& )				= default;
	preference& operator=( preference const& r )		{ this->assign( r ); return *this; }
	~preference()						= default;

	preference( value_type& p )				: preference_base<V*,P>( p ) { }
	operator value_type() const				{ return this->get(); }
	value_type* operator&()					{ return this->m_p; }

	template <typename U, Policy Q, typename = std::enable_if_t<std::is_assignable<V*&,U&>::value>>
	 preference& operator=( preference_base<U,Q> const& r )	{ this->assign( r ); return *this; }
	preference& operator=( ccopy_pointer const& p )		{ this->assign( p ); return *this; }

	V& operator*() const					{ return **this->m_p; }
	V* operator->() const					{ return  *this->m_p; }
};

template <typename V, Policy P>
class preference<V[],P> : public preference_base<V[],P>
{
	typedef typename preference_base<V[],P>::value_type  	value_type;
	typedef typename preference_base<V[],P>::ccopy_pointer	ccopy_pointer;

    public:
	preference()						= default;
	preference( preference const& )				= default;
	preference& operator=( preference const& r )		{ this->assign( r ); return *this; }
	~preference()						= default;

	preference( value_type& p )				: preference_base<V[],P>( p ) { }
	operator value_type() const				{ return this->get(); }
	value_type* operator&()					{ return this->m_p; }

	template <typename U, Policy Q, typename = std::enable_if_t<std::is_assignable<V*&,U&>::value>>
	 preference& operator=( preference_base<U,Q> const& r )	{ this->assign( r ); return *this; }
	preference& operator=( ccopy_pointer const& p )		{ this->assign( p ); return *this; }

	V& operator[]( size_t n ) const				{ return *( this->m_p )[ n ]; }
};

template <typename V, Policy P, size_t N>
class preference<V[N],P> : public preference_base<V[N],P>
{
	typedef typename preference_base<V[N],P>::value_type  	value_type;
	typedef typename preference_base<V[N],P>::ccopy_pointer	ccopy_pointer;

    public:
	preference()						= default;
	preference( preference const& )				= default;
	preference& operator=( preference const& r )		{ this->assign( r ); return *this; }
	~preference()						= default;

	preference( value_type& p )				: preference_base<V[N],P>( p ) { }
	operator value_type() const				{ return this->get(); }
	value_type* operator&()					{ return this->m_p; }

	template <typename U, Policy Q, typename = std::enable_if_t<std::is_assignable<V*&,U&>::value>>
	 preference& operator=( preference_base<U,Q> const& r )	{ this->assign( r ); return *this; }
	preference& operator=( ccopy_pointer const& p )		{ this->assign( p ); return *this; }

	V& operator[]( size_t n ) const				{ return ( *this->m_p )[ n ]; }
};

// ------------------------------------------------------------------------------
template <typename T, Policy P, size_t N = 0>
class const_preference : public preference_base<T,P>
{
	typedef typename preference_base<T,P>::value_type	value_type;

    public:
	const_preference()					= default;
	const_preference( const_preference const& )		= default;
	const_preference& operator=( const_preference const& )	= delete;
	~const_preference()					= default;

	const_preference( value_type const& p )			: preference_base<T,P>( const_cast<value_type&>( p )) { }
	operator value_type() const				{ return this->get(); }
	const value_type* operator&() const			{ return this->m_p; }
};

template <typename V, Policy P>
class const_preference<V*,P> : public preference_base<V*,P>
{
	typedef typename preference_base<V*,P>::value_type	value_type;

    public:
	const_preference()					= default;
	const_preference( const_preference const& )		= default;
	const_preference& operator=( const_preference const& )	= delete;
	~const_preference()					= default;

	const_preference( value_type const& p )			: preference_base<V*,P>( const_cast<value_type&>( p )) { }
	operator value_type() const				{ return this->get(); }
	const value_type* operator&() const			{ return this->m_p; }

	V& operator*() const					{ return **this->m_p; }
	V* operator->() const					{ return  *this->m_p; }
};

template <typename V, Policy P>
class const_preference<V[],P> : public preference_base<V[],P>
{
	typedef typename preference_base<V[],P>::value_type	value_type;

    public:
	const_preference()					= default;
	const_preference( const_preference const& )		= default;
	const_preference& operator=( const_preference const& )	= delete;
	~const_preference()					= default;

	const_preference( value_type const& p )			: preference_base<V[],P>( const_cast<value_type&>( p )) { }
	operator value_type() const				{ return this->get(); }
	const value_type* operator&() const			{ return this->m_p; }

	V& operator[]( size_t n ) const				{ return ( *this->m_p )[ n ]; }
};

template <typename V, Policy P, size_t N>
class const_preference<V[N],P> : public preference_base<V[N],P>
{
	typedef typename preference_base<V[N],P>::value_type	value_type;

    public:
	const_preference()					= default;
	const_preference( const_preference const& )		= default;
	const_preference& operator=( const_preference const& )	= delete;
	~const_preference()					= default;

	const_preference( value_type const& p )			: preference_base<V[N],P>( const_cast<value_type&>( p )) { }
	operator value_type() const				{ return this->get(); }
	const value_type* operator&() const			{ return this->m_p; }

	V& operator[]( size_t n ) const				{ return ( *this->m_p )[ n ]; }
};

// Difference of Iterator
// ============================================================================
template <typename It>
inline std::ptrdiff_t i_difference( It const& a, It const& b )	  // a - b
{
	std::ptrdiff_t		r = 0;
	It			i;

	if ( a && b ) {
		for ( i = b, r = 0; i; ++i, ++r )
			if ( i == a ) return r;
		for ( i = a, r = 0; i; ++i, --r )
			if ( i == b ) return r;
		throw std::invalid_argument( "different iterator origins" );
	} else if ( b ) {
		for ( i = b, r = 0; i; ++i, ++r );
	} else if ( a ) {
		for ( i = a, r = 0; i; ++i, --r );
	}
	return r;
}

#ifndef NO_NAMESPACE
}
#endif

#endif // __PCTL_PCONTAINER_H
