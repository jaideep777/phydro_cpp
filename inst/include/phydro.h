#ifndef PHYDRO_PHYDRO_H
#define PHYDRO_PHYDRO_H


#include "hyd_analytical_solver.h"

namespace phydro{

struct PHydroResult{
	double a;
	double e;
	double gs;
	double ci;
	double chi;
	double vcmax;
	double jmax;
	double dpsi;
	double psi_l;
	double nfnct;
	double niter;
};

inline PHydroResult phydro_analytical(double tc, double ppfd, double vpd, double co2, double elv, double fapar, double kphio, double psi_soil, double rdark, ParPlant par_plant, ParCost par_cost = ParCost(0.1,1)){
	
	double pa = calc_patm(elv);

	ParPhotosynth par_photosynth(tc, pa, kphio, co2, ppfd, fapar, rdark);
	ParEnv        par_env(tc, pa, vpd);

	auto     bounds = calc_dpsi_bound(psi_soil, par_plant, par_env, par_photosynth, par_cost);
	auto   dpsi_opt = pn::zero(bounds.Iabs_bound*0.001, bounds.Iabs_bound*0.999, [&](double dpsi){return dFdx(dpsi, psi_soil, par_plant, par_env, par_photosynth, par_cost).dPdx;}, 1e-6);
	double        x = calc_x_from_dpsi(dpsi_opt.root, psi_soil, par_plant, par_env, par_photosynth, par_cost);  	
	double       gs = calc_gs(dpsi_opt.root, psi_soil, par_plant, par_env);
	double        J = calc_J(gs, x, par_photosynth);	
	double     jmax = calc_jmax_from_J(J, par_photosynth); 
	double    vcmax = (J/4.0)*(x*par_photosynth.ca + par_photosynth.kmm)/(x*par_photosynth.ca + 2*par_photosynth.gammastar);
	double        a = gs*(par_photosynth.ca/par_photosynth.patm*1e6)*(1-x);

	PHydroResult res;
	res.a = a;
	res.e = 1.6*gs*vpd/par_env.patm;
	res.ci = x*par_photosynth.ca;
	res.gs = gs;
	res.chi = x;
	res.vcmax = vcmax;
	res.jmax = jmax;
	res.dpsi = dpsi_opt.root;
	res.psi_l = psi_soil - dpsi_opt.root;
	res.nfnct = dpsi_opt.nfnct;

	return res;

}

} // phydro

#include "hyd_numerical_solver.h"


namespace phydro{

inline PHydroResult phydro_numerical(double tc, double ppfd, double vpd, double co2, double elv, double fapar, double kphio, double psi_soil, double rdark, ParPlant par_plant, ParCost par_cost = ParCost(0.1,1)){
	
	double pa = calc_patm(elv);

	ParPhotosynth par_photosynth(tc, pa, kphio, co2, ppfd, fapar, rdark);
	ParEnv        par_env(tc, pa, vpd);

	auto     opt = optimize_midterm_multi(psi_soil, par_cost, par_photosynth, par_plant, par_env);
	double    gs = calc_gs(opt.dpsi, psi_soil, par_plant, par_env);
	auto      aj = calc_assim_light_limited(gs, opt.jmax, par_photosynth); 	
	double vcmax = vcmax_coordinated_numerical(aj.a, aj.ci, par_photosynth);

	PHydroResult res;
	res.a = aj.a;
	res.e = 1.6*gs*vpd/par_env.patm;
	res.ci = aj.ci;
	res.gs = gs;
	res.chi = aj.ci/par_photosynth.ca;
	res.vcmax = vcmax;
	res.jmax = opt.jmax;
	res.dpsi = opt.dpsi;
	res.psi_l = psi_soil - opt.dpsi;

	return res;

}

} // phydro



#include "hyd_instantaneous_solver_numerical.h"


namespace phydro{

inline PHydroResult phydro_instantaneous_numerical(double vcmax, double jmax, double tc, double ppfd, double vpd, double co2, double elv, double fapar, double kphio, double psi_soil, double rdark, ParPlant par_plant, ParCost par_cost = ParCost(0.1,1)){
	
	double pa = calc_patm(elv);

	ParPhotosynth par_photosynth(tc, pa, kphio, co2, ppfd, fapar, rdark);
	ParEnv        par_env(tc, pa, vpd);

	double  dpsi = optimize_shortterm_multi(vcmax, jmax, psi_soil, par_cost, par_photosynth, par_plant, par_env);
	double    gs = calc_gs(dpsi, psi_soil, par_plant, par_env);
	auto       A = calc_assimilation_limiting(vcmax, jmax, gs, par_photosynth); 	

	PHydroResult res;
	res.a = A.a;
	res.e = 1.6*gs*vpd/par_env.patm;
	res.ci = A.ci;
	res.gs = gs;
	res.chi = A.ci/par_photosynth.ca;
	res.vcmax = vcmax;
	res.jmax = jmax;
	res.dpsi = dpsi;
	res.psi_l = psi_soil - dpsi;

	return res;

}

} // phydro


#include "hyd_instantaneous_solver_analytical.h"


namespace phydro{

inline PHydroResult phydro_instantaneous_analytical(double vcmax, double jmax, double tc, double ppfd, double vpd, double co2, double elv, double fapar, double kphio, double psi_soil, double rdark, ParPlant par_plant, ParCost par_cost = ParCost(0.1,1)){
	
	double pa = calc_patm(elv);

	ParPhotosynth par_photosynth(tc, pa, kphio, co2, ppfd, fapar, rdark);
	ParEnv        par_env(tc, pa, vpd);

	auto dpsi_opt = pn::zero(0, 20, [&](double dpsi){return calc_dP_ddpsi(dpsi, vcmax, jmax, psi_soil, par_plant, par_env, par_photosynth, par_cost);}, 1e-6);
	double     gs = calc_gs(dpsi_opt.root, psi_soil, par_plant, par_env);
	auto        A = calc_assimilation_limiting(vcmax, jmax, gs, par_photosynth); 	

	PHydroResult res;
	res.a = A.a;
	res.e = 1.6*gs*vpd/par_env.patm;
	res.ci = A.ci;
	res.gs = gs;
	res.chi = A.ci/par_photosynth.ca;
	res.vcmax = vcmax;
	res.jmax = jmax;
	res.dpsi = dpsi_opt.root;
	res.psi_l = psi_soil - dpsi_opt.root;

	return res;

}

} // phydro




// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//       R Interface
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef USINGRCPP

#include <Rcpp.h>

namespace phydro{

inline Rcpp::List PHydroResult_to_List(const phydro::PHydroResult& res){
	return Rcpp::List::create(
	           Named("a") = res.a,
	           Named("e") = res.e,
	           Named("gs") = res.gs,
	           Named("ci") = res.ci,
	           Named("chi") = res.chi,
	           Named("vcmax") = res.vcmax,
	           Named("jmax") = res.jmax,
	           Named("dpsi") = res.dpsi,
	           Named("psi_l") = res.psi_l,
	           Named("nfnct") = res.nfnct,
	           Named("niter") = res.niter,
	           Named("chi_jmax_lim") = 0,
	           Named("profit") = 0
	       );
}


inline Rcpp::List rphydro_numerical(double tc, double ppfd, double vpd, double co2, double elv, double fapar, double kphio, double psi_soil, double rdark, Rcpp::List par_plant, Rcpp::List par_cost){
	ParPlant par_plant_cpp(par_plant["conductivity"], par_plant["psi50"], par_plant["b"]);
	ParCost  par_cost_cpp(par_cost["alpha"], par_cost["gamma"]);
	return PHydroResult_to_List(phydro_numerical(tc, ppfd, vpd, co2, elv, fapar, kphio, psi_soil, rdark, par_plant_cpp, par_cost_cpp));
}

inline Rcpp::List rphydro_analytical(double tc, double ppfd, double vpd, double co2, double elv, double fapar, double kphio, double psi_soil, double rdark, Rcpp::List par_plant, Rcpp::List par_cost){
	ParPlant par_plant_cpp(par_plant["conductivity"], par_plant["psi50"], par_plant["b"]);
	ParCost  par_cost_cpp(par_cost["alpha"], par_cost["gamma"]);
	return PHydroResult_to_List(phydro_analytical(tc, ppfd, vpd, co2, elv, fapar, kphio, psi_soil, rdark, par_plant_cpp, par_cost_cpp));
}

inline Rcpp::List rphydro_instantaneous_numerical(double vcmax, double jmax, double tc, double ppfd, double vpd, double co2, double elv, double fapar, double kphio, double psi_soil, double rdark, Rcpp::List par_plant, Rcpp::List par_cost){
	ParPlant par_plant_cpp(par_plant["conductivity"], par_plant["psi50"], par_plant["b"]);
	ParCost  par_cost_cpp(par_cost["alpha"], par_cost["gamma"]);
	return PHydroResult_to_List(phydro_instantaneous_numerical(vcmax, jmax, tc, ppfd, vpd, co2, elv, fapar, kphio, psi_soil, rdark, par_plant_cpp, par_cost_cpp));
}

inline Rcpp::List rphydro_instantaneous_analytical(double vcmax, double jmax, double tc, double ppfd, double vpd, double co2, double elv, double fapar, double kphio, double psi_soil, double rdark, Rcpp::List par_plant, Rcpp::List par_cost){
	ParPlant par_plant_cpp(par_plant["conductivity"], par_plant["psi50"], par_plant["b"]);
	ParCost  par_cost_cpp(par_cost["alpha"], par_cost["gamma"]);
	return PHydroResult_to_List(phydro_instantaneous_analytical(vcmax, jmax, tc, ppfd, vpd, co2, elv, fapar, kphio, psi_soil, rdark, par_plant_cpp, par_cost_cpp));
}


} // phydro namespace

#endif


#endif
