#include "scenetouchpoint.h"
#include "rigidwidgetcontroller.h"
#include "globallog.h"
#include <QtDebug>

// --------------------------------------------------------
PointWidgetConnection * SceneTouchPoint::connectToWidget( TouchWidgetController * twc, bool mutual )
{
//	qDebug() << "pushing owner " << owner;
	PointWidgetConnection * pwc = twc->touchPointConnection(this);
	if(pwc == NULL)
	{
		pwc = new PointWidgetConnection(twc, this);
	}
	_widget_connections << pwc;

	Q_ASSERT(widgetConnection(twc) != NULL);

	if(mutual)
	{
		twc->connectToTouchPoint(this, false);
	}
	Q_ASSERT(twc->touchPointConnection(this) == pwc);

	return pwc;
}

// --------------------------------------------------------
void SceneTouchPoint::disconnectFromWidget( TouchWidgetController * twc, bool mutual )
{
	PointWidgetConnection * pwc;
	QList<PointWidgetConnection *>::iterator it = _widget_connections.begin();
	while(it != _widget_connections.end())
	{
		if((*it)->widgetController() == twc)
		{
			pwc = *it;
			it = _widget_connections.erase(it);
		}
		else
		{
			++it;
		}
	}

	if(mutual)
	{
		twc->disconnectFromTouchPoint(this, false);
		delete pwc;
	}
	//_owners.removeOne(owner);
}

// --------------------------------------------------------
SceneTouchPoint::SceneTouchPoint(
	const QTouchEvent::TouchPoint & p, QSizeF parent_size)
{
	Q_ASSERT(p.startPos() != QPointF());
	//qDebug() << "CON START POS" << p.startPos();
	//this->_original_point = p;
	//this->_transformed_point = p;
	//this->_original_point = p;
	_parent_size = parent_size;
	setPoint(p);
	this->_is_transformed = false;
	//if(owner != NULL)
	//{
	//	pushOwner(owner);
	//}
	//this->_owner = owner;
	_age.start();
	this->_visibility_in_magnification = AlwaysVisible;
	this->_restarted = false;
	this->_visible_in_scene = true;

	_old_start_pos = _original_point.startPos();

}

// --------------------------------------------------------
int SceneTouchPoint::age() const
{
	return _age.elapsed();
}

// --------------------------------------------------------
bool SceneTouchPoint::isYoung() const
{
	return age() < 200;
}

// --------------------------------------------------------
const QList<PointWidgetConnection *> & SceneTouchPoint::widgetConnections() const
{
	return _widget_connections;
}

// --------------------------------------------------------
void SceneTouchPoint::setPoint( const QTouchEvent::TouchPoint & point )
{
	Q_ASSERT(point.startPos() != QPointF());
	//qDebug() << "SET START POS" << point.startPos();
	//this->_transformed_point = point;
	//qDebug() << point.pos() << point.rect();
	this->_original_point = point;
	//if(this->_original_point.rect().size().isNull())
	{
		this->_original_point.setRect(QRectF(0,0,0.02,0.02));
	}
	this->_original_point.setPos(normalizedToParent(point.pos()));
	if(!_restarted)
	{
		this->_original_point.setStartPos(normalizedToParent(point.startPos()));
	}
	else
	{
		this->_original_point.setStartPos(_old_start_pos);
	}
	this->_transformed_point = this->_original_point;
}

// --------------------------------------------------------
void SceneTouchPoint::restart()
{
	_transformed_point.setStartPos(_transformed_point.pos());
	_original_point.setStartPos(_original_point.pos());
	
	_old_start_pos = _original_point.startPos();
	_restarted = true;
	logger() << "stp_restart" << this->id();
}

// --------------------------------------------------------
void SceneTouchPoint::untransform()
{
	_transformed_point = _original_point;
	_is_transformed = false;

	// list of closest widgets will not be up to date anymore now
	_closest_rigid_widgets.clear();
	logger() << "stp_untransform" << this << id();
}

// --------------------------------------------------------
int SceneTouchPoint::id() const
{
	//Q_ASSERT(_original_point.id() == _transformed_point.id());
	//return _original_point.id();
	return _transformed_point.id();
}

// --------------------------------------------------------
Qt::TouchPointState SceneTouchPoint::state() const
{
	//int s1 = _original_point.state();
	//int s2 = _transformed_point.state();
	//qDebug() << _original_point.pos() << _transformed_point.pos();
	//Q_ASSERT(_original_point.state() == _transformed_point.state());
	//return _original_point.state();
	return _transformed_point.state();
}

//// --------------------------------------------------------
//QTouchEvent::TouchPoint * SceneTouchPoint::point()
//{
//	return &_transformed_point;
//}
//
//// --------------------------------------------------------
//const QTouchEvent::TouchPoint * SceneTouchPoint::point() const
//{
//	return &_transformed_point;
//}

// --------------------------------------------------------
bool SceneTouchPoint::isFree() const
{
	foreach(PointWidgetConnection * pwc, _widget_connections)
	{
		TouchWidgetController * twc = pwc->widgetController();
		if(pwc->isDragging() && !twc->forwardTouchPoint(*this))
		{
			return false;
		}
	}
	return true;
}

// --------------------------------------------------------
PointWidgetConnection * SceneTouchPoint::widgetConnection( TouchWidgetController * widget_controller )
{
	foreach(PointWidgetConnection * pwc, _widget_connections)
	{
		if(pwc->widgetController() == widget_controller)
		{
			return pwc;
		}
	}
	return NULL;
}

// --------------------------------------------------------
int SceneTouchPoint::numPressedWidgets() const
{
	int count = 0;
	foreach(PointWidgetConnection * pwc, _widget_connections)
	{
		if(pwc->isDragging())
		{
			++count;
		}
	}
	return count;
}

// --------------------------------------------------------
QList<TouchWidgetController *> SceneTouchPoint::pressedTouchWidgetControllers()
{
	QList<TouchWidgetController *> result;
	foreach(PointWidgetConnection * pwc, _widget_connections)
	{
		if(pwc->isDragging())
		{
			result << pwc->widgetController();
		}
	}
	return result;
}

// --------------------------------------------------------
const QList<RigidWidgetController *> & SceneTouchPoint::closestRigidWidgets() const
{
	return _closest_rigid_widgets;
}

// --------------------------------------------------------
QList<RigidWidgetController *> & SceneTouchPoint::closestRigidWidgets()
{
	return _closest_rigid_widgets;
}

// --------------------------------------------------------
QRectF SceneTouchPoint::bubbleRect() const
{
	//const QList<RigidWidgetController *> & closest_widgets = closestRigidWidgets();
	if(_closest_rigid_widgets.size()>=2)
	{
		float con_d_i = _closest_rigid_widgets[0]->containmentDistance(_transformed_point.pos());
		float int_d_j = _closest_rigid_widgets[1]->intersectingDistance(_transformed_point.pos());
		float radius = qMin(con_d_i, int_d_j);
		return QRectF(_transformed_point.pos().x()-radius, _transformed_point.pos().y()-radius, 2*radius, 2*radius);
	}
	else
	{
		return QRectF();
	}
}

// --------------------------------------------------------
const float SceneTouchPoint::radius() const
{
	//qDebug() << _transformed_point.rect().width() << _transformed_point.rect().height();
	Q_ASSERT(fabs(_transformed_point.rect().width()-_transformed_point.rect().height())<=0.0001f);
	return _transformed_point.rect().width()/2;
}

// --------------------------------------------------------
void SceneTouchPoint::transform( const QPointF & pos, const QPointF & start_pos, const QSizeF & size )
{
	_is_transformed = true;
	_transformed_point.setRect(QRectF(QPointF(), size)); // gets retranslated anyway by setpos
	_transformed_point.setPos(pos);
	_transformed_point.setStartPos(start_pos);
	//logger() << "stp_utransform" << this << id();
	logger() << "stp_transform" << this << id() << pos << size.width();
}

// --------------------------------------------------------
QPointF SceneTouchPoint::normalizedToParent( const QPointF & p )
{
	// rescale
	float aspect_ratio = (float)_parent_size.width()/_parent_size.height();
	float margin = (1.0f-(1.0f/aspect_ratio))/2.0f;
	QPointF result(p.x()/_parent_size.width(), margin+p.y()/_parent_size.height()*(1-2*margin));

	if(result.x()<0) result.setX(0.0f);
	if(result.x()>1) result.setX(1.0f);
	if(result.y()<margin) result.setY(margin);
	if(result.y()>1-margin) result.setY(1-margin);

	return result;
}

// --------------------------------------------------------
void SceneTouchPoint::transform( const QTransform & transform_ )
{
	QSizeF new_size = transform_.mapRect(_transformed_point.rect()).size();
	this->transform(transform_.map(pos()), transform_.map(startPos()), new_size);
}

// --------------------------------------------------------
PointWidgetConnection * SceneTouchPoint::bottomWidgetConnection()
{
	if(_widget_connections.empty())
	{
		return NULL;
	}
	return _widget_connections.last();
}

// --------------------------------------------------------
const PointWidgetConnection * SceneTouchPoint::bottomWidgetConnection() const
{
	if(_widget_connections.empty())
	{
		return NULL;
	}
	return _widget_connections.last();
}

// --------------------------------------------------------
void SceneTouchPoint::pushToFront( PointWidgetConnection * pwc )
{
	Q_ASSERT(_widget_connections.contains(pwc));
	_widget_connections.removeOne(pwc);
	Q_ASSERT(!_widget_connections.contains(pwc));
	_widget_connections.push_front(pwc);
}

// --------------------------------------------------------
bool SceneTouchPoint::hasWidgetConnection( const TouchWidgetController * twc ) const
{
	foreach(const PointWidgetConnection * pwc, _widget_connections)
	{
		if(pwc->widgetController() == twc)
		{
			return true;
		}
	}
	return false;
}