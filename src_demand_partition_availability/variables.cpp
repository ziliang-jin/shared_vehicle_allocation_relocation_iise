#include "variables.h"

//=================================== Primal Variables ===============================

primal_variable::primal_variable(initial_parameter initial, basic_parameter basic){
	flow = NumVar6DMatrix(basic.env, basic.nb_firms);
	for (int i = 0; i < basic.nb_firms; i++){
		flow[i] = NumVar5DMatrix(basic.env, initial.nb_points);
		for (int j = 0; j < initial.nb_points; j++){
			flow[i][j] = NumVar4DMatrix(basic.env, initial.nb_points);
			for (int k = 0; k < initial.nb_points; k++){
				flow[i][j][k] = NumVar3DMatrix(basic.env, initial.T + 1);
				for (int s = 0; s <= initial.T; s++){
					flow[i][j][k][s] = NumVar2DMatrix(basic.env, basic.nb_arcs);
					for (int a = 0; a < basic.nb_arcs; a++){
						flow[i][j][k][s][a] = IloNumVarArray(basic.env, initial.nb_nodes);
						for (int n = 0; n < initial.nb_nodes; n++){
							flow[i][j][k][s][a][n] = IloNumVar(basic.env, 0, IloInfinity, ILOINT);

							//char flowName[50];
							//sprintf(flowName, "flow(%d,%d,%d,%d,%d,%d)", (int)i, (int)j, (int)k, (int)s, (int)a, (int)n);
							//flow[i][j][k][s][a][n].setName(flowName);
						}
					}
				}
			}
		}
	}

	park = NumVar2DMatrix(basic.env, basic.nb_firms);
	for (int i = 0; i < basic.nb_firms; i++){
		park[i] = IloNumVarArray(basic.env, initial.nb_points);
		for (int j = 0; j < initial.nb_points; j++){
			park[i][j] = IloNumVar(basic.env, 0, IloInfinity, ILOFLOAT);

			//char uName[50];
			//sprintf(uName, "x(%d,%d)", (int)i, (int)j);
			//park[i][j].setName(uName);
		}
	}

	demandvar = NumVar5DMatrix(basic.env, basic.nb_firms);
	for (int i = 0; i < basic.nb_firms; i++){
		demandvar[i] = NumVar4DMatrix(basic.env, initial.nb_points);
		for (int j = 0; j < initial.nb_points; j++){
			demandvar[i][j] = NumVar3DMatrix(basic.env, initial.nb_points);
			for (int k = 0; k < initial.nb_points; k++){
				demandvar[i][j][k] = NumVar2DMatrix(basic.env, initial.T + 1);
				for (int s = 0; s <= initial.T; s++){
					demandvar[i][j][k][s] = IloNumVarArray(basic.env, initial.nb_nodes);
					for (int n = 0; n < initial.nb_nodes; n++){
						demandvar[i][j][k][s][n] = IloNumVar(basic.env, 0, IloInfinity, ILOFLOAT);

						//char dName[50];
						//sprintf(dName, "d(%d,%d,%d,%d,%d)", (int)i, (int)j, (int)k, (int)s, (int)n);
						//demandvar[i][j][k][s][n].setName(dName);
					}
				}
			}
		}
	}

	re = NumVar5DMatrix(basic.env, basic.nb_firms);
	for (int k = 0; k < basic.nb_firms; k++){
		re[k] = NumVar4DMatrix(basic.env, initial.nb_points);
		for (int i = 0; i < initial.nb_points; i++){
			re[k][i] = NumVar3DMatrix(basic.env, initial.T + 1);
			for (int t = 0; t <= initial.T; t++){
				re[k][i][t] = NumVar2DMatrix(basic.env, 3);
				for (int d = 0; d < 3; d++){
					re[k][i][t][d] = IloNumVarArray(basic.env, initial.nb_nodes);
					for (int n = 0; n < initial.nb_nodes; n++){
						if (basic.sharing_option == 1) {
							re[k][i][t][d][n] = IloNumVar(basic.env, 0, IloInfinity, ILOINT);
						}
						else {
							re[k][i][t][d][n] = IloNumVar(basic.env, 0, 0, ILOINT);
						}

						//char reName[50];
						//sprintf(reName, "re(%d,%d,%d,%d,%d)", (int)k, (int)i, (int)t, (int)d, (int)n);
						//re[k][i][t][d][n].setName(reName);
					}
				}
			}
		}
	}

	z_trans = NumVar3DMatrix(basic.env, initial.nb_points);
	for (int i = 0; i < initial.nb_points; i++){
		z_trans[i] = NumVar2DMatrix(basic.env, initial.T + 1);
		for (int t = 0; t <= initial.T; t++){
			z_trans[i][t] = IloNumVarArray(basic.env, initial.nb_nodes);
			for (int n = 0; n < initial.nb_nodes; n++){
				z_trans[i][t][n] = IloNumVar(basic.env, 0, 1, ILOBOOL);
				
				//char z_transName[50];
				//sprintf(z_transName, "z_trans(%d,%d,%d)", (int)i, (int)t, (int)n);
				//z_trans[i][t][n].setName(z_transName);
			}
		}
	}

	z_trans_cap = NumVar4DMatrix(basic.env, basic.nb_firms); 
	trans_aux = NumVar4DMatrix(basic.env, basic.nb_firms);
	for (int k = 0; k < basic.nb_firms; k++) {
		z_trans_cap[k] = NumVar3DMatrix(basic.env, initial.nb_points);
		trans_aux[k] = NumVar3DMatrix(basic.env, initial.nb_points);
		for (int i = 0; i < initial.nb_points; i++) {
			z_trans_cap[k][i] = NumVar2DMatrix(basic.env, initial.T + 1);
			trans_aux[k][i] = NumVar2DMatrix(basic.env, initial.T + 1);
			for (int t = 0; t <= initial.T; t++) {
				z_trans_cap[k][i][t] = IloNumVarArray(basic.env, initial.nb_nodes);
				trans_aux[k][i][t] = IloNumVarArray(basic.env, initial.nb_nodes);
				for (int n = 0; n < initial.nb_nodes; n++) {
					z_trans_cap[k][i][t][n] = IloNumVar(basic.env, 0, 1, ILOBOOL);
					trans_aux[k][i][t][n] = IloNumVar(basic.env, -IloInfinity, IloInfinity, ILOFLOAT);
				}
			}
		}
	}

	//=== for bilinear
	y = NumVar3DMatrix(basic.env, initial.nb_points);
	for (int i = 0; i < initial.nb_points; i++) {
		y[i] = NumVar2DMatrix(basic.env, initial.T + 1);
		for (int t = 0; t <= initial.T; t++) { 
			y[i][t] = IloNumVarArray(basic.env, initial.nb_nodes);
			for (int n = 0; n < initial.nb_nodes; n++) {
				y[i][t][n] = IloNumVar(basic.env, 0, 1, ILOFLOAT);
			}
		}
	}

	z_binary = NumVar6DMatrix(basic.env, basic.M+1); // 10 bits: [0,M]
	sigma = NumVar6DMatrix(basic.env, basic.M+1);
	for (int b = 0; b <= basic.M; b++) {
		z_binary[b] = NumVar5DMatrix(basic.env, initial.nb_points);
		sigma[b] = NumVar5DMatrix(basic.env, initial.nb_points);
		for (int j = 0; j < initial.nb_points; j++) {
			z_binary[b][j] = NumVar4DMatrix(basic.env, initial.nb_points);
			sigma[b][j] = NumVar4DMatrix(basic.env, initial.nb_points);
			for (int k = 0; k < initial.nb_points; k++) {
				z_binary[b][j][k] = NumVar3DMatrix(basic.env, initial.T + 1);
				sigma[b][j][k] = NumVar3DMatrix(basic.env, initial.T + 1);
				for (int s = 0; s <= initial.T; s++) {
					z_binary[b][j][k][s] = NumVar2DMatrix(basic.env, basic.nb_arcs);
					sigma[b][j][k][s] = NumVar2DMatrix(basic.env, basic.nb_arcs);
					for (int a = 0; a < basic.nb_arcs; a++) {
						z_binary[b][j][k][s][a] = IloNumVarArray(basic.env, initial.nb_nodes);
						sigma[b][j][k][s][a] = IloNumVarArray(basic.env, initial.nb_nodes);
						for (int n = 0; n < initial.nb_nodes; n++) {
							if (a == 0 || a == 2) {
							//if (a <= 2) {
								z_binary[b][j][k][s][a][n] = IloNumVar(basic.env, 0, 1, ILOBOOL);
							}
							else {
								z_binary[b][j][k][s][a][n] = IloNumVar(basic.env, 0, 1, ILOFLOAT);
							}
							//z_binary[b][j][k][s][a][n] = IloNumVar(basic.env, 0, 1, ILOBOOL);
							sigma[b][j][k][s][a][n] = IloNumVar(basic.env, 0, 1, ILOFLOAT);
						}
					}
				}
			}
		}
	}

	sigma_re = NumVar5DMatrix(basic.env, basic.M + 1);
	z_re_binary = NumVar5DMatrix(basic.env, basic.M + 1);
	for (int m = 0; m <= basic.M; m++) {
		sigma_re[m] = NumVar4DMatrix(basic.env, initial.nb_points);
		z_re_binary[m] = NumVar4DMatrix(basic.env, initial.nb_points);
		for (int i = 0; i < initial.nb_points; i++) {
			sigma_re[m][i] = NumVar3DMatrix(basic.env, initial.T + 1);
			z_re_binary[m][i] = NumVar3DMatrix(basic.env, initial.T + 1);
			for (int t = 0; t <= initial.T; t++) {
				sigma_re[m][i][t] = NumVar2DMatrix(basic.env, 3);
				z_re_binary[m][i][t] = NumVar2DMatrix(basic.env, 3);
				for (int d = 0; d < 3; d++) {
					sigma_re[m][i][t][d] = IloNumVarArray(basic.env, initial.nb_nodes);
					z_re_binary[m][i][t][d] = IloNumVarArray(basic.env, initial.nb_nodes);
					for (int n = 0; n < initial.nb_nodes; n++) {
						sigma_re[m][i][t][d][n] = IloNumVar(basic.env, 0, 1, ILOFLOAT);
						//z_re_binary[m][i][t][d][n] = IloNumVar(basic.env, 0, 1, ILOBOOL);
						z_re_binary[m][i][t][d][n] = IloNumVar(basic.env, 0, 1, ILOFLOAT);
					}
				}
			}
		}
	}
};
primal_variable::~primal_variable(){};

//=================================== Dual Variables ===============================
dual_variable::dual_variable(initial_parameter initial, basic_parameter basic){
	tau = IloNumVarArray(basic.env, basic.nb_firms);
	z_tau = IloNumVarArray(basic.env, basic.nb_firms);
	for (int k = 0; k < basic.nb_firms; k++){
		tau[k] = IloNumVar(basic.env, 0, IloInfinity, ILOFLOAT);
		z_tau[k] = IloNumVar(basic.env, 0, 1, ILOBOOL);

		//char tauName[50];
		//sprintf(tauName, "tau(%d)", (int)k);
		//tau[k].setName(tauName);
		//char z_tauName[50];
		//sprintf(z_tauName, "z_tau(%d)", (int)k);
		//z_tau[k].setName(z_tauName);
	}
	beta = NumVar2DMatrix(basic.env, basic.nb_firms);
	kappa = NumVar2DMatrix(basic.env, basic.nb_firms);
	z_beta = NumVar2DMatrix(basic.env, basic.nb_firms);
	z_kappa = NumVar2DMatrix(basic.env, basic.nb_firms);
	for (int k = 0; k < basic.nb_firms; k++){
		beta[k] = IloNumVarArray(basic.env, initial.nb_points);
		kappa[k] = IloNumVarArray(basic.env, initial.nb_points);
		z_beta[k] = IloNumVarArray(basic.env, initial.nb_points);
		z_kappa[k] = IloNumVarArray(basic.env, initial.nb_points);
		for (int i = 0; i < initial.nb_points; i++){
			beta[k][i] = IloNumVar(basic.env, 0, IloInfinity, ILOFLOAT);
			kappa[k][i] = IloNumVar(basic.env, -IloInfinity, 0, ILOFLOAT);
			z_beta[k][i] = IloNumVar(basic.env, 0, 1, ILOBOOL);
			z_kappa[k][i] = IloNumVar(basic.env, 0, 1, ILOBOOL);
			/*
			char betaName[50];
			sprintf(betaName, "beta(%d,%d)", (int)k, (int) i);
			beta[k][i].setName(betaName);
			char kappaName[50];
			sprintf(kappaName, "kappa(%d,%d)", (int)k, (int)i);
			kappa[k][i].setName(kappaName);
			char z_betaName[50];
			sprintf(z_betaName, "z_beta(%d,%d)", (int)k, (int)i);
			z_beta[k][i].setName(z_betaName);
			char z_kappaName[50];
			sprintf(z_kappaName, "z_kappa(%d,%d)", (int)k, (int)i);
			z_kappa[k][i].setName(z_kappaName);
			*/
		}
	}
	rho = NumVar6DMatrix(basic.env, basic.nb_firms);
	etaR = NumVar6DMatrix(basic.env, basic.nb_firms);
	dwR = NumVar6DMatrix(basic.env, basic.nb_firms);
	dwL = NumVar6DMatrix(basic.env, basic.nb_firms);
	z_etaR = NumVar6DMatrix(basic.env, basic.nb_firms);
	z_dwR = NumVar6DMatrix(basic.env, basic.nb_firms);
	z_dwL = NumVar6DMatrix(basic.env, basic.nb_firms);
	for (int k = 0; k < basic.nb_firms; k++){
		rho[k] = NumVar5DMatrix(basic.env, initial.nb_points);
		etaR[k] = NumVar5DMatrix(basic.env, initial.nb_points);
		dwR[k] = NumVar5DMatrix(basic.env, initial.nb_points);
		dwL[k] = NumVar5DMatrix(basic.env, initial.nb_points);
		z_etaR[k] = NumVar5DMatrix(basic.env, initial.nb_points);
		z_dwR[k] = NumVar5DMatrix(basic.env, initial.nb_points);
		z_dwL[k] = NumVar5DMatrix(basic.env, initial.nb_points);
		for (int i = 0; i < initial.nb_points; i++){
			rho[k][i] = NumVar4DMatrix(basic.env, initial.nb_points);
			etaR[k][i] = NumVar4DMatrix(basic.env, initial.nb_points);
			dwR[k][i] = NumVar4DMatrix(basic.env, initial.nb_points);
			dwL[k][i] = NumVar4DMatrix(basic.env, initial.nb_points);
			z_etaR[k][i] = NumVar4DMatrix(basic.env, initial.nb_points);
			z_dwR[k][i] = NumVar4DMatrix(basic.env, initial.nb_points);
			z_dwL[k][i] = NumVar4DMatrix(basic.env, initial.nb_points);
			for (int j = 0; j < initial.nb_points; j++){
				rho[k][i][j] = NumVar3DMatrix(basic.env, initial.T + 1);
				etaR[k][i][j] = NumVar3DMatrix(basic.env, initial.T + 1);
				dwR[k][i][j] = NumVar3DMatrix(basic.env, initial.T + 1);
				dwL[k][i][j] = NumVar3DMatrix(basic.env, initial.T + 1);
				z_etaR[k][i][j] = NumVar3DMatrix(basic.env, initial.T + 1);
				z_dwR[k][i][j] = NumVar3DMatrix(basic.env, initial.T + 1);
				z_dwL[k][i][j] = NumVar3DMatrix(basic.env, initial.T + 1);
				for (int t = 0; t <= initial.T; t++){
					rho[k][i][j][t] = NumVar2DMatrix(basic.env, initial.T + 1);
					etaR[k][i][j][t] = NumVar2DMatrix(basic.env, initial.T + 1);
					dwR[k][i][j][t] = NumVar2DMatrix(basic.env, initial.T + 1);
					dwL[k][i][j][t] = NumVar2DMatrix(basic.env, initial.T + 1);
					z_etaR[k][i][j][t] = NumVar2DMatrix(basic.env, initial.T + 1);
					z_dwR[k][i][j][t] = NumVar2DMatrix(basic.env, initial.T + 1);
					z_dwL[k][i][j][t] = NumVar2DMatrix(basic.env, initial.T + 1);
					for (int tt = 0; tt <= initial.T; tt++){
						rho[k][i][j][t][tt] = IloNumVarArray(basic.env, initial.nb_nodes);
						etaR[k][i][j][t][tt] = IloNumVarArray(basic.env, initial.nb_nodes);
						dwR[k][i][j][t][tt] = IloNumVarArray(basic.env, initial.nb_nodes);
						dwL[k][i][j][t][tt] = IloNumVarArray(basic.env, initial.nb_nodes);
						z_etaR[k][i][j][t][tt] = IloNumVarArray(basic.env, initial.nb_nodes);
						z_dwR[k][i][j][t][tt] = IloNumVarArray(basic.env, initial.nb_nodes);
						z_dwL[k][i][j][t][tt] = IloNumVarArray(basic.env, initial.nb_nodes);
						for (int s = 0; s < initial.nb_nodes; s++){
							rho[k][i][j][t][tt][s] = IloNumVar(basic.env, -IloInfinity, IloInfinity, ILOFLOAT);
							etaR[k][i][j][t][tt][s] = IloNumVar(basic.env, 0, IloInfinity, ILOFLOAT);
							dwR[k][i][j][t][tt][s] = IloNumVar(basic.env, -IloInfinity, 0, ILOFLOAT);
							dwL[k][i][j][t][tt][s] = IloNumVar(basic.env, -IloInfinity, 0, ILOFLOAT);
							z_etaR[k][i][j][t][tt][s] = IloNumVar(basic.env, 0, 1, ILOBOOL);
							z_dwR[k][i][j][t][tt][s] = IloNumVar(basic.env, 0, 1, ILOBOOL);
							z_dwL[k][i][j][t][tt][s] = IloNumVar(basic.env, 0, 1, ILOBOOL);
							
							/*
							char rhoName[50];
							sprintf(rhoName, "rho(%d,%d,%d,%d,%d,%d)", (int)k, (int)i, (int)j, (int)t, (int)tt, (int)s);
							rho[k][i][j][t][tt][s].setName(rhoName);
							char etaRName[50];
							sprintf(etaRName, "yetaR(%d,%d,%d,%d,%d,%d)", (int)k, (int)i, (int)j, (int)t, (int)tt, (int)s);
							etaR[k][i][j][t][tt][s].setName(etaRName);
							char dwRName[50];
							sprintf(dwRName, "dwR(%d,%d,%d,%d,%d,%d)", (int)k, (int)i, (int)j, (int)t, (int)tt, (int)s);
							dwR[k][i][j][t][tt][s].setName(dwRName);
							char dwLName[50];
							sprintf(dwLName, "dwL(%d,%d,%d,%d,%d,%d)", (int)k, (int)i, (int)j, (int)t, (int)tt, (int)s);
							dwL[k][i][j][t][tt][s].setName(dwLName);
							char z_etaRName[50];
							sprintf(z_etaRName, "z_etaR(%d,%d,%d,%d,%d,%d)", (int)k, (int)i, (int)j, (int)t, (int)tt, (int)s);
							z_etaR[k][i][j][t][tt][s].setName(z_etaRName);
							char z_dwRName[50];
							sprintf(z_dwRName, "z_dwR(%d,%d,%d,%d,%d,%d)", (int)k, (int)i, (int)j, (int)t, (int)tt, (int)s);
							z_dwR[k][i][j][t][tt][s].setName(z_dwRName);
							char z_dwLName[50];
							sprintf(z_dwLName, "z_dwL(%d,%d,%d,%d,%d,%d)", (int)k, (int)i, (int)j, (int)t, (int)tt, (int)s);
							z_dwL[k][i][j][t][tt][s].setName(z_dwLName);
							*/
						}
					}
				}
			}
		}
	}
	gamma = NumVar4DMatrix(basic.env, basic.nb_firms);
	etaI = NumVar4DMatrix(basic.env, basic.nb_firms);
	dwI = NumVar4DMatrix(basic.env, basic.nb_firms);
	dwT = NumVar4DMatrix(basic.env, basic.nb_firms);
	z_etaI = NumVar4DMatrix(basic.env, basic.nb_firms);
	z_dwI = NumVar4DMatrix(basic.env, basic.nb_firms);
	z_dwT = NumVar4DMatrix(basic.env, basic.nb_firms);
	for (int k = 0; k < basic.nb_firms; k++){
		gamma[k] = NumVar3DMatrix(basic.env, initial.nb_points);
		etaI[k] = NumVar3DMatrix(basic.env, initial.nb_points);
		dwI[k] = NumVar3DMatrix(basic.env, initial.nb_points);
		dwT[k] = NumVar3DMatrix(basic.env, initial.nb_points);
		z_etaI[k] = NumVar3DMatrix(basic.env, initial.nb_points);
		z_dwI[k] = NumVar3DMatrix(basic.env, initial.nb_points);
		z_dwT[k] = NumVar3DMatrix(basic.env, initial.nb_points);
		for (int i = 0; i < initial.nb_points; i++){
			gamma[k][i] = NumVar2DMatrix(basic.env, initial.T + 1);
			etaI[k][i] = NumVar2DMatrix(basic.env, initial.T + 1);
			dwI[k][i] = NumVar2DMatrix(basic.env, initial.T + 1);
			dwT[k][i] = NumVar2DMatrix(basic.env, initial.T + 1);
			z_etaI[k][i] = NumVar2DMatrix(basic.env, initial.T + 1);
			z_dwI[k][i] = NumVar2DMatrix(basic.env, initial.T + 1);
			z_dwT[k][i] = NumVar2DMatrix(basic.env, initial.T + 1);
			for (int t = 0; t <= initial.T; t++){
				gamma[k][i][t] = IloNumVarArray(basic.env, initial.nb_nodes);
				etaI[k][i][t] = IloNumVarArray(basic.env, initial.nb_nodes);
				dwI[k][i][t] = IloNumVarArray(basic.env, initial.nb_nodes);
				dwT[k][i][t] = IloNumVarArray(basic.env, initial.nb_nodes);
				z_etaI[k][i][t] = IloNumVarArray(basic.env, initial.nb_nodes);
				z_dwI[k][i][t] = IloNumVarArray(basic.env, initial.nb_nodes);
				z_dwT[k][i][t] = IloNumVarArray(basic.env, initial.nb_nodes);
				for (int s = 0; s < initial.nb_nodes; s++){
					gamma[k][i][t][s] = IloNumVar(basic.env, -IloInfinity, IloInfinity, ILOFLOAT);
					etaI[k][i][t][s] = IloNumVar(basic.env, 0, IloInfinity, ILOFLOAT);
					dwI[k][i][t][s] = IloNumVar(basic.env, -IloInfinity, 0, ILOFLOAT);
					dwT[k][i][t][s] = IloNumVar(basic.env, -IloInfinity, 0, ILOFLOAT);
					z_etaI[k][i][t][s] = IloNumVar(basic.env, 0, 1, ILOBOOL);
					z_dwI[k][i][t][s] = IloNumVar(basic.env, 0, 1, ILOBOOL);
					z_dwT[k][i][t][s] = IloNumVar(basic.env, 0, 1, ILOBOOL);
					/*
					char gammaName[50];
					sprintf(gammaName, "gamma(%d,%d,%d,%d)", (int)k, (int)i, (int)t, (int)s);
					gamma[k][i][t][s].setName(gammaName);
					char etaIName[50];
					sprintf(etaIName, "yetaI(%d,%d,%d,%d)", (int)k, (int)i, (int)t, (int)s);
					etaI[k][i][t][s].setName(etaIName);
					char dwIName[50];
					sprintf(dwIName, "dwI(%d,%d,%d,%d)", (int)k, (int)i, (int)t, (int)s);
					dwI[k][i][t][s].setName(dwIName);
					char dwTName[50];
					sprintf(dwTName, "dwT(%d,%d,%d,%d)", (int)k, (int)i, (int)t, (int)s);
					dwT[k][i][t][s].setName(dwTName);
					char z_etaIName[50];
					sprintf(z_etaIName, "z_etaI(%d,%d,%d,%d)", (int)k, (int)i, (int)t, (int)s);
					z_etaI[k][i][t][s].setName(z_etaIName);
					char z_dwIName[50];
					sprintf(z_dwIName, "z_dwI(%d,%d,%d,%d)", (int)k, (int)i, (int)t, (int)s);
					z_dwI[k][i][t][s].setName(z_dwIName);
					char z_dwTName[50];
					sprintf(z_dwTName, "z_dwT(%d,%d,%d,%d)", (int)k, (int)i, (int)t, (int)s);
					z_dwT[k][i][t][s].setName(z_dwTName);
					*/
				}
			}
		}
	}

	y = NumVar3DMatrix(basic.env, basic.nb_firms);
	for (int k = 0; k < basic.nb_firms; k++){
		y[k] = NumVar2DMatrix(basic.env, initial.T + 1);
		for (int t = 0; t <= initial.T; t++){
			y[k][t] = IloNumVarArray(basic.env, initial.nb_nodes);
			for (int s = 0; s < initial.nb_nodes; s++){
				y[k][t][s] = IloNumVar(basic.env, -IloInfinity, IloInfinity, ILOFLOAT);

				//char yName[50];
				//sprintf(yName, "y(%d,%d,%d)", (int)k, (int)t, (int)s);
				//y[k][t][s].setName(yName);
			}
		}
	}

	dre = NumVar5DMatrix(basic.env, basic.nb_firms);
	z_dre = NumVar5DMatrix(basic.env, basic.nb_firms);
	for (int k = 0; k < basic.nb_firms; k++){
		dre[k] = NumVar4DMatrix(basic.env, initial.nb_points);
		z_dre[k] = NumVar4DMatrix(basic.env, initial.nb_points);
		for (int i = 0; i < initial.nb_points; i++){
			dre[k][i] = NumVar3DMatrix(basic.env, initial.T + 1);
			z_dre[k][i] = NumVar3DMatrix(basic.env, initial.T + 1);
			for (int t = 0; t <= initial.T; t++){
				dre[k][i][t] = NumVar2DMatrix(basic.env, 3);
				z_dre[k][i][t] = NumVar2DMatrix(basic.env, 3);
				for (int d = 0; d < 3; d++){
					dre[k][i][t][d] = IloNumVarArray(basic.env, initial.nb_nodes);
					z_dre[k][i][t][d] = IloNumVarArray(basic.env, initial.nb_nodes);
					for (int s = 0; s < initial.nb_nodes; s++){
						dre[k][i][t][d][s] = IloNumVar(basic.env, -IloInfinity, 0, ILOFLOAT);
						z_dre[k][i][t][d][s] = IloNumVar(basic.env, 0, 1, ILOBOOL);
						/*
						char dreName[50];
						sprintf(dreName, "dre(%d,%d,%d,%d,%d)", (int)k, (int)i, (int)t, (int)d, (int)s);
						dre[k][i][t][d][s].setName(dreName);
						char z_dreName[50];
						sprintf(z_dreName, "z_dre(%d,%d,%d,%d,%d)", (int)k, (int)i, (int)t, (int)d, (int)s);
						z_dre[k][i][t][d][s].setName(z_dreName);
						*/
					}
				}
			}
		}
	}
};
dual_variable::~dual_variable(){};