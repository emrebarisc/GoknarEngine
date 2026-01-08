#pragma once
#include "Goknar/Delegates/stdafx.h"

typedef uint64_t ns;
typedef uint32_t rep;

class Benchmark {

public:

	static void Run() {
		// 20000, 10000
		OperationSet<10000, 10000, 10000, Delegate<double(int)>, double, int> set;
		set.Run(11);
	} //Run

private:

	template<size_t delegate_count, rep creation_repeat_count, rep call_count, typename DELEGATE, typename RET, typename... PARAMS>
	class OperationSet {
	private:
		ns DelegateCreation() {
			Sample sample;
			Start();
			for (rep repeat_index = 0; repeat_index < creation_repeat_count; ++repeat_index)
				for (DELEGATE *j = delegates; j != delegates + delegate_count; ++j)
					*j = DELEGATE:: template Create<Sample, &Sample::A>(&sample);
			return Stop();
		}; //DelegateCreation
		ns LambdaDelegateCreation()  {
			auto lambda = [](PARAMS...) -> RET { return 0.0; };
			Start();
			for (rep repeat_index = 0; repeat_index < creation_repeat_count; ++repeat_index)
				for (DELEGATE *j = delegates; j != delegates + delegate_count; ++j)
					*j = Delegate<RET(PARAMS...)>:: template Create<decltype(lambda)>(lambda);
			return Stop();
		} //LambdaDelegateCreation
		ns DelegateCall(PARAMS... arg) {
			Start();
			for (size_t Delegate = 0; Delegate < delegate_count; ++Delegate)
				for (size_t index = 0; index < call_count; ++index)
					delegates[Delegate](arg...);
			return Stop();
		} //DelegateCall
		ns FunctionCreation() {
			Sample sample;
			using namespace std::placeholders;
			Start();
			for (rep repeat_index = 0; repeat_index < creation_repeat_count; ++repeat_index)
				for (std::function<RET(PARAMS...)> *j = functions; j != functions + delegate_count; ++j)
					*j = std::bind(&Sample::A, &sample, _1);
			return Stop();
		} //FunctionCreation
		ns LambdaFunctionCreation() {
			auto lambda = [](PARAMS...) -> RET { return 0.0; };
			Start();
			for (unsigned int repeat_index = 0; repeat_index < creation_repeat_count; ++repeat_index)
				for (std::function<RET(PARAMS...)> *j = functions; j != functions + delegate_count; ++j)
					*j = std::function<RET(PARAMS...)>(lambda);
			return Stop();
		}; //LambdaFunctionCreation
		ns FunctionCall(PARAMS... arg) {
			Start();
			for (size_t fun = 0; fun < delegate_count; ++fun)
				for (size_t index = 0; index < call_count; ++index)
					functions[fun](arg...);
			return Stop();
		}; //FunctionCall

	public:
		void Run(PARAMS... arg) {
			functionCreationTime = FunctionCreation();
			functionCallTime = FunctionCall(arg...);
			delegateCreationTime = DelegateCreation();
			delegateCallTime = DelegateCall(arg...);
			// lambda:
			lambdaFunctionCreationTime = LambdaFunctionCreation();
			lambdaFunctionCallTime = FunctionCall(arg...);
			lambdaDelegateCreationTime = LambdaDelegateCreation();
			lambdaDelegateCallTime = DelegateCall(arg...);
			Show();
		} //Run
	private:
		DELEGATE delegates[delegate_count];
		std::function<RET(PARAMS...)> functions[delegate_count];
		ns	delegateCreationTime,	lambdaDelegateCreationTime,	delegateCallTime,	lambdaDelegateCallTime,
			functionCreationTime,	lambdaFunctionCreationTime,	functionCallTime,	lambdaFunctionCallTime;
		void Show() {
			auto comparison = [&](ns creationTime1, ns callTime1, ns creationTime2, ns callTime2, const char* title) -> void {
				ns total1 = creationTime1 + callTime1;
				ns total2 = creationTime2 + callTime2;
				long double averageCreation1 = 1.0 * creationTime1 / (delegate_count * creation_repeat_count);
				long double averageCreation2 = 1.0 * creationTime2 / (delegate_count * creation_repeat_count);
				long double averageCall1 = 1.0 * callTime1 / (delegate_count * call_count);
				long double averageCall2 = 1.0 * callTime2 / (delegate_count * call_count);
				GOKNAR_CORE_INFO("{}: ", title);
				GOKNAR_CORE_INFO("\tcreation: {} vs {}", creationTime1, creationTime2);
				GOKNAR_CORE_INFO("\taverage creation: {} vs {}", averageCreation1, averageCreation2);
				GOKNAR_CORE_INFO("\tcall: {} vs {}", callTime1, callTime2);
				GOKNAR_CORE_INFO("\taverage call: {} vs {}", averageCall1, averageCall2);
				GOKNAR_CORE_INFO("\ttotal: {} vs {}", total1, total2);
				GOKNAR_CORE_INFO("\tcreation gain: {}\t", 1.0*creationTime2/creationTime1);
				GOKNAR_CORE_INFO("\tcall gain: {}\t", 1.0*callTime2/ callTime1);
				GOKNAR_CORE_INFO("\ttotal gain: {}\t", 1.0*total2/total1);
			};
			comparison(delegateCreationTime, delegateCallTime, functionCreationTime, functionCallTime,
				"Instance methods of a class, Delegate vs. std::function");
			comparison(lambdaDelegateCreationTime, lambdaDelegateCallTime, lambdaFunctionCreationTime, lambdaFunctionCallTime,
				"Lambda functions, Delegate vs. std::function");
		} //Show
		std::chrono::high_resolution_clock clock;
		std::chrono::high_resolution_clock::time_point before;
		void Start() { before = clock.now(); }
		ns Stop() {
			auto after = clock.now();
			auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(after - before).count();
			return duration;
		} //Stop
	}; //OperationSet

	class Sample {
	public:
		double A(int) { return 0.1; }
		static double B(int) { return 0.2; }
	}; //class Sample

}; /* class Benchmark */
