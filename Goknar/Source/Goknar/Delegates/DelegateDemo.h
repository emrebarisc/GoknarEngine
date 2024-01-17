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

class DelegateSample {

	static int F(char value) {
		printf("Passed parameter: %c\n", value);
		return (char)value;
	} //F

	int instanceField;
	double A(unsigned long value, char c) {
		printf("Passed parameters: %lu, '%c; instance field: %d\n", value, c, instanceField);
		return 1.0 * instanceField / value;
	} //A

	double B(unsigned long value, char& c) const {
		printf("Passed parameters: %lu, '%c; instance field: %d\n", value, c, instanceField);
		return 1.0 * instanceField / value;
	} //B

	static double StaticMember(unsigned long value, char c) {
		printf("Passed parameters: %lu, %c\n", value, c);
		return 1.0 / value;
	} //B

	void Noret(int value) {
		printf("No return; passed parameter: %d\n", value);
	} //Noret

public:

	DelegateSample(int aField) : instanceField(aField) {}
	double InstanceMember(unsigned long, char) { return 0.1;  }

	static void Demo() {
		printf("Single-cast delegates:\n\n");
		DelegateSample sample(11);
		DelegateSample anotherSample(12);
		int touchPoint = 1;
		auto lambda = [&touchPoint](unsigned long a, char b) -> double {
			touchPoint++;
			printf("Delegate is assigned to lambda expression\n");
			return a * 0.5;
		};
		Delegate<double(unsigned long, char)> dlambda;
		dlambda = lambda; // template instantiation deduced (inferred)
		double lambdaResult = dlambda(1, 'a');
		printf("lambda result: %g, captured: %d\n", lambdaResult, touchPoint);
		auto d1 = Delegate<double(unsigned long, char)>::create<DelegateSample, &DelegateSample::A>(&sample);
		auto dcompare = Delegate<double(unsigned long, char)>::create<DelegateSample, &DelegateSample::A>(&sample);
		if (d1 == dcompare)
			printf("Two delegates are the same\n");
		auto d2 = Delegate<double(unsigned long, char)>::create<DelegateSample, &DelegateSample::A>(&anotherSample);
		auto d3 = Delegate<double(unsigned long, char&)>::create<DelegateSample, &DelegateSample::B>(&sample);
		auto d4 = Delegate<double(unsigned long, char&)>::create<DelegateSample, &DelegateSample::B>(&anotherSample);
		auto d5 = Delegate<int(char)>::create<&DelegateSample::F>();
		double ret1 = d1(1, 'a');
		double ret2 = d2(2, 'b');
		printf("Returned: %g, %g\n", ret1, ret2);
		char byRef = 'c';
		double ret3 = d3(3, byRef);
		byRef = 'd';
		double ret4 = d4(4, byRef);
		printf("Returned: %g, %g\n", ret3, ret4);
		d1 = d2;
		int ret5 = d5('e');
		printf("Returned: %d\n", ret5);
		printf("Returned: %g, %g\n", ret3, ret4);
		printf("\nMulticast delegates:\n\n");
		MulticastDelegate<double(unsigned long, char)> md1, md2;
		if (!md2.isNull())
			md2(111, '1');
		((md1 += d1) += d2) += d2;
		md2 += d2;
		md2 += d1;
		md2 += d1;
		md2 += d1;
		md2 += Delegate<double(unsigned long, char)>::create<&DelegateSample::StaticMember>();
		md2 += dlambda; // template instantiation deduced (inferred)
		if (!md1.isNull())
			md1(5, 'F');
		if (!md2.isNull())
			md2(6, 'G');
		printf("\nMulticast delegates, invocation with return:\n\n");
		md2(14, 'm');
		int closure = 0;
		md2(15, 'M', [=, &closure](size_t index, double* result) -> void {
			printf("\t\treturns: %g at %zd\n", *result, index);
			closure++;
		});
		// ----- Multicast return handlers with capture: ------------------------------------------------
		printf("Multicast return handlers with capture: ================\n");
		double total = 0;
		double first = -1;
		double last = -1;
		md2(16, 'N', [&total](size_t index, double* result) -> void {
			printf("\t\treturns: %g at %zd\n", *result, index);
			total += *result;
		});
		printf("Captured total: %g, average: %g \n", total, total/md2.size());
		md2(16, 'N', [&total, &first, &last, &md2](size_t index, double* result) -> void {
			if (index == 0) first = *result;
			if (index == md2.size() - 1) last = *result;
			printf("\t\treturns: %g at %zd\n", *result, index);
			total += *result;
		});
		printf("Captured total: %g, first: %g, last: %g \n", total, first, last);
		printf("not capturing, equivalent to md2(17, 'O'):\n");
	} //Demo

}; /* DelegateSample */