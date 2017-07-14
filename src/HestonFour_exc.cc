/**
 *       @file  HestonFour_exc.cc
 *      @brief  The HestonFour BarbequeRTRM application
 *
 * Description: to be done...
 *
 *     @author  Name Surname (nickname), your@email.com
 *
 *     Company  Your Company
 *   Copyright  Copyright (c) 20XX, Name Surname
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */


#include "HestonFour_exc.h"

#include <cstdio>
#include <bbque/utils/utility.h>

HestonFour::HestonFour(std::string const & name,
		std::string const & recipe,
		RTLIB_Services_t *rtlib, double S0, double K, double r, double T, double V0, double rho, double kappa, double theta, double xi,
		int N_SIM, int DISCR) :
	BbqueEXC(name, recipe, rtlib) {

	logger->Warn("New HestonFour::HestonFour()");

	// NOTE: since RTLib 1.1 the derived class construct should be used
	// mainly to specify instantiation parameters. All resource
	// acquisition, especially threads creation, should be palced into the
	// new onSetup() method.

	logger->Info("EXC Unique IDentifier (UID): %u", GetUniqueID());

	this->S0 = S0;
	this->K = K;
	this->r = r;
	this->T = T;
	this->V0 = V0;
	this->rho = rho;
	this->kappa = kappa;
	this->theta = theta;
	this->xi = xi;

	if(N_SIM < WORKERS_SIM) {
		std::cout << "Lower than allowed number. Minimun is: " << WORKERS_SIM << std::endl;
		N_SIM = WORKERS_SIM;
	}

	this->TODO_SIMULATIONS = N_SIM;
	this->DISCRETIZATION = DISCR;
	this->DONE_SIMULATIONS = 0;
	
	std::cout << std::endl;

	std::cout << "S0: " << this->S0 << std::endl;
	std::cout << "K: " << this->K << std::endl;
	std::cout << "r: " << this->r << std::endl;
	std::cout << "T: " << this->T << std::endl;

	std::cout << "V0: " << this->V0 << std::endl;
	std::cout << "rho: " << this->rho << std::endl;
	std::cout << "kappa: " << this->kappa << std::endl;
	std::cout << "theta: " << this->theta << std::endl;
	std::cout << "xi: " << this->xi << std::endl;

	std::cout << "SIMULATIONS TO-DO: " << this->TODO_SIMULATIONS << std::endl;
	std::cout << "DISCRETIZATION: " << this->DISCRETIZATION << std::endl;

	std::cout << std::endl;

}

RTLIB_ExitCode_t HestonFour::onSetup() {
	// This is just an empty method in the current implementation of this
	// testing application. However, this is intended to collect all the
	// application specific initialization code, especially the code which
	// acquire system resources (e.g. thread creation)
	logger->Warn("HestonFour::onSetup()");
	
	workersFinalSum = 0.0;

	/**
	 * @brief Number of max processor in the computer
	 */
	NUM_PROC = (int) std::thread::hardware_concurrency();
	std::cout << "Number of detected processors: " << NUM_PROC << std::endl;


	/**
	 * @brief Create the workers with the NUM_PROC variables
	 */	
		
	for(int i=0;i<NUM_PROC; i++){
		logger->Warn("Creating new worker"); 
		workers[i] = new HestonWorker( S0, K, r, T, V0, rho, kappa, theta, xi);
	}
	
	return RTLIB_OK;
}

RTLIB_ExitCode_t HestonFour::onConfigure(int8_t awm_id) {

	logger->Warn("HestonFour::onConfigure(): EXC [%s] => AWM [%02d]",
		exc_name.c_str(), awm_id);

	int32_t proc_quota, proc_nr, mem;
	GetAssignedResources(PROC_ELEMENT, proc_quota);
	GetAssignedResources(PROC_NR, proc_nr);
	GetAssignedResources(MEMORY, mem);
	logger->Notice("TestWorkload::onConfigure(): "
		"EXC [%s], AWM[%02d] => R<PROC_quota>=%3d, R<PROC_nr>=%2d, R<MEM>=%3d",
		exc_name.c_str(), awm_id, proc_quota, proc_nr, mem);

	WORKERS = proc_nr;

	return RTLIB_OK;
}

RTLIB_ExitCode_t HestonFour::onRun() {
	RTLIB_WorkingModeParams_t const wmp = WorkingModeParams();

	// Return when all the simulations are done
	if (DONE_SIMULATIONS >= TODO_SIMULATIONS){
		
		return RTLIB_EXC_WORKLOAD_NONE;
	}

	if(DONE_SIMULATIONS + WORKERS * WORKERS_SIM > TODO_SIMULATIONS) {
		WORKERS = (TODO_SIMULATIONS - DONE_SIMULATIONS) / WORKERS_SIM;
		if(WORKERS == 0)
			WORKERS = 1;
	}

	for(int i = 0; i < WORKERS; i++){
		workers[i]->start(WORKERS_SIM, DISCRETIZATION);
	}
	
	for(int i = 0; i < WORKERS; i++){
		workers[i]->join();
		DONE_SIMULATIONS += WORKERS_SIM;
		double temp =  ( ( workers[i]->getCalculus() / (double) ( WORKERS_SIM * 2)) * exp( -(r) * (T) ) );
		logger->Warn("Worker %d computed price: %f ", i, temp );

		workersFinalSum += workers[i]->getCalculus();
	}

	// Do one more cycle
	logger->Warn("HestonMultiThread::onRun()      : EXC [%s]  @ AWM [%02d]",
		exc_name.c_str(), wmp.awm_id);

	return RTLIB_OK;
}

RTLIB_ExitCode_t HestonFour::onMonitor() {
	RTLIB_WorkingModeParams_t const wmp = WorkingModeParams();

	logger->Warn("HestonFour::onMonitor()  : EXC [%s]  @ AWM [%02d], Cycle [%4d]",
		exc_name.c_str(), wmp.awm_id, Cycles());

	threadFinalPrice = ( ( workersFinalSum / (double) ((DONE_SIMULATIONS * 2))) * exp( -(r) * (T) ) );
	logger->Warn("ON_MONITOR: Price updated: %f", threadFinalPrice);

	return RTLIB_OK;
}

RTLIB_ExitCode_t HestonFour::onRelease() {

	logger->Warn("HestonFour::onRelease()  : exit");

	for(int i=0; i<NUM_PROC; i++){
		delete workers[i];
	}

	return RTLIB_OK;
}
