#include "eos.h"

#include "../splinter/include/datatable.h"
#include "../splinter/include/bspline.h"
#include "../splinter/include/bsplinebuilder.h"
#include <gsl/gsl_multiroots.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_permutation.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

using std::vector;
using std::string;

using namespace SPLINTER;

// Compile:         gcc eos4D.cpp -c -I /usr/include/eigen3 -Lsplinter/build -lm -lgsl -lgslcblas -lstdc++ -lsplinter-3-0


//EoS constructor. Builds the splines of degree "degree" for each quantitiy and initializes the position at (30,0,0,0)
eos::eos(string quantityFile, string derivFile, int degree) : pSpline(4), entrSpline(4), bSpline(4), sSpline(4), qSpline(4), eSpline(4), cs2Spline(4), db2Spline(4), dq2Spline(4), ds2Spline(4), dt2Spline(4), dbdqSpline(4), dbdsSpline(4), dtdbSpline(4), dqdsSpline(4), dtdqSpline(4), dtdsSpline(4), tbqsPosition(4) {
    init(quantityFile, derivFile, degree);
}

//EoS default constructor. This function exists to satisfy the compiler
//This function should never be called unless init is called directly afterward
eos::eos() : pSpline(4), entrSpline(4), bSpline(4), sSpline(4), qSpline(4), eSpline(4), cs2Spline(4), db2Spline(4), dq2Spline(4), ds2Spline(4), dt2Spline(4), dbdqSpline(4), dbdsSpline(4), dtdbSpline(4), dqdsSpline(4), dtdqSpline(4), dtdsSpline(4), tbqsPosition(4) {}

void eos::init(string quantityFile, string derivFile, int degree) {
    std::ifstream dataFile;
    std::ifstream derFile;
    dataFile.open(quantityFile);
    derFile.open(derivFile);

    DataTable psamples, entrsamples, bsamples, ssamples, qsamples, esamples, cs2samples;
    DataTable db2samples, ds2samples, dq2samples, dt2samples, dbdssamples, dbdqsamples, dqdssamples, dtdssamples, dtdqsamples, dtdbsamples;

    double tit, muBit, muQit, muSit, pit, entrit, bit, sit, qit, eit, cs2it;
    double db2it, dq2it, ds2it, dt2it, dbdqit, dbdsit, dqdsit, dtdbit, dtdsit, dtdqit;
    vector<double> toAdd;

    int count = 0;
    double hbarc = 197.327;
    while (dataFile >> tit >> muBit >> muQit >> muSit >> pit >> entrit >> bit >> sit >> qit >> eit >> cs2it) {
        derFile >> tit >> muBit >> muQit >> muSit >> db2it >> dq2it >> ds2it >> dbdqit >> dbdsit >> dqdsit >> dtdbit >> dtdqit >> dtdsit >> dt2it;  //read data from files
        if(count == 0) {
            minT = tit;
            maxT = tit;
            minMuB = muBit;
            maxMuB = muBit;     //initilaize eos range variables
            minMuQ = muQit;
            maxMuQ = muQit;
            minMuS = muSit;
            maxMuS = muSit;
            ++count;
        }
        if(maxT < tit) {
            maxT = tit;
        }
        if(minT > tit) {
            minT = tit;
        }
        if(maxMuB < muBit) {
            maxMuB = muBit;
        }
        if(minMuB > muBit) {
            minMuB = muBit;
        }
        if(maxMuQ < muQit) {
            maxMuQ = muQit;
        }
        if(minMuQ > muQit) {
            minMuQ = muQit;
        }
        if(maxMuS < muSit) {
            maxMuS = muSit;
        }
        if(minMuS > muSit) {
            minMuS = muSit;
        }

        toAdd.push_back(tit);
        toAdd.push_back(muBit);
        toAdd.push_back(muQit);
        toAdd.push_back(muSit);

        pit = pit*(tit*tit*tit*tit)/(hbarc*hbarc*hbarc);
        entrit = entrit*(tit*tit*tit)/(hbarc*hbarc*hbarc);
        bit = bit*(tit*tit*tit)/(hbarc*hbarc*hbarc);
        sit = sit*(tit*tit*tit)/(hbarc*hbarc*hbarc);        //convert to MeV and fm units
        qit = qit*(tit*tit*tit)/(hbarc*hbarc*hbarc);
        eit = eit*(tit*tit*tit*tit)/(hbarc*hbarc*hbarc);

        psamples.addSample(toAdd, pit);
        entrsamples.addSample(toAdd, entrit);
        bsamples.addSample(toAdd, bit);
        ssamples.addSample(toAdd, sit);
        qsamples.addSample(toAdd, qit);
        esamples.addSample(toAdd, eit);
        cs2samples.addSample(toAdd, cs2it);
        db2samples.addSample(toAdd, db2it);
        dq2samples.addSample(toAdd, dq2it);     //add datapoint to table for spline builder
        ds2samples.addSample(toAdd, ds2it);
        dbdqsamples.addSample(toAdd, dbdqit);
        dbdssamples.addSample(toAdd, dbdsit);
        dqdssamples.addSample(toAdd, dqdsit);
        dtdbsamples.addSample(toAdd, dtdbit);
        dtdqsamples.addSample(toAdd, dtdqit);
        dtdssamples.addSample(toAdd, dtdsit);
        dt2samples.addSample(toAdd, dt2it);
        toAdd.clear();
    }

    dataFile.close();
    derFile.close();

    pSpline = BSpline::Builder(psamples).degree(degree).build();
    entrSpline = BSpline::Builder(entrsamples).degree(degree).build();
    bSpline = BSpline::Builder(bsamples).degree(degree).build();
    sSpline = BSpline::Builder(ssamples).degree(degree).build();
    qSpline = BSpline::Builder(qsamples).degree(degree).build();
    eSpline = BSpline::Builder(esamples).degree(degree).build();
    cs2Spline = BSpline::Builder(cs2samples).degree(degree).build();
    db2Spline = BSpline::Builder(db2samples).degree(degree).build();
    dq2Spline = BSpline::Builder(dq2samples).degree(degree).build();
    ds2Spline = BSpline::Builder(ds2samples).degree(degree).build();        //make splines from table
    dbdqSpline = BSpline::Builder(dbdqsamples).degree(degree).build();
    dbdsSpline = BSpline::Builder(dbdssamples).degree(degree).build();
    dqdsSpline = BSpline::Builder(dqdssamples).degree(degree).build();
    dtdbSpline = BSpline::Builder(dtdbsamples).degree(degree).build();
    dtdqSpline = BSpline::Builder(dtdqsamples).degree(degree).build();
    dtdsSpline = BSpline::Builder(dtdssamples).degree(degree).build();
    dt2Spline = BSpline::Builder(dt2samples).degree(degree).build();

    return;
}

void eos::tbqs(double setT, double setmuB, double setmuQ, double setmuS) {
    if(setT < minT || setT > maxT) {
        std::cout << "T = " << setT << " is out of range. Valid values are between [" << minT << "," << maxT << "]" << std::endl;
        return;
    }
    if(setmuB < minMuB || setmuB > maxMuB) {
        std::cout << "muB = " << setmuB << " is out of range. Valid values are between [" << minMuB << "," << maxMuB << "]" << std::endl;
        return;
    }
    if(setmuQ < minMuQ || setmuQ > maxMuQ) {
        std::cout << "muQ = " << setmuQ << " is out of range. Valid values are between [" << minMuQ << "," << maxMuQ << "]" << std::endl;
        return;
    }
    if(setmuS < minMuS || setmuS > maxMuS) {
        std::cout << "muS = " << setmuS << " is out of range. Valid values are between [" << minMuS << "," << maxMuS << "]" << std::endl;
        return;
    }
    tbqsPosition(0) = setT;
    tbqsPosition(1) = setmuB;
    tbqsPosition(2) = setmuQ;
    tbqsPosition(3) = setmuS;

    pVal = pSpline.eval(tbqsPosition);
    BVal = bSpline.eval(tbqsPosition);
    SVal = sSpline.eval(tbqsPosition);
    QVal = qSpline.eval(tbqsPosition);
    eVal = eSpline.eval(tbqsPosition);
    cs2Val = cs2Spline.eval(tbqsPosition);
    db2 = db2Spline.eval(tbqsPosition);
    ds2 = ds2Spline.eval(tbqsPosition);
    dq2 = dq2Spline.eval(tbqsPosition);
    dt2 = dt2Spline.eval(tbqsPosition);
    dbdq = dbdqSpline.eval(tbqsPosition);
    dbds = dbdsSpline.eval(tbqsPosition);
    dsdq = dqdsSpline.eval(tbqsPosition);
    dtdb = dtdbSpline.eval(tbqsPosition);
    dtds = dtdsSpline.eval(tbqsPosition);
    dtdq = dtdqSpline.eval(tbqsPosition);

    entrVal = (eVal + pVal - setmuB*BVal - setmuQ*QVal - setmuS*SVal)/setT;
}

double eos::p() {
    return pVal;
}

double eos::s() {
    return entrVal;
}

double eos::B() {
    return BVal;
}

double eos::S() {
    return SVal;
}

double eos::Q() {
    return QVal;
}

double eos::e() {
    return eVal;
}

double eos::cs2() {
    return cs2Val;
}


double eos::w() {
    return eVal + pVal;
}



double eos::T() {
    return tbqsPosition(0);
}

double eos::muB() {
    return tbqsPosition(1);
}

double eos::muQ() {
    return tbqsPosition(2);
}

double eos::muS() {
    return tbqsPosition(3);
}



double eos::dwds() {
    return T() + entrVal/dentr_dt() + BVal/dentr_dmub() + QVal/dentr_dmuq() + SVal/dentr_dmus();
}

double eos::dwdB() {
    return muB() + entrVal/db_dt() + BVal/db_dmub() + QVal/db_dmuq() + SVal/db_dmus();
}

double eos::dwdS() {
    return muS() + entrVal/ds_dt() + BVal/ds_dmub() + QVal/ds_dmuq() + SVal/ds_dmus();
}

double eos::dwdQ() {
    return muQ() + entrVal/dq_dt() + BVal/dq_dmub() + QVal/dq_dmuq() + SVal/dq_dmus();
}

double eos::dentr_dt() {
    return calc_term_1();
}

double eos::dentr_dmub() {
    return calc_term_2("b");
}

double eos::dentr_dmuq() {
    return calc_term_2("q");
}

double eos::dentr_dmus() {
    return calc_term_2("s");
}

double eos::db_dt() {
    return calc_term_3("b");
}

double eos::db_dmub() {
    return calc_term_4("b","b");
}

double eos::db_dmuq() {
    return calc_term_4("b","q");
}

double eos::db_dmus() {
    return calc_term_4("b","s");
}

double eos::ds_dt() {
    return calc_term_3("s");
}

double eos::ds_dmub() {
    return calc_term_4("s","b");
}

double eos::ds_dmuq() {
    return calc_term_4("s","q");
}

double eos::ds_dmus() {
    return calc_term_4("s","s");
}

double eos::dq_dt() {
    return calc_term_3("q");
}

double eos::dq_dmub() {
    return calc_term_4("q","b");
}

double eos::dq_dmuq() {
    return calc_term_4("q","q");
}

double eos::dq_dmus() {
    return calc_term_4("q","s");
}

double eos::calc_term_1() {
    gsl_vector *v = gsl_vector_alloc(3);
    gsl_matrix *m = gsl_matrix_alloc(3,3);

    gsl_vector_set(v,0,dtdb);
    gsl_vector_set(v,1,dtds);
    gsl_vector_set(v,2,dtdq);

    gsl_matrix_set(m,0,0,db2);
    gsl_matrix_set(m,0,1,dbds);
    gsl_matrix_set(m,0,2,dbdq);
    gsl_matrix_set(m,1,0,dbds);
    gsl_matrix_set(m,1,1,ds2);
    gsl_matrix_set(m,1,2,dsdq);
    gsl_matrix_set(m,2,0,dbdq);
    gsl_matrix_set(m,2,1,dsdq);
    gsl_matrix_set(m,2,2,dq2);

    double toReturn = dt2 - deriv_mult_aTm_1b(v,m,v);

    gsl_matrix_free(m);
    gsl_vector_free(v);
    return toReturn;
}

double eos::calc_term_2(string i_char) {
    gsl_vector *a = gsl_vector_alloc(3);
    gsl_matrix *m = gsl_matrix_alloc(3,3);
    gsl_vector *b = gsl_vector_alloc(3);
    double toReturn = 0;

    if (i_char == "b") {
        gsl_vector_set(a,0,dt2);
        gsl_vector_set(a,1,dtds);
        gsl_vector_set(a,2,dtdq);

        gsl_vector_set(b,0,db2);
        gsl_vector_set(b,1,dbds);
        gsl_vector_set(b,2,dbdq);

        gsl_matrix_set(m,0,0,dtdb);
        gsl_matrix_set(m,0,1,dbds);
        gsl_matrix_set(m,0,2,dbdq);
        gsl_matrix_set(m,1,0,dtds);
        gsl_matrix_set(m,1,1,ds2);
        gsl_matrix_set(m,1,2,dsdq);
        gsl_matrix_set(m,2,0,dtdq);
        gsl_matrix_set(m,2,1,dsdq);
        gsl_matrix_set(m,2,2,dq2);

        toReturn = dtdb - deriv_mult_aTm_1b(a,m,b);
    } else if (i_char == "s") {
        gsl_vector_set(a,0,dt2);
        gsl_vector_set(a,1,dtdb);
        gsl_vector_set(a,2,dtdq);

        gsl_vector_set(b,0,dbds);
        gsl_vector_set(b,1,ds2);
        gsl_vector_set(b,2,dsdq);

        gsl_matrix_set(m,0,0,dtdb);
        gsl_matrix_set(m,0,1,db2);
        gsl_matrix_set(m,0,2,dbdq);
        gsl_matrix_set(m,1,0,dtds);
        gsl_matrix_set(m,1,1,dbds);
        gsl_matrix_set(m,1,2,dsdq);
        gsl_matrix_set(m,2,0,dtdq);
        gsl_matrix_set(m,2,1,dbdq);
        gsl_matrix_set(m,2,2,dq2);

        toReturn = dtds - deriv_mult_aTm_1b(a,m,b);
    } else if (i_char == "q") {
        gsl_vector_set(a,0,dt2);
        gsl_vector_set(a,1,dtdb);
        gsl_vector_set(a,2,dtds);

        gsl_vector_set(b,0,dbdq);
        gsl_vector_set(b,1,dsdq);
        gsl_vector_set(b,2,dq2);

        gsl_matrix_set(m,0,0,dtdb);
        gsl_matrix_set(m,0,1,db2);
        gsl_matrix_set(m,0,2,dbdq);
        gsl_matrix_set(m,1,0,dtds);
        gsl_matrix_set(m,1,1,dbds);
        gsl_matrix_set(m,1,2,dsdq);
        gsl_matrix_set(m,2,0,dtdq);
        gsl_matrix_set(m,2,1,dbdq);
        gsl_matrix_set(m,2,2,dq2);

        toReturn = dtdq - deriv_mult_aTm_1b(a,m,b);
    } else {
        std::cout << "Error calculating derivative term 2" << std::endl;
    }

    gsl_vector_free(a);
    gsl_matrix_free(m);
    gsl_vector_free(b);
    return toReturn;
}

double eos::calc_term_3(string i_char) {
    gsl_vector *a = gsl_vector_alloc(3);
    gsl_matrix *m = gsl_matrix_alloc(3,3);
    gsl_vector *b = gsl_vector_alloc(3);
    double toReturn = 0;

    if (i_char == "b") {
        gsl_vector_set(a,0,db2);
        gsl_vector_set(a,1,dbds);
        gsl_vector_set(a,2,dbdq);

        gsl_vector_set(b,0,dt2);
        gsl_vector_set(b,1,dtds);
        gsl_vector_set(b,2,dtdq);

        gsl_matrix_set(m,0,0,dtdb);
        gsl_matrix_set(m,0,1,dtds);
        gsl_matrix_set(m,0,2,dtdq);
        gsl_matrix_set(m,1,0,dbds);
        gsl_matrix_set(m,1,1,ds2);
        gsl_matrix_set(m,1,2,dsdq);
        gsl_matrix_set(m,2,0,dbdq);
        gsl_matrix_set(m,2,1,dsdq);
        gsl_matrix_set(m,2,2,dq2);

        toReturn = dtdb - deriv_mult_aTm_1b(a,m,b);
    } else if (i_char == "s") {
        gsl_vector_set(a,0,dbds);
        gsl_vector_set(a,1,ds2);
        gsl_vector_set(a,2,dsdq);

        gsl_vector_set(b,0,dt2);
        gsl_vector_set(b,1,dtdb);
        gsl_vector_set(b,2,dtdq);

        gsl_matrix_set(m,0,0,dtdb);
        gsl_matrix_set(m,0,1,dtds);
        gsl_matrix_set(m,0,2,dtdq);
        gsl_matrix_set(m,1,0,db2);
        gsl_matrix_set(m,1,1,dbds);
        gsl_matrix_set(m,1,2,dbdq);
        gsl_matrix_set(m,2,0,dbdq);
        gsl_matrix_set(m,2,1,dsdq);
        gsl_matrix_set(m,2,2,dq2);

        toReturn = dtds - deriv_mult_aTm_1b(a,m,b);
    } else if (i_char == "q") {
        gsl_vector_set(a,0,dbdq);
        gsl_vector_set(a,1,dsdq);
        gsl_vector_set(a,2,dq2);

        gsl_vector_set(b,0,dt2);
        gsl_vector_set(b,1,dtdb);
        gsl_vector_set(b,2,dtds);

        gsl_matrix_set(m,0,0,dtdb);
        gsl_matrix_set(m,0,1,dtds);
        gsl_matrix_set(m,0,2,dtdq);
        gsl_matrix_set(m,1,0,db2);
        gsl_matrix_set(m,1,1,dbds);
        gsl_matrix_set(m,1,2,dbdq);
        gsl_matrix_set(m,2,0,dbds);
        gsl_matrix_set(m,2,1,ds2);
        gsl_matrix_set(m,2,2,dsdq);

        toReturn = dtdq - deriv_mult_aTm_1b(a,m,b);
    } else {
        std::cout << "Error calculating derivative term 3" << std::endl;
    }

    gsl_vector_free(a);
    gsl_matrix_free(m);
    gsl_vector_free(b);
    return toReturn;
}

double eos::calc_term_4(string j_char, string i_char) {
    gsl_vector *a = gsl_vector_alloc(3);
    gsl_matrix *m = gsl_matrix_alloc(3,3);
    gsl_vector *b = gsl_vector_alloc(3);
    double toReturn = 0;

    if (i_char == "b") {
        if(j_char == "b") {
            gsl_vector_set(a,0,dtdb);
            gsl_vector_set(a,1,dbds);
            gsl_vector_set(a,2,dbdq);

            gsl_vector_set(b,0,dtdb);
            gsl_vector_set(b,1,dbds);
            gsl_vector_set(b,2,dbdq);

            gsl_matrix_set(m,0,0,dt2);
            gsl_matrix_set(m,0,1,dtds);
            gsl_matrix_set(m,0,2,dtdq);
            gsl_matrix_set(m,1,0,dtds);
            gsl_matrix_set(m,1,1,ds2);
            gsl_matrix_set(m,1,2,dsdq);
            gsl_matrix_set(m,2,0,dtdq);
            gsl_matrix_set(m,2,1,dsdq);
            gsl_matrix_set(m,2,2,dq2);

            toReturn = db2 - deriv_mult_aTm_1b(a,m,b);
        } else if (j_char == "s") {
            gsl_vector_set(a,0,dtds);
            gsl_vector_set(a,1,ds2);
            gsl_vector_set(a,2,dsdq);

            gsl_vector_set(b,0,dtdb);
            gsl_vector_set(b,1,db2);
            gsl_vector_set(b,2,dbdq);

            gsl_matrix_set(m,0,0,dt2);
            gsl_matrix_set(m,0,1,dtds);
            gsl_matrix_set(m,0,2,dtdq);
            gsl_matrix_set(m,1,0,dtdb);
            gsl_matrix_set(m,1,1,dbds);
            gsl_matrix_set(m,1,2,dbdq);
            gsl_matrix_set(m,2,0,dtdq);
            gsl_matrix_set(m,2,1,dsdq);
            gsl_matrix_set(m,2,2,dq2);

            toReturn = dbds - deriv_mult_aTm_1b(a,m,b);
        } else if (j_char == "q") {
            gsl_vector_set(a,0,dtdq);
            gsl_vector_set(a,1,dsdq);
            gsl_vector_set(a,2,dq2);

            gsl_vector_set(b,0,dtdb);
            gsl_vector_set(b,1,db2);
            gsl_vector_set(b,2,dbds);

            gsl_matrix_set(m,0,0,dt2);
            gsl_matrix_set(m,0,1,dtds);
            gsl_matrix_set(m,0,2,dtdq);
            gsl_matrix_set(m,1,0,dtdb);
            gsl_matrix_set(m,1,1,dbds);
            gsl_matrix_set(m,1,2,dbdq);
            gsl_matrix_set(m,2,0,dtds);
            gsl_matrix_set(m,2,1,ds2);
            gsl_matrix_set(m,2,2,dsdq);

            toReturn = dbdq - deriv_mult_aTm_1b(a,m,b);
        } else {
            std::cout << "Error calculating derivative term 4" << std::endl;
        }
    } else if (i_char == "s") {
        if(j_char == "b") {
            gsl_vector_set(a,0,dtdb);
            gsl_vector_set(a,1,db2);
            gsl_vector_set(a,2,dbdq);

            gsl_vector_set(b,0,dtds);
            gsl_vector_set(b,1,ds2);
            gsl_vector_set(b,2,dsdq);

            gsl_matrix_set(m,0,0,dt2);
            gsl_matrix_set(m,0,1,dtdb);
            gsl_matrix_set(m,0,2,dtdq);
            gsl_matrix_set(m,1,0,dtds);
            gsl_matrix_set(m,1,1,dbds);
            gsl_matrix_set(m,1,2,dsdq);
            gsl_matrix_set(m,2,0,dtdq);
            gsl_matrix_set(m,2,1,dbdq);
            gsl_matrix_set(m,2,2,dq2);

            toReturn = dbds - deriv_mult_aTm_1b(a,m,b);
        } else if (j_char == "s") {
            gsl_vector_set(a,0,dtds);
            gsl_vector_set(a,1,dbds);
            gsl_vector_set(a,2,dsdq);

            gsl_vector_set(b,0,dtds);
            gsl_vector_set(b,1,dbds);
            gsl_vector_set(b,2,dsdq);

            gsl_matrix_set(m,0,0,dt2);
            gsl_matrix_set(m,0,1,dtdb);
            gsl_matrix_set(m,0,2,dtdq);
            gsl_matrix_set(m,1,0,dtdb);
            gsl_matrix_set(m,1,1,db2);
            gsl_matrix_set(m,1,2,dbdq);
            gsl_matrix_set(m,2,0,dtdq);
            gsl_matrix_set(m,2,1,dbdq);
            gsl_matrix_set(m,2,2,dq2);

            toReturn = ds2 - deriv_mult_aTm_1b(a,m,b);
        } else if (j_char == "q") {
            gsl_vector_set(a,0,dtdq);
            gsl_vector_set(a,1,dbdq);
            gsl_vector_set(a,2,dq2);

            gsl_vector_set(b,0,dtds);
            gsl_vector_set(b,1,dbds);
            gsl_vector_set(b,2,ds2);

            gsl_matrix_set(m,0,0,dt2);
            gsl_matrix_set(m,0,1,dtdb);
            gsl_matrix_set(m,0,2,dtdq);
            gsl_matrix_set(m,1,0,dtdb);
            gsl_matrix_set(m,1,1,db2);
            gsl_matrix_set(m,1,2,dbdq);
            gsl_matrix_set(m,2,0,dtds);
            gsl_matrix_set(m,2,1,dbds);
            gsl_matrix_set(m,2,2,dsdq);

            toReturn = dsdq - deriv_mult_aTm_1b(a,m,b);
        } else {
            std::cout << "Error calculating derivative term 4" << std::endl;
        }
    } else if (i_char == "q") {
        if(j_char == "b") {
            gsl_vector_set(a,0,dtdb);
            gsl_vector_set(a,1,db2);
            gsl_vector_set(a,2,dbds);

            gsl_vector_set(b,0,dtdq);
            gsl_vector_set(b,1,dsdq);
            gsl_vector_set(b,2,dq2);

            gsl_matrix_set(m,0,0,dt2);
            gsl_matrix_set(m,0,1,dtdb);
            gsl_matrix_set(m,0,2,dtds);
            gsl_matrix_set(m,1,0,dtds);
            gsl_matrix_set(m,1,1,dbds);
            gsl_matrix_set(m,1,2,ds2);
            gsl_matrix_set(m,2,0,dtdq);
            gsl_matrix_set(m,2,1,dbdq);
            gsl_matrix_set(m,2,2,dsdq);

            toReturn = dbdq - deriv_mult_aTm_1b(a,m,b);
        } else if (j_char == "s") {
            gsl_vector_set(a,0,dtds);
            gsl_vector_set(a,1,dbds);
            gsl_vector_set(a,2,ds2);

            gsl_vector_set(b,0,dtdq);
            gsl_vector_set(b,1,dbdq);
            gsl_vector_set(b,2,dq2);

            gsl_matrix_set(m,0,0,dt2);
            gsl_matrix_set(m,0,1,dtdb);
            gsl_matrix_set(m,0,2,dtds);
            gsl_matrix_set(m,1,0,dtdb);
            gsl_matrix_set(m,1,1,db2);
            gsl_matrix_set(m,1,2,dbds);
            gsl_matrix_set(m,2,0,dtdq);
            gsl_matrix_set(m,2,1,dbdq);
            gsl_matrix_set(m,2,2,dsdq);

            toReturn = dsdq - deriv_mult_aTm_1b(a,m,b);
        } else if (j_char == "q") {
            gsl_vector_set(a,0,dtdq);
            gsl_vector_set(a,1,dbdq);
            gsl_vector_set(a,2,dsdq);

            gsl_vector_set(b,0,dtdq);
            gsl_vector_set(b,1,dbdq);
            gsl_vector_set(b,2,dsdq);

            gsl_matrix_set(m,0,0,dt2);
            gsl_matrix_set(m,0,1,dtdb);
            gsl_matrix_set(m,0,2,dtds);
            gsl_matrix_set(m,1,0,dtdb);
            gsl_matrix_set(m,1,1,db2);
            gsl_matrix_set(m,1,2,dbds);
            gsl_matrix_set(m,2,0,dtds);
            gsl_matrix_set(m,2,1,dbds);
            gsl_matrix_set(m,2,2,ds2);

            toReturn = dq2 - deriv_mult_aTm_1b(a,m,b);
        } else {
            std::cout << "Error calculating derivative term 4" << std::endl;
        }
    } else {
        std::cout << "Error calculating derivative term 4" << std::endl;
    }

    gsl_vector_free(a);
    gsl_matrix_free(m);
    gsl_vector_free(b);
    return toReturn;
}

double eos::deriv_mult_aTm_1b(gsl_vector* a, gsl_matrix* m, gsl_vector* b) {
    gsl_permutation *p = gsl_permutation_alloc(3);
    int s;

    // Compute the LU decomposition of this matrix
    gsl_linalg_LU_decomp(m, p, &s);

    // Compute the  inverse of the LU decomposition
    gsl_matrix *minv = gsl_matrix_alloc(3, 3);
    gsl_linalg_LU_invert(m, p, minv);

    gsl_vector *y = gsl_vector_alloc(3);

    // Compute y = m^-1 @ b
    gsl_blas_dgemv(CblasNoTrans,1,m,b,0,y);

    double toReturn = 0;
    //compute toReturn = aT @ y
    gsl_blas_ddot(a,y,&toReturn);

    gsl_vector_free(y);
    gsl_matrix_free(minv);
    gsl_permutation_free(p);

    return toReturn;
}

double eos::Atable()
{
    Aout=w()-entrVal*dwds();

    return Aout;
};

double eos::cs2out(double Tt) {  //return cs2 given t and mu's=0
    tbqs(Tt, 0.0, 0.0, 0.0);
    return cs2Val;
}

double eos::cs2out(double Tt, double muBin, double muQin, double muSin) {  //return cs2 given t and mu's
    tbqs(Tt, muBin, muQin, muSin);
    return cs2Val;
}

double eos::wfz(double Tt) {   // return e + p for tbqs
    tbqs(Tt, 0.0, 0.0, 0.0);
    return eVal + pVal;
}

double eos::wfz(double Tt, double muBin, double muQin, double muSin) {   // return e + p for tbqs
    tbqs(Tt, muBin, muQin, muSin);
    return eVal + pVal;
}

bool eos::update_s(double sin) { //update the t position (mu=0) based on input. Returns 1 if found, returns 0 if failed
    return update_s(sin, 0.0, 0.0, 0.0);
}

bool eos::update_s(double sin, double Bin, double Sin, double Qin) { //update the t and mu position based on input. Returns 1 if found, returns 0 if failed
    if (rootfinder4D(sin, 0, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return true;
    }
    double t0 = tbqsPosition(0);
    double mub0 = tbqsPosition(1);
    double muq0 = tbqsPosition(2);
    double mus0 = tbqsPosition(3);
    double t10 = t0*.2;
    double muB10 = mub0*.2;
    double muQ10 = muq0*.2;
    double muS10 = mus0*.2;

    //perturb T
    if(t0 + t10 > maxT) {
        tbqs(maxT - 1, mub0, muq0, mus0);
    } else {
        tbqs(t0 + t10, mub0, muq0, mus0);
    }
    if(rootfinder4D(sin, 0, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return true;
    }
    if(t0 - t10 < minT) {
        tbqs(minT + 1, mub0, muq0, mus0);
    } else {
        tbqs(t0 - t10, mub0, muq0, mus0);
    }
    if(rootfinder4D(sin, 0, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return true;
    }

    //perturb mub
    if(mub0 + muB10 > maxMuB) {
        tbqs(t0, maxMuB - 1, muq0, mus0);
    } else {
        tbqs(t0, mub0 + muB10, muq0, mus0);
    }
    if(rootfinder4D(sin, 0, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return true;
    }
    if(mub0 - muB10 < minMuB) {
        tbqs(t0, minMuB + 1, muq0, mus0);
    } else {
        tbqs(t0, mub0 - muB10, muq0, mus0);
    }
    if(rootfinder4D(sin, 0, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return true;
    }

    //perturn muq
    if(muq0 + muQ10 > maxMuQ) {
        tbqs(t0, mub0, maxMuQ - 1, mus0);
    } else {
        tbqs(t0, mub0, muq0 + muQ10, mus0);
    }
    if(rootfinder4D(sin, 0, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return true;
    }
    if(muq0 - muQ10 < minMuQ) {
        tbqs(t0, mub0, minMuQ + 1, mus0);
    } else {
        tbqs(t0, mub0, muq0 - muQ10, mus0);
    }
    if(rootfinder4D(sin, 0, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return true;
    }

    //perturb mus
    if(mus0 + muS10 > maxMuS) {
        tbqs(t0, mub0, muq0, maxMuS - 1);
    } else {
        tbqs(t0, mub0, muq0, mus0 + muS10);
    }
    if(rootfinder4D(sin, 0, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return true;
    }
    if(mus0 - muS10 < maxMuS) {
        tbqs(t0, mub0, muq0, minMuS + 1);
    } else {
        tbqs(t0, mub0, muq0, mus0 - muS10);
    }
    if(rootfinder4D(sin, 0, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return true;
    }

    //check mu = 0
    tbqs(t0, 0, 0, 0);
    if(rootfinder4D(sin, 0, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return true;
    }

    tbqs(t0, mub0, muq0, mus0);
    return false;
}

double eos::s_out(double ein) {   //update the t position (mu=0) based on input. Returns entropy if found, returns -1 if failed
    return s_out(ein, 0.0, 0.0, 0.0);
}

double eos::s_out(double ein, double Bin, double Sin, double Qin) {   //update the t and mu position based on input. Returns entropy if found, returns -1 if failed
    if (rootfinder4D(ein, 1, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return entrVal;
    }

    double t0 = tbqsPosition(0);
    double mub0 = tbqsPosition(1);
    double muq0 = tbqsPosition(2);
    double mus0 = tbqsPosition(3);
    double t10 = t0*.2;
    double muB10 = mub0*.2;
    double muQ10 = muq0*.2;
    double muS10 = mus0*.2;

    //perturb T
    if(t0 + t10 > maxT) {
        tbqs(maxT - 1, mub0, muq0, mus0);
    } else {
        tbqs(t0 + t10, mub0, muq0, mus0);
    }
    if(rootfinder4D(ein, 1, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return entrVal;
    }
    if(t0 - t10 < minT) {
        tbqs(minT + 1, mub0, muq0, mus0);
    } else {
        tbqs(t0 - t10, mub0, muq0, mus0);
    }
    if(rootfinder4D(ein, 1, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return entrVal;
    }

    //perturb mub
    if(mub0 + muB10 > maxMuB) {
        tbqs(t0, maxMuB - 1, muq0, mus0);
    } else {
        tbqs(t0, mub0 + muB10, muq0, mus0);
    }
    if(rootfinder4D(ein, 1, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return entrVal;
    }
    if(mub0 - muB10 < minMuB) {
        tbqs(t0, minMuB + 1, muq0, mus0);
    } else {
        tbqs(t0, mub0 - muB10, muq0, mus0);
    }
    if(rootfinder4D(ein, 1, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return entrVal;
    }

    //perturn muq
    if(muq0 + muQ10 > maxMuQ) {
        tbqs(t0, mub0, maxMuQ - 1, mus0);
    } else {
        tbqs(t0, mub0, muq0 + muQ10, mus0);
    }
    if(rootfinder4D(ein, 1, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return entrVal;
    }
    if(muq0 - muQ10 < minMuQ) {
        tbqs(t0, mub0, minMuQ + 1, mus0);
    } else {
        tbqs(t0, mub0, muq0 - muQ10, mus0);
    }
    if(rootfinder4D(ein, 1, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return entrVal;
    }

    //perturb mus
    if(mus0 + muS10 > maxMuS) {
        tbqs(t0, mub0, muq0, maxMuS - 1);
    } else {
        tbqs(t0, mub0, muq0, mus0 + muS10);
    }
    if(rootfinder4D(ein, 1, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return entrVal;
    }
    if(mus0 - muS10 < maxMuS) {
        tbqs(t0, mub0, muq0, minMuS + 1);
    } else {
        tbqs(t0, mub0, muq0, mus0 - muS10);
    }
    if(rootfinder4D(ein, 1, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return entrVal;
    }

    //check mu = 0
    tbqs(t0, 0, 0, 0);
    if(rootfinder4D(ein, 1, Bin, Sin, Qin, TOLERANCE, STEPS)) {
        return entrVal;
    }

    tbqs(t0, mub0, muq0, mus0);
    return -1;
}

double eos::s_terms_T(double Tt) { //return entropy at a given temperature for muB = muS = muQ = 0
    tbqs(Tt, 0, 0, 0);
    return entrVal;
}


// UNCOMMENTED BY C. PLUMBERG
void eos::eosin(std::string type) {}
double eos::A() {
    return 0;
}



double eos::efreeze() {
    return 0;
}
double eos::sfreeze() {
    return 0;
}




//struct to pass the target (E, rhoB, rhoQ, rhoS) into the rootfinder function
struct rootfinder_parameters {
    double eorEntGiven;          //these are the desired s and BSQ
    double rhoBGiven;
    double rhoQGiven;
    double rhoSGiven;
    BSpline eorEntSpline;        //the splines that contain interpolations over s, BSQ
    BSpline rhoBSpline;
    BSpline rhoQSpline;
    BSpline rhoSSpline;
    rootfinder_parameters();
    rootfinder_parameters(double seteorEntGiven, double setRhoBGiven, double setRhoQGiven, double setRhoSGiven, BSpline setEntrSpline, BSpline setRhoBSPLine, BSpline setRhoQSpline, BSpline setRhoSSpline);
public:
    void set(double setEorEntGiven, double setRhoBGiven, double setRhoQGiven, double setRhoSGiven, BSpline setEntrSpline, BSpline setRhoBSpline, BSpline setRhoQSpline, BSpline setRhoSSpline);
};
//Default constructor to make the compiler happy. Should never be called
rootfinder_parameters::rootfinder_parameters() : eorEntSpline(4), rhoBSpline(4), rhoQSpline(4), rhoSSpline(4) {}
//constructor which initializes all struct variables
rootfinder_parameters::rootfinder_parameters(double setEorEntGiven, double setRhoBGiven, double setRhoQGiven, double setRhoSGiven, BSpline setEorEntSpline, BSpline setRhoBSpline, BSpline setRhoQSpline, BSpline setRhoSSpline) : eorEntSpline(4), rhoBSpline(4), rhoQSpline(4), rhoSSpline(4)
{
    eorEntGiven = setEorEntGiven;
    rhoBGiven = setRhoBGiven;
    rhoQGiven = setRhoQGiven;
    rhoSGiven = setRhoSGiven;
    eorEntSpline = setEorEntSpline;
    rhoBSpline = setRhoBSpline;
    rhoQSpline = setRhoQSpline;
    rhoSSpline = setRhoSSpline;
}
void rootfinder_parameters::set(double setEorEntGiven, double setRhoBGiven, double setRhoQGiven, double setRhoSGiven, BSpline setEorEntSpline, BSpline setRhoBSpline, BSpline setRhoQSpline, BSpline setRhoSSpline) {
    eorEntGiven = setEorEntGiven;
    rhoBGiven = setRhoBGiven;
    rhoQGiven = setRhoQGiven;
    rhoSGiven = setRhoSGiven;
    eorEntSpline = setEorEntSpline;
    rhoBSpline = setRhoBSpline;
    rhoQSpline = setRhoQSpline;
    rhoSSpline = setRhoSSpline;
}

//helper function for the rootfinder. It provides the correct difference of s, rhoB, rhoQ, rhoS at a given (T, muB, muQ, muS) from the target
//used when rootfinder is given an entropy, baryon density, charge density, strangeness density
//x = (T, muB, muQ, muS), params = (sGiven, rhoBGiven, rhoQGiven, rhoSGiven), f becomes (s - sGiven, rhoB - rhoBGiven, rhoQ - rhoQGiven, rhoS - rhoSGiven)
int rootfinder_fsbqs(const gsl_vector *x, void *params, gsl_vector *f);
int rootfinder_fsbqs(const gsl_vector *x, void *params, gsl_vector *f) {
    //x contains the next (T, muB, muS) coordinate to test
    DenseVector tbqsToEval(4);
    tbqsToEval(0) = gsl_vector_get(x,0);
    tbqsToEval(1) = gsl_vector_get(x,1);      //convert x into densevector so it can be a BSpline evaluation point
    tbqsToEval(2) = gsl_vector_get(x,2);
    tbqsToEval(3) = gsl_vector_get(x,3);


    double entrGiven, rhoBGiven, rhoQGiven, rhoSGiven, entr, rhoB, rhoQ, rhoS;
    entrGiven = ((rootfinder_parameters*)params)->eorEntGiven;
    rhoBGiven = ((rootfinder_parameters*)params)->rhoBGiven;            //given variables contain the target point
    rhoQGiven = ((rootfinder_parameters*)params)->rhoQGiven;
    rhoSGiven = ((rootfinder_parameters*)params)->rhoSGiven;
    entr = (((rootfinder_parameters*)params)->eorEntSpline).eval(tbqsToEval);    //s, rhoB, rhoQ, rhoS contain the current point
    rhoB = (((rootfinder_parameters*)params)->rhoBSpline).eval(tbqsToEval);
    rhoQ = (((rootfinder_parameters*)params)->rhoQSpline).eval(tbqsToEval);
    rhoS = (((rootfinder_parameters*)params)->rhoSSpline).eval(tbqsToEval);

    gsl_vector_set(f, 0, (entr - entrGiven)); //f[0] contains (s(T,muB,muQ,muS) - sGiven)
    gsl_vector_set(f, 1, (rhoB - rhoBGiven)); //f[1] contains (rhoB(T,muB,muQ,muS) - rhoBGiven)
    gsl_vector_set(f, 2, (rhoQ - rhoQGiven)); //f[2] contains (rhoQ(T,muB,muQ,muS) - rhoQGiven)
    gsl_vector_set(f, 3, (rhoS - rhoSGiven)); //f[2] contains (rhoS(T,muB,muQ,muS) - rhoSGiven)

    return GSL_SUCCESS;
}

//helper function for the rootfinder. It provides the correct difference of E and rhoB at a given (T, muB, muQ, muS) from the target
//used when rootfinder is given an energy density and a baryon density
//x = (T, muB, muQ, muS), params = ((eGiven, rhoBGiven, rhoQGiven, rhoSGiven), f becomes (e - eGiven, rhoB - rhoBGiven, rhoQ - rhoQGiven, rhoS - rhoSGiven)
int rootfinder_febqs(const gsl_vector *x, void *params, gsl_vector *f);
int rootfinder_febqs(const gsl_vector *x, void *params, gsl_vector *f) {
    //x contains the next (T, muB, muQ, muS) coordinate to test
    DenseVector tbqsToEval(4);
    tbqsToEval(0) = gsl_vector_get(x,0);
    tbqsToEval(1) = gsl_vector_get(x,1);      //convert x into densevector so it can be a BSpline evaluation point
    tbqsToEval(2) = gsl_vector_get(x,2);
    tbqsToEval(3) = gsl_vector_get(x,3);

    double eGiven, rhoBGiven, rhoQGiven, rhoSGiven, e, rhoB, rhoQ, rhoS;
    eGiven = ((rootfinder_parameters*)params)->eorEntGiven;
    rhoBGiven = ((rootfinder_parameters*)params)->rhoBGiven;            //given variables contain the target point
    rhoQGiven = ((rootfinder_parameters*)params)->rhoQGiven;
    rhoSGiven = ((rootfinder_parameters*)params)->rhoSGiven;
    e = (((rootfinder_parameters*)params)->eorEntSpline).eval(tbqsToEval);    //e, rhoB, rhoQ, rhoS contain the current point
    rhoB = (((rootfinder_parameters*)params)->rhoBSpline).eval(tbqsToEval);
    rhoQ = (((rootfinder_parameters*)params)->rhoQSpline).eval(tbqsToEval);
    rhoS = (((rootfinder_parameters*)params)->rhoSSpline).eval(tbqsToEval);

    gsl_vector_set(f, 0, (e - eGiven)); //f[0] contains (e(T,muB,muQ,muS) - eGiven)
    gsl_vector_set(f, 1, (rhoB - rhoBGiven)); //f[1] contains the (rhoB(T,muB,muQ,muS) - rhoBGiven)
    gsl_vector_set(f, 2, (rhoQ - rhoQGiven)); //f[2] contains the (rhoQ(T,muB,muQ,muS) - rhoQGiven)
    gsl_vector_set(f, 3, (rhoS - rhoSGiven)); //f[2] contains the (rho2(T,muB,muQ,muS) - rhoSGiven)

    return GSL_SUCCESS;
}



bool eos::rootfinder4D(double e_or_s_Given, int e_or_s_mode, double rhoBGiven, double rhoSGiven, double rhoQGiven, double error, size_t steps) {

    //declare x = (T, muB, muS)
    gsl_vector *x = gsl_vector_alloc(4);
    gsl_vector_set(x, 0, T());
    gsl_vector_set(x, 1, muB());
    gsl_vector_set(x, 2, muQ());
    gsl_vector_set(x, 3, muS());

    //initialize the rootfinder equation to the correct variable quantities
    bool isEntropy = false;
    if(e_or_s_mode == 0) {
        isEntropy = true;
    }
    rootfinder_parameters p;
    if(isEntropy) {
        p.set(e_or_s_Given, rhoBGiven, rhoQGiven, rhoSGiven, entrSpline, bSpline, qSpline, sSpline);
    } else {
        p.set(e_or_s_Given, rhoBGiven, rhoQGiven, rhoSGiven, eSpline, bSpline, qSpline, sSpline);
    }

    //initialize multiroot solver
    gsl_multiroot_fsolver *solver;
    gsl_multiroot_function f;
    if(isEntropy) {
        f.f = &rootfinder_fsbqs;
    } else {
        f.f = &rootfinder_febqs;
    }
    f.n = 4;
    f.params = &p;

    solver = gsl_multiroot_fsolver_alloc(TYPE, 4);
    gsl_multiroot_fsolver_set(solver, &f, x);

    int status;
    size_t iter = 0;

    do {
        ++iter;
        status = gsl_multiroot_fsolver_iterate(solver);

        if(status) {
            return 0;      //break if the rootfinder gets stuck
        }
        if(gsl_vector_get(solver->x, 0) < minT) {
            return 0;
        } else if(gsl_vector_get(solver->x, 0) > maxT) {
            return 0;
        } else if (gsl_vector_get(solver->x, 1) < minMuB) {
            return 0;
        } else if (gsl_vector_get(solver->x, 1) > maxMuB) {
            return 0;
        } else if (gsl_vector_get(solver->x, 2) < minMuQ) {     //break if the rootfinder goes out of bounds
            return 0;
        } else if (gsl_vector_get(solver->x, 2) > maxMuQ) {
            return 0;
        } else if (gsl_vector_get(solver->x, 3) < minMuS) {
            return 0;
        } else if (gsl_vector_get(solver->x, 3) > maxMuS) {
            return 0;
        }

        status = gsl_multiroot_test_residual(solver->f, error);

    } while (status == GSL_CONTINUE && iter < steps);


    bool found = true; //to return variable
    if(iter >= steps) {
        found = false;
    }


    if(found) {
        tbqs(gsl_vector_get(solver->x, 0), gsl_vector_get(solver->x, 1), gsl_vector_get(solver->x, 2), gsl_vector_get(solver->x, 3));    //set T, muB, muQ, muS
    }

    //memory deallocation
    gsl_multiroot_fsolver_free(solver);
    gsl_vector_free(x);
    return found;
}







//struct to pass the target quantities into the rootfinder function
struct quant_rootfinder_parameters {
    double tGiven;
    double muBGiven;
    double muQGiven;
    double muSGiven;
    double quantGiven;          //the value we are looking for in the desired quantity
    int whichIndep;
    BSpline pSpline;        //the spine of the correct quantity
    BSpline entrSpline;
    BSpline eSpline;
    string quantity;
    quant_rootfinder_parameters();
    quant_rootfinder_parameters(string setQuantity, int setWhichIndep, double setQuantGiven, double setT, double setmuB, double setmuQ, double setmuS, BSpline setPSpline, BSpline setEntrSpline, BSpline setESpline);
public:
    void set(string setQuantity, int setWhichIndep, double setQuantGiven, double setT, double setmuB, double setmuQ, double setmuS, BSpline setPSpline, BSpline setEntrSpline, BSpline setESpline);
};
//Default constructor
quant_rootfinder_parameters::quant_rootfinder_parameters() : pSpline(4), entrSpline(4), eSpline(4) {}
//constructor which initializes all struct variables
quant_rootfinder_parameters::quant_rootfinder_parameters(string setQuantity, int setWhichIndep, double setQuantGiven, double setT, double setmuB, double setmuQ, double setmuS, BSpline setPSpline, BSpline setEntrSpline, BSpline setESpline) : pSpline(4), entrSpline(4), eSpline(4)
{
    whichIndep = setWhichIndep;
    quantity = setQuantity;
    quantGiven = setQuantGiven;
    pSpline = setPSpline;
    entrSpline = setEntrSpline;
    eSpline = setESpline;
    tGiven = setT;
    muBGiven = setmuB;
    muQGiven = setmuQ;
    muSGiven = setmuS;
}
void quant_rootfinder_parameters::set(string setQuantity, int setWhichIndep, double setQuantGiven, double setT, double setmuB, double setmuQ, double setmuS, BSpline setPSpline, BSpline setEntrSpline, BSpline setESpline) {
    whichIndep = setWhichIndep;
    quantity = setQuantity;
    quantGiven = setQuantGiven;
    pSpline = setPSpline;
    entrSpline = setEntrSpline;
    eSpline = setESpline;
    tGiven = setT;
    muBGiven = setmuB;
    muQGiven = setmuQ;
    muSGiven = setmuS;
}

//helper function for the rootfinder. It provides the correct difference of quantity from the target
int quant_rootfinder_f(const gsl_vector *x, void *params, gsl_vector *f);
int quant_rootfinder_f(const gsl_vector *x, void *params, gsl_vector *f) {
    int whichIndep = ((quant_rootfinder_parameters*)params)->whichIndep;
    //x contains the next (T, muB, muS) coordinate to test
    DenseVector tbqsToEval(4);
    if(whichIndep == 1) {
        tbqsToEval(0) = gsl_vector_get(x,0);
        tbqsToEval(1) = ((quant_rootfinder_parameters*)params)->muBGiven;      //convert x into densevector so it can be a BSpline evaluation point
        tbqsToEval(2) = ((quant_rootfinder_parameters*)params)->muQGiven;
        tbqsToEval(3) = ((quant_rootfinder_parameters*)params)->muSGiven;
    } else if(whichIndep == 2) {
        tbqsToEval(0) = ((quant_rootfinder_parameters*)params)->tGiven;
        tbqsToEval(1) = gsl_vector_get(x,0);      //convert x into densevector so it can be a BSpline evaluation point
        tbqsToEval(2) = ((quant_rootfinder_parameters*)params)->muQGiven;
        tbqsToEval(3) = ((quant_rootfinder_parameters*)params)->muSGiven;
    } else if(whichIndep == 3) {
        tbqsToEval(0) = ((quant_rootfinder_parameters*)params)->tGiven;
        tbqsToEval(1) = ((quant_rootfinder_parameters*)params)->muBGiven;      //convert x into densevector so it can be a BSpline evaluation point
        tbqsToEval(2) = gsl_vector_get(x,0);
        tbqsToEval(3) = ((quant_rootfinder_parameters*)params)->muSGiven;
    } else {
        tbqsToEval(0) = ((quant_rootfinder_parameters*)params)->tGiven;
        tbqsToEval(1) = ((quant_rootfinder_parameters*)params)->muBGiven;      //convert x into densevector so it can be a BSpline evaluation point
        tbqsToEval(2) = ((quant_rootfinder_parameters*)params)->muQGiven;
        tbqsToEval(3) = gsl_vector_get(x,0);
    }



    double quantGiven, quant;
    if(((quant_rootfinder_parameters*)params)->quantity == "e") {
        quantGiven = ((quant_rootfinder_parameters*)params)->quantGiven;
        quant = (((quant_rootfinder_parameters*)params)->eSpline).eval(tbqsToEval);
    } else if(((quant_rootfinder_parameters*)params)->quantity == "p") {
        quantGiven = ((quant_rootfinder_parameters*)params)->quantGiven;
        quant = (((quant_rootfinder_parameters*)params)->pSpline).eval(tbqsToEval);
    } else if(((quant_rootfinder_parameters*)params)->quantity == "entr") {
        quantGiven = ((quant_rootfinder_parameters*)params)->quantGiven;
        quant = (((quant_rootfinder_parameters*)params)->entrSpline).eval(tbqsToEval);
    } else if(((quant_rootfinder_parameters*)params)->quantity == "gibbs") {
        quantGiven = ((quant_rootfinder_parameters*)params)->quantGiven;
        quant = (((quant_rootfinder_parameters*)params)->eSpline).eval(tbqsToEval);
        quant += (((quant_rootfinder_parameters*)params)->pSpline).eval(tbqsToEval);
        quant -= ((((quant_rootfinder_parameters*)params)->entrSpline).eval(tbqsToEval))*tbqsToEval(0);
    }

    gsl_vector_set(f, 0, (quant - quantGiven));

    return GSL_SUCCESS;
}

bool eos::quant_rootfinder4D(double quantGiven, string quantType, int whichIndep, double error, size_t steps) {

    //declare x = T
    gsl_vector *x = gsl_vector_alloc(1);
    quant_rootfinder_parameters p;
    if(whichIndep == 1) {
        gsl_vector_set(x, 0, T());
    } else if(whichIndep == 2) {
        gsl_vector_set(x, 0, muB());
    } else if(whichIndep == 3) {
        gsl_vector_set(x, 0, muQ());
    } else if(whichIndep == 4) {
        gsl_vector_set(x, 0, muS());
    } else {
        std::cout << "Please select a gridded quantity to vary during search" << std::endl;
        std::cout << "t = 1,  muB = 2,  muQ = 3,  muS = 4" << std::endl;
        return 0;
    }
    p.set(quantType, whichIndep, quantGiven, T(), muB(), muQ(), muS(), pSpline, entrSpline, eSpline);

    //initialize multiroot solver
    const gsl_multiroot_fsolver_type *type;
    gsl_multiroot_fsolver *solver;
    gsl_multiroot_function f;
    f.f = &quant_rootfinder_f;
    f.n = 1;
    f.params = &p;

    //type options: dnewton, hybrids, hybrid, broyden
    //the dnewton and broyden methods crash the program with a bad guess. Hybrid and hybrids do not
    //dnewton works the fastest on guesses that it can successfully reach. Hybrids is faster than hybrid
    //I am choosing hybrids because it is the most accurate. Hybrids is slower than dnewton, but more reliable
    type = gsl_multiroot_fsolver_hybrids;
    solver = gsl_multiroot_fsolver_alloc(type, 1);
    gsl_multiroot_fsolver_set(solver, &f, x);

    int status;
    size_t iter = 0;

    do {
        ++iter;
        status = gsl_multiroot_fsolver_iterate(solver);

        if(status) {
            return 0;      //break if the rootfinder gets stuck
        }
        if(whichIndep == 1) {
            if(gsl_vector_get(solver->x, 0) < minT) {
                return 0;
            } else if(gsl_vector_get(solver->x, 0) > maxT) {
                return 0;
            }
        } else if(whichIndep == 2) {
            if(gsl_vector_get(solver->x, 0) < minMuB) {
                return 0;
            } else if(gsl_vector_get(solver->x, 0) > maxMuB) {
                return 0;
            }
        } else if(whichIndep == 3) {
            if(gsl_vector_get(solver->x, 0) < minMuQ) {
                return 0;
            } else if(gsl_vector_get(solver->x, 0) > maxMuQ) {
                return 0;
            }
        } else if(whichIndep == 4) {
            if(gsl_vector_get(solver->x, 0) < minMuS) {
                return 0;
            } else if(gsl_vector_get(solver->x, 0) > maxMuS) {
                return 0;
            }
        }


        status = gsl_multiroot_test_residual(solver->f, error);

    } while (status == GSL_CONTINUE && iter < steps);


    bool found = true; //to return variable
    if(iter >= steps) {
        found = false;
    }


    if(found) {
        tbqs(gsl_vector_get(solver->x, 0), muB(), muQ(), muS());    //set T, muB, muQ, muS
    }

    //memory deallocation
    gsl_multiroot_fsolver_free(solver);
    gsl_vector_free(x);
    return found;
}
