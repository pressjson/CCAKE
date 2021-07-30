#ifndef NM_SOLVER_H
#define NM_SOLVER_H

#include "Functions.h"

using namespace std;

namespace nm_solver
{
	void solve( double & e0, double & B0, double & S0, double & Q0,
				double & Tout, double & muBout, double & muSout, double & muQout );
}

#endif
