//
// Animal Crossing New Horizon - Plant Hybrid Simulator
//
//			2020/05/21
//	ver. 0.11	2020/06/13
//		
//	copyright(C) 2020 Kazunari Saitoh (issay376)
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY;  without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//	See the GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see <http://www.gnu.org/licenses/>.
//
 
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <cassert>
#include <string>
#include <initializer_list>
#include <utility>
#include <pctl/plist.h>

// flower gene, and hybrid operation
// -----------------------------------------------------------------------------
typedef unsigned char gene;		// Japanese local bit order: WW-YY-RR-SS (rose) / 00-WW-YY-RR (others)

gene cross( gene x, gene y )
{
	unsigned	h = rand() % 256;		// higher 4 bits for x, lower for y

	return (((( 0x80 & h ) ? 0x80 : 0x40 ) & x ) ? 0x80 : 0 ) |
	       (((( 0x08 & h ) ? 0x80 : 0x40 ) & y ) ? 0x40 : 0 ) |
	       (((( 0x40 & h ) ? 0x20 : 0x10 ) & x ) ? 0x20 : 0 ) |
	       (((( 0x04 & h ) ? 0x20 : 0x10 ) & y ) ? 0x10 : 0 ) |
	       (((( 0x20 & h ) ? 0x08 : 0x04 ) & x ) ? 0x08 : 0 ) |
	       (((( 0x02 & h ) ? 0x08 : 0x04 ) & y ) ? 0x04 : 0 ) |
	       (((( 0x10 & h ) ? 0x02 : 0x01 ) & x ) ? 0x02 : 0 ) |
	       (((( 0x01 & h ) ? 0x02 : 0x01 ) & y ) ? 0x01 : 0 );
}

// flower color
// -----------------------------------------------------------------------------
enum flowerColor { NA = 0, Wh, Ye, Rd, Or, Pk, Pu, Bu, Bk, Gr, YR, Gl };
	
constexpr char const* const colorString( flowerColor c )
{
	char const* const	s[ 12 ] = {
		"NA", "Wh", "Ye", "Rd", "Or", "Pk", "Pu", "Bu", "Bk", "Gr", "YR", "Gl"
	};
	return s[ c ];
}

// flower type
// -----------------------------------------------------------------------------
enum flowerType { Cv = 0, Ro, Hy, Li, An, Pa, Co, Ch, Tu, WL };

constexpr char const* const flowerTypeString( flowerType t )
{
	char const* const 	s[ 10 ] = {
		"N/A",
		"Rose", "Hyacinth", "Lily", "Anemone", "Pansie", "Cosmos", "Chrysanthemum", "Tulip", "Wild Lily"
	};
	return s[ t ];
}

constexpr const char flowerTypeChar( flowerType t )
{
	constexpr const char 	c[ 11 ] = { '-', 'R', 'H', 'L', 'A', 'P', 'C', 'M', 'T', 'G', 'W' };
	return c[ t ];
}

// number of watering
// -----------------------------------------------------------------------------
typedef unsigned char		watered;	// 0 .. 20
typedef unsigned char		visitor;	// 0 .. 5 

const watered MaxWaterCount = 20;
const visitor MaxVisitorCount = 5;

constexpr unsigned hybrid_ratio( watered w, visitor v )
{
	const unsigned visitor_bonus[ 6 ] = { 0, 20, 30, 45, 60, 75 };
	
	// may be over 100
	return visitor_bonus[ v ] + (( w < 3 ) ? 5 : ( w - 2 ) * 5 );
}

// flower glowth
// -----------------------------------------------------------------------------
typedef unsigned char		growth;		// 0 .. 3

constexpr unsigned char growthChar( growth n )
{
	const char 	c[ 4 ] = { '.', ',', '\'', '*' };	// sprout, stem, bud, flower
	return c[ n ];
}

// flower data class
// -----------------------------------------------------------------------------
class flower
{
    protected:
	const flowerType	m_type;
	const gene		m_gene;	
	const flowerColor	m_color;	// Woops! Gold Rose is Rose(0000)!
	watered			m_water;	// 0 .. 20
	visitor			m_visitor;	// 0 .. 5
	growth			m_growth;	// 0 .. 3 
	const bool		m_clone;
	bool			m_hydro;
	bool			m_goldflag;
	bool			m_partner;

	// static members
	static pctl::plist<flower*, pctl::pStore> m_pool;

	// constructor
	flower( flowerType t, gene g, flowerColor c, growth growth, bool clone = false )
		: m_type( t ), m_gene( g ), m_color( c ), m_water( 0 ), m_visitor( 0 ),
		  m_growth( growth ), m_clone( clone ),
		  m_hydro( false ), m_goldflag( false ), m_partner( false ) { }

    public:
	virtual ~flower() { };
	
	gene get_gene() const		{ return m_gene; }
	flowerColor get_color() const	{ return m_color; }

	virtual bool is_samekind( const flower* f ) const = 0;
	virtual bool is_goldrose() const { return false; } 
	virtual bool is_rollable_goldrose() const { return false; }

	// user acitivities
	void hydrate( visitor n = 0 )
	{
		m_hydro = true;
		m_visitor = ( MaxVisitorCount < n ) ? MaxVisitorCount : n;
	}
	void hydrateGold( visitor n = 0 )
	{
		hydrate( n );
		m_goldflag = true;
	}

	// daily refresh process
	void daily_init()		{ m_partner = true; }
	void grow()			{ if ( m_growth < 3 ) ++m_growth; }
	void increment_water() 		{ if ( m_water < MaxWaterCount ) ++m_water; }
	void reset_after_goldrose()	{ m_goldflag = false; }
	void reset_after_hybrid()	{ m_partner = false; m_water = 0; }
	void daily_reset()		{ m_hydro = false; m_visitor = 0; }

	bool is_hydrate() const { return m_hydro; }
	bool is_applicable() const
	{
		return ( is_partner() && is_growth( 3 ) && !is_goldrose() && 
			 hybrid_ratio( m_water, m_visitor ) > ( rand() % 100 ));
	}

	bool is_growth( growth n ) const { return m_growth == n; }
	bool is_partner() const { return m_partner; }
	bool is_color( flowerColor c ) const { return m_color == c; }
	bool is_color( flower* f ) const { return m_color == f->m_color; }

	virtual flower* hybrid( const flower* f = nullptr ) = 0;	// generate a flower and return the owner pointer
	const bool is_clone() const { return m_clone; }

	virtual void print( bool gn = false, bool cr = false, FILE* f = stderr );
	virtual void print_gene( FILE* f = stderr );

	// static methonds
	static void clear_pool() { m_pool.clear(); }
	static constexpr const unsigned flower_no( const gene g )
	{
		return (( g >> 2 ) ? flower_no( g >> 2 ) * 3 : 0 ) +
			 ((( g & 3 ) == 3 ) ? 2 : (( g & 3 ) == 0 ) ? 0 : 1 );
	}
};

pctl::plist<flower*, pctl::pStore> flower::m_pool;
	
// -----------------------------------------------------------------------------
inline void flower::print( bool gn, bool cr, FILE* f )
{
	fprintf( f, "%c%c%c%s", is_partner() ? ' ' : '>',
				flowerTypeChar( m_type ),
				growthChar( m_growth ),
				colorString( m_color ));
	if ( gn ) {
		putc( '[', f );
		print_gene( f );
		putc( ']', f );
	}
	if ( cr ) putc( '\n', f );
}
		
// -----------------------------------------------------------------------------
inline void flower::print_gene( FILE* f )
{
	// excepting rose

#ifdef __JLOCAL
	fprintf( f, " %c%c%c", 
		 (( 0x30 & m_gene ) == 0x30 ) ? '8' : (( 0x30 & m_gene ) == 0 ) ? '.' : 'o',
		 (( 0x0C & m_gene ) == 0x0C ) ? '8' : (( 0x0C & m_gene ) == 0 ) ? '.' : 'o',
		 (( 0x03 & m_gene ) == 0x03 ) ? '8' : (( 0x03 & m_gene ) == 0 ) ? '.' : 'o' );
#else
	// Global RYW style: 0x34(b00110100) -> "0012"
	fprintf( f, " %c%c%c", 
		 (( 0x03 & m_gene ) == 0x03 ) ? '2' : (( 0x03 & m_gene ) == 0 ) ? '0' : '1',
		 (( 0x0C & m_gene ) == 0x0C ) ? '2' : (( 0x0C & m_gene ) == 0 ) ? '0' : '1',
		 (( 0x30 & m_gene ) == 0x30 ) ? '2' : (( 0x30 & m_gene ) == 0 ) ? '0' : '1' );
#endif
}

// Rose
// -----------------------------------------------------------------------------
const gene gWhRose = 0x40;	// Shop
const gene gYeRose = 0x30;	// Shop
const gene gRdRose = 0x0D;	// Shop
const gene gPkRose = 0xCF;	// Mystery Island
const gene gOrRose = 0x7D;	// Mystery Island
const gene gGlRose = 0x00;

class rose : public flower
{
	static constexpr flowerColor color[ 81 ] = {
                                      Wh, Wh, Wh, Rd, Pk, Wh, Bk, Rd, Pk,
                                      Ye, Ye, Ye, Or, Ye, Ye, Or, Or, Ye,
                                      Ye, Ye, Ye, Or, Ye, Ye, Or, Or, Ye,
                                      Wh, Wh, Wh, Rd, Pk, Wh, Bk, Rd, Pk,
                                      Wh, Wh, Wh, Rd, Pk, Wh, Rd, Rd, Wh,  
                                      Ye, Ye, Ye, Or, Ye, Ye, Or, Or, Ye, 
                                      Pu, Pu, Pu, Rd, Pk, Pu, Bk, Rd, Pk,
                                      Pu, Pu, Pu, Rd, Pk, Pu, Bk, Rd, Pu,
                                      Wh, Wh, Wh, Rd, Pk, Wh, Bu, Rd, Wh
	};

	rose( gene g, growth growth, bool clone, bool golden )
		: flower( Ro, g, golden ? Gl : color[ flower_no( g ) ], growth, clone ) { }

    public:
	~rose() { }

	bool is_samekind( const flower* f ) const override
	{
		return dynamic_cast<const rose*>( f ) ? true : false;
	}
	bool is_goldrose() const override
	{
		return m_color == Gl;
	}
	bool is_rollable_goldrose() const override
	{
		return m_color == Bk && m_goldflag == true;
	}
	flower* hybrid( const flower* f = nullptr ) override
	{
		if ( is_samekind( f )) 
			return rose::create( cross( m_gene, f->get_gene()), 2 );
		else
			return rose::create( m_gene, 2, true );
	}
	void print_gene( FILE* f = stdout ) override;

	// static methods
	static flower* create( gene g, unsigned growth = 0, bool clone = false, bool golden = false )
	{
		flower* r = new rose( g, growth, clone, golden );
		m_pool.push_front( r );
		return r;
	}
};

inline void rose::print_gene( FILE* f )
{
#ifdef __JLOCAL
	fprintf( f, "%c%c%c%c", 
		 (( 0xC0 & m_gene ) == 0xC0 ) ? '8' : (( 0xC0 & m_gene ) == 0 ) ? '.' : 'o',
		 (( 0x30 & m_gene ) == 0x30 ) ? '8' : (( 0x30 & m_gene ) == 0 ) ? '.' : 'o',
		 (( 0x0C & m_gene ) == 0x0C ) ? '8' : (( 0x0C & m_gene ) == 0 ) ? '.' : 'o',
		 (( 0x03 & m_gene ) == 0x03 ) ? '*' : (( 0x03 & m_gene ) == 0 ) ? '.' : '+' );
#else
	// Global RYWS style: 0x34(b00110100) -> "1200"
	fprintf( f, "%c%c%c%c", 
		 (( 0x0C & m_gene ) == 0x0C ) ? '2' : (( 0x0C & m_gene ) == 0 ) ? '0' : '1',
		 (( 0x30 & m_gene ) == 0x30 ) ? '2' : (( 0x30 & m_gene ) == 0 ) ? '0' : '1',
		 (( 0xC0 & m_gene ) == 0xC0 ) ? '2' : (( 0xC0 & m_gene ) == 0 ) ? '0' : '1',
		 (( 0x03 & m_gene ) == 0x03 ) ? '2' : (( 0x03 & m_gene ) == 0 ) ? '0' : '1' );
#endif
}

// Hyacinth
// -----------------------------------------------------------------------------
const gene gWhHyacinth = 0x10;		// Shop
const gene gYeHyacinth = 0x0C;		// Shop
const gene gRdHyacinth = 0x13;		// Shop
const gene gPkHyacinth = 0x11;		// Island
const gene gOrHyacinth = 0x0D;		// Island
const gene gBuHyacinth = 0x07;		// Island

class hyacinth : public flower
{
	static constexpr flowerColor color[ 27 ] = {
					Wh, Rd, Rd, Ye, Or, Bu, Ye, Or, Pu,
					Wh, Pk, Rd, Ye, Ye, Rd, Ye, Ye, Pu,
					Bu, Wh, Rd, Wh, Ye, Rd, Ye, Ye, Pu
	};

	hyacinth( gene g, growth growth, bool clone ) : flower( Hy, g, color[ flower_no( g ) ], growth, clone ) { }

    public:
	~hyacinth() { }

        bool is_samekind( const flower* f ) const override
        {       
                return dynamic_cast<const hyacinth*>( f ) ? true : false;
        }
        flower* hybrid( const flower* f = nullptr ) override
        {       
		if ( is_samekind( f )) 
			return hyacinth::create( cross( m_gene, f->get_gene()), 2 );
		else
			return hyacinth::create( m_gene, 2, true );
        }
        
        // static methods
        static flower* create( gene g, unsigned growth = 0, bool clone = false )
        {       
                flower* r = new hyacinth( g, growth, clone );
                m_pool.push_front( r );
                return r;
        }
};

// Lily
// -----------------------------------------------------------------------------
const gene gWhLily = 0x30;		// Shop
const gene gYeLily = 0x0C;		// Shop
const gene gRdLily = 0x13;		// Shop
const gene gPkLily = 0x37;		// Island
const gene gOrLily = 0x1F;		// Island

class lily : public flower
{
	static constexpr flowerColor color[ 27 ] = {
					Wh, Rd, Bk, Ye, Or, Bk, Ye, Or, Or,
					Wh, Pk, Rd, Wh, Ye, Rd, Ye, Ye, Or,
					Wh, Wh, Pk, Wh, Ye, Pk, Wh, Ye, Wh
	};

        lily( gene g, growth growth, bool clone ) : flower( Li, g, color[ flower_no( g ) ], growth, clone ) { }
        
    public:
        ~lily() { }
        
        bool is_samekind( const flower* f ) const override
        {
                return dynamic_cast<const lily*>( f ) ? true : false;
        }
        flower* hybrid( const flower* f = nullptr ) override
        {
		if ( is_samekind( f )) 
			return lily::create( cross( m_gene, f->get_gene()), 2 );
		else
			return lily::create( m_gene, 2, true );
        }
        
        // static methods
        static flower* create( gene g, unsigned growth = 0, bool clone = false )
        {       
                flower* r = new lily( g, growth, clone );
                m_pool.push_front( r );
                return r;
        }
};

// Anemone
// -----------------------------------------------------------------------------
const gene gWhAnemone = 0x10;		// Shop
const gene gOrAnemone = 0x0C;		// Shop
const gene gRdAnemone = 0x03;		// Shop
const gene gPkAnemone = 0x1F;		// Island
const gene gBuAnemone = 0x31;		// Island

class anemone : public flower
{
	static constexpr flowerColor color[ 27 ] = {
					Wh, Rd, Rd, Or, Pk, Rd, Or, Or, Pk,
					Wh, Rd, Rd, Or, Pk, Rd, Or, Or, Pk,
					Bu, Bu, Pu, Bu, Pk, Pu, Or, Or, Pu
	};

        anemone( gene g, growth growth, bool clone ) : flower( An, g, color[ flower_no( g ) ], growth, clone ) { }
        
    public:
        ~anemone() { }
        
        bool is_samekind( const flower* f ) const override
        {
                return dynamic_cast<const anemone*>( f ) ? true : false;
        }
        flower* hybrid( const flower* f = nullptr ) override
        {
		if ( is_samekind( f )) 
			return anemone::create( cross( m_gene, f->get_gene()), 2 );
		else
			return anemone::create( m_gene, 2, true );
        }
        
        // static methods
        static flower* create( gene g, unsigned growth = 0, bool clone = false )
        {       
                flower* r = new anemone( g, growth, clone );
                m_pool.push_front( r );
                return r;
        }
};

// Pansie
// -----------------------------------------------------------------------------
const gene gWhPansie = 0x10;		// Shop
const gene gYePansie = 0x0C;		// Shop
const gene gRdPansie = 0x03;		// Shop
const gene gBuPansie = 0x31;		// Island
const gene gYRPansie = 0x1F;		// Island

class pansie : public flower
{
	static constexpr flowerColor color[ 27 ] = {
					Wh, Rd, Rd, Ye, YR, Rd, Ye, Ye, YR,
					Wh, Rd, Rd, Ye, YR, Rd, Ye, Ye, YR,
					Bu, Bu, Pu, Bu, YR, Pu, Ye, Ye, Pu
	};

        pansie( gene g, growth growth, bool clone ) : flower( Pa, g, color[ flower_no( g ) ], growth, clone ) { }
        
    public:
        ~pansie() { }
        
        bool is_samekind( const flower* f ) const override
        {
                return dynamic_cast<const pansie*>( f ) ? true : false;
        }
        flower* hybrid( const flower* f = nullptr ) override
        {
		if ( is_samekind( f )) 
			return pansie::create( cross( m_gene, f->get_gene()), 2 );
		else
			return pansie::create( m_gene, 2, true );
        }
        
        // static methods
        static flower* create( gene g, unsigned growth = 0, bool clone = false )
        {       
                flower* r = new pansie( g, growth, clone );
                m_pool.push_front( r );
                return r;
        }
};

// Cosmos
// -----------------------------------------------------------------------------
const gene gWhCosmos = 0x10;		// Shop
const gene gYeCosmos = 0x1C;		// Shop
const gene gRdCosmos = 0x03;		// Shop
const gene gPkCosmos = 0x35;		// Island
const gene gOrCosmos = 0x17;		// Island

class cosmos : public flower
{
	static constexpr flowerColor color[ 27 ] = {
					Wh, Pk, Rd, Ye, Or, Or, Ye, Or, Bk,
					Wh, Pk, Rd, Ye, Or, Or, Ye, Or, Bk,
					Wh, Pk, Rd, Wh, Pk, Rd, Ye, Or, Rd
	};

        cosmos( gene g, growth growth, bool clone ) : flower( Co, g, color[ flower_no( g ) ], growth, clone ) { }
        
    public:
        ~cosmos() { }
        
        bool is_samekind( const flower* f ) const override
        {
                return dynamic_cast<const cosmos*>( f ) ? true : false;
        }
        flower* hybrid( const flower* f = nullptr ) override
        {
		if ( is_samekind( f )) 
			return cosmos::create( cross( m_gene, f->get_gene()), 2 );
		else
			return cosmos::create( m_gene, 2, true );
        }
        
        // static methods
        static flower* create( gene g, unsigned growth = 0, bool clone = false )
        {       
                flower* r = new cosmos( g, growth, clone );
                m_pool.push_front( r );
                return r;
        }
};

// Chrysanthemum
// -----------------------------------------------------------------------------
const gene gWhMum = 0x10;		// Shop
const gene gYeMum = 0x0C;		// Shop
const gene gRdMum = 0x03;		// Shop
const gene gPuMum = 0x17;		// Island
const gene gPkMum = 0x35;		// Island

class mum : public flower
{
	static constexpr flowerColor color[ 27 ] = {
					Wh, Pk, Rd, Ye, Ye, Pu, Ye, Pu, Gr,
					Wh, Pk, Rd, Ye, Rd, Pu, Ye, Pu, Gr,
					Pu, Pk, Rd, Wh, Pk, Rd, Ye, Pu, Rd
	};

        mum( gene g, growth growth, bool clone ) : flower( Ch, g, color[ flower_no( g ) ], growth, clone ) { }
        
    public:
        ~mum() { }
        
        bool is_samekind( const flower* f ) const override
        {
                return dynamic_cast<const mum*>( f ) ? true : false;
        }
        flower* hybrid( const flower* f = nullptr ) override
        {
		if ( is_samekind( f )) 
			return mum::create( cross( m_gene, f->get_gene()), 2 );
		else
			return mum::create( m_gene, 2, true );
        }
        
        // static methods
        static flower* create( gene g, unsigned growth = 0, bool clone = false )
        {       
                flower* r = new mum( g, growth, clone );
                m_pool.push_front( r );
                return r;
        }
};

// Tulip
// -----------------------------------------------------------------------------
const gene gWhTulip = 0x10;		// Shop
const gene gYeTulip = 0x0C;		// Shop
const gene gRdTulip = 0x13;		// Shop
const gene gPkTulip = 0x11;		// Island
const gene gOrTulip = 0x0D;		// Island
const gene gBkTulip = 0x07;		// Island

class tulip : public flower
{
	static constexpr flowerColor color[ 27 ] = {
					Wh, Rd, Bk, Ye, Or, Bk, Ye, Or, Pu,
					Wh, Pk, Rd, Ye, Ye, Rd, Ye, Ye, Pu,
					Wh, Wh, Rd, Wh, Ye, Rd, Ye, Ye, Pu
	};

        tulip( gene g, growth growth, bool clone ) : flower( Tu, g, color[ flower_no( g ) ], growth, clone ) { }
        
    public:
        ~tulip() { }
        
        bool is_samekind( const flower* f ) const override
        {
                return dynamic_cast<const tulip*>( f ) ? true : false;
        }
        flower* hybrid( const flower* f = nullptr ) override
        {
		if ( is_samekind( f )) 
			return tulip::create( cross( m_gene, f->get_gene()), 2 );
		else
			return tulip::create( m_gene, 2, true );
        }
        
        // static methods
        static flower* create( gene g, unsigned growth = 0, bool clone = false )
        {       
                flower* r = new tulip( g, growth, clone );
                m_pool.push_front( r );
                return r;
        }
};

// Wild Lily
// -----------------------------------------------------------------------------
class wildlily : public flower
{
	wildlily() : flower( WL, 0, Wh, 3 ) { }

    public:
	~wildlily() { }

        bool is_samekind( const flower* f ) const override
        {       
                return dynamic_cast<const wildlily*>( f ) ? true : false;
        }
	flower* hybrid( const flower* f = nullptr ) override
	{
		return nullptr;
	}

        // static methods
        static flower* create()
        {       
                flower* r = new wildlily();
                m_pool.push_front( r );
                return r;
        }
};

// flower_field
// -----------------------------------------------------------------------------
template <unsigned C>
struct coord 
{
	unsigned 	r;
	unsigned	c;

	constexpr coord() : r( 0 ), c( 0 ) { }
            	constexpr coord( const std::initializer_list<unsigned>& l ) : r( 0 ), c( 0 )
	{
		auto i = l.begin();

		if ( i != l.end()) {
			r = *i++;
			if ( i != l.end()) c = *i;
		}
	}

	constexpr unsigned index() const { return c + r * C; }

	bool operator==( const coord& t ) { return r == t.r && c == t.c; }
	bool operator!=( const coord& t ) { return !( *this == t ); }

	void print( FILE* f = stderr ) const { fprintf( f, "{%u,%u}", r, c ); }
};

// -----------------------------------------------------------------------------
template <unsigned R, unsigned C> class planting_plan;
template <unsigned R, unsigned C> class harvesting_plan;

// -----------------------------------------------------------------------------
class ff_base
{
	// virtual base class for flower_field<R,C>

    protected:
	std::string	m_name;
	unsigned 	m_fstep;

	static bool	m_verbose;

    public:
	ff_base( const char* n ) : m_name( n ), m_fstep( 0 ) { }
	virtual ~ff_base() { }

	// re-initialize the field for new simulation steps
	virtual void recycle() { m_fstep = 0; }

	// plant queue I/F
	virtual bool put( flower* f ) = 0;
	virtual bool is_open() const = 0;

	// simulation steps
	virtual void harvest() = 0;
	virtual void plant() = 0;
	virtual void hydrate( visitor n = 0 ) = 0;
	virtual void daily_refresh() = 0;

	// field I/F
	virtual unsigned count() const = 0;
	virtual unsigned count( flowerColor c ) const = 0;
	//virtual const unsigned field_units() const = 0;
	virtual const flower* const* begin() const = 0;
	virtual const flower* const* end() const = 0;
	virtual const flower* const* begin( unsigned r ) const = 0;
	virtual const flower* const* end( unsigned r ) const = 0;

	// misc.
	const char* name() const { return m_name.c_str(); }
	unsigned steps() const { return m_fstep; }
	virtual void print( bool w_gene = false ) const = 0;

	// static member functions
	static void verbose( bool v ) { m_verbose = v; }
};

bool ff_base::m_verbose = false;

template <unsigned R, unsigned C>
class flower_field : public ff_base
{
    protected:
	class plant_queue
	{
		// with planting order which controls flower position in the field.
		//
		// m_pos <= m_cur <= m_size
		// 	put operation increments m_cur
		// 	get(plant) operation increments m_pos, then [m_pos..m_cur) is active queue.

		flower_field<R,C>*	m_owner;
		coord<C>		m_order[ R * C ];
		flower*			m_queue[ R * C ];
		unsigned 		m_size;
		unsigned 		m_cur;
		unsigned 		m_pos;

	    public:
		plant_queue( flower_field<R,C>* f, const std::initializer_list<coord<C>>& l )
			: m_owner( f ), m_size( 0 ), m_cur( 0 ), m_pos( 0 )
		{
			for ( const coord<C> c : l ) set_order( c ); 
		}

		void set_order( const coord<C>& c )
		{
			if ( m_size < R * C && c.r < R && c.c < C ) m_order[ m_size++ ] = c;
		}

		unsigned open()
		{
			if ( is_open()) clean();
			m_cur = m_pos = 0;

			return m_size;
		}
		bool is_open() const { return m_cur < m_size; }
		void close() { clean(); m_pos = m_cur = m_size; }

		bool put( flower* f )
		{
			if ( m_cur < m_size ) {
				m_queue[ m_cur++ ] = f;
				return true;
			} else {
				return false;
			}
		}
		unsigned plant()
		{
			unsigned	n = m_cur - m_pos;
	
			while ( m_pos < m_cur ) {
				m_owner->get( m_order[ m_pos ] ) = m_queue[ m_pos ];
				++m_pos;
			}
			return n;
		}
		flower* pick()
		{
			flower*		r = nullptr;

			if ( 0 < m_pos ) {
				flower*& f = m_owner->get( m_order[ --m_pos ] );	// don't use queued old flower
				r = f; 
				f = nullptr;
			}
			return r;
		}
		void clean()
		{
			if ( m_pos < m_cur ) {
				if ( m_verbose ) fprintf( stderr, "Warning: queued %u flowers unused\n", m_cur - m_pos );
				m_pos = m_cur;
			}
		}

		unsigned size() const { return m_size; }
		unsigned count() const { return m_cur; }

		// iterator ---------------------------------------------------
		typedef const coord<C>* const_iterator;
	
		const_iterator	begin() const { return &m_order[ 0 ]; }
		const_iterator	end() const { return &m_order[ m_size ]; }
	};

	class harvest_plan
	{
	    public:
		struct hp_case {
			flowerColor	color;
			ff_base*	field;
		};
		static const unsigned MaxCases = 8;

	    private:
		hp_case		m_cases[ MaxCases ];
		hp_case*	m_pos;

	    public:
		harvest_plan() : m_pos( &m_cases[ 0 ] ) { }

		void set( flowerColor fc, ff_base* ff )
		{
			if ( m_pos < &m_cases[ MaxCases ] ) {
				m_pos->color = fc;
				m_pos->field = ff;
				++m_pos;
			} else {
				fprintf( stderr, "Warning: too many harvest plans\n" );
			}
		}
				
		// iterator ---------------------------------------------------
		typedef hp_case* 	iterator;
		typedef const hp_case*	const_iterator;

		iterator	begin() { return &m_cases[ 0 ]; }
		const_iterator	begin() const { return &m_cases[ 0 ]; }
		iterator	end() { return m_pos; }
		const_iterator	end() const { return m_pos; }
	};

	class random_list
	{
		coord<C>	m_list[ R * C ];
		int		m_keys[ R * C ];
		unsigned	m_pos;

	    public:
		random_list() : m_pos( 0 ) { }

		void put( int k, const coord<C>& c )
		{
			if ( m_pos < R * C ) {
				bool	s = false;

				for ( unsigned i = m_pos; 0 < i; --i ) {
					if ( k < m_keys[ i - 1 ] ) {
						m_keys[ i ] = m_keys[ i - 1 ];
						m_list[ i ] = m_list[ i - 1 ];
					} else {
						m_keys[ i ] = k;
						m_list[ i ] = c;
						s = true;
						break;
					}
				}
				if ( !s ) {
					m_keys[ 0 ] = k;
					m_list[ 0 ] = c;
				}
				++m_pos;
			}
		}
		void clear() { m_pos = 0; }
				
		// iterator ---------------------------------------------------
		typedef coord<C>* 	iterator;
		typedef const coord<C>*	const_iterator;

		iterator	begin() { return &m_list[ 0 ]; }
		const_iterator	begin() const { return &m_list[ 0 ]; }
		iterator	end() { return &m_list[ m_pos ]; }
		const_iterator	end() const { return &m_list[ m_pos ]; }
	};

	typedef pctl::plist<coord<C>>		 hybrid_list;

	// member variables
	flower*		m_field[ R * C ];
	bool		m_cover[ R * C ];
	hybrid_list	m_generation;

    public:
	flower_field( const char* n ) : ff_base( n )
	{
		for ( unsigned i = 0; i < R * C; ++i ) {
			m_field[ i ] = nullptr;
			m_cover[ i ] = false;
		}
	}
	~flower_field() { }

	void recycle() override
	{
		for ( unsigned i = 0; i < R * C; ++i ) {
			m_field[ i ] = nullptr;
			m_cover[ i ] = false;
		}
		m_generation.clear();				// for fail safe
		ff_base::recycle();
	}

	// plant queue I/F
	bool put( flower* f ) override { return false; }	
	bool is_open() const override { return false; }
 
	// simulation steps - defaults
	void harvest() override { }
	void plant() override { }
	void hydrate( visitor n = 0 ) override;
	void daily_refresh() final;	// increment m_fstep here

	// flower field I/F
	flower* get( const coord<C>& pos ) const { return m_field[ pos.index() ]; }
	// flower*& get( const coord<C>& pos ) is protected
	const flower* get( unsigned ix ) const { return m_field[ ix ]; }
	void set( flower* f, const coord<C>& pos )
	{
		if ( pos.c < C && pos.r < R && !covered( pos ))
		m_field[ pos.index() ] = f;
	}
	flower* pull( const coord<C>& pos );
	void cover( const coord<C>& pos ) { m_cover[ pos.index() ] = true; }
	bool covered( const coord<C>& pos ) const { return m_cover[ pos.index() ]; }
	void hydrate( visitor n, const coord<C>& pos ) { if ( flower* p = get( pos )) p->hydrate( n ); }

	unsigned count() const override;
	unsigned count( flowerColor c ) const override;
	constexpr const unsigned field_units() const { return ( R / 6 + 1 ) * ( C / 4 + 1 ); }
	const flower* const* begin() const override { return &m_field[ 0 ]; }			// iterator
	const flower* const* end() const override { return &m_field[ R * C ]; }
	const flower* const* begin( unsigned r ) const override					// row iterator
	{
		return ( r < R ) ? &m_field[ r * C ] : nullptr;
	}
	const flower* const* end( unsigned r ) const override
	{
		return ( r < R ) ? &m_field[ r * C + C ] : nullptr;
	}

	// for debug and test
	void print( bool w_gene = false ) const override;

    protected:
	flower*& get( const coord<C>& pos ) { return m_field[ pos.index() ]; }
	coord<C>* planting_space( coord<C> c );	// return ptr to elem of m_around, or nullptr if no planting space
	coord<C>* search_for_partner( flower* p, coord<C> c );
};

// member of flower_field<R,C>
// -----------------------------------------------------------------------------
template <unsigned R, unsigned C>
void flower_field<R,C>::hydrate( visitor n )
{
	for ( unsigned i = 0; i < R * C; ++i ) {
		if ( flower* fp = m_field[ i ] ) fp->hydrate( n );
	}
}

// -----------------------------------------------------------------------------
template <unsigned R, unsigned C>
void flower_field<R,C>::daily_refresh()
{
	m_generation.clear();

	random_list	shuffle;

	// shuffle process order
	for ( unsigned r = 0; r < R; ++r ) {
		for ( unsigned c = 0; c < C; ++c ) {
			flower* fp = get( { r, c } );
			if ( fp != nullptr ) {
				fp->daily_init();
				shuffle.put( std::rand(), { r, c } );
			}
		}
	}

	// loop
	for ( coord<C> cf : shuffle ) {
		flower* f = get( cf );

		f->grow();
		if ( f->is_hydrate()) {
			f->increment_water();
			if ( f->is_applicable()) {
				if ( coord<C>* ps = planting_space( cf )) {
					coord<C>* pc = search_for_partner( f, cf );
					flower*   p = pc ? get( *pc ) : nullptr;
					flower*   hy;
					if (( f->is_rollable_goldrose() || ( p && p->is_rollable_goldrose())) &&
					      std::rand() % 100 < 50 ) {
						hy = rose::create( gGlRose, 2, false, true );	// Gold Rose Bud
						f->reset_after_goldrose();
						if ( p ) p->reset_after_goldrose();
					} else {
						hy = f->hybrid( p );
					}

					if ( m_verbose ) {
						fprintf( stderr, "*** %c:", this->name()[ 0 ]);
						f->print( true );
						cf.print();
						if ( p ) {
							fprintf( stderr, " x " );
							p->print( true );
							pc->print();
						}
						fprintf( stderr, " => " );
						hy->print( true );
						ps->print();
					}

					set( hy, *ps );
					m_generation.push_back( *ps );

					f->reset_after_hybrid();
					if ( p ) p->reset_after_hybrid();
				}
			}
		}
		f->daily_reset();
	}
	++m_fstep;
}

// -----------------------------------------------------------------------------
template <unsigned R, unsigned C>
flower* flower_field<R,C>::pull( const coord<C>& pos )
{
	flower*	r = get( pos );

	if ( r ) set( nullptr, pos );

	return r;
}

// -----------------------------------------------------------------------------
template <unsigned R, unsigned C>
unsigned flower_field<R,C>::count() const
{
	unsigned n = 0;

	for ( unsigned i = 0; i < R * C; ++i )
		if ( m_field[ i ] != nullptr ) ++n;

	return n;
}

// -----------------------------------------------------------------------------
template <unsigned R, unsigned C>
unsigned flower_field<R,C>::count( flowerColor c ) const
{
	unsigned n = 0;

	for ( unsigned i = 0; i < R * C; ++i )
		if ( m_field[ i ] && m_field[ i ]->get_color() == c ) ++n;

	return n;
}

// -----------------------------------------------------------------------------
template <unsigned R, unsigned C>
coord<C>* flower_field<R,C>::planting_space( coord<C> cf )
{
	static coord<C>	around[ 8 ];

	unsigned	n = 0;
	unsigned char	cb = ( 0 < cf.c ) ? cf.c - 1 : 0;
	unsigned char	rb = ( 0 < cf.r ) ? cf.r - 1 : 0;
	unsigned char	ce = ( cf.c + 1 < C ) ? cf.c + 2 : C;
	unsigned char	re = ( cf.r + 1 < R ) ? cf.r + 2 : R;

	for ( unsigned char c = cb; c < ce; ++c ) {
		for ( unsigned char r = rb; r < re; ++r ) {
			if ( !covered( { r, c } ) && !( c == cf.c && r == cf.r ) && get( { r, c } ) == nullptr )
				around[ n++ ] = { r, c };
		}
	}
	return ( 0 < n ) ? &around[ rand() % n ] : nullptr;
}

// -----------------------------------------------------------------------------
template <unsigned R, unsigned C>
coord<C>* flower_field<R,C>::search_for_partner( flower* p, coord<C> cf )
{
	static coord<C>	around[ 8 ];

	unsigned char	cb = ( 0 < cf.c ) ? cf.c - 1 : 0;
	unsigned char	rb = ( 0 < cf.r ) ? cf.r - 1 : 0;
	unsigned char	ce = ( cf.c + 1 < C ) ? cf.c + 2 : C;
	unsigned char	re = ( cf.r + 1 < R ) ? cf.r + 2 : R;
	unsigned	n = 0;

	for ( unsigned char c = cb; c < ce; ++c ) {
		for ( unsigned char r = rb; r < re; ++r ) {
			if ( !covered( { r, c } ) && !( c == cf.c && r == cf.r )) {
				if ( flower* f = get( { r, c } )) {
					if ( f->is_growth( 3 ) && f->is_samekind( p ) && f->is_partner())
						around[ n++ ] = { r, c };
				}
			}
		}
	}
	return ( n == 0 ) ? nullptr : ( n == 1 ) ? &around[ 0 ] : &around[ rand() % n ];
}

// -----------------------------------------------------------------------------
template <unsigned R, unsigned C>
void flower_field<R,C>::print( bool w_gene ) const
{
	fprintf( stderr, "Flower Field %s (%ux%u)\n", name(), R, C );
	for ( unsigned char r = 0; r < R; ++r ) {
		for ( unsigned char c = 0; c < C; ++c ) {
			//putc( ' ', stderr );
			if ( flower* p = get( { r, c } )) 
				p->print( false );
			else if ( covered( { r, c } ))
				fprintf( stderr, " xxxx" );
			else
				fprintf( stderr, " -   " );
		}
		putc( '\n', stderr );
		if ( w_gene ) {
			for ( unsigned char c = 0; c < C; ++c ) {
				putc( ' ', stderr );
				if ( flower* p = get( { r, c } )) 
					p->print_gene();
				else
					fprintf( stderr, "    " );
			}
			putc( '\n', stderr );
		}
	}
}

//
// Stock Type Flower Fields: self harvesting, picking if required
// -----------------------------------------------------------------------------

// ff_storageN (for flower with the gene which will not be changed by hybrid)  
// -----------------------------------------------------------------------------
template <unsigned C>
class ff_storage : public flower_field<5,C>
{
	typedef flower_field<5,C>	super;

	typename super::plant_queue	m_pq;

    public:
	ff_storage( const char* nm ) : flower_field<5,C>( nm ), m_pq( this, { } )
	{
		for ( unsigned i = 0; i < C; i += 2 ) {
			m_pq.set_order( { 2, i } );
			m_pq.set_order( { 1, i } );
			m_pq.set_order( { 3, i } );
			m_pq.set_order( { 4, i } );
			m_pq.set_order( { 0, i } );
		}
		for ( unsigned i = 1; i < C; i += 2 ) m_pq.set_order( { 2, i } );
		for ( unsigned i = 1; i < C; i += 2 ) m_pq.set_order( { 0, i } );
		for ( unsigned i = 1; i < C; i += 2 ) m_pq.set_order( { 4, i } );
		for ( unsigned i = 1; i < C; i += 2 ) m_pq.set_order( { 3, i } );
		for ( unsigned i = 1; i < C; i += 2 ) m_pq.set_order( { 1, i } );
		m_pq.open();
	}
	~ff_storage() { m_pq.close(); }

	void recycle() override
	{
		m_pq.close();
		super::recycle();
		m_pq.open();
	}

	bool put( flower* p ) override { return m_pq.put( p ); }
	bool is_open() const override { return m_pq.is_open(); }

	void plant() override { m_pq.plant(); }
	void harvest() override
	{
		for ( auto i = super::m_generation.begin(); i != super::m_generation.end(); ++i ) {
			if ( !put( super::pull( *i ))) {
				if ( ff_base::m_verbose )
					fprintf( stderr, "Warning: ff_storage(%s) is full\n", super::name());
				break;
			}
		}
	}

	flower* pick() { return m_pq.pick(); }
};

typedef ff_storage<3>  ff_storage_1;
typedef ff_storage<7>  ff_storage_2;
typedef ff_storage<11> ff_storage_3;
typedef ff_storage<15> ff_storage_4;
typedef ff_storage<19> ff_storage_5;

// ff_isoratedN (for flower with the gene which may be changed by hybrid) 
// -----------------------------------------------------------------------------
template <unsigned C>
class ff_isolated : public flower_field<5,C>
{
	typedef flower_field<5,C>	super;

	typename super::plant_queue	m_pq;

   public:
	ff_isolated( const char* nm ) : flower_field<5,5>( nm ), m_pq( this, { } )
	{
		for ( unsigned i = 0; i < C; i += 2 ) {
			m_pq.set_order( { 2, i } );
			m_pq.set_order( { 4, i } );
			m_pq.set_order( { 0, i } );
		}
		m_pq.open();
	}
	~ff_isolated() { m_pq.close(); }

	void recycle() override
	{
		m_pq.close();
		super::recycle();
		m_pq.open();
	}

	bool put( flower* p ) override { return m_pq.put( p ); }
	bool is_open() const override { return m_pq.is_open(); }

	void plant() override { m_pq.plant(); }
	void harvest() override 
	{
		for ( auto i = super::m_generation.begin(); i != super::m_generation.end(); ++i ) {
			if ( !super::put( super::pull( *i ))) {
				if ( ff_base::m_verbose )
					fprintf( stderr, "Warning: ff_isolated(%s) is full\n", super::name());
				break;
			}
		}
	}

	flower* pick() { return m_pq.pick(); }
};	

typedef ff_isolated<3>  ff_isolated_1;
typedef ff_isolated<7>  ff_isolated_2;
typedef ff_isolated<11> ff_isolated_3;
typedef ff_isolated<15> ff_isolated_4;
typedef ff_isolated<19> ff_isolated_5;

// Hybrid Fields 
// -----------------------------------------------------------------------------

// ff_selfHybrid (hybrid by the same gene, no clones)	
// -----------------------------------------------------------------------------
template <unsigned C>
class ff_shbase : public flower_field<5,C>
{
   protected:
	typedef flower_field<5,C>	super;

	typename super::plant_queue	m_pq;
	typename super::harvest_plan	m_hp;

   public:
	ff_shbase( const char* nm ) : super( nm ), m_pq( this, { } ) { }
	~ff_shbase() { m_pq.close(); }

	void define_harvest( flowerColor c, ff_base* f ) { m_hp.set( c, f ); }

	void recycle() override
	{
		m_pq.close();
		super::recycle();
		m_pq.open();
	}

	bool put( flower* p ) override { return m_pq.put( p ); }
	bool is_open() const override { return m_pq.is_open(); }

	void plant() override { m_pq.plant(); }

	void hydrate( visitor n ) override
	{
		for ( auto i = m_pq.begin(); i != m_pq.end(); ++i ) {

			flower*		f = super::get( *i );

			if ( f && f->is_growth( 3 )) {	// don't hydrate immatrue flower for not creating clone 
				f->hydrate( n );
			} else {
				break;
			}
		}
	}

	void harvest() override 
	{
		for ( auto i = super::m_generation.begin(); i != super::m_generation.end(); ++i ) {

			flower* f = super::pull( *i );
			bool	hv = false;

			for ( auto p = m_hp.begin(); p != m_hp.end(); ++p ) {
				if ( f->is_color( p->color )) {
					hv = p->field->put( f );
					break;
				}
			}
			if ( !hv && ff_base::m_verbose ) {
				fprintf( stderr, "Info: harvest from %s, scrap ", super::name());
				f->print( true, true );
			}
		}
	}
};	

// -----------------------------------------------------------------------------
template <unsigned C>
class ff_selfHybrid : public ff_shbase<C>
{
	typedef ff_shbase<C>	super;

     public:
	ff_selfHybrid( const char* nm ) : super( nm ) 
	{
		for ( unsigned i = 0; i < C; i += 2 ) {
			super::m_pq.set_order( { 0, i } );
			super::m_pq.set_order( { 1, i } );
			super::m_pq.set_order( { 3, i } );
			super::m_pq.set_order( { 4, i } );
		}
		super::m_pq.open();
	}
	~ff_selfHybrid() { }

	void hydrate( visitor n ) override
	{
		// don't hydrate isolate or immatrue flower for not creating clone 

		auto i = super::m_pq.begin();

		while ( i != super::m_pq.end() && i + 1 != super::m_pq.end()) {

			flower*		f1 = super::get( *i++ );
			flower*		f2 = super::get( *i++ );

			if ( f1 && f1->is_growth( 3 ) && f2 && f2->is_growth( 3 )) {
				f1->hydrate( n );
				f2->hydrate( n );
			} else {
				break;
			}
		}
	}
};

typedef ff_selfHybrid<3>  ff_selfHybrid_1;
typedef ff_selfHybrid<7>  ff_selfHybrid_2;
typedef ff_selfHybrid<11> ff_selfHybrid_3;
typedef ff_selfHybrid<15> ff_selfHybrid_4;
typedef ff_selfHybrid<19> ff_selfHybrid_5;

// -----------------------------------------------------------------------------
template <unsigned C>
class ff_initialHybrid : public ff_shbase<C>
{
	// Pros.
	//    In the case of no visitor, aboud 1.5 production of ff_selfHybrid
	//
	// Cons.
	//    Only for the case the target flower is different color
	// 	ie. Red Rose(Shop) -> Black Rose
	//	    White Rose(Shop) -> Purple Rose
	//    If it's not be full planted, some clones will be made

	typedef ff_shbase<C>	super;

     public:
	ff_initialHybrid( const char* nm ) : super( nm ) 
	{
		for ( unsigned i = 0; i < C; i += 2 ) {
			super::m_pq.set_order( { 0, i } );
			if ( i + 1 < C ) super::m_pq.set_order( { 0, i + 1 } );
			super::m_pq.set_order( { 1, i } );
			super::m_pq.set_order( { 2, i } );
			if ( i + 1 < C ) super::m_pq.set_order( { 2, i + 1 } );
			super::m_pq.set_order( { 3, i } );
			super::m_pq.set_order( { 4, i } );
			if ( i + 1 < C ) super::m_pq.set_order( { 4, i + 1 } );
		}
		super::m_pq.open();
	}
	~ff_initialHybrid() { }
};

typedef ff_initialHybrid<3>  ff_initialHybrid_1;
typedef ff_initialHybrid<7>  ff_initialHybrid_2;
typedef ff_initialHybrid<11> ff_initialHybrid_3;
typedef ff_initialHybrid<15> ff_initialHybrid_4;
typedef ff_initialHybrid<19> ff_initialHybrid_5;

// ff_pairHybrid (hybrid 2 flowers with different gene, no clones)
// -----------------------------------------------------------------------------
template <unsigned C>
class ff_phbase : public flower_field<5,C>
{
    protected:
	typedef flower_field<5,C>	super;

	typename super::plant_queue	m_q1;
	typename super::plant_queue	m_q2;
	flowerColor			m_c1;
	flowerColor			m_c2;
	typename super::harvest_plan	m_hp;

   public:
	ff_phbase( const char* nm, flowerColor c1, flowerColor c2 )
		: super( nm ), m_q1( this, { } ), m_q2( this, { } ), m_c1( c1 ), m_c2( c2 )
	{ }
	~ff_phbase()
	{
		m_q2.close();
		m_q1.close();
	}

	void define_harvest( flowerColor c, ff_base* f ) { m_hp.set( c, f ); }

	void recycle() override
	{
		m_q2.close();
		m_q1.close();
		super::recycle();
		m_q1.open();
		m_q2.open();
	}

	bool put( flower* p ) override
	{
		bool	r = false;

		if      ( p->is_color( m_c1 )) r = m_q1.put( p );
		else if ( p->is_color( m_c2 )) r = m_q2.put( p );

		return r;
	}
	bool is_open() const override { return m_q1.is_open() || m_q2.is_open(); }

	void plant() override
	{
		m_q1.plant();
		m_q2.plant();
	}
	void hydrate( visitor n ) override
	{
		// don't hydrate solitary or immatrue flower for not creating clone 

		auto		i = m_q1.begin();
		auto		j = m_q2.begin();

		while ( i != m_q1.end() && j != m_q2.end()) {
			flower*		f1 = super::get( *i++ );
			flower*		f2 = super::get( *j++ );

			if ( f1 && f1->is_growth( 3 ) &&
			     f2 && f2->is_growth( 3 )) {
				f1->hydrate( n );
				f2->hydrate( n );
			} else {
				break;
			}
		}
	}
	void harvest() override 
	{
		for ( auto i = super::m_generation.begin(); i != super::m_generation.end(); ++i ) {

			flower* f = super::pull( *i );
			bool	hv = false;

			for ( auto p = m_hp.begin(); p != m_hp.end(); ++p ) {
				if ( f->is_color( p->color )) {
					hv = p->field->put( f );
					break;
				}
			}
			if ( !hv && ff_base::m_verbose ) {
				fprintf( stderr, "Info: harvest from %s, scrap ", super::m_name.c_str());
				f->print( false, true, stderr );
			}
		}
	}
};	

// -----------------------------------------------------------------------------
template <unsigned C>
class ff_pairHybrid : public ff_phbase<C>
{
	typedef ff_phbase<C>	super;

    public:
	ff_pairHybrid( const char* nm, flowerColor c1, flowerColor c2 ) : super( nm, c1, c2 )
	{
		for ( unsigned i = 0; i < C; i += 2 ) {
			super::m_q1.set_order( { 0, i } );
			super::m_q2.set_order( { 1, i } );
			super::m_q2.set_order( { 3, i } );
			super::m_q1.set_order( { 4, i } );
		}
		super::m_q1.open();
		super::m_q2.open();
	}
	~ff_pairHybrid() { }
};

typedef ff_pairHybrid<3>  ff_pairHybrid_1;
typedef ff_pairHybrid<7>  ff_pairHybrid_2;
typedef ff_pairHybrid<11> ff_pairHybrid_3;
typedef ff_pairHybrid<15> ff_pairHybrid_4;
typedef ff_pairHybrid<19> ff_pairHybrid_5;

// -----------------------------------------------------------------------------
template <unsigned C>
class ff_lineHybrid : public ff_phbase<C>
{
	// Pros.
	//    In the case of no visitor, aboud 1.2 production of ff_pairHybrid
	//
	// Cons.
	//    Only for the case the target flower color is not c1

	typedef ff_phbase<C>	super;

    public:
	ff_lineHybrid( const char* nm, flowerColor c1, flowerColor c2 ) : super( nm, c1, c2 )
	{
		for ( unsigned i = 0; i < C; i += 2 ) {
			super::m_q1.set_order( { 0, i } );
			super::m_q2.set_order( { 1, i } );
			super::m_q2.set_order( { 3, i } );
			super::m_q1.set_order( { 4, i } );
		}
		for ( unsigned i = 0; i < C; i += 2 ) {
			super::m_q1.set_order( { 2, i } );
		}
		super::m_q1.open();
		super::m_q2.open();
	}
	~ff_lineHybrid() { }

	void define_harvest( flowerColor c, ff_base* f )
	{
		assert( c != super::m_c1 );

		super::m_hp.set( c, f );
	}
};

typedef ff_lineHybrid<3>  ff_lineHybrid_1;
typedef ff_lineHybrid<7>  ff_lineHybrid_2;
typedef ff_lineHybrid<11> ff_lineHybrid_3;
typedef ff_lineHybrid<15> ff_lineHybrid_4;
typedef ff_lineHybrid<19> ff_lineHybrid_5;

// -----------------------------------------------------------------------------
template <unsigned C>
class ff_linePacked : public ff_phbase<C>
{
	// Only for the reference, don't use this for your plant model
	
	typedef ff_phbase<C>	super;

    public:
	ff_linePacked( const char* nm, flowerColor c1, flowerColor c2 ) : super( nm, c1, c2 )
	{
		// the result flower color cannot specified as c2 (c2 x c2 may be crossed)

		for ( unsigned i = 0; i < C; i += 2 ) {
			super::m_q1.set_order( { 0, i } );
			if ( i + 1 < C ) super::m_p2.set_order( { 0, i + 1 } );
			super::m_q2.set_order( { 1, i } );
			super::m_q1.set_order( { 2, i } );
			if ( i + 1 < C ) super::m_p2.set_order( { 2, i + 1 } );
			super::m_q2.set_order( { 3, i } );
			super::m_q1.set_order( { 4, i } );
			if ( i + 1 < C ) super::m_p2.set_order( { 2, i + 1 } );
		}
		super::m_q1.open();
		super::m_q2.open();
	}
	~ff_linePacked() { }

	void define_harvest( flowerColor c, ff_base* f )
	{
		// only if layout C=5, c1 clone cannot be produced.
		assert( c != super::m_c2 && ( C == 5 || c != super::m_c1 ));

		super::m_hp.set( c, f );
	}
};

typedef ff_linePacked<3>  ff_linePacked_1;
typedef ff_linePacked<7>  ff_linePacked_2;
typedef ff_linePacked<11> ff_linePacked_3;
typedef ff_linePacked<15> ff_linePacked_4;
typedef ff_linePacked<19> ff_linePacked_5;

// ff_swapHybrid ( target gene condenser, especially for Paleh Method )
// -----------------------------------------------------------------------------
template <unsigned C>
class ff_swapHybrid: public flower_field<5,C>
{
	static const unsigned MaxTests = 2 * ( C + 1 ) / 3;
	static const unsigned MaxPlans = 16;

	constexpr const coord<C> kernel( const unsigned u ) const
	{
		return { ( u % 2 ) * 3, ( u / 2 ) * 3 };
	}
	constexpr const coord<C> swappee( const unsigned u ) const
	{
		return { ( u % 2 ) * 3, ( u / 2 ) * 3 + 1 };
	}
	constexpr const coord<C> swapper( const unsigned u, const unsigned i ) const
	{
		assert ( i < 3 );

		return ( i == 0 ) ? coord<C>( { ( u % 2 ) * 3 + 1, ( u / 2 ) * 3 } ) : 
				    coord<C>( { ( u % 2 ) * 3 + 1, ( u / 2 ) * 3 + 1 } );
	}

	typedef flower_field<5,C>	super;

	typename super::plant_queue	m_q1;
	typename super::plant_queue	m_q2;
	flowerColor			m_ck;

	struct swap_plan
	{
		flowerColor	color;
		flowerColor	next;
		ff_base*	opt;	// if not nullptr, not to swap but to transplant into opt
	};

	unsigned			m_plans;
	swap_plan			m_sp[ MaxPlans ];

   public:
	ff_swapHybrid( const char* nm, flowerColor ck )
		: super( nm ), m_q1( this, { } ), m_q2( this, { } ), m_ck( ck ), m_plans( 0 )
	{
		for ( unsigned i = 0; i < MaxTests; ++i ) {
			m_q1.set_order( kernel( i ));
			m_q2.set_order( swappee( i ));
		}
		for ( unsigned c = 0; c < C; ++c ) {
			for ( unsigned r = 0; r < 5; ++r ) {
				if ( r == 2 || c % 3 == 2 ) super::cover( { r, c } );
			}
		}
		m_q1.open();
		m_q2.open();
	}
	~ff_swapHybrid()
	{
		m_q2.close();
		m_q1.close();
	}

	void define_swap( flowerColor c1, flowerColor c2, ff_base* f = nullptr )
	{
		m_sp[ m_plans++ ] = { c1, c2, f };
	}

	void recycle() override
	{
		m_q2.close();
		m_q1.close();
		super::recycle();
		m_q1.open();
		m_q2.open();
	}

	bool put( flower* p ) override
	{
		bool	r = false;

		if      ( p->is_color( m_ck )) 		  r = m_q1.put( p );
		else if ( p->is_color( m_sp[ 0 ].color )) r = m_q2.put( p );

		return r;
	}
	bool is_open() const override { return m_q1.is_open() || m_q2.is_open(); }

	void plant() override
	{
		m_q1.plant();
		m_q2.plant();
	}

        void hydrate( visitor n ) override
        {
		// don't hydrate solitary or immatrue flower for not creating clone 

		auto		i = m_q1.begin();
		auto		j = m_q2.begin();

		while ( i != m_q1.end() && j != m_q2.end()) {
			flower*		f1 = super::get( *i++ );
			flower*		f2 = super::get( *j++ );

			if ( f1 && f1->is_growth( 3 ) &&
			     f2 && f2->is_growth( 3 )) {
				f1->hydrate( n );
				f2->hydrate( n );
			} else {
				break;
			}
		}
        }
        void harvest() override
        {
		for ( unsigned i = 0; i < MaxTests; ++i ) {

			flower*		kr = super::get( kernel( i ));	
			flower*&	ee = super::get( swappee( i ));

			if ( kr && ee ) {	
				flower*		     er = super::pull( swapper( i, 0 ));
				if ( er == nullptr ) er = super::pull( swapper( i, 1 ));

				if ( er ) {
					for ( unsigned p = 0; p < m_plans; ++p ) {

						if ( ee->is_color( m_sp[ p ].color ) &&
						     er->is_color( m_sp[ p ].next )) {

							if ( ff_base* ff = m_sp[ p ].opt ) {
								ff->put( er );
							} else {
								ee = er;
							}
							break;
						}
					}
				}
			} else {
				break;
			}
		}
	}

	unsigned count() const override
	{
		unsigned	i = 0;

		for ( ; i < MaxTests; ++i ) {
			if ( super::get( kernel( i ))  == nullptr || super::get( swappee( i )) == nullptr ) break;
		}
		return i;
	}
};


typedef ff_swapHybrid<5>  ff_swapHybrid_2;	//  4 kernels
typedef ff_swapHybrid<11> ff_swapHybrid_3;	//  8 kernels
typedef ff_swapHybrid<23> ff_swapHybrid_6;	// 16 kernels

// ff_hybridTest ( gene tester especially for Paleh and BackwardsN Method )
// -----------------------------------------------------------------------------
template <unsigned R, unsigned C>
class ff_hybridTest: public flower_field<R,C>
{
	static const unsigned TR = ( R + 1 ) / 4;
	static const unsigned TC = ( C + 1 ) / 4;
	static const unsigned MaxTests = TR * TC;

	constexpr const coord<C> sample( const unsigned u ) const
	{
		return { ( u / TC ) * 4 + 1, ( u % TC ) * 4 + 1 };
	}
	constexpr const coord<C> tester( const unsigned u, const unsigned i ) const
	{
		assert ( i < 4 );

		return ( i == 0 ) ? coord<C>( { ( u / TC ) * 4,     ( u % TC ) * 4 } ) : 
		       ( i == 1 ) ? coord<C>( { ( u / TC ) * 4 + 2, ( u % TC ) * 4 + 2 } ) : 
		       ( i == 2 ) ? coord<C>( { ( u / TC ) * 4,     ( u % TC ) * 4 + 2 } ) :
				    coord<C>( { ( u / TC ) * 4 + 2, ( u % TC ) * 4 } );
	}
	constexpr const coord<C> result( const unsigned u, const unsigned i ) const
	{
		assert ( i < 4 );

		return ( i == 0 ) ? coord<C>( { ( u / TC ) * 4 + 1, ( u % TC ) * 4 } ) : 
		       ( i == 1 ) ? coord<C>( { ( u / TC ) * 4 + 1, ( u % TC ) * 4 + 2 } ) : 
		       ( i == 2 ) ? coord<C>( { ( u / TC ) * 4,     ( u % TC ) * 4 + 1 } ) :
				    coord<C>( { ( u / TC ) * 4 + 2, ( u % TC ) * 4 + 1 } );
	}

	typedef flower_field<R,C>	super;

	typename super::plant_queue	m_qt;
	flowerColor			m_cSucc;
	ff_base*			m_fSucc;
	ff_base*			m_fFail;

	flower*				m_qs[ MaxTests ];
	unsigned			m_pos;
	unsigned			m_tests;
	unsigned			m_inTest;

   public:
        ff_hybridTest( const char* nm )
                : flower_field<R,C>( nm ), m_qt( this, { } ),
		  m_cSucc( NA ), m_fSucc( nullptr ), m_fFail( nullptr ), m_pos( 0 ), m_tests( 0 ), m_inTest( 0 )
        {
		for ( unsigned i = 0; i < 4; ++i ) {
			// do loop u last, for increment of m_test in put_tester()
			for ( unsigned u = 0; u < MaxTests; ++u ) m_qt.set_order( tester( u, i ));
		}
		for ( unsigned r = 0; r < R; ++r ) {
			for ( unsigned c = 0; c < C; ++c ) {
				if ( r % 4 == 3 || c % 4 == 3 ) super::cover( { r, c } );
			}
		}

		m_qt.open();
	}
	~ff_hybridTest()
	{
		m_qt.close();
	}

	void define_test( flowerColor cSucc, ff_base* fSucc, ff_base* fFail )
	{
		m_cSucc = cSucc;
		m_fSucc = fSucc;
		m_fFail = fFail;
	}

	void recycle() override
	{
		m_qt.close();

		flower_field<R,C>::recycle();
	
		m_pos = 0;	
		m_tests = 0;
		m_inTest = 0;

		for ( unsigned r = 0; r < R; ++r ) {
			for ( unsigned c = 0; c < C; ++c ) {
				if ( r % 4 == 3 || c % 4 == 3 ) super::cover( { r, c } );
			}
		}

		m_qt.open();
	}

	bool put_tester( flower* p )
	{
		if ( m_tests < MaxTests ) m_tests++;
		return m_qt.put( p );
	}
        bool put_sample( flower* p ) 
	{
		if ( m_pos < MaxTests ) {
			m_qs[ m_pos++ ] = p;
			return true;
		} else {
			return false;
		}
	}
        bool put( flower* p ) override { return put_sample( p ); }
	bool is_open() const override { return m_pos < MaxTests; }

        void plant() override
	{
		m_qt.plant();

		if ( 0 < m_pos && m_inTest < m_tests ) {
			for ( unsigned u = 0; u < m_tests; ++u ) {
				flower*& f = super::get( sample( u ));
				if ( f == nullptr ) {
					f = m_qs[ --m_pos ];
					++m_inTest;
					break;
				}
			}
		}
	}

        void hydrate( visitor n ) override
        {
		// Note: Why ps->is_growth( 3 )?
		//   This code is mandatory to avoid the problem of random process order in daily_refresh().
		//   If the tester has been checked before the sample which may be still in growth level 2
		//   (looks bud, it cannot be a partner), then the tester will make a pseudo-success clone.
		//
		for ( unsigned u = 0; u < m_tests; ++u ) {
			flower* ps = super::get( sample( u ));
			if ( ps && ps->is_growth( 3 )) {
				ps->hydrate( n );
				for ( unsigned i = 0; i < 4; ++i ) super::hydrate( n, tester( u, i ));
			}
		}
        }
        void harvest() override
        {
		// there is one sample hybrid, others are tester clones.	

		for ( unsigned u = 0; u < m_tests; ++u ) {

			if ( flower* f = super::get( sample( u ))) {
				bool ts = false;
				int  cf = 0;

				for ( unsigned j = 0; j < 4; ++j ) {
					if ( flower* r = super::pull( result( u, j ))) {
						if ( r->is_color( super::get( tester( u, 0 )))) {
							ts = true;
						} else if ( r->is_color( m_cSucc )) {
							cf = 1;
						} else {
							cf = -1;
						}
					}
				}
				if ( cf != 0 || ts == true ) {
					f = super::pull( sample( u ));
					if ( 0 <= cf ) {
						assert( m_fSucc );
						m_fSucc->put( f );
					} else {
						assert( m_fFail );
						m_fFail->put( f );
					}
					--m_inTest;
				}
			}
		}
        }

	unsigned count() const override { return m_inTest; }		// count samples
};

typedef ff_hybridTest<11,3>  ff_hybridTest_2;		//  3 samples
typedef ff_hybridTest<11,7>  ff_hybridTest_4;		//  6 samples
typedef ff_hybridTest<11,11> ff_hybridTest_6;		//  9 samples
typedef ff_hybridTest<11,15> ff_hybridTest_8;		// 12 samples
typedef ff_hybridTest<11,19> ff_hybridTest_10;		// 15 samples are tested in the same time.

//
// Simulator
// -----------------------------------------------------------------------------
const unsigned DefaultMaxSteps = 1000;

// Simulator Base
// -----------------------------------------------------------------------------
class simulator
{
    protected:
	pctl::plist<ff_base*>	m_list;
	unsigned		m_step;
	visitor			m_visitor;

	const unsigned		m_maxsteps;

    public:
	simulator( const unsigned maxsteps = DefaultMaxSteps )
		: m_step( 0 ), m_visitor( 0 ), m_maxsteps( maxsteps )
	{ }
	virtual ~simulator() { }

	virtual void initialize() { }
	virtual bool terminate_sim() = 0;
	virtual bool terminate_reg() = 0;
	unsigned simulate();
	void regression( const unsigned n = 1000 );
	virtual void finalize() { }
	
	void step();
	virtual void print() = 0;
	virtual void stat() const { }

	void set_visitor( unsigned char n ) { m_visitor = n; }
};

unsigned simulator::simulate()
{
	initialize();

	m_step = 0;
	do {
		if ( 0 < m_step ) print();
		if ( m_maxsteps < ++m_step ) {
			fprintf( stderr, "Error: over %u steps\n", m_maxsteps );
			exit( 1 );
		}
		step();
	}
	while ( !terminate_sim() );

	finalize();

	return m_step;
}

void simulator::regression( const unsigned n )
{
	unsigned	min = m_maxsteps;
	unsigned	max = 0;
	unsigned	sum = 0;
	unsigned	sqs = 0;

	for ( unsigned i = 0; i < n; ++i ) {

		initialize();

		m_step = 0;
		do {
			if ( m_maxsteps < ++m_step ) {
				fprintf( stderr, "Error: over %u steps\n", m_maxsteps );
				exit( 1 );
			}
			step();
		}
		while ( !terminate_reg() ); 

		finalize();

		if ( m_maxsteps <= m_step ) {
			fprintf( stderr, "*** Error: too many steps in regression\n" );
			print();
			exit( 1 );
		}

		// stat
		if ( m_step < min ) min = m_step;
		if ( max < m_step ) max = m_step;
		sum += m_step;
		sqs += m_step * m_step;

		if ( i != 0 && i % 100 == 0 )
			printf( "%u traials, min = %u, max = %u, average = %f\n", i, min, max, 1.0 * sum / i );

		for ( ff_base* p : m_list ) p->recycle();
		flower::clear_pool();
	}

	double	avg = 1.0 * sum / n;
	double	sigma = sqrt( sqs / n - avg * avg );

	printf( "%u traials, min = %u, max = %u, average = %f, sigma = %f\n", n, min, max, avg, sigma );
}

void simulator::step()
{
	for ( ff_base* p : m_list ) p->harvest();
	for ( ff_base* p : m_list ) p->plant();
	for ( ff_base* p : m_list ) p->hydrate( m_visitor );
	for ( ff_base* p : m_list ) p->daily_refresh();
}

