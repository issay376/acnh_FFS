//
// Animal Crossing New Horizon - Plant Hybrid Simulator
//
//					2020/06/05	Issay376
//

#include <acnh.h>

// Simulator of Flower Layout (White Rose(0010) -> Purple Rose(0020))
// -----------------------------------------------------------------------------
template <unsigned C>
class ff_layout : public flower_field<5,C>
{
    protected:
	typedef	flower_field<5,C>	super;

	unsigned			m_prods;
	unsigned			m_clone;
	unsigned			m_pu002;
	unsigned			m_wh001;
	unsigned			m_wh000;

    public:
	ff_layout<C>( const char* n )
		: flower_field<5,C>( n ),
		  m_prods( 0 ), m_clone( 0 ), m_pu002( 0 ), m_wh001( 0 ), m_wh000( 0 )
	{ }
	~ff_layout<C>() { }

	void harvest() override
	{
		for ( auto i = super::m_generation.begin(); i != super::m_generation.end(); ++i ) {
 			flower*	f = super::pull( *i );

			if ( f->get_gene() == 0xC0 ) ++m_pu002;
			if ( f->get_gene() == 0x80 ) ++m_wh001;
			if ( f->get_gene() == gWhRose ) ++m_wh001;
			if ( f->get_gene() == 0x00 ) ++m_wh000;
			if ( f->is_clone()) ++m_clone;
			++m_prods; 
		}
	}

	void stats()
	{
		printf( "%s: pu002/total=%u/%u, pu/wh1/wh0=(%.2f%%/%.2f%%/%.2f%%), %u(%.3f%%) clones\n",
			ff_base::m_name.c_str(), m_pu002, m_prods,
			100.0 * m_pu002 / m_prods,
			100.0 * m_wh001 / m_prods,
			100.0 * m_wh000 / m_prods,
			m_clone, 100.0 * m_clone / m_prods );

		m_prods = m_clone = m_pu002 = m_wh001 = m_wh000 = 0;
	}		
};

template <unsigned C>
class ff_layout8 : public ff_layout<C>
{
    public:
	ff_layout8() : ff_layout<C>( "layout   8" )
	{
		for ( unsigned c = 0; c < C; c += 2 ) {
			this->set( rose::create( gWhRose, 3 ), { 0, c + 0 } );
			this->set( rose::create( gWhRose, 3 ), { 1, c + 0 } );
			this->set( rose::create( gWhRose, 3 ), { 3, c + 0 } );
			this->set( rose::create( gWhRose, 3 ), { 4, c + 0 } );
		}
	}
};

template <unsigned C>
class ff_layout9 : public ff_layout<C>
{
    public:
	ff_layout9() : ff_layout<C>( "layout   9" )
	{
		for ( unsigned c = 0; c < C; c += 2 ) {
			this->set( rose::create( gWhRose, 3 ), { 0, c + 0 } );
			this->set( rose::create( gWhRose, 3 ), { 1, c + 0 } );
			if ( c + 1 < C ) this->set( rose::create( gWhRose, 3 ), { 2, c + 1 } );
			this->set( rose::create( gWhRose, 3 ), { 3, c + 0 } );
			this->set( rose::create( gWhRose, 3 ), { 4, c + 0 } );
		}
	}
};

template <unsigned C>
class ff_layout10 : public ff_layout<C>
{
    public:
	ff_layout10() : ff_layout<C>( "layout  10" )
	{
		for ( unsigned c = 0; c < C; c += 2 ) {
			if ( c + 1 < C ) this->set( rose::create( gWhRose, 3 ), { 0, c + 1 } );
			this->set( rose::create( gWhRose, 3 ), { 1, c + 0 } );
			if ( c + 1 < C ) this->set( rose::create( gWhRose, 3 ), { 1, c + 1 } );
			this->set( rose::create( gWhRose, 3 ), { 2, c + 0 } );
			this->set( rose::create( gWhRose, 3 ), { 3, c + 0 } );
			if ( c + 1 < C ) this->set( rose::create( gWhRose, 3 ), { 3, c + 1 } );
			if ( c + 1 < C ) this->set( rose::create( gWhRose, 3 ), { 4, c + 1 } );
		}
	}
};

template <unsigned C>
class ff_layout11 : public ff_layout<C>
{
    public:
	ff_layout11() : ff_layout<C>( "layout  11" )
	{
		for ( unsigned c = 0; c < C; c += 2 ) {
			this->set( rose::create( gWhRose, 3 ), { 0, c + 0 } );
			if ( c + 1 < C ) this->set( rose::create( gWhRose, 3 ), { 0, c + 1 } );
			this->set( rose::create( gWhRose, 3 ), { 1, c + 0 } );
			this->set( rose::create( gWhRose, 3 ), { 1, c + 2 } );
			if ( c + 1 < C ) this->set( rose::create( gWhRose, 3 ), { 2, c + 1 } );
			this->set( rose::create( gWhRose, 3 ), { 3, c + 0 } );
			this->set( rose::create( gWhRose, 3 ), { 3, c + 2 } );
			this->set( rose::create( gWhRose, 3 ), { 4, c + 0 } );
			if ( c + 1 < C ) this->set( rose::create( gWhRose, 3 ), { 4, c + 1 } );
		}
	}
};

template <unsigned C>
class ff_layout12O : public ff_layout<C>
{
    public:
	ff_layout12O() : ff_layout<C>( "layout 12O" )
	{
		for ( unsigned c = 0; c < C; c += 2 ) {
			this->set( rose::create( gWhRose, 3 ), { 0, c + 0 } );
			if ( c + 1 < C ) this->set( rose::create( gWhRose, 3 ), { 0, c + 1 } );
			this->set( rose::create( gWhRose, 3 ), { 1, c + 0 } );
			this->set( rose::create( gWhRose, 3 ), { 2, c + 0 } );
			this->set( rose::create( gWhRose, 3 ), { 3, c + 0 } );
			this->set( rose::create( gWhRose, 3 ), { 4, c + 0 } );
			if ( c + 1 < C ) this->set( rose::create( gWhRose, 3 ), { 4, c + 1 } );
		}
	}
};

template <unsigned C>
class ff_layout12H : public ff_layout<C>
{
    public:
	ff_layout12H() : ff_layout<C>( "layout 12H" )
	{
		for ( unsigned c = 0; c < C; c += 2 ) {
			this->set( rose::create( gWhRose, 3 ), { 0, c + 0 } );
			this->set( rose::create( gWhRose, 3 ), { 1, c + 0 } );
			if ( c + 1 < C ) this->set( rose::create( gWhRose, 3 ), { 1, c + 1 } );
			this->set( rose::create( gWhRose, 3 ), { 2, c + 0 } );
			this->set( rose::create( gWhRose, 3 ), { 3, c + 0 } );
			if ( c + 1 < C ) this->set( rose::create( gWhRose, 3 ), { 3, c + 1 } );
			this->set( rose::create( gWhRose, 3 ), { 4, c + 0 } );
		}
	}
};

template <unsigned C>
class ff_layout13 : public ff_layout<C>
{
    public:
	ff_layout13() : ff_layout<C>( "layout  13" )
	{
		for ( unsigned c = 0; c < C; c += 2 ) {
			this->set( rose::create( gWhRose, 3 ), { 0, c + 0 } );
			if ( c + 1 < C ) this->set( rose::create( gWhRose, 3 ), { 0, c + 1 } );
			this->set( rose::create( gWhRose, 3 ), { 1, c + 0 } );
			this->set( rose::create( gWhRose, 3 ), { 2, c + 0 } );
			if ( c + 1 < C ) this->set( rose::create( gWhRose, 3 ), { 2, c + 1 } );
			this->set( rose::create( gWhRose, 3 ), { 3, c + 0 } );
			this->set( rose::create( gWhRose, 3 ), { 4, c + 0 } );
			if ( c + 1 < C ) this->set( rose::create( gWhRose, 3 ), { 4, c + 1 } );
		}
	}
};

template <unsigned C>
class ff_layout7X : public ff_layout<C>
{
    public:
	ff_layout7X() : ff_layout<C>( "layout  7X" )
	{
		for ( unsigned c = 0; c < C; c += 2 ) {
			if ( c + 1 < C ) this->set( rose::create( gWhRose, 3 ), { 0, c + 1 } );
			this->set( rose::create( gWhRose, 3 ), { 1, c + 0 } );
			if ( c + 1 < C ) this->set( rose::create( gWhRose, 3 ), { 2, c + 1 } );
			this->set( rose::create( gWhRose, 3 ), { 3, c + 0 } );
			if ( c + 1 < C ) this->set( rose::create( gWhRose, 3 ), { 4, c + 1 } );
		}
	}
};

template <unsigned C>
class ff_layout8X : public ff_layout<C>
{
    public:
	ff_layout8X() : ff_layout<C>( "layout  8X" )
	{
		for ( unsigned c = 0; c < C; c += 2 ) {
			this->set( rose::create( gWhRose, 3 ), { 0, c + 0 } );
			if ( c + 1 < C ) this->set( rose::create( gWhRose, 3 ), { 1, c + 1 } );
			this->set( rose::create( gWhRose, 3 ), { 2, c + 0 } );
			if ( c + 1 < C ) this->set( rose::create( gWhRose, 3 ), { 3, c + 1 } );
			this->set( rose::create( gWhRose, 3 ), { 4, c + 0 } );
		}
	}
};

template <unsigned C>
class LO_simulator : public simulator
{
	ff_layout8<C>		m_f8;
	ff_layout9<C>		m_f9;
	ff_layout10<C>		m_fA;
	ff_layout11<C>		m_fB;
	ff_layout12O<C>		m_fC;
	ff_layout12H<C>		m_fH;
	ff_layout13<C>		m_fD;

	ff_layout7X<C>		m_fV;
	ff_layout8X<C>		m_fX;

    public:
	LO_simulator( const unsigned n = 1000 ) : simulator( n )
	{
		m_list = { &m_f8, &m_f9, &m_fA, &m_fB, &m_fC, &m_fH, &m_fD, &m_fV, &m_fX };
		for ( ff_base* f : m_list ) f->print();
		fprintf( stderr, "\n" );
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
	else			 LO_simulator_1( steps ).stat( visitor );;

	return 0;
}

