#include "rigidwidgetcontroller.h"
#include "timesingleton.h"
#include "globallog.h"

#include <QtDebug>
#include <QLineF>


// --------------------------------------------------------
RigidWidgetController::RigidWidgetController(TouchWidgetType type) :
	TouchWidgetController(type)
{
	this->_movable = true;
	this->_original_pos = NULL;
	this->_radius = 0;
	this->_touch_point_halo_visible = false;
	this->_attached_to_handle_widget = false;
	this->_highlighted = false;
	//this->_after_glow_halo_visible = false;
	this->_after_glow_bubble = NULL;
	this->_selection_mode = DirectPointSelection;
	//this->_selectable_by_bubble_only = false;
}

// --------------------------------------------------------
void RigidWidgetController::touchPointReleased( SceneTouchPoint * p )
{
	Q_UNUSED(p);
	if(_after_glow_bubble != NULL)
	{
		_after_glow_bubble->restartTimer();
		//_after_glow_bubble->move(_pos - *_original_pos);
		if(_after_glow_bubble->hasBubble())
		{
			_after_glow_bubble->moveTo(p->pos());
		}
	}
	qDebug() << "released rigid widget";
	logger() << "rwc_release" << this << p->id() << pos();

	delete _original_pos;
	_original_pos = 0;
}

// --------------------------------------------------------
void RigidWidgetController::touchPointMoved( SceneTouchPoint * p )
{
	Q_ASSERT(_original_pos != NULL);

	// for now, as we are only moving
	Q_ASSERT(numPressedTouchPoints() == 1);

	_pos = *_original_pos + (p->pos() - p->startPos());

	logger() << "rwc_move" << this << p->id() << pos();
}

// --------------------------------------------------------
void RigidWidgetController::touchPointPressed( SceneTouchPoint * p )
{
	Q_UNUSED(p);
	qDebug() << "pressed rigid widget";

	Q_ASSERT(_original_pos == NULL);
	_original_pos = new QPointF(_pos);

	logger() << "rwc_press" << this << p->id() << pos();
	//if(_after_glow_bubble != NULL)
	//{
	//	delete _after_glow_bubble;
	//	_after_glow_bubble = NULL;
	//
}

// --------------------------------------------------------
bool RigidWidgetController::acceptTouchPoint( const SceneTouchPoint & p ) const
{
	//if(!p.isTransformed())
	//	return false;
//qDebug() << "rw accepting?";
	if(!isMovable())
		return false;

	// for now, as we are only moving
	if(numPressedTouchPoints() >= 1)
		return false;

	if((_selection_mode==IndirectPointSelection || _selection_mode==IndirectBubbleSelection) && !_after_glow_bubble)
		return false;

	// accept if object or halo hit by area cursor
	float touch_radius = _selection_mode==DirectAreaSelection || _selection_mode==IndirectBubbleSelection ? p.radius() : 0;
	if(QLineF(p.pos(), this->_pos).length() < totalRadius() + touch_radius)
	{
		//qDebug() << "rw accepting!";
		return true;
	}

	// accept if afterglow bubble is hit by area cursor
	if(_after_glow_bubble != NULL && _after_glow_bubble->hasBubble())
	{
		return _after_glow_bubble->intersectsCircle(p.pos(), p.radius());
	}

	return false;
}

// --------------------------------------------------------
bool RigidWidgetController::forwardTouchPoint( const SceneTouchPoint & p ) const
{
	Q_UNUSED(p);
	return false;
}

// --------------------------------------------------------
//bool RigidWidgetController::needsHalo( const TouchPoint & p ) const
//{
//	return isMovable() && QLineF(p.pos(), this->_pos).length() < radius()*3 + p.rect().width()/2;
//}

// --------------------------------------------------------
void RigidWidgetController::setTouchPointHaloVisible( bool visible, const SceneTouchPoint * stp )
{
	_touch_point_halo_visible = visible;

	// if the halo is just about to disappear, check if this is because of a release event.
	// in this case, we want to start the afterglow halo bubble.
	if(!visible && /*stp->closestRigidWidgets().front() == this*/stp->state()==Qt::TouchPointReleased)
	{
		if(this->_after_glow_bubble != NULL)
		{
			delete this->_after_glow_bubble;
			this->_after_glow_bubble = NULL;
		}
		if(_selection_mode == IndirectBubbleSelection)
		{
			this->_after_glow_bubble = new AfterGlowBubble(&stp->bubbleRect());
		}
		else
		{
			this->_after_glow_bubble = new AfterGlowBubble(NULL);
		}
		//this->_after_glow_halo_visible = true;
		if(!TimeSingleton::instance()->isSubscribed(this))
		{
			TimeSingleton::instance()->subscribe(this);
		}
	}
}

// --------------------------------------------------------
float RigidWidgetController::intersectingDistance( const QPointF & p )
{
	return QLineF(pos(), p).length() - coreRadius();
}

// --------------------------------------------------------
float RigidWidgetController::containmentDistance( const QPointF & p )
{
	return QLineF(pos(), p).length() + coreRadius();
}

// --------------------------------------------------------
RigidWidgetController::~RigidWidgetController()
{
	if(_original_pos != NULL) delete _original_pos;
	if(_after_glow_bubble != NULL) delete _after_glow_bubble;
	if(TimeSingleton::instance()->isSubscribed(this))
	{
		TimeSingleton::instance()->unsubscribe(this);
	}
}

// --------------------------------------------------------
bool RigidWidgetController::finished() const
{
	return TouchWidgetController::finished() && _after_glow_bubble == NULL;
}

// --------------------------------------------------------
void RigidWidgetController::timeStep()
{
	TouchWidgetController::timeStep();
	if(_after_glow_bubble != NULL && !isMoving() && _after_glow_bubble->expired())
	{
		delete _after_glow_bubble;
		_after_glow_bubble = NULL;
		//_after_glow_halo_visible = false;
	}
}

// --------------------------------------------------------
bool RigidWidgetController::isHaloVisible() const
{
	return _touch_point_halo_visible || _after_glow_bubble != NULL;
}

// --------------------------------------------------------
bool RigidWidgetController::isMoving() const
{
	return numPressedTouchPoints() > 0 || _attached_to_handle_widget;
}

// --------------------------------------------------------
void RigidWidgetController::setAttachedToHandleWidget( bool attached )
{
	Q_ASSERT(_attached_to_handle_widget != attached);
	_attached_to_handle_widget = attached;
}

// --------------------------------------------------------
const AfterGlowBubble * RigidWidgetController::afterGlowBubble() const
{
	return isMoving() ? NULL : _after_glow_bubble;
}