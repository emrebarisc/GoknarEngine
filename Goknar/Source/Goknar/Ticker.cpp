#include "pch.h"

#include "Ticker.h"

Ticker::Ticker() : TimeDependentObject()
{

}

void Ticker::Tick(float deltaSecond)
{
	Operate(deltaSecond);
}