//
// Animal Crossing New Horizon - Plant Hybrid Simulator
//
//					2020/06/05	Issay376
//

#include <acnh.h>

// Simulator of Flower Layout (Purple Rose(0120) x Orange Rose(1100) -> Orange Rose(1210))
// -----------------------------------------------------------------------------
const gene gPu012 = 0xD0;
const gene gOr110 = 0x14;
const gene gOr121 = 0x74;
const unsigned nOr121 = flower::flower_no( gOr121 );

template <unsigned C>
class ff_layout : public flower_field<5,C>
{
    protected:
	typedef	flower_field<5,C>	super;

	unsigned			m_prods;
	unsigned			m_clone;
	unsigned			m_pu012;	// clone Pu012
	unsigned			m_or110;	// clone Or110
	unsigned			m_or121;	// target
	unsigned			m_purple;
	unsigned			m_orange;

    public:
	ff_layout<C>( const char* n )
		: flower_field<5,C>( n ),
		  m_prods( 0 ), m_clone( 0 ), m_pu012( 0 ), m_or110( 0 ), m_or121( 0 ),
		  m_purple( 0 ), m_orange( 0 )
	{ }
	~ff_layout<C>() { }

	void harvest() override
	{
		for ( auto i = super::m_generation.begin(); i != super::m_generation.end(); ++i ) {
 			flower*	f = super::pull( *i );

			if ( flower::flower_no( f->get_gene()) == nOr121 ) ++m_or121;
			if ( f->is_clone()) {
				if ( f->get_gene() == gPu012 ) ++m_pu012;
				if ( f->get_gene() == gOr110 ) ++m_or110;
				++m_clone;
			}
			if ( f->is_color( Pu )) ++m_purple;
			if ( f->is_color( Or )) ++m_orange;
			++m_prods; 
		}
	}

	void stats() 
	{
		printf( "%s: or121/total = %u/%u(%.2f%%), clones:%u(%.3f%%) [or:%u/%u, pu:%u/%u]\n",
			ff_base::m_name.c_str(), m_or121, m_prods, 100.0 * m_or121 / m_prods,
			m_clone, 100.0 * m_clone / m_prods,
			m_or110, m_orange, m_pu012, m_purple );

		m_prods = m_clone = m_pu012 = m_or110 = m_or121 = m_purple = m_orange = 0;
	}		
};

template <unsigned C>
class ff_layout7 : public ff_layout<C>
{
    public:
	ff_layout7( gene ga, gene gb ) : ff_layout<C>(( ga == gOr110 ) ? "layout 7N" : "layout 7R" )
	{
		for ( unsigned c = 0; c < C; c += 2 ) {
			if ( c + 1 < C ) this->set( rose::create( ga, 3 ), { 0, c + 1 } );
			this->set( rose::create( gb, 3 ), { 1, c + 0 } );
			if ( c + 1 < C ) this->set( rose::create( ga, 3 ), { 2, c + 1 } );
			this->set( rose::create( gb, 3 ), { 3, c + 0 } );
			if ( c + 1 < C ) this->set( rose::create( ga, 3 ), { 4, c + 1 } );
		}
	}
};

template <unsigned C>
class ff_layoutX : public ff_layout<C>
{
    public:
	ff_layoutX( gene ga, gene gb ) : ff_layout<C>(( ga == gOr110 ) ? "layout XN" : "layout XR" )
	{
		for ( unsigned c = 0; c < C; c += 2 ) {
			this->set( rose::create( ga, 3 ), { 0, c + 0 } );
			if ( c + 1 < C ) this->set( rose::create( gb, 3 ), { 1, c + 1 } );
			this->set( rose::create( ga, 3 ), { 2, c + 0 } );
			if ( c + 1 < C ) this->set( rose::create( gb, 3 ), { 3, c + 1 } );
			this->set( rose::create( ga, 3 ), { 4, c + 0 } );
		}
	}
};

template <unsigned C>
class ff_layout8 : public ff_layout<C>
{
    public:
	ff_layout8( gene ga, gene gb ) : ff_layout<C>( "layout  8" )
	{
		for ( unsigned c = 0; c < C; c += 2 ) {
			this->set( rose::create( ga, 3 ), { 0, c + 0 } );
			this->set( rose::create( gb, 3 ), { 1, c + 0 } );
			this->set( rose::create( gb, 3 ), { 3, c + 0 } );
			this->set( rose::create( ga, 3 ), { 4, c + 0 } );
		}
	}
};

template <unsigned C>
class ff_layout9 : public ff_layout<C>
{
    public:
	ff_layout9( gene ga, gene gb ) : ff_layout<C>(( ga == gOr110 ) ? "layout 9N" : "layout 9R" )
	{
		for ( unsigned c = 0; c < C; c += 2 ) {
			this->set( rose::create( ga, 3 ), { 0, c + 0 } );
			this->set( rose::create( gb, 3 ), { 1, c + 0 } );
			if ( c + 1 < C ) this->set( rose::create( ga, 3 ), { 2, c + 1 } );
			this->set( rose::create( gb, 3 ), { 3, c + 0 } );
			this->set( rose::create( ga, 3 ), { 4, c + 0 } );
		}
	}
};

template <unsigned C>
class ff_layout10 : public ff_layout<C>
{
    public:
	ff_layout10( gene ga, gene gb ) : ff_layout<C>(( ga == gOr110 ) ? "layout LN" : "layout LR" )
	{
		for ( unsigned c = 0; c < C; c += 2 ) {
			this->set( rose::create( ga, 3 ), { 0, c + 0 } );
			this->set( rose::create( gb, 3 ), { 1, c + 0 } );
			this->set( rose::create( ga, 3 ), { 2, c + 0 } );
			this->set( rose::create( gb, 3 ), { 3, c + 0 } );
			this->set( rose::create( ga, 3 ), { 4, c + 0 } );
		}
	}
};

template <unsigned C>
class LO_simulator : public simulator
{
	ff_layout7<C>		m_f7N;
	ff_layout7<C>		m_f7R;
	ff_layoutX<C>		m_fXN;
	ff_layoutX<C>		m_fXR;
	ff_layout8<C>		m_f8;
	ff_layout9<C>		m_f9N;
	ff_layout9<C>		m_f9R;
	ff_layout10<C>		m_fLN;
	ff_layout10<C>		m_fLR;

    public:
	LO_simulator( const unsigned n = 1000 )
		: simulator( n ),
		  m_f7N( gOr110, gPu012 ), m_f7R( gPu012, gOr110 ),
		  m_fXN( gOr110, gPu012 ), m_fXR( gPu012, gOr110 ),
		  m_f8( gOr110, gPu012 ),
		  m_f9N( gOr110, gPu012 ), m_f9R( gPu012, gOr110 ),
		  m_fLN( gOr110, gPu012 ), m_fLR( gPu012, gOr110 )
	{
		m_list = { &m_f7N, &m_f7R, &m_fXN, &m_fXR, &m_f8, &m_f9N, &m_f9R, &m_fLN, &m_fLR };
		for ( ff_base* f : m_list ) f->print();
	}
	~LO_simulator() { }

	bool terminate_sim() override { return m_maxsteps <= m_step; }
	bool terminate_reg() override { return true; }
	void print() override { }

	void stat( unsigned visitors )
	{
		if ( 5 < visitors ) {
			for ( unsigned v = 0; v <= 5; ++v ) {
				printf( "visitors = %d, ", v );
				stat( v );
			}
		} else {
			set_visitor( visitors );

			if ( 0 < m_maxsteps ) {
				simulate();

				printf( "%u steps for each Flower Layouts\n", m_maxsteps );
				for ( ff_base* f : m_list ) {
					if ( ff_layout<C>* p = dynamic_cast<ff_layout<C>*>( f )) p->stats();
				}
				printf( "\n" );

			}
		}
	}
};

typedef LO_simulator<3>		LO_simulator_1;
typedef LO_simulator<5>		LO_simulator_1h;
typedef LO_simulator<7>		LO_simulator_2;
typedef LO_simulator<11>	LO_simulator_3;
typedef LO_simulator<15>	LO_simulator_4;

//
// main
// 
// -----------------------------------------------------------------------------
int main( int argc, char** argv )
{
	std::srand( std::time(nullptr) );

	unsigned	visitor = 0;
	unsigned	steps   = 1000;
	unsigned	ff_unit = 0;
	
	for ( int i = 1; i < argc; ++i ) {
		if ( *argv[ i ] == '-' ) {
			switch ( *( argv[ i ] + 1 )) {
				case 'a': visitor = 6; break;
				case 'v': visitor = strtoul( argv[ ++i ], nullptr, 0 ); break;
				case 's': steps   = strtoul( argv[ ++i ], nullptr, 0 ); break;
				case 'h': ff_unit = 1; break;
				case 'w': ff_unit = 2; break;
				case 't': ff_unit = 3; break;
				case 'q': ff_unit = 4; break;
				default:
					fprintf( stderr, "Usage: %s [-a|-v visitors][-s steps][-h|-w|-t|-q]\n", argv[ 0 ] );
					exit( 1 );
			}
		}
	}

	if      ( ff_unit == 1 ) LO_simulator_1h( steps ).stat( visitor );
	else if ( ff_unit == 2 ) LO_simulator_2( steps ).stat( visitor );
	else if ( ff_unit == 3 ) LO_simulator_3( steps ).stat( visitor );
	else if ( ff_unit == 4 ) LO_simulator_4( steps ).stat( visitor );
	else		         LO_simulator_1( steps ).stat( visitor );

	return 0;
}

