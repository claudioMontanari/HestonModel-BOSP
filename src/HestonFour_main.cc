/**
 *       @file  HestonFour_main.cc
 *      @brief  The HestonFour BarbequeRTRM application
 *
 * Description: The main program to start the HestonFour application
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

#include <cstdio>
#include <iostream>
#include <random>
#include <cstring>
#include <memory>

#include <libgen.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "version.h"
#include "HestonFour_exc.h"
#include <bbque/utils/utility.h>
#include <bbque/utils/logging/logger.h>

// Setup logging
#undef  BBQUE_LOG_MODULE
#define BBQUE_LOG_MODULE "HestonFour"

namespace bu = bbque::utils;
namespace po = boost::program_options;

/**
 * @brief A pointer to an EXC
 */
std::unique_ptr<bu::Logger> logger;

/**
 * @brief A pointer to an EXC
 */
typedef std::shared_ptr<BbqueEXC> pBbqueEXC_t;

/**
 * The decription of each HestonFour parameters
 */
po::options_description opts_desc("HestonFour Configuration Options");

/**
 * The map of all HestonFour parameters values
 */
po::variables_map opts_vm;

/**
 * The services exported by the RTLib
 */
RTLIB_Services_t *rtlib;

/**
 * @brief The application configuration file
 */
std::string conf_file = BBQUE_PATH_PREFIX "/" BBQUE_PATH_CONF "/HestonFour.conf" ;

/**
 * @brief The recipe to use for all the EXCs
 */
std::string recipe;

/**
 * @brief The EXecution Context (EXC) registered
 */
pBbqueEXC_t pexc;

/**
 * @brief Variables used in our computation, only to take parameters from user
 */
double S0;
double K;
double r;
double T;

double V0;
double rho;
double kappa;
double theta;
double xi;

int N_SIM;
int DISCR;

void ParseCommandLine(int argc, char *argv[]) {
	// Parse command line params
	try {
	po::store(po::parse_command_line(argc, argv, opts_desc), opts_vm);
	} catch(...) {
		std::cout << "Usage: " << argv[0] << " [options]\n";
		std::cout << opts_desc << std::endl;
		::exit(EXIT_FAILURE);
	}
	po::notify(opts_vm);

	// Check for help request
	if (opts_vm.count("help")) {
		std::cout << "Usage: " << argv[0] << " [options]\n";
		std::cout << opts_desc << std::endl;
		::exit(EXIT_SUCCESS);
	}

	// Check for version request
	if (opts_vm.count("version")) {
		std::cout << "HestonFour (ver. " << g_git_version << ")\n";
		std::cout << "Copyright (C) 2011 Politecnico di Milano\n";
		std::cout << "\n";
		std::cout << "Built on " <<
			__DATE__ << " " <<
			__TIME__ << "\n";
		std::cout << "\n";
		std::cout << "This is free software; see the source for "
			"copying conditions.  There is NO\n";
		std::cout << "warranty; not even for MERCHANTABILITY or "
			"FITNESS FOR A PARTICULAR PURPOSE.";
		std::cout << "\n" << std::endl;
		::exit(EXIT_SUCCESS);
	}
}

int main(int argc, char *argv[]) {

		opts_desc.add_options()
		("help,h", "print this help message")
		("version,v", "print program version")

		("conf,C", po::value<std::string>(&conf_file)->
			default_value(conf_file),
			"HestonThree configuration file")

		("recipe,r", po::value<std::string>(&recipe)->
			default_value("HestonThree"),
			"recipe name (for all EXCs)")

		("sims,n", po::value<int>(&N_SIM)->
			default_value(60000),
			"Number of simulations")
		("discr,d", po::value<int>(&DISCR)->
			default_value(300),
			"Discretization value")

		("spot,s", po::value<double>(&S0)->
			default_value(100.0),
			"Option Spot Price")
		("strike,K", po::value<double>(&K)->
			default_value(100.0),
			"Option Strike Price")
		("risk,R", po::value<double>(&r)->
			default_value(0.05),
			"Risk-Free Rate")
		("time,T", po::value<double>(&T)->
			default_value(5.0),
			"Maturity Time [In Years]")

		("vol,v", po::value<double>(&V0)->
			default_value(0.09),
			"Volatility")
		("rho,r", po::value<double>(&rho)->
			default_value(-0.30),
			"Correlation Coefficient")
		("kappa,k", po::value<double>(&kappa)->
			default_value(2.0),
			"Mean Reversion")
		("theta,th", po::value<double>(&theta)->
			default_value(0.09),
			"Long-Term volatility")
		("xi,x", po::value<double>(&xi)->
			default_value(1.0),
			"Volatility of volatility")
	;
	;

	// Setup a logger
	bu::Logger::SetConfigurationFile(conf_file);
	logger = bu::Logger::GetLogger("hestonfour");

	ParseCommandLine(argc, argv);

	// Welcome screen
	logger->Info(".:: HestonFour (ver. %s) ::.", g_git_version);
	logger->Info("Built: " __DATE__  " " __TIME__);

	// Initializing the RTLib library and setup the communication channel
	// with the Barbeque RTRM
	logger->Info("STEP 0. Initializing RTLib, application [%s]...",
			::basename(argv[0]));

	if ( RTLIB_Init(::basename(argv[0]), &rtlib) != RTLIB_OK) {
		logger->Fatal("Unable to init RTLib (Did you start the BarbequeRTRM daemon?)");
		return RTLIB_ERROR;
	}

	assert(rtlib);

	logger->Info("STEP 1. Registering EXC using [%s] recipe...",
			recipe.c_str());
	pexc = pBbqueEXC_t(new HestonFour("HestonFour", recipe, rtlib, S0, K, r, T, V0, rho, kappa, theta, xi, N_SIM, DISCR));
	if (!pexc->isRegistered()) {
		logger->Fatal("Registering failure.");
		return RTLIB_ERROR;
	}


	logger->Info("STEP 2. Starting EXC control thread...");
	pexc->Start();


	logger->Info("STEP 3. Waiting for EXC completion...");
	pexc->WaitCompletion();


	logger->Info("STEP 4. Disabling EXC...");
	pexc = NULL;

	logger->Info("===== HestonFour DONE! =====");
	return EXIT_SUCCESS;

}

