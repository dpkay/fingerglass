#ifndef periodic_h__
#define periodic_h__

#include "timesingleton.h"

#include <QtDebug>
#include <QLineF>
#include <cmath>

template <typename T>
class Periodic : public Timed
{
public:
	Periodic(const T & min, const T & max, float frequency)
	{
		_min = min;
		_max = max;
		_frequency = frequency;
		_t = 0;
		updateValue();
		TimeSingleton::instance()->subscribe(this);
	}

	~Periodic()
	{
		if(TimeSingleton::instance()->isSubscribed(this))
		{
			TimeSingleton::instance()->unsubscribe(this);
		}
	}

	void destroy()
	{
		TimeSingleton::instance()->unsubscribe(this);
	}

	void timeStep()
	{
		++_t;
		updateValue();
	}

	const T & value() const
	{
		return _value;
	}

protected:
	void updateValue()
	{
		float cos_t = cosf(_frequency * _t);
		_value = cos_t * _max + (1-cos_t) * _min;
	}

private:
	T _value;
	T _min;
	T _max;
	float _frequency;
	float _t;

};

#endif // periodic_h__