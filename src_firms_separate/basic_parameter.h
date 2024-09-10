#ifndef _BASICPARAMETER_H_
#define _BASICPARAMETER_H_

#include <vector>
#include <string>
#include <fstream>
#include <iostream> 
#include <stdlib.h>
#include <ilcplex/ilocplex.h>
//using namespace std;
typedef IloArray<IloNumArray> NumMatrix;
using std::vector;

class initial_parameter
{
public:
	int T; // Time Horizon
	int nb_points; // number of regions
	int nb_nodes; // number of scenarios
	int nb_plates; // number of plates in each region
	int budget_A; // given budget for firm A
	int budget_B; // given budget for firm B
	double demand_level; // demand level

	//======================================== method ========================================
	initial_parameter();
	~initial_parameter();
};

class basic_parameter
{

public:

	int nb_firms; // number of firms
	int nb_arcs; // number of arcs' types
	vector<vector<double> > demand_value;
	vector<vector<vector<double> > > base_demand;
	vector<double> time_multiplier;
	vector<vector<int> > period;
	vector<vector<vector<double> > > proportion;
	vector<vector<vector<double> > > demand_region;
	vector<vector<vector<vector<vector<double> > > > > Y1;
	vector<vector<vector<vector<vector<vector<double> > > > > > Y;
	vector<double> probTheta; // possibility
	vector<double> budget;
	vector<double> totallots;
	int interval; // # of unit time intervals in one hour (e.g. if unit time period is 15min, then interval = 4) 
	int start_hour;
	int obj_option;
	bool sharing_option;
	double unsym;
	double service;
	bool competition_option;
	double differ_ratio;
	bool loyal_option;

	//======================================== INPUT PARAMETER

	vector<double> cost_purchase; // the cost of purchasing
	vector<double> penalty; // the penalty for unsatisfied demands
	vector<double> rental_price; // the rental price
	vector<double> trans_price; // the rental price
	vector<vector<double> > h; // cost of different arcs excluding rental arcs
	vector<double> alpha; // loyal rate

	//======================================== MODEL

	IloEnv env;
	IloModel model;
	IloModel model_final;
	IloModel model_independent;
	IloCplex cplex;

	//======================================== Value for output results

	vector<double> totalcost;
	double time;

	//======================================== method ========================================
	basic_parameter(int T = 0, int nb_points = 0, int nb_nodes = 0, int nb_plates = 0, int budget_A = 0, int budget_B = 0, double demand_level = 0);
	~basic_parameter();
};

#endif