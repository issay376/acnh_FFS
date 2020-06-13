//
// Animal Crossing New Horizon - Plant Hybrid Simulator
//
//					2020/06/04	Issay376
//

#include <acnh.h>

// Simulator of BackwardsN Method
// -----------------------------------------------------------------------------
class BN_simulator : public simulator
{
	unsigned		m_fmax;
	unsigned		m_tmax;
	unsigned		m_firstBlue;

	ff_initialHybrid_2	m_f1;
	ff_lineHybrid_2		m_f2;
	ff_lineHybrid_1		m_f3;
	ff_lineHybrid_2		m_f4;
	ff_selfHybrid_1		m_f5;
	ff_hybridTest_4		m_f6;
	ff_lineHybrid_2		m_f7;
	ff_selfHybrid_2		m_f8;
	ff_selfHybrid_1		m_f9;
	ff_storage_1		m_fB;

    public:
	BN_simulator()
		: m_fmax( 0 ), m_tmax( 0 ), m_firstBlue( 0 ),
		  m_f1( "1: Wh_ x Wh_ -> Pu1" ),			// 25%
		  m_f2( "2: Ye_ x Wh_ -> Wh2", Ye, Wh ),		// 50%
		  m_f3( "3: Rd_ x Ye_ -> Or3", Rd, Ye ),		// 50%
		  m_f4( "4: Wh2 x Pu1 -> Pu4", Wh, Pu ),		// 50%
		  m_f5( "5: Or3 x Or3 -> Or5" ),			// 56.25% (25% Or3, 31.25% condensed Or)
		  m_f6( "6: Pu4 x Ye_ -> Ye ? Pu6 : Pu1" ),		// 50%/50%
		  m_f7( "7: Pu6 x Or5 -> Or7", Pu, Or ),		// 22.2% (Or3 x Pu6 -> Or7: 12.5%)
		  //m_f7( "7: Or5 x Pu4 -> Or7", Or, Pu ),		// 11.1%
		  m_f8( "8: Or7 x Or7 -> Wh8/Rd8/Bu_" ),		// 6.25%/12.5%/6.25%
		  m_f9( "9: Rd8 x Rd8 -> Wh8/Rd8/Bu_" ),		// 25%/50%/25%
		  m_fB( "B: [Bu_]" )					// Goal
	{
		m_f1.define_harvest( Pu, &m_f4 );
		m_f2.define_harvest( Wh, &m_f4 );
		m_f3.define_harvest( Or, &m_f5 );
		m_f4.define_harvest( Pu, &m_f6 );
		m_f5.define_harvest( Or, &m_f7 );
		m_f6.define_test( Ye, &m_f7, &m_f4 );
		m_f7.define_harvest( Or, &m_f8 );
		m_f8.define_harvest( Rd, &m_f9 );
		m_f8.define_harvest( Bu, &m_fB );
		m_f9.define_harvest( Rd, &m_f9 );
		m_f9.define_harvest( Bu, &m_fB );

		m_list = { &m_f1, &m_f2, &m_f3, &m_f4, &m_f5, &m_f6, &m_f7, &m_f8, &m_f9, &m_fB };
	}
	~BN_simulator() { }

	void initialize();
	bool terminate_sim();
	bool terminate_reg();
	void print();

	unsigned maxFieldUnits() const { return m_fmax; }
	unsigned maxTestCounts() const { return m_tmax; }
	unsigned firstBlue() const { return m_firstBlue; }

    private:
	unsigned usingFields() const;
};

void BN_simulator::initialize()
{
	while ( m_f1.put( rose::create( gWhRose ))) ;
	while ( m_f2.put( rose::create( gWhRose ))) ;
	while ( m_f2.put( rose::create( gYeRose ))) ;
	while ( m_f3.put( rose::create( gRdRose ))) ;
	while ( m_f3.put( rose::create( gYeRose ))) ;
	while ( m_f6.put_tester( rose::create( gYeRose ))) ;
}

unsigned BN_simulator::usingFields() const
{
	unsigned	n = 0;

	if ( m_f8.is_open()) {
		if ( m_f7.is_open()) {
			if ( m_f4.is_open()) {
				n += m_f1.field_units();
				n += m_f2.field_units();
			}
			if ( m_f5.is_open()) n += m_f3.field_units();
			if ( m_f4.count() != 0 ) n += m_f4.field_units();
			if ( m_f5.count() != 0 ) n += m_f5.field_units();
			if ( m_f6.count() != 0 ) n += m_f6.field_units();
		}
		if ( m_f7.count() != 0 ) n += m_f7.field_units();
	}
	if ( m_f8.count() != 0 ) n += m_f8.field_units();
	if ( m_f9.count() != 0 ) n += m_f9.field_units();
	// not count m_fB

	return n;
}

bool BN_simulator::terminate_reg()
{
	unsigned n;

	if ( m_fmax < ( n = usingFields())) m_fmax = n;

	return 0 < m_fB.count();
}
 
bool BN_simulator::terminate_sim()
{
	if ( m_firstBlue == 0 && m_fB.count() != 0 ) m_firstBlue = m_step;

	return 10 <= m_fB.count();
} 

void BN_simulator::print()
{
	unsigned	t, n;

	if ( m_tmax < ( t = m_f6.count()))  m_tmax = t;
	if ( m_fmax < ( n = usingFields())) m_fmax = n;

	fprintf( stderr, "\n*** step %u *** ( %u fields )\n", m_step, n );

	if ( m_f8.is_open()) {
		if ( m_f7.is_open()) {
			if ( m_f4.is_open()) {
				m_f1.print();
				m_f2.print();
			}
			if ( m_f5.is_open()) m_f3.print();
			if ( m_f4.count() != 0 ) m_f4.print();
			if ( m_f5.count() != 0 ) m_f5.print();
			if ( m_f6.count() != 0 ) m_f6.print( true );
		}
		if ( m_f7.count() != 0 ) m_f7.print( true );
	}
	if ( m_f8.count() != 0 ) m_f8.print();
	if ( m_f9.count() != 0 ) m_f9.print();
	if ( m_fB.count() != 0 ) m_fB.print();
}		

//
// main
// 
// -----------------------------------------------------------------------------
int main( int argc, char** argv )
{
	std::srand( std::time(nullptr) );

	unsigned	visitor = 0;
	unsigned	repeats = 0;
	
	for ( int i = 1; i < argc; ++i ) {
		if ( *argv[ i ] == '-' ) {
			switch ( *( argv[ i ] + 1 )) {
				case 'v': visitor = strtoul( argv[ ++i ], nullptr, 0 ); break;
				case 's':
				case 'r': repeats = strtoul( argv[ ++i ], nullptr, 0 ); break;
                                default:
                                        fprintf( stderr, "Usage %s [-v visitors][-r repeats]\n", argv[ 0 ] );
                                        return -1;
			}
		}
	}
	
	BN_simulator	s;

	s.set_visitor( visitor );

	if ( repeats == 0 ) {
		unsigned	n = s.simulate();

		printf( "\nBackwardN Method: %u steps, maximum %u field units/%u tests used.\n",
			n, s.maxFieldUnits(), s.maxTestCounts());
		printf( "	first blue rose in %u steps\n", s.firstBlue());
	} else {
		s.regression( repeats );
		printf( "\nBackwardN Method: %u regressions, maximum %u field units used.\n",
			repeats, s.maxFieldUnits());
	}

	return 0;
}

