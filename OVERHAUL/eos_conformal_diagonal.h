#ifndef EOS_CONFORMAL_DIAGONAL_H
#define EOS_CONFORMAL_DIAGONAL_H

#include <cmath>
#include <vector>

#include "constants.h"
#include "eos_base.h"
#include "eos_header.h"

class EoS_conformal_diagonal: public EoS_base
{
using std::abs;
using std::pow;

private:

  double c, T0, muB0, muS0, muQ0;
  static constexpr double four_thirds = 4.0/3.0;
  static constexpr double two_to_two_thirds = std::pow(2.0, 2.0/3.0);

public:
  // default constructor/destructor
  EoS_conformal_diagonal(){}
  virtual ~EoS_conformal_diagonal(){}

  EoS_conformal_diagonal( const double c_in,
                          const double T0_in, const double muB0_in,
                          const double muS0_in, const double muQ0_in,
                          const std::vector<double> & tbqs_minima_in,
                          const std::vector<double> & tbqs_maxima_in,
                          const std::string & name_in = "conformal_diagonal")
    { c = c_in; T0 = T0_in; muB0 = muB0_in; muS0 = muS0_in; muQ0 = muQ0_in;
      tbqs_minima = tbqs_minima_in; tbqs_maxima = tbqs_maxima_in; name = name_in; }

  // this is the criterion for a guaranteed solution with this EoS
  bool eBSQ_has_solution( const double e0, const double rhoB0,
                          const double rhoS0, const double rhoQ0 )
  {
    return e0 >= 3.0*( pow(muB0*abs(rhoB0), four_thirds)
                      + pow(muS0*abs(rhoS0), four_thirds)
                      + pow(muQ0*abs(rhoQ0), four_thirds) )
                  / ( 4.0 * two_to_two_thirds * pow(c, 1.0/3.0) )
  }


  double p(double T, double muB, double muQ, double muS)
	{
		double x = (T/T0)*(T/T0)*(T/T0)*(T/T0)
                + (muB/muB0)*(muB/muB0)*(muB/muB0)*(muB/muB0)
                + (muS/muS0)*(muS/muS0)*(muS/muS0)*(muS/muS0)
                + (muQ/muQ0)*(muQ/muQ0)*(muQ/muQ0)*(muQ/muQ0);
    double cp = c*T0*T0*T0*T0;  // c dimensionless, cp - 1/fm^4
		return cp*x;
	}
	
	double s(double T, double muB, double muQ, double muS)
	{
    double cp = c*T0*T0*T0*T0;  // c dimensionless, cp - 1/fm^4
		return 4.0*cp*T*T*T/(T0*T0*T0*T0);
	}

	double B(double T, double muB, double muQ, double muS)
	{
    double cp = c*T0*T0*T0*T0;  // c dimensionless, cp - 1/fm^4
		return 4.0*cp*muB*muB*muB/(muB0*muB0*muB0*muB0);
	}

	double S(double T, double muB, double muQ, double muS)
	{
    double cp = c*T0*T0*T0*T0;  // c dimensionless, cp - 1/fm^4
		return 4.0*cp*muS*muS*muS/(muS0*muS0*muS0*muS0);
	}

	double Q(double T, double muB, double muQ, double muS)
	{
    double cp = c*T0*T0*T0*T0;  // c dimensionless, cp - 1/fm^4
		return 4.0*cp*muQ*muQ*muQ/(muQ0*muQ0*muQ0*muQ0);
	}

	double P2B2(double T, double muB, double muQ, double muS)
	{
    double cp = c*T0*T0*T0*T0;  // c dimensionless, cp - 1/fm^4
		return 12.0*cp*muB*muB/(muB0*muB0*muB0*muB0);
	}
	double P2Q2(double T, double muB, double muQ, double muS)
	{
    double cp = c*T0*T0*T0*T0;  // c dimensionless, cp - 1/fm^4
		return 12.0*cp*muQ*muQ/(muQ0*muQ0*muQ0*muQ0);
	}
	double P2S2(double T, double muB, double muQ, double muS)
	{
    double cp = c*T0*T0*T0*T0;  // c dimensionless, cp - 1/fm^4
		return 12.0*cp*muS*muS/(muS0*muS0*muS0*muS0);
	}
	
	double P2BQ(double T, double muB, double muQ, double muS) { return 0.0; }
	double P2BS(double T, double muB, double muQ, double muS) { return 0.0; }
	double P2QS(double T, double muB, double muQ, double muS) { return 0.0; }
	
	double P2TB(double T, double muB, double muQ, double muS) { return 0.0; }
	double P2TQ(double T, double muB, double muQ, double muS) { return 0.0; }
	double P2TS(double T, double muB, double muQ, double muS) { return 0.0; }

	double P2T2(double T, double muB, double muQ, double muS)
	{
    double cp = c*T0*T0*T0*T0;  // c dimensionless, cp - 1/fm^4
		return 12.0*cp*T*T/(T0*T0*T0*T0);
	}

  void get_eBSQ( double point[], double results[] )
  {
    // point: (T, muB, muQ, muS)
    const double Tsol  = point[0],
      muBsol           = point[1],
      muQsol           = point[2],
      muSsol           = point[3];
    double POut        = p(Tsol, muBsol, muQsol, muSsol);
    double sOut        = s(Tsol, muBsol, muQsol, muSsol);
    double BOut        = B(Tsol, muBsol, muQsol, muSsol);
    double SOut        = S(Tsol, muBsol, muQsol, muSsol);
    double QOut        = Q(Tsol, muBsol, muQsol, muSsol);
    double eOut        = (sOut*Tsol + muBsol*BOut + muQsol*QOut + muSsol*SOut) - POut;
    results[0]         = eOut;
    results[1]         = BOut;
    results[2]         = SOut;
    results[3]         = QOut;
  }

  void get_sBSQ( double point[], double results[] )
  {
    // point: (T, muB, muQ, muS)
    results[0] = s( point[0], point[1], point[2], point[3] );
    results[1] = B( point[0], point[1], point[2], point[3] );
    results[2] = S( point[0], point[1], point[2], point[3] );
    results[3] = Q( point[0], point[1], point[2], point[3] );
  }

  void get_full_thermo( double point[], double results[] )
  {
    // point: (T, muB, muQ, muS)
    const double Tsol   = point[0], muBsol = point[1],
                 muQsol = point[2], muSsol = point[3];
    double POut = p(Tsol, muBsol, muQsol, muSsol);
    double sOut = s(Tsol, muBsol, muQsol, muSsol);
    double BOut = B(Tsol, muBsol, muQsol, muSsol);
    double SOut = S(Tsol, muBsol, muQsol, muSsol);
    double QOut = Q(Tsol, muBsol, muQsol, muSsol);
    double eOut = (sOut*Tsol + muBsol*BOut + muQsol*QOut + muSsol*SOut) - POut;

    results[0]  = POut;
    results[1]  = sOut;
    results[2]  = BOut;
    results[3]  = SOut;
    results[4]  = QOut;
    results[5]  = eOut;
    results[6]  = 1.0/3.0;  // conformal

    results[7]  = P2B2(Tsol, muBsol, muQsol, muSsol);
    results[8]  = P2Q2(Tsol, muBsol, muQsol, muSsol);
    results[9]  = P2S2(Tsol, muBsol, muQsol, muSsol);
    results[10] = P2BQ(Tsol, muBsol, muQsol, muSsol);
    results[11] = P2BS(Tsol, muBsol, muQsol, muSsol);
    results[12] = P2QS(Tsol, muBsol, muQsol, muSsol);
    results[13] = P2TB(Tsol, muBsol, muQsol, muSsol);
    results[14] = P2TQ(Tsol, muBsol, muQsol, muSsol);
    results[15] = P2TS(Tsol, muBsol, muQsol, muSsol);
    results[16] = P2T2(Tsol, muBsol, muQsol, muSsol);
  }
  
};

#endif