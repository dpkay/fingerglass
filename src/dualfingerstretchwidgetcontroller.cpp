#include "dualfingerstretchwidgetcontroller.h"
#include "rigidwidgetcontroller.h"
#include "globallog.h"
#include "variousattachments.h"

// --------------------------------------------------------
DualFingerStretchWidgetController::DualFingerStretchWidgetController(
	const SettingContainer & tool_setting_container
) :
	TouchWidgetController(MagnifyingWidgetType),
	_tool_setting_container(tool_setting_container)
{ 
	this->_mg = NULL;
	this->_defining_point = NULL;
	this->_resizing_point = NULL;
	this->_attached_rwc_start_pos = NULL;
	logger() << "dfs_open" << this;
}

// --------------------------------------------------------
DualFingerStretchWidgetController::~DualFingerStretchWidgetController()
{
	if(this->_mg != NULL)
	{
		delete this->_mg;
	}
	if(_attached_rwc_start_pos != NULL)
	{
		delete _attached_rwc_start_pos;
	}
}

// --------------------------------------------------------
void DualFingerStretchWidgetController::touchPointPressed( SceneTouchPoint * p )
{
	//Q_ASSERT((_release_time!=NULL) == TimeSingleton::instance()->isSubscribed(this));
	Q_ASSERT(debug_closingTimerConsistent());
	if(_defining_point != NULL)
	{
		if(p == _defining_point->stp)
		{
			// this has to be the first touch event ever, just after creating the widget
			if(_mg == NULL)
			{
				// case (1)
				this->_mg = new MagnifyingGlass(
					1.0f, // roundness
					_tool_setting_container.floatProperty("radius"),
					1.0f, // ratio
					QPointF(p->pos())
				);
			}
		}
		else
		{
			// additional finger
			//if(QLineF(p->pos(), _mg->dstCenterT()).length() < _mg->dstRadiusT())
			//{
			//	//_mg->setSrcRadius(_tool_setting_container.floatProperty("radius"));
			//	qDebug() << "resetting";
			//	_mg->setDstRadius(_tool_setting_container.floatProperty("radius"));
			//	//_mg->setDstCenter(_mg->srcCenterT());
			//	//_mg->setSrcCenter(p->untransformedPos());
			//	//_mg->setDstCenter(p->untransformedPos());
			//}
			//else
			{
				Q_ASSERT(_resizing_point == NULL);
				_resizing_point = p;
				p->setVisiblityInMagnification(SceneTouchPoint::NeverVisible);
			}
		}
	}
	else
	{
		// re-catch
		Q_ASSERT(isClosingTimerRunning());
		Q_ASSERT(QLineF(p->pos(), _mg->dstCenterT()).length() < _tool_setting_container.floatProperty("radius"));
		_defining_point = new DeprecatedDefiningPoint(p, _mg);
		//_mg->setDstRadius(_tool_setting_container.floatProperty("radius"));
 
		// stop timer
		stopClosingTimer();
	}
}


// --------------------------------------------------------
void DualFingerStretchWidgetController::touchPointMoved( SceneTouchPoint * p )
{
	Q_ASSERT(debug_closingTimerConsistent());
	if(this->_defining_point != NULL && p == this->_defining_point->stp)
	{
		_mg->setDstCenter(_defining_point->stp->pos());
		_defining_point->transform(_mg->ratio());
		_mg->setSrcCenter(_defining_point->stp->pos());

		// try setting up the attached rwc
		if(_attached_rwc_start_pos == NULL)
		{
			RigidWidgetController * rwc = attachedRigidWidgetController();
			if(rwc != NULL)
			{
				_attached_rwc_start_pos = new QPointF(rwc->pos());
			}
		}
	}
	else if(p == this->_resizing_point)
	{
		// TODO, change radius
		Q_ASSERT(_mg != NULL);
		qreal new_radius = QLineF(_mg->dstCenterT(), _resizing_point->pos()).length();
		_mg->setDstRadius(qMax(new_radius, _mg->srcRadiusT()));
	}
	else
	{
		Q_ASSERT(false);
	}
}

// --------------------------------------------------------
void DualFingerStretchWidgetController::touchPointReleased( SceneTouchPoint * p )
{
	Q_ASSERT(debug_closingTimerConsistent());
	if(_defining_point != NULL && p == _defining_point->stp)
	{
		_defining_point->transform(_mg->ratio());

		PointWidgetConnection * pwc = _defining_point->stp->bottomWidgetConnection();
		RigidWidgetController * rwc = dynamic_cast<RigidWidgetController *>(pwc->widgetController());
		delete _defining_point;
		_defining_point = NULL;
		if(rwc != NULL && pwc->isDragging())
		{
			MagnifyingGlass * mg = new MagnifyingGlass(*_mg);
			mg->setDstCenter(rwc->pos());
			mg->setDstRadius(rwc->coreRadius());
			mg->setSpeed(0.3f);
			rwc->attach(new InteractionFinishedAttachment(rwc, mg));
			close();
		}
		else
		{
			startClosingTimer(_tool_setting_container.intProperty("release_delay"));
		}

	}
	else
	{
		Q_ASSERT(p == _resizing_point);
		_resizing_point = NULL;
		if(_defining_point != NULL)
		{
			_mg->setDstRadius(_tool_setting_container.floatProperty("radius"));
			_defining_point->reset();
			//_defining_point->setStartPos(_defining_point->stp->untransformedPos() );
			RigidWidgetController * rwc = attachedRigidWidgetController();
			if(rwc != NULL)
			{
				Q_ASSERT(_attached_rwc_start_pos != NULL);
				//if(_attached_rwc_start_pos == NULL)
				//{
				//	//_attached_rwc_start_pos = new QPointF(rwc->pos());
				//	//_attached_rwc_start_pos = new QPointF(_defining_point->stp->untransformedPos());//new QPointF(rwc->pos());
				//}
				_defining_point->setStartPos(*_attached_rwc_start_pos);
				//_mg->setDstCenter(_defining_point->stp->pos());
				//_defining_point->transform(_mg->ratio());
				//_mg->setSrcCenter(_defining_point->stp->pos());
			}
		}
	}
}

// --------------------------------------------------------
bool DualFingerStretchWidgetController::acceptTouchPoint( const SceneTouchPoint & p ) const
{

	if(_defining_point == NULL)
	{
		Q_ASSERT(_mg != NULL);
		return QLineF(p.pos(), _mg->dstCenterT()).length() < _tool_setting_container.floatProperty("radius");
	}
	else
	{
		return _resizing_point == NULL;
	}
}

// --------------------------------------------------------
bool DualFingerStretchWidgetController::forwardTouchPoint( const SceneTouchPoint & p ) const
{
	return &p != _resizing_point;
}

// --------------------------------------------------------
void DualFingerStretchWidgetController::setDefiningTouch( SceneTouchPoint * stp )
{
	if(_defining_point != NULL)
	{
		delete _defining_point;
	}
	_defining_point = new DeprecatedDefiningPoint(stp);
}

// --------------------------------------------------------
RigidWidgetController * DualFingerStretchWidgetController::attachedRigidWidgetController()
{
	PointWidgetConnection * pwc = _defining_point->stp->bottomWidgetConnection();
	if(pwc == NULL)
		return NULL;
	if(!pwc->isDragging())
		return NULL;
	return dynamic_cast<RigidWidgetController *>(pwc->widgetController());
}