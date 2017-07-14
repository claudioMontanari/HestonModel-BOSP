# Heston Model on BarbequeRTRM
_**Authors:** [Claudio Montanari](https://github.com/claudioMontanari)  **&** [Luca Napoletano](https://github.com/lnapo94)_

### Welcome to our repository of the Project on the BarbequeRTRM framework!
This is our README file. We create this document to help you to understand the usage and the general idea of our project.

### What is our application about?
Nowadays options are one of the most traded financial derivatives. Different kinds of this contract types exist, such as European, American and Exotic. 
The process to price them is not trivial, thus, a portfolio strategy requires an accurate and quick pricing tool in order to be successful. To describe options value oscillations as a function of time, in the most appropriate mathematical way, we need to use stochastic volatility models. Among them the Heston model is the best one in describing the market behavior. 
Due to the complexity of the Heston model, only European options may be analytically evaluated, hence, Monte Carlo simulations are used to overcome this obstacle.
Since every simulation is independent from each other, this algorithm can be executed in parallels

### Why we have developed this algorithm under the BarbequeRTRM?
This is a project done during our bachelor degree, and we have chosen this algorithm like an example of _approximate calculus model_ 
to create a case study for the BarbequeRTRM. We have developed a reconfigurable application that changes its precision according to the 
resources parameters given by the framework 
