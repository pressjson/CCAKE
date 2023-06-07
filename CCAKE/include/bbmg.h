#ifndef BBMG_H
#define BBMG_H

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "constants.h"
#include "kernel.h"
#include "particle.h"
#include "settings.h"
#include "system_state.h"

using namespace std;

template <int D>
class BBMG {
private:
    struct field  // contains jet's line integral (quantitues needed for integrand)
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
    double TD;
    double area;
    double kappa, vjet;
    double Cg, Cq, q;
    double rho0tot; // total density, NOT just T>TD!!!
    double Pfg, Pfq;

    double phi[15];
    double Rq[15], Rg[15];
    vector<double> rr;
    vector<field> ff;

    void initial();
    void inter(field &f); // interpolation
    double flow(field &f);
    double efluc();
    double gft(double p);
    double qft(double p);

public:
    BBMG<D>(){}
    BBMG<D>( Settings * settingsPtr_in, SystemState * systemPtr_in );
    void propagate();
};


template <int D>
BBMG<D>::BBMG( Settings * settingsPtr_in, SystemState * systemPtr_in )
  : settingsPtr{ settingsPtr_in },
    systemPtr{ systemPtr_in }
{
  srand( time( NULL ) );
  TD    = 150;
  q     = 1;
  Cg    = 3;
  Cq    = 4./3;
  z     = 1; // path length dependence
  a     = 0; //factor for E
  c     = 2+z-a; //factor for T
  kappa = 0.17;
  vjet  = 1;
  area  = PI*pow(2.*systemPtr->h,2);
  rr.resize(systemPtr->n());

  for (int i = 0; i < 15; i++)
  {
    Rq[i]  = 0;
    Rg[i]  = 0;
    phi[i] = i*PI/7;
  }

  Pfg = 10;
  Pfq = 10;

  // call initial() to complete initialization
  initial();
}


template <int D>
void BBMG<D>::initial()
{
  rho0tot = 0;
  for ( int i = 0; i < systemPtr->particles.size(); ++i )
  {
    auto & p = systemPtr->particles[i];
    double rsub = p.p() / p.T();
    rho0tot    += rsub;

    if ( p.T() * constants::hbarc_MeVfm > TD )
    {
      field sub;
      sub.r[0] = p.r(0);
      sub.r[1] = p.r(1);
      sub.rho0 = rsub;
      sub.sph  = i;
      sub.line = 0.5 * pow(settingsPtr->t0, z) * pow(sub.rho0, c) * systemPtr->dt; // only if initial flow=0

      for (int j=0; j<14; j++)
      {
        sub.phi = phi[j];
        sub.pid = j;
        sub.on  = 1;
        ff.push_back(sub);
      }
    }
  }
}


template <int D>
double BBMG<D>::flow(field &f) { return f.gam*(1-f.vmag*cos(f.phi-f.vang)); }

template <int D>
double BBMG<D>::gft(double p) { return 2*p; }

template <int D>
double BBMG<D>::qft(double p) { return 2*p; }

template <int D>
double BBMG<D>::efluc()
{
  int random_variable = std::rand()/RAND_MAX;
  double zeta         = random_variable*(q+2.);
  return (1.+q) / pow(q+2, 1+q) * pow(q+2.-zeta, q);
}


template <int D>
void BBMG<D>::propagate()
{
  double tau  = systemPtr->t + settingsPtr->t0;
  int stillon = 0;
  int tot     = ff.size();

  for (int i = 0; i < tot; i++)
  {
    //propagate x,y position of jet
    ff[i].r[0] += vjet * systemPtr->dt * cos(ff[i].phi);
    ff[i].r[1] += vjet * systemPtr->dt * cos(ff[i].phi);

    inter( ff[i] );

    if ( ( ff[i].on == 1 ) && ( ff[i].T > TD ) )
    {
      ff[i].line += pow(tau, z) * pow(ff[i].rho, c) * flow(ff[i]) * systemPtr->dt;
      stillon++;
    }
    else
    {
      ff[i].on    = 0;
      ff[i].line += 0.5 * pow(tau,z) * pow(ff[i].rho, c) * flow(ff[i]) * systemPtr->dt;
      ff[i].line *= kappa * efluc();

      double P0g  = Pfg + Cg * pow(Pfg, 1-a) * ff[i].line;
      double P0q  = Pfq + Cq * pow(Pfq, 1-a) * ff[i].line;

      int jj      = ff[i].pid;

      Rq[jj]     += pow(P0g/Pfg, 1+a) * ff[i].rho0 * gft(P0g) / gft(Pfg);
      Rq[jj]     += pow(P0q/Pfq, 1+a) * ff[i].rho0 * qft(P0g) / qft(Pfg);
    }
  }

  if ( stillon == 0 )
  {
    for (int j=0; j<14; j++)
    {
      Rq[j] /= rho0tot;
      Rg[j] /= rho0tot;
    }
  }
}

template <int D>
void BBMG<D>::inter( field &f )
{
  double den = 0, den2 = 0;
  for ( auto & p : systemPtr->particles )
  {
    double dx    = p.r(0)-f.r[0];
    double dy    = p.r(1)-f.r[1];

    double rdiff = sqrt(dx*dx+dy*dy)/systemPtr->h;

    if (rdiff<2)
    {
      ++den;
      den2     += p.norm_spec.s;
      double kk = kernel::kernel(rdiff);
      f.T      += p.T()*0.06*0.06*kk;
      f.rho    += (p.p()/p.T())*kk;
      f.v[0]   += p.hydro.v(0)*kk;
      f.v[1]   += p.hydro.v(1)*kk;

      cout << dx << " " << dy << " " << p.T()*constants::hbarc_MeVfm << " " << p.hydro.v << endl;
    }
  }

  double fac = den / area;
  f.T       *= constants::hbarc_MeVfm;
  f.rho     /= fac;
  f.v[0]    /= fac;
  f.v[1]    /= fac;
  f.vmag     = sqrt( f.v[0]*f.v[0] + f.v[1]*f.v[1] );
  f.vang     = atan2( f.v[1], f.v[0] );
  f.gam      = 1.0 / sqrt( f.vmag*f.vmag + 1.0 );
}


#endif