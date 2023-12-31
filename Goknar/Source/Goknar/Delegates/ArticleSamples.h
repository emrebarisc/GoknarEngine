/*

	Copyright (C) 2017 by Sergey A Kryukov: derived work
	http://www.SAKryukov.org
	http://www.codeproject.com/Members/SAKryukov

	Based on original work by Sergey Ryazanov:
	"The Impossibly Fast C++ Delegates", 18 Jul 2005
	https://www.codeproject.com/articles/11015/the-impossibly-fast-c-delegates

	MIT license:
	http://en.wikipedia.org/wiki/MIT_License

	Original publication: https://www.codeproject.com/Articles/1170503/The-Impossibly-Fast-Cplusplus-Delegates-Fixed

*/

#pragma once
#include "stdafx.h"

class ArticleSampleSet {
public:

	double InstanceFunction(int, char, const char*) { return 0.1; }
	double ConstInstanceFunction(int, char, const char*) const { return 0.2; }
	static double StaticFunction(int, char, const char*) { return 0.3; }

	void Demo() {
		using namespace SA;
		typedef ArticleSampleSet Sample;

		ArticleSampleSet sample;

		delegate<double (int, char, const char*)> d;
		auto dInstance = decltype(d)::create<Sample, &Sample::InstanceFunction>(&sample);
		auto dConst = decltype(d)::create<Sample, &Sample::ConstInstanceFunction>(&sample);
		auto dFunc = decltype(d)::create<&Sample::StaticFunction>(); // same thing with non-class functions
		dInstance(0, 'A', "Instance method call");
		dConst(1, 'B', "Constant instance method call");
		dFunc(2, 'C', "Static function call");

		int touchPoint = 1;
		auto lambda = [&touchPoint](int i, char c, const char* msg) -> double {
			std::cout << msg << std::endl;
			// touch point is captured by ref, can change:
			return (++touchPoint + (int)c) * 0.1 - i;
		}; //lambda

		d = lambda;
		decltype(d) dLambda = lambda;
		// or:
		//decltype(d) dLambda(lambda);

		if (d != nullptr) // true
			d(1, '2', "lambda call"); //won't

		d = dLambda; // delegate to delegate

		if (d == dLambda) // true, and also d != nullptr
			d(3, '4', "another lambda call"); //will be called

											  // multicast delegates:

		multicast_delegate<double(int, char, const char*)> md;
		multicast_delegate<double(int, char, const char*)> mdSecond;

		if (md == nullptr) // true
			md(5, '6', "zero calls"); //won't

									  // add some of the delegate instances:
		md += mdSecond; // nothing happens to md
		md += d; // invocation list: size=1
		md += dLambda; // invocation list: size=2

		if (md == dLambda) //false
			std::cout << "md == dLambda" << std::endl;
		if (dLambda == md) //false
			std::cout << "dLambda = md" << std::endl;
		if (md == mdSecond) //false
			std::cout << "md == mdSecond" << std::endl;

		//adding lambda directly:
		md += lambda; // invocation list: size=3

		md(7, '8', "call them all");

		double total = 0;
		md(9, 'a', "handling the return values:",
			[&total](size_t index, double* ret) -> void {
			std::cout << "\t"
				<< "index: "
				<< index
				<< "; returned: " << *ret
				<< std::endl;
			total += *ret;
		});

	} //Demo

}; /* class ArticleSampleSet */

