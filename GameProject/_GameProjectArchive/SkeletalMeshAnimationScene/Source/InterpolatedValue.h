#pragma once

#include "Goknar/Log.h"

template<typename T>
struct InterpolatedValue
{
	virtual void Reset(const T& c)
	{
		initial = c;
		current = c;
		destination = c;
		interpolationValue = 1.f;
		timer = 1.f;
		innerTimer = timer;
		speed = 1.f;
		limitWithTime = false;
	}

	void UpdateDestination(const T& newDestination)
	{
		if (this->limitWithTime && 0.f < this->innerTimer)
		{
			return;
		}

		this->destination = newDestination;
		this->initial = this->current;
		this->interpolationValue = 0.f;
		this->innerTimer = this->timer;
	}

	virtual void Interpolate(float deltaTime)
	{
		this->innerTimer -= deltaTime;
		if (this->interpolationValue < 1.f)
		{
			this->interpolationValue += deltaTime * this->speed;
			if (1.f < this->interpolationValue)
			{
				this->interpolationValue = 1.f;
			}

			if (0.f < this->interpolationValue)
			{
				this->current = GoknarMath::Lerp(this->initial, this->destination, this->interpolationValue);
			}
		}
	}

	T initial;
	T current;
	T destination;
	float interpolationValue;
	float speed;
	float timer;
	float innerTimer;
	bool limitWithTime;
};

template<typename T>
struct TwoDestinationInterpolatedValue : public InterpolatedValue<T>
{
	TwoDestinationInterpolatedValue() :
		InterpolatedValue<T>(),
		outSpeed(1.f)
	{

	}

	void UpdateDestination(const T& destinationVector, const T& secondDestinationVector, bool force = false)
	{
		if (this->limitWithTime && 0.f < this->timer)
		{
			return;
		}

		this->destination = destinationVector;
		this->secondDestination = secondDestinationVector;
		this->initial = this->current;
		this->interpolationValue = 0.f;
		this->timer = 1.f;
	}

	virtual void Reset(const T& c) override
	{
		InterpolatedValue<T>::Reset(c);
		secondDestination = c;
	}

	virtual void Interpolate(float deltaTime) override
	{
		this->timer -= deltaTime;
		if (this->interpolationValue < 2.f)
		{
			if (1.f <= this->interpolationValue && this->interpolationValue < 2.f)
			{
				this->interpolationValue += deltaTime * this->outSpeed;
				if (2.f < this->interpolationValue)
				{
					this->interpolationValue = 2.f;
				}
				this->current = GoknarMath::Lerp(this->destination, this->secondDestination, this->interpolationValue - 1.f);
			}
			else if (0.f <= this->interpolationValue && this->interpolationValue < 1.f)
			{
				this->interpolationValue += deltaTime * this->speed;
				this->current = GoknarMath::Lerp(this->initial, this->destination, this->interpolationValue);
			}
		}
	}

	T secondDestination;
	float outSpeed;
};

template<typename T>
struct RotatingInterpolatedValue : public InterpolatedValue<T>
{
	RotatingInterpolatedValue() : InterpolatedValue<T>()
	{
		this->initial = Vector3::ForwardVector;
		this->current = Vector3::ForwardVector;
		this->destination = Vector3::ForwardVector;
	}

	void UpdateDestination(const T& newDestination)
	{
		if (this->limitWithTime && 0.f < this->innerTimer)
		{
			return;
		}

		InterpolatedValue::UpdateDestination(newDestination);

		const float newDestinationAngle = atan2f(newDestination.y, newDestination.x);
		const float currentAngle = atan2f(current.y, current.x);

		this->destinationAngle = newDestinationAngle - currentAngle; 
		if(PI < this->destinationAngle)
		{
			this->destinationAngle -= 2.f * PI;
		}
		else if(this->destinationAngle <= -PI)
		{
			this->destinationAngle += 2.f * PI;
		}

		std::cout << this->destinationAngle << std::endl;
	}

	virtual void Interpolate(float deltaTime)
	{
		this->innerTimer -= deltaTime;

		if (this->interpolationValue < 1.f)
		{
			this->interpolationValue += deltaTime * this->speed;
			if (1.f < this->interpolationValue)
			{
				this->interpolationValue = 1.f;
			}

			if (0.f < this->interpolationValue)
			{
				this->current = this->initial.Rotate(this->upVector * this->destinationAngle * this->interpolationValue, false);
			}
		}
	}

	Vector3 upVector{ Vector3::UpVector };
	float destinationAngle{ 0.f };
};