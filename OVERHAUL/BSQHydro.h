#ifndef BSQHYDRO_H
#define BSQHYDRO_H

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "constants.h"
#include "eos.h"
#include "equations_of_motion.h"
#include "input_output.h"
#include "system_state.h"
#include "settings.h"
#include "kernel.h"
#include "sph_workstation.h"

using std::string;
using std::vector;

class BSQHydro
{

public:

  BSQHydro();
  ~BSQHydro(){}

  void load_settings_file( string path_to_settings_file );
  void set_results_directory( string path_to_results_directory );
  void read_in_initial_conditions();
  void initialize_hydrodynamics();
  void run();
  void find_freeze_out_surface();
  void print_results();



private:

  string input_directory;
  string output_directory;


  // equation of state information
  EquationOfState eos;

  // equation of motion object
  EquationsOfMotion eom;

  // input/output
  InputOutput io;

  // hold freeze-out surface
  //FreezeOutSurface freeze_out_surface;

  // the current state of the hydrodynamic simulation
  SystemState system;

  // the workstation for performing SPH-related actions on the system
  SPHWorkstation ws;

  // all settings for the hydro simulation
  Settings settings;

};

#endif
