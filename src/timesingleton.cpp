#include "timesingleton.h"
#include <QtDebug>

TimeSingleton * TimeSingleton::_instance = 0;

// --------------------------------------------------------
TimeSingleton * TimeSingleton::instance()
{
	if(_instance == 0)
	{
		_instance = new TimeSingleton;
	}

	return _instance;
}

// --------------------------------------------------------
void TimeSingleton::subscribe( Timed * obj )
{
	//qDebug() << "subscribing" << obj;
	Q_ASSERT(!_subscribers.contains(obj));
	_subscribers.insert(obj);
}

// --------------------------------------------------------
//#include "warping.h"
void TimeSingleton::timeStep()
{
	_removed_during_iteration.clear();
	//if(!_subscribers.empty())
		//qDebug() << "TIME STEP" << _subscribers.size();
	foreach(Timed * obj, _subscribers) // todo: optimize with qtconcurrent
	{
		//// maybe it already got killed, TimeSingleton will not be notified in these cases
		//if(obj == NULL)
		//{
		//	
		//}
		// we need this since foreach will actually operate on a copy of the container.
		if(_removed_during_iteration.contains(obj)) continue;

		// send timestep to everybody
		obj->timeStep();
		//qDebug() << "timestepping obj" << obj << ((Warping<qreal> *) obj)->value() << ((Warping<qreal> *) obj)->targetValue();
		
		// in case the timeStep hasn't unsubscribed the object yet, check if we need to do so
		if(_subscribers.contains(obj) && obj->finished())
		{
			_subscribers.remove(obj);
		}
	}
}

// --------------------------------------------------------
void TimeSingleton::unsubscribe( Timed * obj )
{
	Q_ASSERT(_subscribers.contains(obj));
	_subscribers.remove(obj);
	_removed_during_iteration << obj;
}

// --------------------------------------------------------
bool TimeSingleton::hasSubscribers() const
{
	return !_subscribers.empty();
}

// --------------------------------------------------------
bool TimeSingleton::isSubscribed( Timed * obj )
{
	return _subscribers.contains(obj);
}