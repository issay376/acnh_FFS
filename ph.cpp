//
// Animal Crossing New Horizon - Plant Hybrid Simulator
//
//				      2020/06/05      Issay376
//

#include <acnh.h>

// Simulator of Paleh Method
// -----------------------------------------------------------------------------
class PH_simulator : public simulator
{
	unsigned		m_fmax;
	unsigned		m_firstBlue;

	ff_initialHybrid_3	m_f1;
	ff_lineHybrid_2		m_f2;
	ff_initialHybrid_1	m_f3;
	ff_lineHybrid_2		m_f4;
	ff_hybridTest_4		m_f5;
	ff_selfHybrid_2		m_f6;
	ff_swapHybrid_3		m_f7;
	ff_selfHybrid_2		m_f9;
	ff_selfHybrid_2		m_fX;
	ff_selfHybrid_2		m_fY;
	ff_storage_1		m_fB;

    public:
	PH_simulator()
		: m_fmax( 0 ), m_firstBlue( 0 ),
		  m_f1( "1: Wh_ x Wh_ -> Pu1" ),			// 25%
		  m_f2( "2: Ye_ x Wh_ -> Wh2", Ye, Wh ),		// 50%
		  m_f3( "3: Rd_ x Rd_ -> Bk3" ),			// 25%
		  m_f4( "4: Wh2 x Pu1 -> Pu4", Wh, Pu ),		// 50%
		  m_f5( "5: Pu4 x Ye_ -> Ye ? Pu5 : Pu1" ),		// 50%/50%
		  m_f6( "6: Pu5 x Pu5 -> Wh6/Pu4" ),			// 25%/(Pu5:50%,Pu1:25%)
		  m_f7( "7: Wh6 x Bk3 -> Rd7 -> Or7 -> Rd8", Wh ),	// Rd7:100%
									// Or7:12.5% (Wh6:12.5%, RdM:37.5%)
									// Rd8:25% (Or7:50%, Wh6:25%)	
		  m_f9( "9: Rd8 x Rd8 -> Bu9/Rd8/Wh6" ),		// 25%/50%/25%
		  m_fX( "X: Or7 x Or7 -> Bu9/Rd8/Wh6" ),		// 6.25%/12.5%/6.25%
		  m_fY( "Y: RdM x RdM -> Bu9" ),			// 10.9%
		  m_fB( "B: [Bu9]" )					// Goal
	{
		m_f1.define_harvest( Pu, &m_f4 );
		m_f2.define_harvest( Wh, &m_f4 );
		m_f3.define_harvest( Bk, &m_f7 );
		m_f4.define_harvest( Pu, &m_f5 );
		m_f5.define_test( Ye, &m_f6, &m_f4 );
		m_f6.define_harvest( Wh, &m_f7 );
		m_f6.define_harvest( Pu, &m_f5 );
		m_f7.define_swap( Bk, Rd );
		m_f7.define_swap( Rd, Or );
		m_f7.define_swap( Rd, Wh, &m_f7 );
		m_f7.define_swap( Rd, Rd, &m_fY );
		m_f7.define_swap( Or, Rd, &m_f9 );
		m_f7.define_swap( Or, Wh, &m_f7 );
		m_f7.define_swap( Or, Or, &m_fX );
		m_f9.define_harvest( Bu, &m_fB );
		m_f9.define_harvest( Rd, &m_f9 );
		m_f9.define_harvest( Wh, &m_f7 );
		m_fX.define_harvest( Bu, &m_fB );
		m_fX.define_harvest( Rd, &m_f9 );
		m_fX.define_harvest( Wh, &m_f7 );
		m_fY.define_harvest( Bu, &m_fB );

		m_list = { &m_f1, &m_f2, &m_f3, &m_f4, &m_f5, &m_f6, &m_f7, &m_f9, &m_fX, &m_fY, &m_fB };
	}
	~PH_simulator() { }

	void initialize() override;
	bool terminate_sim() override;
	bool terminate_reg() override;
	void finalize() override;
	void print() override;

	unsigned maxFieldUnits() const { return m_fmax; }
	unsigned firstBlue() const { return m_firstBlue; }

    private:
	unsigned usingFields() const;
};

void PH_simulator::initialize()
{
	while ( m_f1.put( rose::create( gWhRose ))) ;
	while ( m_f2.put( rose::create( gWhRose ))) ;
	while ( m_f2.put( rose::create( gYeRose ))) ;
	while ( m_f3.put( rose::create( gRdRose ))) ;
	while ( m_f5.put_tester( rose::create( gYeRose ))) ;
}

unsigned PH_simulator::usingFields() const
{
	unsigned	n = 0;

	if ( m_f9.is_open()) {
		if ( m_f7.is_open()) {
			if ( m_f6.is_open()) {
				if ( m_f4.is_open()) {
					n += m_f1.field_units();
					n += m_f2.field_units();
				}
				if ( m_f4.count() != 0 ) n += m_f4.field_units();
				if ( m_f5.count() != 0 ) n += m_f5.field_units();
			}
			if ( m_f3.count() != 0 ) n += m_f3.field_units();
			if ( m_f6.count() != 0 ) n += m_f6.field_units();
		}
		if ( m_f7.count() != 0 ) n += m_f7.field_units();
	}
	if ( m_f9.count() != 0 ) n += m_f9.field_units();
	if ( m_fX.count() != 0 ) n += m_fX.field_units();
	if ( m_fY.count() != 0 ) n += m_fY.field_units();
	// not count m_fB

	return n;
}

bool PH_simulator::terminate_reg()
{
	return 0 < m_fB.count();
}
 
bool PH_simulator::terminate_sim()
{
	if ( m_firstBlue == 0 && m_fB.count() != 0 ) m_firstBlue = m_step;

	return 10 <= m_fB.count();
} 

void PH_simulator::finalize()
{
	unsigned n = usingFields();

	if ( m_fmax < n ) m_fmax = n;
}

void PH_simulator::print()
{
	unsigned n = usingFields();

	if ( m_fmax < n ) m_fmax = n;

	fprintf( stderr, "\n*** step %u *** ( %d fields )\n", m_step, usingFields());

	if ( m_f9.is_open()) {
		if ( m_f7.is_open()) {
			if ( m_f6.is_open()) {
				if ( m_f4.is_open()) {
					m_f1.print();
					m_f2.print();
				}
				if ( m_f4.count() != 0 ) m_f4.print();
				if ( m_f5.count() != 0 ) m_f5.print();
			}
			if ( m_f3.count() != 0 ) m_f3.print();
			if ( m_f6.count() != 0 ) m_f6.print();
		}
		if ( m_f7.count() != 0 ) m_f7.print();
	}
	if ( m_f9.count() != 0 ) m_f9.print();
	if ( m_fX.count() != 0 ) m_fX.print();
	if ( m_fY.count() != 0 ) m_fY.print();
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

	PH_simulator    s;

	s.set_visitor( visitor );

	if ( repeats == 0 ) {
		unsigned	n = s.simulate();

		printf( "\nPaleh Method: %u steps, maximum %u field units used.\n", n, s.maxFieldUnits());
		printf( "       first blue rose in %u steps\n", s.firstBlue());
	} else {
		s.regression( repeats );
		printf( "\nPaleh Method: %u regressions, maximum %u field units used.\n", repeats, s.maxFieldUnits());
	}

	return 0;
}

