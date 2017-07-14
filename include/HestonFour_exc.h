/**
 *       @file  HestonFour_exc.h
 *      @brief  The HestonFour BarbequeRTRM application
 *
 * Description: to be done...
 *
 *     @author  Name Surname (nickname), your@email.com
 *
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 20XX, Name Surname
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#ifndef HESTONFOUR_EXC_H_
#define HESTONFOUR_EXC_H_

#include <bbque/bbque_exc.h>

#include "HestonWorker.h"

#include <iostream>
#include <random>
#include <time.h>
#include <math.h>

using bbque::rtlib::BbqueEXC;

class HestonFour : public BbqueEXC {

public:

	HestonFour(std::string const & name,
			std::string const & recipe,
			RTLIB_Services_t *rtlib, double, double, double, double, double, double, double, double, double, int, int);


private:
	
	HestonWorker* workers;
	//std::vector<HestonWorker*> workers;
	int WORKERS;
	int DONE_SIMULATIONS;
	int TODO_SIMULATIONS;
	int DISCRETIZATION;
	int NUM_PROC;
	const int WORKERS_SIM = 10000;

	double finalPrice;
	/**
	 * Variable used to accumulate the results from each run
	 */
	double workersFinalSum;
	double threadFinalPrice;
	/**
	 *  Variables used to setup the heston simulation
	 */

	double V0;
	double rho;
	double kappa;
	double theta;
	double xi;

	/**
	 * Variables used to setup the option
	 */
	
	double S0;
	double K;
	double r;
	double T;
	

	RTLIB_ExitCode_t onSetup();
	RTLIB_ExitCode_t onConfigure(int8_t awm_id);
	RTLIB_ExitCode_t onRun();
	RTLIB_ExitCode_t onMonitor();
	RTLIB_ExitCode_t onRelease();

};

#endif // HESTONFOUR_EXC_H_
