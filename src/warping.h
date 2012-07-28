#ifndef warping_h__
#define warping_h__

#include "timesingleton.h"

#include <QtDebug>
#include <QLineF>
#include <cmath>

template <typename T>
class Warping : public Timed
{
public:
	Warping(const T & value, float speed)
	{
		_value = value;
		_target_value = value;
		_speed = speed;
		Q_ASSERT(speed > 0.0 && speed <= 1.0);
	}

	~Warping()
	{
		if(TimeSingleton::instance()->isSubscribed(this))
		{
			TimeSingleton::instance()->unsubscribe(this);
		}
	}

	void setSpeed(float speed)
	{
		_speed = speed;
	}

	void setTarget(const T & target)
	{
		_target_value = target;
		if(_speed == 1.0f)
		{
			_value = _target_value;
		}
		else
		if(!finished() && !TimeSingleton::instance()->isSubscribed(this))
		{
			TimeSingleton::instance()->subscribe(this);
		}
	}

	void restart()
	{
		if(!finished() && !TimeSingleton::instance()->isSubscribed(this))
		{
			TimeSingleton::instance()->subscribe(this);
		}
	}

	//void setInstantly(const T & value)
	//{
	//	_target_value = value;
	//	_value = value;
	//	if(!finished() && !TimeSingleton::instance()->isSubscribed(this))
	//	{
	//		TimeSingleton::instance()->subscribe(this);
	//	}
	//}

	void destroy()
	{
		if(TimeSingleton::instance()->isSubscribed(this))
		{
			TimeSingleton::instance()->unsubscribe(this);
		}
	}

	bool finished() const
	{
		return norm(_value-_target_value) < 0.0001f;
	}

	void finish()
	{
		_value = _target_value;
		TimeSingleton::instance()->unsubscribe(this);
	}

	void timeStep()
	{
		_value = (1-_speed) * _value + _speed * _target_value;
	}

	const T & value() const
	{
		return _value;
	}

	const T & targetValue() const
	{
		return _target_value;
	}

protected:
	inline qreal norm(const T & v) const;

private:
	T _value;
	T _target_value;
	float _speed;
};

template <>
inline qreal Warping<qreal>::norm(const qreal & v) const
{
	return fabs(v);
}

template <>
inline qreal Warping<QPointF>::norm(const QPointF& v) const
{
	return QLineF(QPointF(), v).length();
}
#endif // warping_h__