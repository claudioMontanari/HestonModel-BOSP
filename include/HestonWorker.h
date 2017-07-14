
#ifndef HESTONWORKER_H_
#define HESTONWORKER_H_

#include <bbque/bbque_exc.h>

#include <iostream>
#include <random>
#include <time.h>
#include <math.h>
#include <thread>
#define DEFAULT_SIMULATIONS 10000

using bbque::rtlib::BbqueEXC;

class HestonWorker {

public:

	HestonWorker(double S0, double K, double r, double T, double V0, double rho, double kappa, double theta, double xi);
	void start(int simulationToDo, int discretization);
	void start(int discretization);
	int stop();
	void join();
	void hestonSimulation();
	double getCalculus();
	int getSimulationsDone();
	int getDefSimulations();

private:
	
	int SIMULATIONSTODO;
	int SIMULATIONSDONE;
	int DISCRETIZATION;
	bool HASTOWORK;

	double finalPrice;
	/**
	 * Variable used to accumulate the results from each run
	 */
	double totalSum;
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
	
	/**
	 * Random Generator 
	 */
	
	std::mt19937 generator;	
	std::thread worker;	

	double rationalApproximation(double t);
	double normalCDFInverse(double p);
	double maxValue(double, double);
	double europeanCall(double, double);

};

#endif // HESTONWORKER_H_
