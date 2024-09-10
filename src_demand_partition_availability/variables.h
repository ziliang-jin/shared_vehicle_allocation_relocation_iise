#ifndef _VARIABLES_H_
#define _VARIABLES_H_

#include "basic_parameter.h"
#include <stdlib.h>
#include <time.h>
#include <ilcplex/ilocplex.h>

typedef IloArray<IloNumVarArray> NumVar2DMatrix;
typedef IloArray<NumVar2DMatrix> NumVar3DMatrix;
typedef IloArray<NumVar3DMatrix> NumVar4DMatrix;
typedef IloArray<NumVar4DMatrix> NumVar5DMatrix;
typedef IloArray<NumVar5DMatrix> NumVar6DMatrix;

//********************************* Primal Variables *******************************

class primal_variable
{
public:
	NumVar6DMatrix flow;
	NumVar2DMatrix park;
	NumVar5DMatrix demandvar;
	NumVar5DMatrix re;
	NumVar3DMatrix z_trans;
	NumVar4DMatrix z_trans_cap;
	NumVar4DMatrix trans_aux;

	//=== for bilinear
	NumVar3DMatrix y;
	NumVar6DMatrix z_binary;
	NumVar6DMatrix sigma;
	NumVar5DMatrix sigma_re;
	NumVar5DMatrix z_re_binary;
	//========================= method =========================
	primal_variable(initial_parameter initial, basic_parameter basic);
	~primal_variable();
};

//********************************* Dual Variables *********************************

class dual_variable
{
public:
	// dual variables
	IloNumVarArray tau;
	NumVar2DMatrix beta;
	NumVar6DMatrix rho;
	NumVar4DMatrix gamma;
	NumVar6DMatrix etaR;
	NumVar4DMatrix etaI;
	NumVar2DMatrix kappa;
	NumVar3DMatrix y;
	NumVar5DMatrix dre;
	NumVar6DMatrix dwR;
	NumVar6DMatrix dwL;
	NumVar4DMatrix dwI;
	NumVar4DMatrix dwT;
	// binary variables for complementary slackness
	IloNumVarArray z_tau;
	NumVar2DMatrix z_beta;
	NumVar6DMatrix z_etaR;
	NumVar4DMatrix z_etaI;
	NumVar2DMatrix z_kappa;
	NumVar6DMatrix z_dwR;
	NumVar6DMatrix z_dwL;
	NumVar4DMatrix z_dwI;
	NumVar4DMatrix z_dwT;
	NumVar5DMatrix z_dre;

	//========================= method =========================
	dual_variable(initial_parameter initial, basic_parameter basic);
	~dual_variable();
};


#endif