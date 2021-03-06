/**
 *       @file  HestonWorker.cc
 *
 * Description: The most important part of our application. This class calculates the Heston simulations option price.
 *		Every worker has thread to calculate the option price, in this way the application can run in a parallel
 *		way
 *     @author  Luca Napoletano luca.napoletano@mail.polimi.it, Claudio Montanari claudio1.montanari@mail.polimi.it
 *
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2017, Luca Napoletano, Claudio Montanari
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */
#include "HestonWorker.h"

#include <cstdio>
#include <bbque/utils/utility.h>

#include <cmath>


/**
 * @brief		The constructor of the HestonWorker class
 *
 * @param[in] S0	The spot price of the option
 * @param[in] K		The strike price of the option
 * @param[in] r		The risk-free rate of the option
 * @param[in] T		The maturity time of the option (in years)
 * @param[in] V0	The initial volatility of the option
 * @param[in] rho	The Correlation Coefficient parameter of Heston model for the specified option
 * @param[in] kappa	The mean reversion rate of the Heston Model for the considered option
 * @param[in] theta	The long-term volatility value
 * @param[in] xi	The volatility of volatility (V0)
 */
HestonWorker::HestonWorker(double S0, double K, double r, double T, double V0, double rho, double kappa, double theta, double xi){

	this->S0 = S0;
	this->K = K;
	this->r = r;
	this->T = T;
	this->V0 = V0;
	this->rho = rho;
	this->kappa = kappa;
	this->theta = theta;
	this->xi = xi;

	//SetUp the Random Number Generator and the Normal extractor 
	std::random_device device;
	
	generator.seed(device());

}

/**
 * @brief			Method used to start a simulation
 * @param[in] simulationToDo	The number of the simulations that a single worker has to do
 * @param[in] discretization	The value of discretization of the simulation
 */
void HestonWorker::start(int simulationToDo, int discretization){
	
	//Set the number of simulations and the discretization level
	this->SIMULATIONSTODO = simulationToDo;
	this->DISCRETIZATION = discretization;
	this->SIMULATIONSDONE = 0;
	this->totalSum = 0;
	//Start the Worker	
	worker = std::thread(&HestonWorker::hestonSimulation, this);

}

/**
 * @brief			Method used to start a simulation with a fixed number of simulations
 * @param[in] discretization	The value of discretization of the simulation
 */
void HestonWorker::start(int discretization){
	
	//Set the number of simulations and the discretization level
	this->SIMULATIONSTODO = DEFAULT_SIMULATIONS;
	this->DISCRETIZATION = discretization;
	this->SIMULATIONSDONE = 0;
	this->totalSum = 0;
	//Start the Worker	
	worker = std::thread(&HestonWorker::hestonSimulation, this);

}

/**
 * @brief			Method used to stop a worker
 */
int HestonWorker::stop(){

	//Wait the Worker ends and return the number of simulations done
	this->HASTOWORK = false;
	//Wait the Thread ends
		

	return SIMULATIONSDONE;

}

/**
 * @brief			Method used to wait a computation of a worker
 */
void HestonWorker::join(){

	worker.join();
}

/**
 * @brief			Method used to do an Heston Simulation. It is used for the thread function
 */
void HestonWorker::hestonSimulation(){

	double deltaT = (T / ((double) DISCRETIZATION));

    	double random_spot;
    	double random_volatility;
    	double correlated_random_spot;
	
	double antithetic_random_spot;
	double antithetic_random_volatility;
    	double antithetic_correlated_random_spot;
	
    	double correct_volatility;
    	double volatility;
    	double spot_price;

    	double antithetic_correct_volatility;
    	double antithetic_volatility;
    	double antithetic_spot_price;

    	double sum = 0;

	for (int i = 0; i < SIMULATIONSTODO; i++) {
	
        	volatility = V0;
        	spot_price = S0;
	
		antithetic_volatility = volatility;
		antithetic_spot_price = spot_price;

		for (int j = 0; j < DISCRETIZATION; j++) {

			random_spot = normalCDFInverse((((double)generator())+ 0.5)*(1.0/4294967296.0));             				/**<Random Number with uniform distribution*/
			random_volatility = normalCDFInverse((((double)generator()) + 0.5)*(1.0/4294967296.0));  					/**<Random Number with uniform distribution*/

			antithetic_random_spot = -random_spot;					/**<Antithetic Random Number with uniform distribution*/
			antithetic_random_volatility = -random_volatility;			/**<Antithetic Random Number with uniform distribution*/ 		
			correlated_random_spot = (rho * random_volatility) + (random_spot * sqrt(1 - rho * rho));       
				/**<Correlation between the two Normal Distribution*/
			antithetic_correlated_random_spot = (rho * antithetic_random_volatility) + (antithetic_random_spot * sqrt(1 - rho * rho));
				/**<Correlation between the two Antithetic Normal Distribution*/
		 	
			correct_volatility = maxValue(volatility, 0.0);     	/**<Value for sqrt use, then it must be positive*/
			antithetic_correct_volatility = maxValue(antithetic_volatility , 0.0);	

			volatility = volatility +  kappa * deltaT * (theta - correct_volatility) + xi * sqrt(correct_volatility * deltaT) * random_volatility;
			    /**<Calculating volatility value in time using Euler discretization*/

			spot_price = spot_price * exp( (r - 0.5 * correct_volatility) * deltaT + sqrt(correct_volatility * deltaT) * correlated_random_spot);
			    /**<Calculating spot price value in time using Euler discretization*/


			antithetic_volatility = antithetic_volatility +  kappa * deltaT * (theta - antithetic_correct_volatility) + xi * sqrt(antithetic_correct_volatility * deltaT) * antithetic_random_volatility;
			    /**<Calculating antithetic volatility value in time using Euler discretization*/

			antithetic_spot_price = antithetic_spot_price * exp( (r - 0.5 * antithetic_correct_volatility) * deltaT + sqrt(antithetic_correct_volatility * deltaT) * antithetic_correlated_random_spot);
			    /**<Calculating antithetic spot price value in time using Euler discretization*/

			}
	
	    SIMULATIONSDONE++;		
	    sum = sum + europeanCall(spot_price, K) + europeanCall(antithetic_spot_price, K);   
								/** This line aims to calculate the simulated option value using a Option function, 
		                                                *   in this way we can personalize the option payoff.
		                                                */
	    }

	    totalSum += sum;

}


/**
 * @brief		Method used to calculate an approximation of the passed value
 * @param[in] t		The number to approximate	
 */
double HestonWorker::rationalApproximation(double t){

   // Abramowitz and Stegun formula 26.2.23.
    // The absolute value of the error should be less than 4.5 e-4.
    double c[] = {2.515517, 0.802853, 0.010328};
    double d[] = {1.432788, 0.189269, 0.001308};
    return t - ((c[2]*t + c[1])*t + c[0]) /
                (((d[2]*t + d[1])*t + d[0])*t + 1.0);
}

/**
 * @brief		Method used to calculate the inverse of the standard normal function
 * @param[in] p		A value between 0 and 1 (exclused) that represents a value of a standard normal distribution
 */
double HestonWorker::normalCDFInverse(double p){

    if (p <= 0.0 || p >= 1.0)
    {
/*
        stringstream os;
        os << "Invalid input argument (" << p
        << "); must be larger than 0 but less than 1.";
        throw invalid_argument( os.str() );
*/
	return -1;
    }
 
    if (p < 0.5)
    {
        // F^-1(p) = - G^-1(p)
        return -rationalApproximation( sqrt(-2.0*log(p)) );
    }
    else
    {
        // F^-1(p) = G^-1(1-p)
        return rationalApproximation( sqrt(-2.0*log(1-p)) );
    }
}

/**
 * @brief	Method used to calculate the max value between to given numbers
 */
double HestonWorker::maxValue(double x, double y){
	if(x > y)
		return x;
	return y;
}
	
double HestonWorker::europeanCall(double S, double K){

	return maxValue(0.0, S - K);
}

double HestonWorker::getCalculus(){
	return totalSum;
}

int HestonWorker::getSimulationsDone(){
	return SIMULATIONSDONE;
}

int HestonWorker::getDefSimulations(){
	return DEFAULT_SIMULATIONS;
}



