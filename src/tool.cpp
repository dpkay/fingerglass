#include "tool.h"
#include "touchpointmanager.h"

// --------------------------------------------------------
QDebug operator<<(QDebug dbg, const Tool &t)
{
	dbg.nospace() << "Tool " << &t << "has the following settings:\n" << *t._setting_container;
	return dbg.space();
}

// --------------------------------------------------------
Tool::Tool( SettingContainer * setting_container )
{
	this->_setting_container = setting_container;
	this->_twm = NULL;
	this->_tpm = NULL;
}

// --------------------------------------------------------
SceneTouchPoint * Tool::getFreeTouch()
{
	QList<SceneTouchPoint *> new_points;
	QList<SceneTouchPoint *> existing_points;
	touchPointManager()->getFreeTouches(&new_points, &existing_points);
	if(new_points.size()>0)
	{
		return new_points.back();
	}
	else if(existing_points.size()>0)
	{
		// to catch reselections
		return existing_points.back();
	}
	else
	{
		return NULL;
	}
}

// --------------------------------------------------------
bool Tool::getFreeTouchPair( QPair<SceneTouchPoint *, SceneTouchPoint *> * out_pair )
{
	int primary = -1;
	int secondary = -1;
	QList<SceneTouchPoint *> new_points;
	QList<SceneTouchPoint *> existing_points;
	touchPointManager()->getFreeTouches(&new_points, &existing_points);
	if(new_points.size()>0)
	{
		primary = new_points.back()->id();
		new_points.pop_back();
		if(new_points.size()>0)
		{
			secondary = new_points.back()->id();
		}
		else if(existing_points.size()>0)
		{
			secondary = existing_points.back()->id();
		}
		if(secondary>=0)
		{
			out_pair->first = touchPointManager()->touch(primary);
			out_pair->second = touchPointManager()->touch(secondary);
			return true;
		}
	}
	return false;
}

// --------------------------------------------------------
bool Tool::allowRigidWidgetInteraction( const SceneTouchPoint & stp, const RigidWidgetController & rwc ) const
{
	Q_UNUSED(stp);
	Q_UNUSED(rwc);
	return false;
}

// --------------------------------------------------------
Tool::~Tool()
{
	delete _setting_container;
}