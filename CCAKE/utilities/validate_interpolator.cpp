/// @file validate_interpolator
/// @author Willian M. Serenone
/// @date 2024-03-19
/// @brief This file contains a few tests to ensure that the EoS interpolator
/// is working correctly.
/// @details The tests are based on the EoS table generated by
/// generate_eos_table. The tests are:
/// 1. Evaluate the EoS at the grid points and check that the interpolation is
/// correct
/// 2. Evaluate the EoS at random points and prints the maximum errors found
/// during the interpolation (for T, muB, muS, muQ and P)

#include <memory>
#include <filesystem>

#include "H5Cpp.h"
#include <omp.h>

#include <Cabana_Core.hpp>

#include "particle.h"
#include "eos.h"
#include "settings.h"
#include "system_state.h"
#include "thermodynamic_info.h"
#include "eos_interpolator.h"
#include "stopwatch.h"
#include "constants.h"

#define TOL 1.E-7

int check_args(int argc, char** argv){
  //Usage: ./generate_eos_table <nthreads> <eos_type> <eos_path>
  if (argc != 4){
    std::cerr << "Usage: " << argv[0] <<
    " <nthreads> <eos_type> <eos_path>" << std::endl;
    std::cerr << "nthreads: number of threads to use" << std::endl;
    std::cerr << "eos_type: 'table', 'tanh_conformal', 'conformal' or 'conformal_diagonal'" << std::endl;
    std::cerr << "eos_path: path to EoS table" << std::endl;
    return 1;
  } else {
    std::cout << "Number of threads: " << argv[1] << std::endl;
    std::cout << "EoS type.........: " << argv[2] << std::endl;
    std::cout << "EoS path.........: " << argv[3] << std::endl;
    return 0;
  }
}

/// @brief Evaluates the EoS at the grid points and checks that the
/// interpolation is correct
/// @param file_path path to the EoS table generated by generate_eos_table
/// @param eos_type type of EoS to use
/// @param nthreads number of threads to use
void test1(std::string file_path, std::string eos_type, int nthreads){

    ccake::EoS_Interpolator eos_interpolator(file_path);
    ccake::EquationOfState eos[nthreads];

    std::shared_ptr<Settings> settings = std::make_shared<Settings>();

    //Setup EoS
    settings->eos_type = eos_type;
    if (settings->eos_type == "table"){
        settings->eos_path = "EoS/Houston/thermo.h5";
    }

    //Cannot be initialized in parallel to avoid simultaneous access to HDF5 file
    for(int i=0; i<nthreads; ++i){
      eos[i].set_SettingsPtr(settings);
      eos[i].init();
    }

    H5::H5File file = H5::H5File(file_path, H5F_ACC_RDONLY);

    //==========================================================================
    // Open the HDF5 file and retrieve the grid points
    //==========================================================================

    cout << "Creating particles for EoS interpolation at grid points" << endl;
    std::vector<std::string> table_names;

    auto get_table_names = [](hid_t loc_id, const char *name,
                              const H5L_info_t *info, void *operator_data)->herr_t
    {
      H5O_info_t object_info;
      H5Oget_info_by_name(loc_id, name, &object_info, H5P_DEFAULT);
      if (object_info.type == H5O_TYPE_GROUP) {
        ((std::vector<std::string>*) operator_data)->push_back(name);
      }
      return 0;
    };

    H5Literate(file.getId(), H5_INDEX_NAME, H5_ITER_NATIVE, NULL,
                get_table_names, &table_names);

    for (auto name : table_names){
      H5::Group group = file.openGroup(name);

      H5::Attribute s_min_attr = group.openAttribute("s_min");
      H5::Attribute s_max_attr = group.openAttribute("s_max");
      H5::Attribute ds_attr = group.openAttribute("ds");
      H5::Attribute rhoB_min_attr = group.openAttribute("rhoB_min");
      H5::Attribute rhoB_max_attr = group.openAttribute("rhoB_max");
      H5::Attribute dB_attr = group.openAttribute("drhoB");
      H5::Attribute rhoS_min_attr = group.openAttribute("rhoS_min");
      H5::Attribute rhoS_max_attr = group.openAttribute("rhoS_max");
      H5::Attribute dS_attr = group.openAttribute("drhoS");
      H5::Attribute rhoQ_min_attr = group.openAttribute("rhoQ_min");
      H5::Attribute rhoQ_max_attr = group.openAttribute("rhoQ_max");
      H5::Attribute dQ_attr = group.openAttribute("drhoQ");


      double s_min, s_max, B_min, B_max, Q_min, Q_max, S_min, S_max,
          ds, dB, dQ, dS;
      s_min_attr.read(H5::PredType::NATIVE_DOUBLE, &s_min);
      s_max_attr.read(H5::PredType::NATIVE_DOUBLE, &s_max);
      ds_attr.read(H5::PredType::NATIVE_DOUBLE, &ds);
      rhoB_min_attr.read(H5::PredType::NATIVE_DOUBLE, &B_min);
      rhoB_max_attr.read(H5::PredType::NATIVE_DOUBLE, &B_max);
      dB_attr.read(H5::PredType::NATIVE_DOUBLE, &dB);
      rhoQ_min_attr.read(H5::PredType::NATIVE_DOUBLE, &Q_min);
      rhoQ_max_attr.read(H5::PredType::NATIVE_DOUBLE, &Q_max);
      dQ_attr.read(H5::PredType::NATIVE_DOUBLE, &dQ);
      rhoS_min_attr.read(H5::PredType::NATIVE_DOUBLE, &S_min);
      rhoS_max_attr.read(H5::PredType::NATIVE_DOUBLE, &S_max);
      dS_attr.read(H5::PredType::NATIVE_DOUBLE, &dS);

      int Ns = std::round((s_max - s_min)/ds+1);
      int NB = std::round((B_max - B_min)/dB+1);
      int NS = std::round((S_max - S_min)/dS+1);
      int NQ = std::round((Q_max - Q_min)/dQ+1);

      //==========================================================================
      // We use a fake particle system to evaluate the EoS at the grid points
      // using the EoS interpolator
      //==========================================================================

      std::shared_ptr<ccake::SystemState<1>> systemPtr
        = std::make_shared<ccake::SystemState<1>>(settings);
      systemPtr->n_particles = Ns*NB*NS*NQ;

      systemPtr->t = 2.;
      for (int is=0; is<Ns;++is)
      for (int iB=0; iB<NB;++iB)
      for (int iS=0; iS<NS;++iS)
      for (int iQ=0; iQ<NQ;++iQ){
        ccake::Particle<1> particle;
        systemPtr->particles.push_back(particle);
      }
      cout << "s_min = " << s_min << " s_max = " << s_max << " ds = " << ds << endl;
      int idx = systemPtr->particles.size() - Ns*NB*NS*NQ;
      for (int is=0; is<Ns;++is)
      for (int iB=0; iB<NB;++iB)
      for (int iS=0; iS<NS;++iS)
      for (int iQ=0; iQ<NQ;++iQ)
      {
        auto p = &systemPtr->particles[idx];
        p->ID = idx;
        p->thermo.s    = s_min + is*ds;
        p->thermo.rhoB = B_min + iB*dB;
        p->thermo.rhoS = S_min + iS*dS;
        p->thermo.rhoQ = Q_min + iQ*dQ;
        idx++;
      }

      systemPtr->allocate_cabana_particles();
      //Get start time
      std::chrono::time_point<std::chrono::system_clock> start, end;
      start = std::chrono::system_clock::now();
      eos_interpolator.fill_thermodynamics(systemPtr->cabana_particles, 0.0);
      end = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed_seconds = end-start;
      std::cout << "EoS evaluation finished. Elapsed time: " <<
        elapsed_seconds.count() << "s" << std::endl;
      std::cout << "Performance: " << systemPtr->n_particles/elapsed_seconds.count()
        << " particles/s" << std::endl;
      systemPtr->copy_device_to_host();
      cout << "EoS evaluation finished." << endl;

      //==========================================================================
      // Check that the EoS is correctly interpolated at the grid points using
      // the original EoS inverter
      //==========================================================================


      int idx_a[nthreads]; for(int i=0; i<nthreads; ++i) idx_a[i]=0;
      const int N = systemPtr->n_particles;
      cout << "Testing EoS interpolation at grid points" << endl;
      #pragma omp parallel for schedule(dynamic)
      for (int idx=0; idx < N; ++idx)
      {

        int ith = omp_get_thread_num();
        eos[ith].tbqs(800./197., .0, .0, .0, "default");
        double s_In    = systemPtr->particles[idx].thermo.s;
        double rhoB_In = systemPtr->particles[idx].thermo.rhoB;
        double rhoS_In = systemPtr->particles[idx].thermo.rhoS;
        double rhoQ_In = systemPtr->particles[idx].thermo.rhoQ;
        bool update_s_success = eos[ith].update_s(s_In, rhoB_In,
                                                  rhoS_In, rhoQ_In, false);

        ccake::thermodynamic_info thermo;
        if ( update_s_success ) eos[ith].set_thermo( thermo );
        //Check that the EoS is correctly interpolated
        if ( std::abs(thermo.T - systemPtr->particles[idx].thermo.T ) > TOL ){
          #pragma omp critical
          {
          std::cerr << "EoS interpolation failed at point = " << idx << std::endl;
          std::cerr << "s_rootfinder        = " << std::setprecision(13) << thermo.s << " "
                    << "s_interpolator      = " << std::setprecision(13) << systemPtr->particles[idx].thermo.s << " "
                    << "s_In                = " << std::setprecision(13) << s_In << " " << endl;
          std::cerr << "rhoB_rootfinder     = " << std::setprecision(13) << thermo.rhoB << " "
                    << "rhoB_interpolator   = " << std::setprecision(13) << systemPtr->particles[idx].thermo.rhoB << " "
                    << "rhoB_In             = " << std::setprecision(13) << rhoB_In << " " << endl;
          std::cerr << "rhoS_rootfinder     = " << std::setprecision(13) << thermo.rhoS << " "
                    << "rhoS_interpolator   = " << std::setprecision(13) << systemPtr->particles[idx].thermo.rhoS << " "
                    << "rhoS_In             = " << std::setprecision(13) << rhoS_In << " " << endl;
          std::cerr << "rhoQ_rootfinder     = " << std::setprecision(13) << thermo.rhoQ << " "
                    << "rhoQ_interpolator   = " << std::setprecision(13) << systemPtr->particles[idx].thermo.rhoQ << " "
                    << "rhoQ_In             = " << std::setprecision(13) << rhoQ_In << " " << endl;
          std::cerr << "T_rootfinder        = " << std::setprecision(13) << thermo.T << " fm^-1 " << endl;
          std::cerr << "T_interpolator      = " << std::setprecision(13) << systemPtr->particles[idx].thermo.T << " fm^-1" << endl;
          std::cerr << "muB_rootfinder      = " << std::setprecision(13) << thermo.muB << " fm^-1 " << endl;
          std::cerr << "muB_interpolator    = " << std::setprecision(13) << systemPtr->particles[idx].thermo.muB << " fm^-1" << endl;
          std::cerr << "muS_rootfinder      = " << std::setprecision(13) << thermo.muS << " fm^-1 " << endl;
          std::cerr << "muS_interpolator    = " << std::setprecision(13) << systemPtr->particles[idx].thermo.muS << " fm^-1" << endl;
          std::cerr << "muQ_rootfinder      = " << std::setprecision(13) << thermo.muQ << " fm^-1 " << endl;
          std::cerr << "muQ_interpolator    = " << std::setprecision(13) << systemPtr->particles[idx].thermo.muQ << " fm^-1" << endl;
          abort();
        }
      }
        //Get end time
        if(ith==0 && idx_a[0]%100==0){
          #pragma omp critical
          {
            int idx_s=0;
            for (int i=1; i<nthreads; ++i) idx_s += idx_a[i];
            end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = end-start;
            std::cout << "Test 1 progress: " << (double)idx_s/N*100. << "% done. Elapsed time: " << elapsed_seconds.count() << "s";
            std::cout << " Estimated time remaining: " << elapsed_seconds.count()/(double)idx_s*(N-idx_s) << "s" << std::endl;
          }
        }
        idx_a[ith]++;
      }
        group.close();
    }

    cout << "Test 1 passed" << endl;
}


struct max_pos {
    double val;
    int idx;
};

const max_pos& max( const max_pos& a, const max_pos& b) {
    return a.val > b.val ? a : b;
}

#pragma omp declare reduction( get_max_and_pos: max_pos: omp_out=max( omp_out, omp_in ) )

/// @brief Evaluates the EoS at random points and checks that the
/// interpolation has the expected accuracy
/// @param file_path path to the EoS table generated by generate_eos_table
/// @param eos_type type of EoS to use
/// @param nthreads number of threads to use
void test2(std::string file_path, std::string eos_type, int nthreads){

    ccake::EoS_Interpolator eos_interpolator(file_path);
    ccake::EquationOfState eos[nthreads];

    std::shared_ptr<Settings> settings = std::make_shared<Settings>();

    //Setup EoS
    settings->eos_type = eos_type;
    if (settings->eos_type == "table"){
        settings->eos_path = "EoS/Houston/thermo.h5";
    }

    //Cannot be initialized in parallel to avoid simultaneous access to HDF5 file
    for(int i=0; i<nthreads; ++i){
      eos[i].set_SettingsPtr(settings);
      eos[i].init();
    }

    std::shared_ptr<ccake::SystemState<1>> systemPtr
        = std::make_shared<ccake::SystemState<1>>(settings);


    //==========================================================================
    // We use a fake particle system to evaluate the EoS at the grid points
    // using the EoS interpolator
    //==========================================================================

    settings->xmin = -4;
    settings->ymin = -6;
    settings->etamin = -2;
    settings->hT = 1;
    settings->using_shear = true;
    settings->t0 = 2;
    systemPtr->initialize();

    const int N = 1000000;
    systemPtr->n_particles = N;
    cout << "Allocating fake particle system with "
        << systemPtr->n_particles << " particles."<< endl;
    systemPtr->t = 2.;
    for (int i=0; i<N;++i)
    {
      ccake::Particle<1> particle;
      systemPtr->particles.push_back(particle);
    }

    double s_max = 90; double s_min = 0.001;
    double B_max = 9;  double B_min = 0.001;
    double S_max = 9;  double S_min = 0.001;
    double Q_max = 9;  double Q_min = 0.001;
    //Set seed for reproducibility
    srand(42);
    int idx = 0;
    for (int i=0; i<N;++i)
    {
      ccake::Particle<1>* p = &systemPtr->particles[idx];
      p->ID = idx;
      p->thermo.s = (double)rand()/RAND_MAX*(s_max-s_min)+s_min;
      p->thermo.rhoB = (double)rand()/RAND_MAX*(B_max-B_min)+B_min;
      p->thermo.rhoS = (double)rand()/RAND_MAX*(S_max-S_min)+S_min;
      p->thermo.rhoQ = (double)rand()/RAND_MAX*(Q_max-Q_min)+Q_min;
      idx++;
    }

    std::chrono::time_point<std::chrono::system_clock> start, end;
    cout << "Evaluating EoS at grid points" << endl;
    systemPtr->allocate_cabana_particles();
    start = std::chrono::system_clock::now();
    eos_interpolator.fill_thermodynamics(systemPtr->cabana_particles, 0.0);
    end = std::chrono::system_clock::now();
    cout << "EoS evaluation finished. Performance: "
         << N/std::chrono::duration<double>(end-start).count()
         << " particles/s" << endl;
    systemPtr->copy_device_to_host();


    //==========================================================================
    // Check that the EoS is correctly interpolated at the grid points using
    // the original EoS inverter
    //==========================================================================

    //Get sart time
    start = std::chrono::system_clock::now();

    int idx_a[nthreads]; for(int i=0; i<nthreads; ++i) idx_a[i]=0;
    cout << "Testing EoS interpolation at grid points" << endl;
    max_pos max_error_T = {0, 0};
    max_pos max_error_muB = {0, 0};
    max_pos max_error_muS = {0, 0};
    max_pos max_error_muQ = {0, 0};
    max_pos max_error_P = {0, 0};
    #pragma omp parallel for schedule(dynamic) reduction(get_max_and_pos: max_error_T, max_error_muB, max_error_muS, max_error_muQ, max_error_P)
    for (int idx=0; idx<N;++idx)
    {

      int ith = omp_get_thread_num();
      eos[ith].tbqs(800./197., .0, .0, .0, "default");
      auto p = &systemPtr->particles[idx];
      double s_In = p->thermo.s;
      double rhoB_In = p->thermo.rhoB;
      double rhoS_In = p->thermo.rhoS;
      double rhoQ_In = p->thermo.rhoQ;
      bool update_s_success = eos[ith].update_s(s_In, rhoB_In, rhoS_In, rhoQ_In, false);

      ccake::thermodynamic_info thermo;
      if ( update_s_success ) eos[ith].set_thermo( thermo );
      //Check that the EoS is correctly interpolated
      max_pos T_aux = {std::abs(thermo.T - p->thermo.T), idx};
      max_pos muB_aux = {std::abs(thermo.muB - p->thermo.muB), idx};
      max_pos muS_aux = {std::abs(thermo.muS - p->thermo.muS), idx};
      max_pos muQ_aux = {std::abs(thermo.muQ - p->thermo.muQ), idx};
      max_pos P_aux = {std::abs(thermo.p - p->thermo.p), idx};
      #pragma omp critical
      {
        max_error_T = max(T_aux, max_error_T);
        max_error_muB = max(muB_aux, max_error_muB);
        max_error_muS = max(muS_aux, max_error_muS);
        max_error_muQ = max(muQ_aux, max_error_muQ);
        max_error_P = max(P_aux, max_error_P);
      }

      //Get end time
      if(ith==0 && idx_a[0]%10000==0){
        #pragma omp critical
        {
          int idx_s=0;
          for (int i=1; i<nthreads; ++i) idx_s += idx_a[i];
          end = std::chrono::system_clock::now();
          std::chrono::duration<double> elapsed_seconds = end-start;
          std::cout << "Test 2 progress: " << (double)idx_s/N*100. << "% done. Elapsed time: " << elapsed_seconds.count() << "s";
          std::cout << " Estimated time remaining: " << elapsed_seconds.count()/(double)idx_s*(N-idx_s) << "s" << std::endl;
        }
      }
      idx_a[ith]++;
    }
    end = std::chrono::system_clock::now();
    cout << "Test 2 finished. Report: " << endl;
    cout << "Total time......: " << std::chrono::duration<double>(end-start).count() << " s" << endl;
    cout << "Max error in T..: " << max_error_T.val*constants::hbarc_MeVfm << " MeV" << endl;
    cout << " - s    = " << systemPtr->particles[max_error_T.idx].thermo.s << endl;
    cout << " - rhoB = " << systemPtr->particles[max_error_T.idx].thermo.rhoB << endl;
    cout << " - rhoS = " << systemPtr->particles[max_error_T.idx].thermo.rhoS << endl;
    cout << " - rhoQ = " << systemPtr->particles[max_error_T.idx].thermo.rhoQ << endl;
    cout << "Max error in muB: " << max_error_muB.val*constants::hbarc_MeVfm << " MeV" << endl;
    cout << " - s    = " << systemPtr->particles[max_error_muB.idx].thermo.s << endl;
    cout << " - rhoB = " << systemPtr->particles[max_error_muB.idx].thermo.rhoB << endl;
    cout << " - rhoS = " << systemPtr->particles[max_error_muB.idx].thermo.rhoS << endl;
    cout << " - rhoQ = " << systemPtr->particles[max_error_muB.idx].thermo.rhoQ << endl;
    cout << "Max error in muS: " << max_error_muS.val*constants::hbarc_MeVfm << " MeV" << endl;
    cout << " - s    = " << systemPtr->particles[max_error_muS.idx].thermo.s << endl;
    cout << " - rhoB = " << systemPtr->particles[max_error_muS.idx].thermo.rhoB << endl;
    cout << " - rhoS = " << systemPtr->particles[max_error_muS.idx].thermo.rhoS << endl;
    cout << " - rhoQ = " << systemPtr->particles[max_error_muS.idx].thermo.rhoQ << endl;
    cout << "Max error in muQ: " << max_error_muQ.val*constants::hbarc_MeVfm << " MeV" << endl;
    cout << " - s    = " << systemPtr->particles[max_error_muQ.idx].thermo.s << endl;
    cout << " - rhoB = " << systemPtr->particles[max_error_muQ.idx].thermo.rhoB << endl;
    cout << " - rhoS = " << systemPtr->particles[max_error_muQ.idx].thermo.rhoS << endl;
    cout << " - rhoQ = " << systemPtr->particles[max_error_muQ.idx].thermo.rhoQ << endl;
    cout << "Max error in P..: " << max_error_P.val*constants::hbarc_MeVfm << " MeV/fm³" << endl;
    cout << " - s    = " << systemPtr->particles[max_error_P.idx].thermo.s << endl;
    cout << " - rhoB = " << systemPtr->particles[max_error_P.idx].thermo.rhoB << endl;
    cout << " - rhoS = " << systemPtr->particles[max_error_P.idx].thermo.rhoS << endl;
    cout << " - rhoQ = " << systemPtr->particles[max_error_P.idx].thermo.rhoQ << endl;
}


int main(int argc, char* argv[]){
    Kokkos::ScopeGuard scope_guard(argc, argv); //Create Kokkos scope guard to
    // ensure that Kokkos is properly initialized and finalized
    check_args(argc, argv);

    //Setup openMP
    int nthreads = atoi(argv[1]);
    omp_set_num_threads(nthreads);


    // test1(argv[3], argv[2], nthreads);
    test2(argv[3], argv[2], nthreads);
}
