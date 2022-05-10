#ifndef HYDRODYNAMIC_INFO_H
#define HYDRODYNAMIC_INFO_H

#include <memory>

#include "matrix.h"
#include "vector.h"

// hydrodynamic quantities (with default initialization)
struct hydrodynamic_info
{
  double t               = 0.0; // current time in hydro simulation

  double Agam            = 0.0;
  double Agam2           = 0.0;
  double shv33           = 0.0;

  double div_u           = 0.0; // four-divergence of relativistic velocity
  double gamma           = 0.0; // Lorentz factor
  double s_sub           = 0.0;
  double e_sub           = 0.0;
  double s_an            = 0.0;
  double s_rat           = 0.0;
  double sigsub          = 0.0;
  double Bulk            = 0.0; // Bulk Viscosity weight
  double bigPI           = 0.0; // total bulk viscosity
  double C               = 0.0;
  double tauRelax        = 0.0; // Bulk Relaxation time
  double stauRelax       = 0.0; // Shear Relxation time
  double zeta            = 0.0; // bulk coefficient
  double setas           = 0.0; 
  double Ctot            = 0.0;
  double Btot            = 0.0;
  double Bsub            = 0.0;

  double sv_eta          = 0.0;
  double taupi           = 0.0;

  double sigma           = 0.0; // especific volume
  double dsigma_dt       = 0.0; // derivative of especific volume

  double dw_ds           = 0.0; // derivative of the enthalpy on entropy
  double eta             = 0.0; // entropy density
  double eden            = 0.0;


  double g2              = 0.0;
  double g3              = 0.0;
  double gt              = 0.0;
  double eta_o_tau       = 0.0;
  double dwdsT1          = 0.0;
  double sigl            = 0.0;

  double T               = 0.0;

  // vector members
  Vector<double,2> v;                     // velocity
  Vector<double,2> u;                     // relativistic velocity
  Vector<double,2> qmom;
  Vector<double,2> gradsig;        // relativistic velocity derivative

  Vector<double,2> gradP;                 // Gradient of Pressure
  Vector<double,2> gradBulk;              // Gradient of Bulk Viscosity
  Vector<double,2> gradsigma;             // Gradient of especific volume
  Vector<double,2> divshear, gradshear;


  // matrix members
  Matrix<double,2,2> Msub;
  Matrix<double,2,2> dpidtsub;
  Matrix<double,2,2> Imat;
  Matrix<double,2,2> gradV, gradU;        // Gradient of velocity needed for shear
  Matrix<double,2,2> uu, pimin, piu, piutot;
  Matrix<double,3,3> shv;

};

typedef std::shared_ptr<hydrodynamic_info> pHydrodynamic_info;


#endif