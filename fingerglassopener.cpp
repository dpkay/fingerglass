#include "fingerglassopener.h"

// --------------------------------------------------------
FingerGlassOpener::FingerGlassOpener( TouchPointManager * tpm, int msecs ) : _timer(this)
{
	_timer.setSingleShot(true);
	_timer.start(msecs);
	connect(&_timer, SIGNAL(timeout()), this, SLOT(timeOut()));
	_msecs = msecs;
	_tpm = tpm;
}

// --------------------------------------------------------
void FingerGlassOpener::addTouchById( int id )
{
	_touches.insert(_tpm->touch(id)->pos().x(), id);
	_timer.start(_msecs); // restart
}

// --------------------------------------------------------
void FingerGlassOpener::removeTouchById( int id )
{
	QMap<float, int>::iterator it = _touches.begin();
	forever
	{
		Q_ASSERT(it != _touches.end());
		if(*it == id)
		{
			break;
		}
		++it;
	}
	_touches.erase(it);
}

// --------------------------------------------------------
bool FingerGlassOpener::hasTouchById( int id ) const
{
	QMap<float, int>::iterator it = _touches.begin();
	while(it != _touches.end())
	{
		if(*it == id)
		{
			return true;
		}
		++it;
	}
	return false;
}

// --------------------------------------------------------
bool FingerGlassOpener::hasTouches() const
{
	return !_touches.empty();
}

// --------------------------------------------------------
void FingerGlassOpener::timeOut()
{
	QList<int> id_list = _touches.values();
	QList<SceneTouchPoint *> touch_list;

	while(!id_list.empty() && touch_list.size() < 2)
	{
		// if it has not been released yet, add it
		if(_tpm->touches().contains(id_list.back()))
		{
			touch_list << _tpm->touch(id_list.back());
		}
		id_list.removeLast();
	}
	emit openerTimedOut(touch_list);
}