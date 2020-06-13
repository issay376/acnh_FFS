//
// Animal Crossing New Horizon - Plant Hybrid Simulator 
//
//					2020/06/05	Issay376
//

#include <acnh.h>

// Simulator of Guaranteed Hybrid Red Method
// -----------------------------------------------------------------------------
class GR_simulator : public simulator
{
	unsigned		m_fmax;
	unsigned		m_firstBlue;

	ff_initialHybrid_2	m_f1;
	ff_initialHybrid_2	m_f2;
	ff_lineHybrid_2		m_f3;
	ff_lineHybrid_3		m_f4;
	ff_initialHybrid_5	m_f5;
	ff_initialHybrid_3	m_x1;
	ff_initialHybrid_2	m_x2;
	ff_initialHybrid_1	m_x3;
	ff_storage_1		m_fB;
	ff_storage_3		m_fX;

    public:
	GR_simulator()
		: m_fmax( 0 ), m_firstBlue( 0 ),
		  m_f1( "1: Wh_ x Wh_ -> Pu1" ),			// 25%
		  m_f2( "2: Rd_ x Rd_ -> Bk2" ),			// 25%
		  m_f3( "3: Pu1 x Bk2 -> Rd3", Pu, Bk ),		// 100%
		  m_f4( "4: Ye_ x Rd3 -> Rd4", Ye, Rd ),		// 25%
		  m_f5( "5: Rd4 x Rd4 -> Bu_/Rd5/Or5/Bk5" ),		// 1.56%/40.62%/23.43%/9.37%
		  m_x1( "X1: Rd5 x Rd5 -> Bu_" ),			// 2.36%
		  m_x2( "X2: Or5 x Or5 -> Bu_" ),			// 1.78%
		  m_x3( "X3: Bk5 x Bk5 -> Bu_" ),			// 2.78%
		  m_fB( "B: [Bu_]" ),					// Goal
		  m_fX( "XB: [Bu_]" )					// Additional Blues
	{
		m_f1.define_harvest( Pu, &m_f3 );
		m_f2.define_harvest( Bk, &m_f3 );
		m_f3.define_harvest( Rd, &m_f4 );
		m_f4.define_harvest( Rd, &m_f5 );
		m_f5.define_harvest( Rd, &m_x1 );
		m_f5.define_harvest( Or, &m_x2 );
		m_f5.define_harvest( Bk, &m_x3 );
		m_f5.define_harvest( Bu, &m_fB );
		m_x1.define_harvest( Bu, &m_fX );
		m_x2.define_harvest( Bu, &m_fX );
		m_x3.define_harvest( Bu, &m_fX );

		m_list = { &m_f1, &m_f2, &m_f3, &m_f4, &m_f5, &m_x1, &m_x2, &m_x3, &m_fB, &m_fX };
	}
	~GR_simulator() { }

	void initialize();
	bool terminate_sim();
	bool terminate_reg();
	void print();

	unsigned maxFieldUnits() const { return m_fmax; }
	unsigned firstBlue() const { return m_firstBlue; }

    private:
	unsigned usingFields() const;
};

void GR_simulator::initialize()
{
	while ( m_f1.put( rose::create( gWhRose ))) ;
	while ( m_f2.put( rose::create( gRdRose ))) ;
	while ( m_f4.put( rose::create( gYeRose ))) ;
}		

unsigned GR_simulator::usingFields() const
{
	unsigned	n = 0;

	if ( m_f5.is_open()) {
		if ( m_f4.is_open()) {
			if ( m_f3.is_open()) {
				n += m_f1.field_units();
				n += m_f2.field_units();
			}
			if ( m_f3.count() != 0 ) n += m_f3.field_units();
		}
		if ( m_f4.count() != 0 ) n += m_f4.field_units();
	}
	if ( m_f5.count() != 0 ) n += m_f5.field_units();
	if ( m_x1.count() != 0 ) n += m_x1.field_units();
	if ( m_x2.count() != 0 ) n += m_x2.field_units();
	if ( m_x3.count() != 0 ) n += m_x3.field_units();
	// not count m_fB/m_fX

	return n;
}

bool GR_simulator::terminate_reg()
{
	unsigned n = usingFields();

	if ( m_fmax < n ) m_fmax = n;

	return 0 < m_fB.count() || 0 < m_fX.count();
}
 
bool GR_simulator::terminate_sim()
{
	if ( m_firstBlue == 0 && ( m_fB.count() != 0 || m_fX.count() != 0 )) m_firstBlue = m_step;

	return 10 <= m_fB.count();
} 

void GR_simulator::print()
{
	unsigned n = usingFields();

	if ( m_fmax < n ) m_fmax = n;

	fprintf( stderr, "\n*** step %u *** ( %u fields )\n", m_step, n );

	if ( m_f5.is_open()) {
		if ( m_f4.is_open()) {
			if ( m_f3.is_open()) {
				m_f1.print();
				m_f2.print();
			}
			if ( m_f3.count() != 0 ) m_f3.print();
		}
		if ( 8 < m_f4.count()) m_f4.print();
	}
	if ( m_f5.count() != 0 ) m_f5.print();
	if ( m_x1.count() != 0 ) m_x1.print( true );
	if ( m_x2.count() != 0 ) m_x2.print( true );
	if ( m_x3.count() != 0 ) m_x3.print( true );
	if ( m_fB.count() != 0 ) m_fB.print();
	if ( m_fX.count() != 0 ) m_fX.print();
}

//
// main
//
// -----------------------------------------------------------------------------
int main( int argc, char** argv )
{
        std::srand( std::time(nullptr) );

        unsigned        visitor = 0;
        unsigned        repeats = 0;

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

        GR_simulator    s;

        s.set_visitor( visitor );

        if ( repeats == 0 ) {
                unsigned        n = s.simulate();

                printf( "\nGuaranteed Hybrid Red Method: %u steps, maximum %u field units used.\n",
			n, s.maxFieldUnits());
                printf( "       first blue rose in %u steps\n", s.firstBlue());
        } else {
                s.regression( repeats );
                printf( "\nGuaranteed Hybrid Red Method: %u regressions, maximum %u field units used.\n",
			repeats, s.maxFieldUnits());
        }

        return 0;
}

