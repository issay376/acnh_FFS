//
// plist.h -- pointer based single linked list
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
//	04/01/2003	create
//	06/30/2003	plist version 1.1.0
//	08/18/2014	plist version 1.3.0, C++11 support
//	02/09/2017	plist version 1.3.1, native array pointer support
//	02/22/2017	plist version 1.3.11, c-string support as char[], *iterator returns preference
//	03/05/2017	plist version 1.3.12, C++14 style type_traits
//	02/05/2018	plist Version 2.0, C++11/14 support completed
//

#ifndef __PCTL_PLIST_H
#define __PCTL_PLIST_H

#ifndef PCTL_NAMESPACE
#define PCTL_NAMESPACE pctl
#endif

#include <pctl/pcontainer.h>

#ifndef NO_NAMESPACE
namespace PCTL_NAMESPACE {
#endif // NO_NAMESPACE

//
// Specification
//
// -----------------------------------------------------------------------------
template <typename T, Policy P = pRefer>
class plist
{
	struct item;

	item*	m_top = nullptr;
	item**	m_end = &m_top;

	typedef typename value_p<T>::t		VT;	// value_type shortcut, native array represent as a pointer
	typedef typename ccopy_p<T,P>::t	CP;	// const_if_pCopy shortcut
	typedef typename ccopy_a<T,P>::t	CN;	// native type of const_if_pCopy shortcut for template argument

    public:
	// types
	typedef VT				value_type;
	typedef policy<P,T>			value_policy;

	class 					iterator;
	class 					const_iterator;

	typedef preference<T,P>			reference;
	typedef const_preference<T,P>		const_reference;

	typedef size_t				size_type;
	typedef std::ptrdiff_t			difference_type;

	typedef typename const_p<T>::t		const_value_p;
	typedef CP				const_if_pCopy;
	typedef typename cnref_p<T,P>::t	const_not_pRefer;

	// constructor, assignment operator, destructor -----------------------
	//
	// Note:
	//	For the store-policy plist (P is pStore), copy constructor and assignment (including cross-policy),
	//      iterator constructor, and n-elements constructor duplicate pointed data automatically by dup_fn() or
	//	copy constructor of the data. 
	//	On the other hand, initializer_list constructor doesn't duplicate but just store pointed data, and
	//	the ownership of them will be transfered to plist like push() and enq().
	//	
	plist() { }

	plist( plist const& );
	plist( plist&& );
	plist& operator=( plist const& );
	plist& operator=( plist&& );

	~plist() { clear(); }

	// n-elements constructor
	explicit plist( size_t n, const_not_pRefer const& p = value_type());

	// cross-policy constructor and assignment operator
	template <Policy Q> plist( plist<T,Q>&& pl );
	template <Policy Q> plist& operator=( plist<T,Q>&& pl );

	// cross-pcontainer constructor and assignment operator
	template <typename PC, typename = std::enable_if_t< is_pcontainer<std::remove_reference_t<PC>>::value >>
	 plist( PC const& pc );
	template <typename PC, typename = std::enable_if_t< is_pcontainer<std::remove_reference_t<PC>>::value >>
	 plist& operator=( PC const& pc ) { return *this = plist( pc ); }

	// iterator constructor
        template <typename It, typename = std::enable_if_t< std::is_assignable<CP&, ivalue_t<It>>::value >>
	plist( It const& fst, It const& lst );

	// initializer support
	plist( std::initializer_list<VT> pl );
	plist& operator=( std::initializer_list<VT> pl ) { return *this = plist( pl ); }

	// stack and queue operations ------------------------------------------
	// 
	// Note:
	//	pop() and deq() will release the ownership of pointer from plist.
	//	You must delete (or delete []) the returned pointer of pop()/deq(), or
	//	use pop_front() if no need of the return.
	
	void push( CP const& dt );
	value_type pop();

	void enq( CP const& dt );
	value_type deq() { return pop(); }

	// std style container operations --------------------------------------

	// --- Modifier ---
	void pop_front() { policy<P,T>::del( pop() ); }
	// pop_back() is not offerd
	void push_front( CP const& x ) { push( x ); }
	void push_back( CP const& x )  { enq( x ); }

	// Note:
	//	For the store policy plist, pointed data duplication will be taken in the same way as constuctors.
	//	- 2nd argment of assign( n, T ) and iterated elements of assgin( fst, lst ) will be duplicated.
	//	- initializer list of assign( args ) will not be duplicated and pointer ownership will be transfered.
	//
	void assign( size_t n, const_not_pRefer const& t = value_type())
	{
		*this = plist<T,P>( n, t );
	}
	void assign( std::initializer_list<VT> args )
	{
		*this = plist<T,P>( args );
	}
        template <typename It, typename = std::enable_if_t< std::is_assignable<CP&, ivalue_t<It>>::value >>
	 void assign( It const& fst, It const& lst )
	{
		*this = plist<T,P>( fst, lst );
	}

	// Note:
	//	For the store-policy plist, insert( pos, pl ) will duplicate pointed data in ls using copy constructor.
	//	Other 'insert()'s will just store the pointed data, and the ownership will be transfered to the plist.
	//
	//	ex.
	//	plist<T,pStore>	pl;
	//	...
	//	pl.insert( pos, fst, lst );		// ownership of pointed data in [fst,lst) are transfered to pl
	//	pl.insert( pos, prlist<T>( fst, lst ));	// [fst,lst) will be duplicated without ownership transference
	//
	iterator insert( iterator pos, CP const& x );
	iterator insert( iterator pos, std::initializer_list<VT> ts )
	{
		plist<T,P> l( ts ); splice( pos, l ); return pos;
	}
        template <typename It, typename = std::enable_if_t< std::is_assignable<CP&, ivalue_t<It>>::value >>
	 iterator insert( iterator pos, It const& fst, It const& lst )
	 {
		plist<T,P> l( fst, lst ); splice( pos, l ); return pos;
	 }
	template <typename PC, typename = std::enable_if_t< is_pcontainer<std::remove_reference_t<PC>>::value >>
	 iterator insert( iterator pos, PC const& pl )
	 {
		plist<T,P> l( pl ); splice( pos, l ); return pos;
	 }

	// Note:
	//	For the pointer container, allocator and following emplace member-functions are not offered.
	//		emplace( iterator, Args... );
	//		emplace_front( Args... );
	//		emplace_back( Args... );
	//	The value containers, which value is emplaced in the element of container, need to be executed element
	//	allocation and value construction separately. It is the reason why allocator and emplace are required.
	//	The pointer containers, on the other hand, contain the pointers of data, and it's not necessary to execute
	//	construction without allocation for the pointed data.
	//
	// Note:
	//	If you use erase( pos ) in the loop by the iterator, please care about skipping after erasing.
	//
	//		for ( auto i = l.begin(); i != l.end(); ++i )		//
	//			if ( check( *i )) i = l.erase( i );		// oops! ++i skips an element after erase( i )
	//	----
	//		auto i = l.begin();					//
	//		while ( i != end() )					//
	//			if ( check( *i )) i = l.erase( i ); else ++i;	// this code checks all elements of l
	//
	// Note:
	//	Iterator pointed for from 2nd to last of eraced elements and next to elaced element will be invalid.
	//	Especially, erase( fst, lst ) makes 'lst' invalid. If you keep using them, assign return value like:
	//
	//		lst = erase( fst, lst );
	//
	iterator erase( iterator pos );
	iterator erase( iterator fst, iterator lst );

	void remove( const_value_p const& x ) { remove( x, eql_fn<T>()); }
	template <typename BinaryPredicate>
	 void remove( const_value_p const& x, BinaryPredicate pred );
	template <typename Predicate>
	 void remove_if( Predicate pred );

	void swap( plist& l );
	plist& reverse();
	void clear();

	// --- Accessor ---
	reference at( size_t n );
	const_reference at( size_t n ) const;
	reference operator[]( size_t n );
	const_reference operator[]( size_t n ) const;

	reference front();
	const_reference front() const;
	reference back();
	const_reference back() const;

	// --- Iterator ---
	iterator begin()	      { return iterator( &m_top ); }
	const_iterator begin() const  { return const_iterator( &m_top ); }
	const_iterator cbegin() const { return const_iterator( &m_top ); }
	
	iterator end()		    { return iterator( m_end ); }
	const_iterator end() const  { return const_iterator( m_end ); };
	const_iterator cend() const { return const_iterator( m_end ); };

	// --- Capacity ---
	bool empty() const { return m_top ? false : true; }
	operator bool() const { return m_top ? true : false; }
	// max_size() is not offered
	size_type size() const;		// Note: O(n) 
	void resize( size_t n, const_not_pRefer filler = value_type());

	// --- List operations ---
	template <typename U, Policy Q,
		  typename = std::enable_if_t<std::is_same<std::remove_cv_t<T>, std::remove_cv_t<U>>::value >>
	 bool operator==( plist<U,Q> const& pl ) const;	

	void unique() { unique( eql_fn<T>()); }
	template <typename BinaryPredicate> void unique( BinaryPredicate pred );

	void splice( iterator pos, plist<T,P>& src ) { splice( pos.hpos(), src ); }
	void splice( iterator pos, plist<T,P>& src, iterator frm ) { splice( pos.hpos(), src, frm.hpos()); }
	void splice( iterator pos, plist<T,P>& src, iterator fst, iterator lst )	
	{
		splice( pos.hpos(), src, fst.hpos(), lst.hpos());	// if &src == this, pos must be out of [fst,lst)
	}

	template <Policy Q> void append( plist<T,Q> const& r ) { plist<T,P> l( r ); splice( m_end, l ); }

	template <typename PC, typename = std::enable_if_t<is_pcontainer<PC>::value >>
	 plist<T,P>& operator+=( PC const& pc ) { plist<T,P> l( pc ); splice( end(), l ); return *this; }

	template <typename PC, typename = std::enable_if_t<is_pcontainer<PC>::value >>
	 plist<T,P> operator+( PC const& pl ) const { plist<T,P> r = *this; r += pl; return r; }

	// merge() and sort() is not supported
	//	please convert to/from and use psort<T,P>

	// std style iterator -----------------------
	//
	// Note: operator-> is offered but not work for the pointer/array elements.
	//
	//	std:
	//	     i->member			  ( Ok, equivalent of '( *i ).member' )
	//	     ( *i ).member   		  ( Ok, '*i' is value )
	//	pointer conteiner:
	//	     i->member			  ( Compile Error )
	//	     ( *i )->member  		  ( Ok, '*i' is pointer )
	//
        // Note: iterator::operator* doesn't return T&, but preference<T,P> with policy based memory management support.
        //      This allows users to assign a pointer to the element of pcontainer safely.
	//
	//	plist<V*,P>		lv = { v1, v2, ..., vn };
	//	plist<V*,P>::iterator	i = lv.begin();
	//	V			v = ...;
	//	V*			p = &v;
	//
	//	i = &p;		// Error: iterator manages only the element of pcontainer, and value assignment is not provided
	//	*i = p;		// Ok: The pointer is assigned via preference<V*,P>::operator=
	//	**i = v;	// Ok: If V has copy assignment operator, direct value will be assigned
	//
	class iterator_base
	{
	    protected:
		typedef typename plist<T,P>::item	item; 

		item**	m_hpos;

		iterator_base() : m_hpos( nullptr ) { };
		iterator_base( iterator_base const& ) = default;
                iterator_base( iterator_base&& ) = default;
                iterator_base& operator=( iterator_base const& ) = default;
                iterator_base& operator=( iterator_base&& ) = default;
                ~iterator_base() { }

		// plist internal use
		explicit iterator_base( item** h ) : m_hpos( h ) { }

		void inc() { if ( !is_term() ) m_hpos = &( *m_hpos )->m_next; }
		value_type& get() const { check(); return ( *m_hpos )->m_data; }
		value_type set( CP const& p ) { check(); return ( *m_hpos )->set( p ); } 

		bool eq( iterator_base const& i ) const { return m_hpos && m_hpos == i.m_hpos; }

		void check() const
		{
#ifdef _DEBUG
			if ( m_hpos == nullptr ) pctl_error( "uninitialized iterator" );
#endif
		}
		bool is_term() const { return m_hpos && *m_hpos == nullptr; }
	};

	class iterator : public iterator_base, public std::iterator<std::forward_iterator_tag, value_type>
	{
	     public:
		iterator() = default;
		iterator( iterator const& i ) : iterator_base( i ) { }
		iterator( iterator&& i ) : iterator_base( std::move( i )) { }
		iterator& operator=( iterator const& i ) { iterator_base::operator=( i ); return *this; }
		iterator& operator=( iterator&& i ) { iterator_base::operator=( std::move( i )); return *this; }
		~iterator() = default;

		reference operator*() { return reference( iterator_base::get()); }
		value_type get() const { return iterator_base::get(); }

		explicit operator bool() const { return !iterator_base::is_term(); }
		bool operator==( iterator_base const& i ) const { return  iterator_base::eq( i ); }
		bool operator!=( iterator_base const& i ) const { return !iterator_base::eq( i ); }

		iterator& operator++()
		{
			iterator_base::inc();
			return *this;
		}
		iterator operator++( int )
		{
			iterator r = *this;
			iterator_base::inc();
			return r;
		}
                iterator& operator+=( size_type n )
		{
			while ( !iterator_base::is_term() && n-- > 0 ) iterator_base::inc();
			return *this;
		}
		iterator operator+( size_type n ) const
		{
			iterator r = *this;
			r += n;
			return r;
		}
		difference_type operator-( const_iterator const& i ) const
		{
			return i_difference( static_cast<const_iterator const&>( *this ), i ); 
		}

		// only for non-pointer and non-array value: ex. prlist<std::pair<Fst,Snd>>
                VT*	  operator->() 	     { return &iterator_base::get(); }
                VT const* operator->() const { return &iterator_base::get(); }

	    private:
		explicit iterator( item** h ) : iterator_base( h ) { }
		item** hpos() { return iterator_base::m_hpos; }

		friend class plist;	// for erase, insert, and iterator_base
	};

	class const_iterator : public iterator_base, public std::iterator<std::input_iterator_tag, value_type>
	{
	     public:
                const_iterator() = default;
                const_iterator( const_iterator const& i ) : iterator_base( i ) { }
                const_iterator( const_iterator&& i ) : iterator_base( std::move( i )) { }
                const_iterator& operator=( const_iterator const& i ) { iterator_base::operator=( i ); return *this; }
                const_iterator& operator=( const_iterator&& i ) { iterator_base::operator=( std::move( i )); return *this; }
		~const_iterator() = default;

		// iterator -> const_iterator conversion and assignment
		const_iterator( iterator const& i ) : iterator_base( i ) { }
                const_iterator( iterator&& i ) : iterator_base( std::move( i )) { }
		const_iterator& operator=( iterator const& i ) { iterator_base::operator=( i ); return *this; }
                const_iterator& operator=( iterator&& i ) { iterator_base::operator=( std::move( i )); return *this; }

                const_reference operator*() const { return const_reference( iterator_base::get()); }
		const value_type get() const { return iterator_base::get(); }

		explicit operator bool() const { return !iterator_base::is_term(); }
		bool operator==( iterator_base const& i ) const { return  iterator_base::eq( i ); }
		bool operator!=( iterator_base const& i ) const { return !iterator_base::eq( i ); }

		const_iterator& operator++()
		{
			iterator_base::inc();
			return *this;
		}
		const_iterator operator++( int )
		{
			const_iterator r = *this;
			iterator_base::inc();
			return r;
		}
                const_iterator& operator+=( size_type n )
		{
			while ( !iterator_base::is_term() && n-- > 0 ) iterator_base::inc();
			return *this;
		}
		const_iterator operator+( size_type n ) const
		{
			const_iterator r = *this;
			r += n;
			return r;
		}
		difference_type operator-( const_iterator const& i ) const
		{
			return i_difference( *this, i ); 
		}

                // only for non-pointer and non-array value: ex. prlist<std::pair<Fst,Snd>>
                VT const* operator->() const { return &iterator_base::get(); }

	    private:
		explicit const_iterator( typename plist<T,P>::item* const* h )
			: iterator_base( const_cast<item**>( h ))
		{ }

		friend class plist;	// for erase, insert, and iterator_base
	};

    private:
	struct item
	{
		value_type	m_data;
		item*		m_next;

		item( CP dt, item* nx ) : m_data( policy<P,T>::set( dt )), m_next( nx ) { } 
		~item() { policy<P,T>::del( m_data ); }

		value_type set( CP const& p ) 
		{
			value_type	r = m_data;

			if ( p != m_data ) {
				r = policy<P,T>::del( m_data );
				m_data = policy<P,T>::set( p );
			}
			return r;
		}
		value_type release()
		{
			value_type	r = m_data;

			m_data = value_type();
			return r;
		}
	    private:
		item( item const& );
		item& operator=( item const& );
	};

	item* pnext2item( item** nx ) const
	{
		// nx must not be null, a little tricky implementation
		return ( item* )(( char* )nx - (( char* )&m_top->m_next - ( char* )m_top ));
	}

	item** find_nth( size_t n ) const
	{
		item**	h = const_cast<item**>( &m_top );

		for ( size_t i = 0; i < n && *h; ++i ) h = &( *h )->m_next;

		return h;
	}

	item** splice( item** d, plist<T,P>& src ) { return splice( d, src, &src.m_top, src.m_end ); }
	item** splice( item** d, plist<T,P>& src, item** s );
	item** splice( item** d, plist<T,P>& src, item** s, item** t );

#ifdef _DEBUG
    public:
	bool is_valid() const
	{
		bool r = false;

		if ( m_top == nullptr ) {
			r = ( &m_top == m_end );
		} else {
			item* const*    h = &m_top;

			while ( *h ) h = &(( *h )->m_next );
			r = ( h == m_end );
		}
		return r;
	}
    private:
#endif
	template <typename U, Policy Q> friend class plist;
};

// template alias
// -----------------------------------------------------------------------------
#ifndef PCTL_NO_TEMPLATE_ALIAS
template <typename T> using prlist = plist<T, pRefer>;
template <typename T> using pslist = plist<T, pStore>;
template <typename T> using pclist = plist<T, pCopy>;
#endif

//
// Implementation
//
// -----------------------------------------------------------------------------

// constructor/assignment operator
// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline plist<T,P>::plist( plist<T,P> const& pl )
	: m_top( nullptr ), m_end( &m_top )
{
	for ( value_type const& p : pl ) enq( policy<P,T>::copy( p ));
}

// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline plist<T,P>::plist( plist<T,P>&& pl )
	: m_top( nullptr ), m_end( &m_top )
{
	swap( pl );
}

// -----------------------------------------------------------------------------
template <typename T, Policy P>
template <Policy Q>
inline plist<T,P>::plist( plist<T,Q>&& pl )
{
	static_assert( !( P == pRefer && Q != pRefer ), "entry value refers a value of temporal object" );

	m_top = reinterpret_cast<item*>( pl.m_top );
	m_end = m_top ? reinterpret_cast<item**>( pl.m_end ) : &m_top;
	pl.m_top = nullptr;
	pl.m_end = &pl.m_top;

	if ( P != pRefer && Q == pRefer ) {
		for ( item* p = m_top; p; p = p->m_next ) p->m_data = policy<P,T>::set( policy<P,T>::copy( p->m_data ));
	}
}

// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline plist<T,P>& plist<T,P>::operator=( plist<T,P> const& pl )
{
	if ( &pl != this ) *this = plist<T,P>( pl );
	return *this;
}

// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline plist<T,P>& plist<T,P>::operator=( plist<T,P>&& pl )
{
	if ( &pl != this ) swap( pl );
	return *this;
}

// -----------------------------------------------------------------------------
template <typename T, Policy P>
template <Policy Q>
inline plist<T,P>& plist<T,P>::operator=( plist<T,Q>&& pl )
{
	static_assert( !( P == pRefer && Q != pRefer ), "entry value refers a value of temporal object" );

	clear();

	m_top = reinterpret_cast<item*>( pl.m_top );
	m_end = m_top ? reinterpret_cast<item**>( pl.m_end ) : &m_top;
	pl.m_top = nullptr;
	pl.m_end = &pl.m_top;
		
	if ( P != pRefer && Q == pRefer ) {
		for ( item* p = m_top; p; p = p->m_next ) p->m_data = policy<P,T>::set( policy<P,T>::copy( p->m_data ));
	}
	return *this;
}

// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline plist<T,P>::plist( size_t n, const_not_pRefer const& p )
	: m_top( nullptr ), m_end( &m_top )
{
	for ( size_t i = 0; i < n; ++i ) enq( policy<P,T>::copy( p ));
}

// -----------------------------------------------------------------------------
template <typename T, Policy P>
template <typename PC, typename>
inline plist<T,P>::plist( PC const& pc )
	: m_top( nullptr ), m_end( &m_top )
{
	for ( auto i = pc.begin(); i && i != pc.end(); ++i ) enq( policy<P,T>::copy( *i ));
}

// -----------------------------------------------------------------------------
template <typename T, Policy P>
template <typename It, typename>
inline plist<T,P>::plist( It const& fst, It const& lst )
	: m_top( nullptr ), m_end( &m_top )
{
	for ( auto i = fst; i && i != lst; ++i ) enq( policy<P,T>::copy( *i ));
}

// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline plist<T,P>::plist( std::initializer_list<VT> pl ) 
	: m_top( nullptr ), m_end( &m_top )
{
	for ( auto const& p : pl ) enq( p );
}

// plist members
// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline void plist<T,P>::push( CP const& dt )
{
	m_top = new item( dt, m_top );
	if ( !m_top->m_next ) m_end = &( m_top->m_next );
}

// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline typename plist<T,P>::value_type plist<T,P>::pop()
{
	value_type	r = value_type();

	if ( m_top ) {
		item* d = m_top;
		r = d->release();	// deactivate policy<P,T>::del indestructor of item
		m_top = d->m_next;
		if ( !m_top ) m_end = &m_top;
		delete d;
	}
	return r;
}

// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline void plist<T,P>::enq( CP const& dt )
{
	*m_end = new item( dt, nullptr );
	if ( !m_top ) m_top = *m_end;
	m_end = &(( *m_end )->m_next );
}

// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline typename plist<T,P>::size_type plist<T,P>::size() const
{
	size_t	r = 0;

	for ( item* p = m_top; p; p = p->m_next ) ++r;

	return r;
}

// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline void plist<T,P>::swap( plist<T,P>& pl )
{
	item*	top = m_top;
	m_top = pl.m_top;
	pl.m_top = top;

	item**	end = m_end;
	m_end = m_top ? pl.m_end : &m_top;
	pl.m_end = pl.m_top ? end : &pl.m_top;
}

// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline plist<T,P>& plist<T,P>::reverse()
{
	if ( item* p = m_top ) {

		m_top = nullptr;
		m_end = &p->m_next;
	
		while ( p ) {
			item* q = m_top;
			m_top = p;
			p = m_top->m_next;
			m_top->m_next = q;
		}
	}
	return *this;
}

// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline void plist<T,P>::clear()
{
	while ( item* p = m_top ) {
		m_top = p->m_next;
		delete p;
	}
	m_end = &m_top;
}

// utility operator
// -----------------------------------------------------------------------------
template <typename T, Policy P>
template <typename U, Policy Q, typename>
inline bool plist<T,P>::operator== ( plist<U,Q> const& pl ) const
{
	bool					r = false;

	if ( static_cast<const void*>( this ) == static_cast<const void*>( &pl )) {
		r = true;
	} else {
		typename plist<T,P>::const_iterator	i;
		typename plist<U,Q>::const_iterator	j;

		for ( i = begin(), j = pl.begin(); i != end() && j != pl.end(); ++i, ++j ) 
			if ( !eql_fn<T>()( *i, *j ) ) break;
		r = ( i == end() && j == pl.end() );
	}
	return r;
}

// at, operator[]
// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline typename plist<T,P>::reference plist<T,P>::at( size_t n )
{
	item*	p = *find_nth( n );

	if ( !p ) throw( std::out_of_range( "plist::at() over the length" ));

	return reference( p->m_data );
}

// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline typename plist<T,P>::const_reference plist<T,P>::at( size_t n ) const
{
	item*	p = *find_nth( n );

	if ( !p ) throw( std::out_of_range( "plist::at() const over the length" ));

	return const_reference( p->m_data );
}

// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline typename plist<T,P>::reference plist<T,P>::operator[]( size_t n )
{
	item* p = *find_nth( n );

	return p ? reference( p->m_data ) : reference();
}

// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline typename plist<T,P>::const_reference plist<T,P>::operator[]( size_t n ) const
{
	item* p = *find_nth( n );

	return p ? const_reference( p->m_data ) : const_reference();
}

// front, back
// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline typename plist<T,P>::reference plist<T,P>::front()
{
	return m_top ? reference( m_top->m_data ) : reference();
}

// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline typename plist<T,P>::const_reference plist<T,P>::front() const
{
	return m_top ? const_reference( m_top->m_data ) : const_reference();
}

// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline typename plist<T,P>::reference plist<T,P>::back()
{
	return m_top ? reference( pnext2item( m_end )->m_data ) : reference();
}

// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline typename plist<T,P>::const_reference plist<T,P>::back() const
{
	return m_top ? const_reference( pnext2item( m_end )->m_data ) : const_reference();
}

// insert
// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline typename plist<T,P>::iterator plist<T,P>::insert( typename plist<T,P>::iterator pos, CP const& x )
{
	item**	h = pos.hpos();
	bool	t = ( *h == nullptr );

	*h = new item( x, *h );
	if ( t ) m_end = &( *h )->m_next;

	return pos;
}

// erase
// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline typename plist<T,P>::iterator plist<T,P>::erase( typename plist<T,P>::iterator pos )
{
	plist<T,P>	l;

	return iterator( l.splice( &l.m_top, *this, pos.hpos()) );
}

// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline typename plist<T,P>::iterator plist<T,P>::erase( typename plist<T,P>::iterator fst, typename plist<T,P>::iterator lst )
{
	plist<T,P>	l;

	return ( fst != lst ) ? iterator( l.splice( &l.m_top, *this, fst.hpos(), lst.hpos()) ) : lst;
}

// remove, remove_if
// -----------------------------------------------------------------------------
template <typename T, Policy P>
template <typename BinaryPredicate>
inline void plist<T,P>::remove( const_value_p const& x, BinaryPredicate pred )
{
	auto i = begin();

	while ( i != end() ) {
		if ( pred( *i, x ) ) erase( i ); else ++i;
	}
}

// -----------------------------------------------------------------------------
template <typename T, Policy P>
template <typename Predicate>
inline void plist<T,P>::remove_if( Predicate pred )
{
	auto i = begin();

	while ( i != end() ) {
		if ( pred( *i ) ) erase( i ); else ++i;
	}
}

// resize
// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline void plist<T,P>::resize( size_t n, const_not_pRefer filler )
{
	item**	h = &m_top;

	for ( size_t i = 0; i < n; ++i ) {
		if ( *h == nullptr ) enq( policy<P,T>::copy( filler ));
		h = &( *h )->m_next;
	}
	while ( *h != nullptr ) {
		item*	p = *h;
		*h = p->m_next;
		delete p;
	}
	m_end = h;
}

// unique
// -----------------------------------------------------------------------------
template <typename T, Policy P>
template <typename BinaryPredicate>
inline void plist<T,P>::unique( BinaryPredicate pred )
{
	if ( item* p = m_top ) {
		while ( item* q = p->m_next ) {
			if ( pred( p->m_data, q->m_data )) {
				if ( !q->m_next ) m_end = &p->m_next;
				p->m_next = q->m_next;
				delete q;
			} else {
				p = q;
			}
		}
	}
}

// splice
// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline typename plist<T,P>::item**
	plist<T,P>::splice( plist<T,P>::item** d, plist<T,P>& src, plist<T,P>::item** s )
{
	if ( *s && *d != *s ) {
		item* p = ( *s )->m_next;
		( *s )->m_next = *d;
		if ( d == m_end ) m_end = &( *s )->m_next;
		*d = *s;
		*s = p;			
		if ( p == nullptr ) src.m_end = s;
	}
	return s;
}

// -----------------------------------------------------------------------------
template <typename T, Policy P>
inline typename plist<T,P>::item**
	plist<T,P>::splice( plist<T,P>::item** d, plist<T,P>& src, plist<T,P>::item** s, plist<T,P>::item** t )
{
#ifdef _DEBUG
{
	bool	in = false;
	bool	rg = false;
	item**	h;

	for ( h = &m_top; *h; h = &( *h )->m_next ) {
		if ( h == s ) rg = true;
		if ( h == t ) 
			if ( rg ) rg = false; else pctl_error( "plist::splice(): invalid [fst,lst)" );
		if ( h == d )
			if ( !rg ) in = true; else pctl_error( "plist::splice(): pos must be out of [fst,lst)" ); 
	}
	if ( !in && h != m_end ) pctl_error( "plist::splice(): target pos is not in src" );
}
#endif
	if ( *s && *d != *s ) {
		item*	p = *t;
		if ( *t ) {
			*t = *d;
			if ( d == m_end ) m_end = t;
		} else {
			*src.m_end = *d;
			if ( d == m_end ) m_end = src.m_end;
		}
		*d = *s;
		*s = p; 
		if ( p == nullptr ) src.m_end = s;
	}
	return s;
}

#ifndef NO_NAMESPACE
}
#endif

#endif // __PCTL_PLIST_H

