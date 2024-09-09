# shared_vehicle_allocation_relocation_iise
The data and codes in this project are used in the paper "Integrated Vehicle Allocation and Relocation for Shared Micromobility under Competition and Demand Uncertainty" by Ziliang Jin, Kai Pan, Zuo-Jun Max Shen, and Wenxin Xu. *IISE Transactions*.

## data
The folder "data" includes the data used in the paper:
* demand.csv: average trip demands
* period.csv: trip duration across service regions
* folder "proportion": includes $$\phi_{i,t,j}$$, i.e., percentage of all the trips originating from region $$i$$ in period $$t$$ that eventually go to region $$j$$
* folder "demand_cluster": includes three demand scenarios in $$S$$
* folder "demand_1000": includes all demand scenarios used in Appendix J

## code
* The folder "src_demand_partition_allocation" includes codes for Problems (13) and (14) that determine the share of disloyal consumers based on allocation:
  * basic_parameter.h and basic_parameter.cpp: define input parameters of models
  * IO.h and IO.cpp: define functions for input and output
  * main.cpp: main file
  * problem.h and problem.cpp: define functions for constructing models
  * variables.h and variables.cpp: define decision variables of models

* The folder "src_demand_partition_availability" includes codes for problems in Appendix H that determine the share of disloyal consumers based on available vehicles:
  * basic_parameter.h and basic_parameter.cpp
  * IO.h and IO.cpp
  * main.cpp: main file where Algorithm 1 is applied
  * problem.h and problem.cpp
  * variables.h and variables.cpp

* The folder "src_firms_seperate" includes codes for problems in Appendix G that consider loyal and disloyal consumers separately:
  * basic_parameter.h and basic_parameter.cpp
  * IO.h and IO.cpp
  * main.cpp
  * problem.h and problem.cpp
  * variables.h and variables.cpp
 
## requirements
* A CPLEX solver is necessary for solving the models in the paper.
