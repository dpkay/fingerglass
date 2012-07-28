#include "handlewidgetcontroller.h"
#include <QLineF>
#include "variousattachments.h"
#include "globallog.h"

// --------------------------------------------------------
HandleWidgetController::HandleWidgetController( RigidWidgetController * attached_widget,
											   const SettingContainer & tool_setting_container,
											   const MagnifyingGlass & mg
) :
	TouchWidgetController(HandleWidgetType),
	_tool_setting_container(tool_setting_container),
	timer(this)
{
	_attached_widget = attached_widget;
	_attached_widget->setAttachedToHandleWidget(true);
	//_attached_widget_original_pos = new QPointF(_attached_widget->pos());
	_absolute_handle = new AbsoluteHandle(mg.srcCenterT(), mg.srcRadiusT());
	_absolute_handle->setPos(attached_widget->pos());
	_absolute_handle->createOriginalPos();
	_clutch_handle = NULL;
	_absolute_touch = NULL;
	_absolute_secondary_touch = NULL;
	_clutch_touch = NULL;
	_clutch_resizing_touch_point = NULL;
	timer.start();
	connect(&timer, SIGNAL(timeout()), this, SLOT(timeStep()));
	_virtual_second_touch = NULL;
	_lock_touch = NULL;

	logger() << "hwc_open" << this << mg.dstCenterT() << mg.dstRadiusT() << mg.srcCenterT() << mg.srcRadiusT();
}

// --------------------------------------------------------
void HandleWidgetController::createClutchHandle(
	const QPointF & pos,
	const MagnifyingGlass * clutch_mg )
{
	Q_ASSERT(_clutch_handle == NULL);
	_clutch_handle = new ClutchHandle(pos, clutch_mg);
	//_absolute_handle->setRadius(_clutch_handle->magnifyingGlass()->srcRadiusT());
}

// --------------------------------------------------------
bool HandleWidgetController::acceptAsResizer( const SceneTouchPoint & p ) const
{
	return false;


	float length = QLineF(_clutch_handle->magnifyingGlass()->dstCenter(), p.pos()).length();
	return length/_clutch_handle->magnifyingGlass()->dstRadius() < 1.2f;
}

// --------------------------------------------------------
bool HandleWidgetController::acceptAsAbsoluteTouch( const SceneTouchPoint & p ) const
{
	return true;

	if(showHandle())
	{
		float length = QLineF(_clutch_handle->magnifyingGlass()->dstCenter(), p.pos()).length();
		return length/_clutch_handle->magnifyingGlass()->dstRadius() > 1.2f;	
	}
	else
	{
		return true;
	}
}

// --------------------------------------------------------
bool HandleWidgetController::forwardTouchPoint( const SceneTouchPoint & p ) const
{
	Q_UNUSED(p);
	return false;
}

// --------------------------------------------------------
void HandleWidgetController::touchPointPressed( SceneTouchPoint * p )
{
	// change tool mode
	AbsoluteHandleMode old_mode = absoluteHandleMode();
	allocateTouch(p);
	if(old_mode != absoluteHandleMode())
	{
		setupNewAbsoluteHandleMode();
	}

	// reset state machine
	resetTouchesAndWidgets();

	if(absoluteHandleMode() == TwoFingerHandle)
	{
		emit createMagnifyingWidget();
		logger() << "hwc_close" << this << _clutch_handle->posT() << _clutch_handle->radiusT() << _absolute_handle->posT() << _absolute_handle->radiusT();
		close();
	}
}

// --------------------------------------------------------
void HandleWidgetController::touchPointReleased( SceneTouchPoint * p )
{
	// change tool mode
	AbsoluteHandleMode old_mode = absoluteHandleMode();
	deallocateTouch(p);
	if(old_mode != absoluteHandleMode())
	{
		setupNewAbsoluteHandleMode();
	}

	// reset state machine
	resetTouchesAndWidgets();

	// handle closes
	if(absoluteHandleMode() == NoAbsoluteHandle && _clutch_touch == NULL)
	{
		startClosing();
	}
	else
	// handle transition
	if(absoluteHandleMode() == TwoFingerHandle && _clutch_touch == NULL)
	{
		_attached_widget->attach(new InteractionFinishedAttachment(_attached_widget, NULL));
		emit createMagnifyingWidget();

		logger() << "hwc_close" << this << _clutch_handle->posT() << _clutch_handle->radiusT() << _absolute_handle->posT() << _absolute_handle->radiusT();
		close();
	}
}


// --------------------------------------------------------
bool HandleWidgetController::acceptTouchPoint( const SceneTouchPoint & p ) const
{
	Q_UNUSED(p);
	Q_ASSERT(_clutch_touch != NULL || _absolute_touch != NULL);

	if(_clutch_touch == NULL)
		return true; // accept as clutch

	if(showHandle())
	{
		if(_clutch_resizing_touch_point == NULL && acceptAsResizer(p))
			return true; // accept as clutch resizer
	}

	// at this point, the new touch could just be a new absolute handle
	if(absoluteHandleMode() == TwoFingerHandle)
	{
		return acceptAsLock(p);
	}

	return acceptAsAbsoluteTouch(p);
	//return true;

	//return false;
}

// --------------------------------------------------------
void HandleWidgetController::timeStep()
{
	// preconditions
	Q_ASSERT(_attached_widget != NULL);
	Q_ASSERT(_absolute_handle != NULL);
	Q_ASSERT(_clutch_handle != NULL);

	if(!isLocked())
	{
		updateAbsoluteHandle();
		updateClutchHandle();
	}

	updateAttachedWidget();

	logger() << "hwc_update" << this << _clutch_handle->posT() << _clutch_handle->radiusT() << _absolute_handle->posT() << _absolute_handle->radiusT();
}


// --------------------------------------------------------
void HandleWidgetController::updateAttachedWidget()
{
	// unless TwoFinger, update widget position from absolute handle
	switch(absoluteHandleMode())
	{
	case OneFingerHandle:
		_attached_widget->setPos(_absolute_handle->pos());
		break;
	case TwoFingerHandle:
	case VirtualTwoFingerHandle:
	case NoAbsoluteHandle:
		if(_clutch_touch != NULL)
		{
			QPointF dest_pos = _clutch_handle->magnifyingGlass()->transform(_clutch_touch->pos());

			// check if it would be out of glass (in twofinger case)
			QLineF line(_absolute_handle->posT(), dest_pos);
			float max_radius = _absolute_handle->radiusT() - _attached_widget->coreRadius();
			if(line.length() > max_radius)
			{
				dest_pos = line.unitVector().pointAt(max_radius);
			}
			_attached_widget->setPos(dest_pos);
		}
		break;
	default:
		Q_ASSERT(false);
	}
}


// --------------------------------------------------------
void HandleWidgetController::updateAbsoluteHandle()
{
	if(absoluteHandleMode() == TwoFingerHandle)
	{
		// update from mean of 2 absolute touch positions
		Q_ASSERT(_absolute_secondary_touch != NULL);
		QPointF pos = (_absolute_touch->pos() + _absolute_secondary_touch->pos())/2;
		_absolute_handle->setPos(pos);
		//qDebug() << "new absolute handle pos (twofinger)" << pos;

		float radius = QLineF(_absolute_touch->pos(), _absolute_secondary_touch->pos()).length()/2;
		_absolute_handle->setRadius(radius);
	}
	else
	{
		// update from deltas of 0-1 absolute and 0-1 clutch touch
		QPointF pos = _absolute_handle->originalPos();
		if(_absolute_touch != NULL)
		{
			pos += cdScaledTouchDelta(_absolute_touch);
		}
		if(_clutch_touch != NULL)
		{
			pos += cdScaledTouchDelta(_clutch_touch);
		}
		_absolute_handle->setPos(pos);
		_absolute_handle->setRadius(_clutch_handle->magnifyingGlass()->srcRadiusT());
	}
}

// --------------------------------------------------------
void HandleWidgetController::updateClutchHandle()
{
	// if TwoFinger, also update clutch radius
	if(absoluteHandleMode() == TwoFingerHandle)
	{
		//float radius = QLineF(_absolute_touch->pos(), _absolute_secondary_touch->pos()).length()/2;
		//_clutch_handle->setSrcRadius(radius);
		_clutch_handle->setSrcRadius(_absolute_handle->radiusT());
	}
	else
	{
		// if clutch not zero, update clutch handle position
		if(_clutch_touch != NULL)
		{
			_clutch_handle->setPos(_clutch_touch->pos());
		}
	}

	if(_clutch_resizing_touch_point != NULL)
	{
		_clutch_handle->setDstRadius(QLineF(_clutch_handle->magnifyingGlass()->dstCenter(), _clutch_resizing_touch_point->pos()).length());
	}

	// update clutch src from absolute handle
	_clutch_handle->setSrcPos(_absolute_handle->posT());
}

// --------------------------------------------------------
void HandleWidgetController::touchPointMoved( SceneTouchPoint * p )
{
	Q_UNUSED(p);
}




// --------------------------------------------------------
const QList<const Handle *> HandleWidgetController::handles() const
{
	QList<const Handle *> list;
	if(_absolute_handle != NULL) list << _absolute_handle;
	if(_clutch_handle != NULL) list << _clutch_handle;
	return list;
}

// --------------------------------------------------------
Handle * HandleWidgetController::handleByTouchId( int id ) const
{
	Q_ASSERT_X((_clutch_touch != NULL && id==_clutch_touch->id()) || (_absolute_touch != NULL && id==_absolute_touch->id()), "handleByTouchId", QString("invalid touch id %1").arg(id).toStdString().c_str());
	if(_clutch_touch != NULL && id==_clutch_touch->id())
		return _clutch_handle;
	else
		return _absolute_handle;
	//return (id==_clutch_touch_id) ? _clutch_handle : _absolute_handle;
	//return (false) ? (_clutch_handle) : (_absolute_handle);
}

// --------------------------------------------------------
HandleWidgetController::~HandleWidgetController()
{
	//delete _attached_widget_original_pos;
	_attached_widget->setAttachedToHandleWidget(false);
	// HACK: something is wrong here
	//delete _clutch_handle;
	//delete _absolute_handle;
	//if(_clutch_resizing_touch_point != NULL)
	//	delete _clutch_resizing_touch_point;
}

// --------------------------------------------------------
QPointF HandleWidgetController::cdScaledTouchDelta( const SceneTouchPoint * p ) const
{
	const Handle * handle = handleByTouchId(p->id());
	QPointF delta = p->pos() - p->startPos();
	//qDebug() << "scaling with cdratio" << handle->cdRatio() << "delta length" << QLineF(QPointF(),delta).length();
	return delta / handle->cdRatio();
}

// --------------------------------------------------------
void HandleWidgetController::correctWidgetPosition()
{
	const SceneTouchPoint * p = touchPointConnections().value(_clutch_touch->id())->point();
	QPointF scaled_delta = cdScaledTouchDelta(touchPointConnections().value(_clutch_touch->id())->point());
	//*_attached_widget_original_pos -= scaled_delta;
	_clutch_handle->setOriginalPos(_clutch_handle->posT() - (p->pos() - p->startPos()));
}

// --------------------------------------------------------
const RigidWidgetController * HandleWidgetController::attachedWidget() const
{
	return _attached_widget;
}

// --------------------------------------------------------
const SceneTouchPoint * HandleWidgetController::clutchResizingTouchPoint() const
{
	return _clutch_resizing_touch_point;
}

// --------------------------------------------------------
const bool HandleWidgetController::showHandle() const
{
	return _tool_setting_container.boolProperty("show_glass");
}

// --------------------------------------------------------
HandleWidgetController::AbsoluteHandleMode HandleWidgetController::absoluteHandleMode() const
{
	if(_absolute_touch == NULL)
	{
		Q_ASSERT(_absolute_secondary_touch == NULL);
		Q_ASSERT(_virtual_second_touch == NULL);
		return NoAbsoluteHandle;
	}
	else
	{
		if(_absolute_secondary_touch == NULL)
		{
			if(_virtual_second_touch == NULL)
			{
				return OneFingerHandle;
			}
			else
			{
				return VirtualTwoFingerHandle;
			}
		}
		else
		{
			Q_ASSERT(_virtual_second_touch == NULL);
			return TwoFingerHandle;
		}
	}
}

// --------------------------------------------------------
void HandleWidgetController::allocateTouch( SceneTouchPoint * p )
{
	// decide where to allocate the point to
	if(_absolute_touch == NULL && _clutch_touch == NULL)
	{
		// this only happens at the very beginning
		_clutch_touch = p;
	}
	else
	{
		if(_clutch_touch == NULL)
		{
			// accept as clutch
			_clutch_touch = p;
		}
		else
		{
			p->setVisiblityInMagnification(SceneTouchPoint::NeverVisible);

			if(showHandle() && _clutch_resizing_touch_point == NULL && acceptAsResizer(*p))
			{
				// accept as resizer
				_clutch_resizing_touch_point = p;
			}
			else
			{
				// at this point, the new touch could just be a new absolute handle
				if(absoluteHandleMode() == TwoFingerHandle && acceptAsLock(*p))
				{
					_lock_touch = p;
					return;
				}
				Q_ASSERT(acceptAsAbsoluteTouch(*p));
				if(_absolute_touch == NULL)
				{
					_absolute_touch = p;
				}
				else
				{
					Q_ASSERT(_absolute_secondary_touch == NULL);
					_absolute_secondary_touch = p;
				}
			}
		}
	}
}

// --------------------------------------------------------
void HandleWidgetController::deallocateTouch( SceneTouchPoint * p )
{
	// handle released events
	if(p == _clutch_resizing_touch_point)
	{
		_clutch_resizing_touch_point = NULL;
	}
	if(p == _absolute_touch)
	{
		_absolute_touch = NULL;
		_absolute_secondary_touch = NULL;
		_lock_touch = NULL;
	}
	if(p == _clutch_touch)
	{
		_clutch_touch = NULL;
	}
	if(p == _absolute_secondary_touch)
	{
		_absolute_secondary_touch = NULL;
		_absolute_touch = NULL;
		_lock_touch = NULL;
	}
	if(p == _lock_touch)
	{
		_lock_touch = NULL;
	}
}

// --------------------------------------------------------
void HandleWidgetController::resetTouchesAndWidgets()
{
	foreach(PointWidgetConnection * pwc, touchPointConnections())
	{
		pwc->point()->restart();
	}
	_absolute_handle->resetOriginalPos();
}

// --------------------------------------------------------
void HandleWidgetController::setupNewAbsoluteHandleMode()
{
	qDebug() << "setting up" << absoluteHandleMode();
	switch(absoluteHandleMode())
	{
	case NoAbsoluteHandle:
		_clutch_handle->setRadiusMode(ClutchHandle::SmallRadius); 
		break;
	case OneFingerHandle:
		_absolute_handle->setPos(_absolute_touch->pos());
		_attached_widget->setPos(_absolute_handle->pos());
		_clutch_handle->setRadiusMode(ClutchHandle::SmallRadius); 
		break;
	case TwoFingerHandle:
		updateAbsoluteHandle();
		updateClutchHandle();
		_attached_widget->setPos(_absolute_handle->posT());
		_clutch_handle->setRadiusMode(ClutchHandle::OriginalRadius);

		// try updating clutch handle
		_clutch_handle->goBackIntoBounds(_tool_setting_container.floatProperty("aspect_ratio"));

		// correct harder stuff if that didnt work
		_clutch_handle->optimizePosition(_tool_setting_container.floatProperty("aspect_ratio"));

		break;
	}
	logger() << "hwc_mode" << this << absoluteHandleMode();
}

// --------------------------------------------------------
void HandleWidgetController::startClosing()
{
	logger() << "hwc_close" << this << _clutch_handle->posT() << _clutch_handle->radiusT() << _absolute_handle->posT() << _absolute_handle->radiusT();
	MagnifyingGlass * mg = new MagnifyingGlass(*_clutch_handle->magnifyingGlass());
	mg->setDstCenter(mg->srcCenterT());
	mg->setDstRadius(_attached_widget->coreRadius());
	mg->setSpeed(0.4f);
	_attached_widget->attach(new InteractionFinishedAttachment(_attached_widget, mg));
	close();
}

// --------------------------------------------------------
bool HandleWidgetController::acceptAsLock( const SceneTouchPoint & p ) const
{
	return true;
}

// --------------------------------------------------------
QPair<SceneTouchPoint *, SceneTouchPoint *> HandleWidgetController::absoluteTouches()
{
	Q_ASSERT(absoluteHandleMode() == TwoFingerHandle);
	return qMakePair(_absolute_touch, _absolute_secondary_touch);	
}