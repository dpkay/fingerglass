#include "shiftpluswidgetcontroller.h"
#include "variousattachments.h"
#include "rigidwidgetcontroller.h"
#include "globallog.h"

// --------------------------------------------------------
ShiftPlusWidgetController::ShiftPlusWidgetController(
	const SettingContainer & tool_setting_container
) :
	SingleHandedPrecisionWidgetController(tool_setting_container)
{
	this->_mgm = NULL;	
	this->_relative_resizer = NULL;
	this->_transform = NULL;
	this->_update_timer = new QTimer(this);
	connect(this->_update_timer, SIGNAL(timeout()), this, SLOT(updateTimeout()));
	_update_timer->start();
}

// --------------------------------------------------------
ShiftPlusWidgetController::~ShiftPlusWidgetController()
{
	if(this->_mgm != NULL) delete _mgm;
	delete _transform;
	if(_relative_resizer != NULL)
	{
		delete _relative_resizer;
	}	
}

// --------------------------------------------------------
bool ShiftPlusWidgetController::acceptTouchPoint( const SceneTouchPoint & p ) const
{
	if(!hasDefiningTouch())
	{
		Q_ASSERT(_mgm != NULL);
		return QLineF(p.pos(), gripPoint()).length() < toolSettingContainer().floatProperty("dst_radius"); // HACK
	}
	else
	{
		return additionalPoints().size() < 2;
	}
}

// --------------------------------------------------------
void ShiftPlusWidgetController::initializeMagnifyingGlass( SceneTouchPoint * stp )
{
	_mgm = new MagnifyingGlassManager(stp->pos(),
		toolSettingContainer().floatProperty("ratio"),
		toolSettingContainer().floatProperty("aspect_ratio"),
		toolSettingContainer().floatProperty("angle"));
	_mgm->singleMagnifyingGlass()->setDstRadiusD(toolSettingContainer().floatProperty("dst_radius"));
	_mgm->singleMagnifyingGlass()->setDstRadius(toolSettingContainer().floatProperty("dst_radius"));
	_mgm->baseGlass()->setDstRadius(toolSettingContainer().floatProperty("distance"));
	_mgm->setBaseGlassVisible(false);
	_mgm->setSrcRadius(toolSettingContainer().floatProperty("dst_radius")/toolSettingContainer().floatProperty("ratio"));
	_mgm->singleMagnifyingGlass()->setSpeed(1.5f);

	qDebug() << "ratio is" << ratio();
	// tmp
	//_mgm->singleMagnifyingGlass()->setDstRadius(toolSettingContainer().floatProperty("dst_radius"));
}

// --------------------------------------------------------
const SceneTouchPoint * ShiftPlusWidgetController::resizingTouch() const
{
	if(additionalPoints().size() == 1)
	{ 
		return additionalPoints().front();
	}
	else
	{
		return NULL;
	}
}

// --------------------------------------------------------
void ShiftPlusWidgetController::updateTimeout()
{
	if(_request_update)
	{
		_mgm->updateDstPositions();
		//qDebug() << "update dst pos";
		_request_update = false;
	}
}

// --------------------------------------------------------
void ShiftPlusWidgetController::updateAndTransformPoints()
{
	Q_ASSERT(definingTouch() != NULL);

	// for convenience
	SceneTouchPoint * p = definingTouch()->stp;

	// update grip/tip point and touchpoint
	setGripPoint(p->pos());
	if(definingTouch()->transformed_start_pos == NULL)
	{
		// in the very beginning
		p->transform(map(p->pos()), map(p->startPos()), p->rect().size());
	}
	else
	{
		// after we know the definite tip position (adjusting to object location)
		p->transform(map(p->pos()), *definingTouch()->transformed_start_pos, p->rect().size());
	}
	setTipPoint(p->pos());
	_request_update = true;
}

// --------------------------------------------------------
void ShiftPlusWidgetController::resizeGlass()
{
	float desired_ratio = desiredRelativeChangingValue(_relative_resizer, 2.0f, toolSettingContainer().floatProperty("zoom_speed"));
	desired_ratio = qMax(1.0f, desired_ratio);
	desired_ratio = qMin(8.0f, desired_ratio);
	_mgm->setSrcRadius(_mgm->singleMagnifyingGlass()->dstRadius()/desired_ratio);
}

// --------------------------------------------------------
void ShiftPlusWidgetController::updateAfterMovement()
{
	updateDesiredAngle();
}

// --------------------------------------------------------
void ShiftPlusWidgetController::updateToolMode()
{
	logger() << "shpw_updatemode" << this << additionalPoints().size();

	// update relative resizer
	if(_relative_resizer != NULL)
	{
		delete _relative_resizer;
		_relative_resizer = NULL;
	}
	_relative_resizer = new RelativeValueChanger<float>(ratio(), definingTouch());


	// update transform
	if(_transform != NULL) delete _transform;
	if(additionalPoints().size() == 0 && ratio() != 1.0f)
	{
		_transform = createScalingTransform();
	}
	else if(additionalPoints().size() == 2 || (additionalPoints().size() == 0 && ratio() == 1.0f))
	{
		_transform = createParallelTransform();
	}
	else if(additionalPoints().size() == 1)
	{
		_transform = createSetupTransform();
		_relative_resizer = new RelativeValueChanger<float>(ratio(), definingTouch());
	}
	else
	{
		Q_ASSERT(false);
	}
}

// --------------------------------------------------------
const QSizeF & ShiftPlusWidgetController::gripSize() const
{
	static const QSizeF result = QSizeF(toolSettingContainer().floatProperty("distance"), toolSettingContainer().floatProperty("distance"))*2;
	return result;
	//return QSizeF();
}

// --------------------------------------------------------
void ShiftPlusWidgetController::startClosingAnimation( RigidWidgetController * rwc )
{
	MagnifyingGlass * mg = new MagnifyingGlass(*_mgm->singleMagnifyingGlass());
	mg->setSpeed(0.3f);
	mg->setDstCenter(rwc->pos());
	mg->setDstRadius(rwc->coreRadius());
	rwc->attach(new InteractionFinishedAttachment(rwc, mg));
}

// --------------------------------------------------------
void ShiftPlusWidgetController::handleNewlyAttachedWidget( RigidWidgetController * rwc )
{
	Q_UNUSED(rwc);
	MagnifyingGlass * mg = _mgm->singleMagnifyingGlass();
	mg->setSrcRadiusByRatio(1.0f);
}

// --------------------------------------------------------
void ShiftPlusWidgetController::updateDesiredAngle()
{
	Q_ASSERT(definingTouch() != NULL);
	const float x_threshold = 0.10f;
	const float y_threshold = 0.35f;
	const float transition = 0.025f;

	float angle_in = _mgm->desiredAngle();
	float eps = 0.001f;

	bool cross_top = fabsf(angle_in+90)<=45+eps;
	bool cross_left = fabsf(angle_in+22.5)<=22.5+eps;
	bool cross_right = fabsf(angle_in+135)<=eps || fabsf(angle_in-180)<=eps;
	if(definingTouch()->stp->pos().y()<y_threshold-transition)
	{
		cross_top = true;
	}
	if(definingTouch()->stp->pos().y()>y_threshold+transition)
	{
		cross_top = false;
	}
	if(definingTouch()->stp->pos().x()<x_threshold-transition)
	{
		cross_left = true;	
	}
	if(definingTouch()->stp->pos().x()>x_threshold+transition)
	{
		cross_left = false;
	}
	if(definingTouch()->stp->pos().x()>1-x_threshold-transition)
	{
		cross_right = true;
	}
	if(definingTouch()->stp->pos().x()<1-x_threshold+transition)
	{
		cross_right = false;
	}
	Q_ASSERT(!cross_left || !cross_right);

	float angle_out;
	if(cross_left && !cross_top) angle_out = 0;
	else if(cross_left && cross_top) angle_out = -45;
	else if(!cross_left && !cross_right && cross_top) angle_out = -90;
	else if(!cross_left && !cross_right && !cross_top) angle_out = 90;
	else if(cross_right && !cross_top) angle_out = 180;
	else if(cross_right && cross_top) angle_out = -135;
	else Q_ASSERT(false);

	_mgm->setDesiredAngle(angle_out);
}

// --------------------------------------------------------
void ShiftPlusWidgetController::test( SceneTouchPoint * p)
{
	p->setVisiblityInMagnification(SceneTouchPoint::NeverVisible);
}

// --------------------------------------------------------
QPointF ShiftPlusWidgetController::map( const QPointF & p ) const
{
	Q_ASSERT(_transform != NULL);
	QPointF q = _transform->map(p);
	if(q.x()>1.0f) q.setX(1.0f);
	if(q.x()<0.0f) q.setX(0.0f);
	if(q.y()>1-_mgm->sceneMargin()) q.setY(1-_mgm->sceneMargin());
	if(q.y()<_mgm->sceneMargin()) q.setY(_mgm->sceneMargin());
	qDebug() << p.x() << q.x();
	return q;
}