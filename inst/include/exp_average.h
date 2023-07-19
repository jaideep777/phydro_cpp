#ifndef UTILS_MATH_EXP_MOVING_AVG_H_
#define UTILS_MATH_EXP_MOVING_AVG_H_

#include <iostream>
#include <cmath>
#include <fstream>
#include <cassert>

template <class T>
class ExpAverager{
	private:
	double t_last;
	T      f_last;

	public:
	bool debug = false;
	double tau;

	public:

	ExpAverager(double t0, T f0, double _tau){
		t_last = t0;
		f_last = f0;
		tau = _tau;
	}
	
	void update(double t, double f){
		double dt = t - t_last;
		double alpha = 1 - exp(-dt/tau);
		t_last = t;
		f_last += alpha * (f-f_last);
	}

	T get(){
		return f_last;
	}

	void save(std::ofstream &fout){
		fout << "ExpAverager::v1\n";

		fout << t_last << ' '
		     << f_last << ' '
			 << tau << '\n';		
	}

	void restore(std::ifstream &fin){
		std::string s; fin >> s; // discard version number
		assert(s == "ExpAverager::v1");

		fin >> t_last
			>> f_last
		    >> tau;
	}

};


#endif

