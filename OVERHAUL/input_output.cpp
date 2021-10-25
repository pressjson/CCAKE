#ifndef _ENTERIC_CPP_
#define _ENTERIC_CPP_

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <sstream>
#include <stdio.h>
#include <vector>
#include "mathdef.h"
#include "vector.h"
#include "tables.h"
#include "particle.h"
#include "enteric.h"
#include<dirent.h>
#include <stdlib.h>
#include <algorithm>
#include <cmath>
#include "input_output.h"

// Constructors and destructors.
  input_output::input_output(){}
 ~input_output::input_output(){}


void input_output::read_in_initial_conditions()
{
 
    
  
}

void input_output::load_settings_file( string path_to_settings_file )
{
    string Param_file = path_to_settings_file+"Input_Parameters.inp"
    ifstream infile( Param_file.c_str() );
    if (infile.is_open())
    {
        string line;
        string ignore, param;
        vector<string> all_parameters;
        while ( getline (infile, line) )
        {
            istringstream iss(line);
            iss >> ignore >> param;
            all_parameters.push_back(param)
        }

        input_parameters.IC_tpye = all_parameters[0]
        input_parameters.h = stod(all_parameters[1])
        input_parameters.dt = stod(all_parameters[2])
        input_parameters.t0 = stod(all_parameters[3])
        input_parameters.EoS = all_parameters[4]
        input_parameters.eta = all_parameters[5]
        input_parameters.zeta = all_parameters[6]
        input_parameters.Freeze_Out_Temperature = stod(all_parameters[7])
        input_parameters.Freeze_Out_Type = all_parameters[8]

        infile.close();
    }
}


#endif