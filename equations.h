#pragma once
#ifndef EQUATIONS
#define EQUATIONS

#include <iostream>
#include <memory>
#include <string>
#include <cmath>


class Equations {


public: 
	Equations() {}

	double CalcAnswer(int a, int b, int c, int x, std::string EquationContext) {
		if (EquationContext == "00") {
			return  a * x + b;
		}
		if (EquationContext == "01") {
			return  a * pow(x, 2) + b * x + c;
		}
		if (EquationContext == "11") {
			return (a * b * pow(x,3)) + c;
		}
		if (EquationContext == "10") {
			return x*b - c + a;
		}


	}

};


#endif // !EQUATIONS
