//
// Animal Crossing New Horizon - Plant Hybrid Simulator
//
//					2020/06/05	Issay376
//

#include <acnh.h>

const gene gPuRose = 0xC0;

// Simulator of Flower Reproduction
// -----------------------------------------------------------------------------
class ff_solo : public flower_field<3,6>
{
	typedef	flower_field<3,6>	super;

	unsigned			m_prods;

    public:
	ff_solo() : flower_field( "2 solitude flowers reproduction" ), m_prods( 0 )
	{
		initialize();
	}
	~ff_solo() { }

	void initialize() 
	{
		super::set( rose::create( gPuRose, 3 ), { 1, 1 } );
		super::set( rose::create( gPuRose, 3 ), { 1, 4 } );
	}
	void recycle() override
	{
		m_prods = 0;
		super::recycle();
	}
	void harvest() override
	{
		for ( coord<6> pc : m_generation ) {
			super::pull( pc );
			++m_prods;
		}
	}
	unsigned prods() const { return m_prods; }
};
	
class ff_pair : public flower_field<3,6>
{
	typedef	flower_field<3,6>	super;

	unsigned			m_prods;

    public:
	ff_pair() : flower_field( "1 paired flowers reproduction" ), m_prods( 0 )
	{
		initialize();
	}
	~ff_pair() { }

	void initialize()
	{
		super::set( rose::create( gPuRose, 3 ), { 1, 2 } );
		super::set( rose::create( gPuRose, 3 ), { 1, 3 } );
	}
	void recycle() override
	{
		m_prods = 0;
		super::recycle();
	}
	void harvest() override
	{
		for ( coord<6> pc : m_generation ) {
			super::pull( pc );
			++m_prods;
		}
	}
	unsigned prods() const { return m_prods; }
};

class FP_simulator : public simulator
{
	ff_solo			m_s;
	ff_pair			m_p;
	const unsigned 		m_target;
	bool			m_regs;
	bool			m_regp;

	unsigned		m_smin;
	unsigned		m_smax;
	unsigned		m_ssum;
	unsigned		m_ssqs;

	unsigned		m_pmin;
	unsigned		m_pmax;
	unsigned		m_psum;
	unsigned		m_psqs;

    public:
	FP_simulator( const unsigned t, const unsigned maxsteps = DefaultMaxSteps )
		: simulator( maxsteps ),
		  m_target( t ), m_regs( true ), m_regp( true ),
		  m_smin( maxsteps ), m_smax( 0 ), m_ssum( 0 ), m_ssqs( 0 ),
		  m_pmin( maxsteps ), m_pmax( 0 ), m_psum( 0 ), m_psqs( 0 )
	{
		m_list = { &m_s, &m_p };
	}
	~FP_simulator() { }

	void initialize() override
	{
		m_s.initialize();
		m_p.initialize();
	}
	bool terminate_sim() override
	{
		if ( m_s.prods() < m_target && m_p.prods() < m_target ) {
			return false;
		} else {
			--m_step;		// target has been reached one day before
			return true;
		}
	}
	bool terminate_reg() override
	{
		if ( m_regs && m_target <= m_s.prods()) {

			unsigned s1 = m_step - 1;

			m_regs = false;
			if ( m_step < m_smin ) m_smin = s1;
			if ( m_smax < m_step ) m_smax = s1;
			m_ssum += s1;
			m_ssqs += s1 * s1;
		}
		if ( m_regp && m_target <= m_p.prods()) {

			unsigned s1 = m_step - 1;

			m_regp = false;
			if ( s1 < m_pmin ) m_pmin = s1;
			if ( m_pmax < s1 ) m_pmax = s1;
			m_psum += s1;
			m_psqs += s1 * s1;
		}
		if ( m_regs || m_regp ) {
			return false;
		} else {
			m_regs = m_regp = true;
			return true;
		}
	} 
	void print() override
	{
		fprintf( stderr, "*** step %u ( solo/pair = %u/%u ) ***\n", m_step, m_s.prods(), m_p.prods());
	}
	void stat( unsigned r )
	{
		double	savg = 1.0 * m_ssum / r;
		double	pavg = 1.0 * m_psum / r;
		double	ssgm = sqrt( 1.0 * m_ssqs / r - savg * savg );
		double	psgm = sqrt( 1.0 * m_psqs / r - pavg * pavg );

		printf( "  2 solo: min = %u, max = %u, average = %f, sigma = %f\n", m_smin, m_smax, savg, ssgm );
		printf( "  1 pair: min = %u, max = %u, average = %f, sigma = %f\n", m_pmin, m_pmax, pavg, psgm );
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
	unsigned	targets = 100;
	unsigned	repeats = 1000;
	unsigned	maxstep = 1000;
	bool		simulate = false;
	
	for ( int i = 1; i < argc; ++i ) {
		if ( *argv[ i ] == '-' ) {
			switch ( *( argv[ i ] + 1 )) {
				case 'm': maxstep = strtoul( argv[ ++i ], nullptr, 0 ); break;
				case 'v': visitor = strtoul( argv[ ++i ], nullptr, 0 ); break;
				case 't': targets = strtoul( argv[ ++i ], nullptr, 0 ); break;
				case 's': simulate = true; break;
				default:
					fprintf( stderr, "Usage: %s [-v visitors][-t targets][-m maxstep][-s] [regressions]\n", argv[ 0 ] );
					exit( 1 );
			}
		} else {
			repeats = strtoul( argv[ i ], nullptr, 0 ); 
		}
	}
	
	FP_simulator	s( targets, maxstep );

	s.set_visitor( visitor );

	if ( simulate ) {
		unsigned	n = s.simulate();

		printf( "\n%u steps for %u flowers reproduction.\n", n, targets );
	} else {
		s.regression( repeats );

		printf( "\nFlower Reproduction: %u regressions, target %u flower created.\n", repeats, targets );
		s.stat( repeats );
	}

	return 0;
}

