#ifndef BBMG_H
#define BBMG_H

//#include <cerrno>
//#include <cstdio>
//#include <cstdlib>
//#include <cstring>
//#include <ctime>
//#include <fstream>
//#include <iostream>
//#include <sstream>
//#include <string>
//#include <vector>
//#include <cmath>
#include "constants.h"
#include "kernel.h"
#include "particle.h"
#include "settings.h"
#include "system_state.h"
#include "table_delaunay.h"

//using namespace std;


class BBMG {
private:
    struct field  // All fetched info for jet initialization and propagation
    {
        int sph, on;
        double rho, rho0, T, v[2];
        double r[2], phi, line;
        int pid;
        double gam, vmag, vang;
    };

    Settings    * settingsPtr = nullptr;
    SystemState * systemPtr   = nullptr;

    int z, a, c;
    double Freezeout_Temp;
    double area;
    double vjet; // Taken to be c for jets
    double Cg, Cq, q;
    
    double rho0tot; // total density, NOT just T>Freezeout_Temp!!!
    double Pfg, Pfq;

    double phi[15];
    double Rq[15], Rg[15];
    vector<double> rr;
    vector<field> full_sph_field;

    void inter(field &f); // interpolation
    double efluc();
    double get_kappa(double T);


public:
    BBMG(){}
    BBMG( Settings * settingsPtr_in, SystemState * systemPtr_in );
    void propagate();
    double flow(field &f);
    double qft(double p);
    double gft(double p);

    void initial();
};



#endif
