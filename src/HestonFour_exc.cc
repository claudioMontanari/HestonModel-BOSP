/**
 *       @file  HestonFour_exc.cc
 *      @brief  The HestonFour BarbequeRTRM application
 *
 * Description: This file represents all the mainly method implementation of our application.
 * 		In this file you can file all the initialize operations for our code. But, the most importants operations
 *		are implemented in HestonWorker.cc (you can find it in contrib/user/HestonFour/src/).
 *
 *     @author  Luca Napoletano luca.napoletano@mail.polimi.it, Claudio Montanari claudio1.montanari@mail.polimi.it
 *
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2017, Luca Napoletano, Claudio Montanari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */


#include "HestonFour_exc.h"

#include <cstdio>
#include <bbque/utils/utility.h>

/**
 * @brief		Implementation of the constructor of the HestonFour class. It needs all the Option parameters and the user's 
 *			preferred values for the number of the simulations and the correct discretization
 * @param[in] name	The name of the application
 * @param[in] recipe	A reference to the recipe of the application
 * @param[in] S0	The spot price of the option
 * @param[in] K		The strike price of the option
 * @param[in] r		The risk-free rate of the option
 * @param[in] T		The maturity time of the option (in years)
 * @param[in] V0	The initial volatility of the option
 * @param[in] rho	The Correlation Coefficient parameter of Heston model for the specified option
 * @param[in] kappa	The mean reversion rate of the Heston Model for the considered option
 * @param[in] theta	The long-term volatility value
 * @param[in] xi	The volatility of volatility (V0)
 * @param[in] N_SIM	The number of wanted simulations
 * @param[in] DISCR	The discretization value
 */
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
	
	this->pricesToCompute = (int) (TODO_SIMULATIONS / WORKERS_SIM);
	this->computedPrices = new double[pricesToCompute];
	this->computedPricesIndex = 0;	

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

/**
 * @brief	Method used to do all the Setup operations
 */
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
	workers = new HestonWorker*[NUM_PROC]; 	

	for(int i=0;i<NUM_PROC; i++){
		logger->Warn("Creating new worker"); 
		workers[i] = new HestonWorker( S0, K, r, T, V0, rho, kappa, theta, xi);
	}
	
	return RTLIB_OK;
}

/**
 * @brief	Method used to do all the Configure operations. Every time the BarbequeRTRM changes the resources assigned to this application,
 *		the Resource Manager calls this method to reconfigure the application
 */
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

/**
 * @brief	Method used to start the computation of an Option price after our app is configured correctly in onCofigure() method
 */
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
		computedPrices[computedPricesIndex] = temp;
		computedPricesIndex++;
	}

	// Do one more cycle
	logger->Warn("HestonMultiThread::onRun()      : EXC [%s]  @ AWM [%02d]",
		exc_name.c_str(), wmp.awm_id);

	return RTLIB_OK;
}

/**
 * @brief	After every onRun(), this method will be executed and print the updated result
 */
RTLIB_ExitCode_t HestonFour::onMonitor() {
	RTLIB_WorkingModeParams_t const wmp = WorkingModeParams();

	logger->Warn("HestonFour::onMonitor()  : EXC [%s]  @ AWM [%02d], Cycle [%4d]",
		exc_name.c_str(), wmp.awm_id, Cycles());

	threadFinalPrice = ( ( workersFinalSum / (double) ((DONE_SIMULATIONS * 2))) * exp( -(r) * (T) ) );
	logger->Warn("ON_MONITOR: Price updated: %f", threadFinalPrice);

	return RTLIB_OK;
}

/**
 * @brief	Called when the application is closing itself. This method does all the closing operations, such as the deallocation
 *		of the dynamic memory
 */
RTLIB_ExitCode_t HestonFour::onRelease() {

	logger->Warn("HestonFour::onRelease()  : exit");
	
	for(int i=0; i < this->pricesToCompute; i++){
		
		logger->Warn("Price %d = %f", i, computedPrices[i]);		
	} 

	for(int i=0; i<NUM_PROC; i++){
		delete workers[i];
	}
	delete[] workers;

	return RTLIB_OK;
}
