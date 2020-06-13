//
// Animal Crossing New Horizon - Plant Hybrid Simulator
//
//					2020/06/05	Issay376
//

#include <acnh.h>

// Simulator of Flower Reproduction
// -----------------------------------------------------------------------------
class ff_reproduction : public flower_field<1,2>
{
	typedef	flower_field<1,2>	super;

	unsigned			m_prods;

    public:
	ff_reproduction() : flower_field( "flower reproduction" ), m_prods( 0 ) { }
	~ff_reproduction() { }

	void recycle() override { m_prods = 0; super::pull( { 0, 1 } ); }
	void hydrate( visitor n ) override { super::hydrate( n, { 0, 0 } ); }
	void harvest() override { if ( super::pull( { 0, 1 } )) ++m_prods; }

	unsigned prods() const { return m_prods; }
};
	
class FR_simulator : public simulator
{
	ff_reproduction		m_f;
	const unsigned 		m_target;

    public:
	FR_simulator( const unsigned t ) : m_target( t ) { m_list.push_back( &m_f ); }
	~FR_simulator() { }

	void initialize() { m_f.set( rose::create( gRdRose, 3 ), { 0, 0 } ); }
	bool terminate_sim()
	{
		if ( m_f.prods() < m_target ) {
			return false;
		} else {
			--m_step;		// target has been reached one day before
			return true;
		}
	}
	bool terminate_reg() { return terminate_sim(); } 
	void print()
	{
		fprintf( stderr, "\n*** step %u *** ( %u )\n", m_step, m_f.prods() );
		m_f.print();
	}
};

//
// main
// 
// -----------------------------------------------------------------------------
int main( int argc, char** argv )
{
	std::srand( std::time(nullptr) );

	unsigned	visitor = 0;
	unsigned	targets = 1;
	unsigned	repeats = 1000;
	bool		simulate = false;
	
	for ( int i = 1; i < argc; ++i ) {
		if ( *argv[ i ] == '-' ) {
			switch ( *( argv[ i ] + 1 )) {
				case 'v': visitor = strtoul( argv[ ++i ], nullptr, 0 ); break;
				case 't': targets = strtoul( argv[ ++i ], nullptr, 0 ); break;
				case 's': simulate = true; break;
				default:
					fprintf( stderr, "Usage: %s [-v visitors][-t clones][-s] [regressions]\n", argv[ 0 ] );
					exit( 1 );
			}
		} else {
			repeats = strtoul( argv[ i ], nullptr, 0 ); 
		}
	}
	
	FR_simulator	s( targets );

	s.set_visitor( visitor );

	if ( simulate ) {
		unsigned	n = s.simulate();

		printf( "\nFlower Reproduction: %u steps for %u clones.\n", n, targets );
	} else {
		s.regression( repeats );
		printf( "\nFlower Reproduction: %u regressions, target %u flower created.\n", repeats, targets );
	}

	return 0;
}

