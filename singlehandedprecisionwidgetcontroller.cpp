#include "singlehandedprecisionwidgetcontroller.h"
#include "rigidwidgetcontroller.h"
#include "globallog.h"

// --------------------------------------------------------
SingleHandedPrecisionWidgetController::SingleHandedPrecisionWidgetController(
	const SettingContainer & tool_setting_container
) :
	_tool_setting_container(tool_setting_container),
	TouchWidgetController(MagnifyingWidgetType)
{
	_defining_touch = NULL;
	logger() << "shwc_open" << this;
}

// --------------------------------------------------------
SingleHandedPrecisionWidgetController::~SingleHandedPrecisionWidgetController()
{
	if(_defining_touch != NULL)
	{
		delete _defining_touch;
	}
}

// --------------------------------------------------------
void SingleHandedPrecisionWidgetController::adjustTipToAttachedWidget()
{
	RigidWidgetController * rwc = NULL;
	PointWidgetConnection * pwc = definingTouch()->stp->bottomWidgetConnection();
	qDebug() << "pwc" << pwc;
	if(pwc != NULL && pwc->isDragging())
	{
		rwc = dynamic_cast<RigidWidgetController *>(pwc->widgetController());
	}
	qDebug() << "rwc" << pwc;
	if(rwc != NULL)
	{
		qDebug() << "setting tip point from" << tipPoint() << "to" << rwc->pos();
		setTipPoint(rwc->pos());
		handleNewlyAttachedWidget(rwc);
		qDebug() << "0tipPoint()" << tipPoint();
	}
}

// --------------------------------------------------------
void SingleHandedPrecisionWidgetController::setDefiningTouch( SceneTouchPoint * stp )
{
	qDebug() << "setting defining";
	Q_ASSERT(definingTouch() == NULL);
	createDefiningTouch(stp);
	initializeMagnifyingGlass(stp);
	setGripPoint(stp->pos());
	setTipPoint(stp->pos());
	updateToolMode();
	updateAndTransformPoints();
	stp->setVisiblityInMagnification(SceneTouchPoint::VisibleOnlyWithAreaCursor);
	qDebug() << "/setting defining";
}


// --------------------------------------------------------
void SingleHandedPrecisionWidgetController::touchPointPressed( SceneTouchPoint * p )
{
	// only gets called on repeated press (after its already visible)

	Q_ASSERT(debug_closingTimerConsistent());
	if(definingTouch() == NULL)
	{
		//Q_ASSERT(QLineF(p->pos(), _mg->dstCenterT()).length() < _mg->dstRadiusT());
		createDefiningTouch(p);
		stopClosingTimer();
		logger() << "shpw_press defining" << this << p->id();
	}
	else
	{
		Q_ASSERT(!_additional_points.contains(p));
		_additional_points << p;
		logger() << "shpw_press additional" << this << p->id();
	}
	test(p);
	setGripPoint(definingTouch()->stp->untransformedPos());
	setTipPoint(map(gripPoint()));

	updateToolMode();
	updateAndTransformPoints();
}

// --------------------------------------------------------
void SingleHandedPrecisionWidgetController::touchPointMoved( SceneTouchPoint * p )
{
	Q_ASSERT(debug_closingTimerConsistent());

	if(p == resizingTouch() && definingTouch() != NULL)
	{
		resizeGlass();
	}

	if(definingTouch() != NULL && p == definingTouch()->stp) 
	{
		if(definingTouch()->transformed_start_pos == NULL)
		{
			// adjustment hasn't been done yet
			adjustTipToAttachedWidget();
			setGripPoint(p->pos());
			createTransformedStartPos(tipPoint());
			updateToolMode();
		}
		//qDebug() << gripPoint() << tipPoint() << "B";
		updateAndTransformPoints();
		updateAfterMovement();
		//qDebug() << gripPoint() << tipPoint() << "C";
	}
}

// --------------------------------------------------------
void SingleHandedPrecisionWidgetController::touchPointReleased( SceneTouchPoint * p )
{
	qDebug() << "released";
	Q_ASSERT(debug_closingTimerConsistent());
	if(definingTouch() != NULL && p == definingTouch()->stp)
	{
		updateAndTransformPoints();
		PointWidgetConnection * pwc = definingTouch()->stp->bottomWidgetConnection();
		RigidWidgetController * rwc = dynamic_cast<RigidWidgetController *>(pwc->widgetController());
		deleteDefiningTouch();
		if(pwc != NULL && pwc->isDragging() && rwc != NULL)
		{
			startClosingAnimation(rwc);
			close();
		}
		else
		{
			startClosingTimer(toolSettingContainer().intProperty("release_delay"));
		}
	}
	else
	{
		// update list of additional points
		Q_ASSERT(_additional_points.contains(p));
		_additional_points.removeOne(p);
		Q_ASSERT(!_additional_points.contains(p));

		// update widget state
		if(definingTouch() != NULL)
		{
			setGripPoint(definingTouch()->stp->untransformedPos());
			setTipPoint(map(gripPoint()));
			updateToolMode();
		}
	}
	qDebug() << "/released";
}

// --------------------------------------------------------
bool SingleHandedPrecisionWidgetController::forwardTouchPoint( const SceneTouchPoint & p ) const
{
	Q_UNUSED(p);
	return definingTouch()==NULL || &p==definingTouch()->stp;
}

// --------------------------------------------------------
void SingleHandedPrecisionWidgetController::createTransformedStartPos( const QPointF & pos )
{
	_defining_touch->transformed_start_pos = new QPointF(pos);
}

// --------------------------------------------------------
void SingleHandedPrecisionWidgetController::createDefiningTouch( SceneTouchPoint * stp )
{
	Q_ASSERT(_defining_touch == NULL);
	_defining_touch = new DefiningTouch(stp);
}

// --------------------------------------------------------
void SingleHandedPrecisionWidgetController::deleteDefiningTouch()
{
	Q_ASSERT(_defining_touch != NULL);
	delete _defining_touch;
	_defining_touch = NULL;
}

// --------------------------------------------------------
float SingleHandedPrecisionWidgetController::desiredRelativeChangingValue(
	const RelativeValueChanger<float> * relative_resizer,
	float base,
	float factor
) const
{
	const SceneTouchPoint * p = additionalPoints().front();
	QPointF defining_off = definingTouch()->stp->untransformedPos() - relative_resizer->start_defining_point_pos;
	float desired_offset = QLineF(p->pos(), gripPoint()).length() - QLineF(p->startPos()+defining_off, gripPoint()).length();

	return relative_resizer->start_value * powf(base,desired_offset*factor);
}

// --------------------------------------------------------
QTransform * SingleHandedPrecisionWidgetController::createScalingTransform()
{
	QTransform * result = new QTransform;
	QPointF new_center = tipPoint() + (tipPoint() - gripPoint())/(ratio() - 1);
	result->translate(new_center.x(), new_center.y());
	result->scale(1.0f/ratio(), 1.0f/ratio());
	result->translate(-new_center.x(), -new_center.y());
	logger() << "shpw_onefingermode" << this << tipPoint() << ratio();
	return result;
}

// --------------------------------------------------------
QTransform * SingleHandedPrecisionWidgetController::createParallelTransform()
{
	QPointF offset = tipPoint() - gripPoint();
	QTransform * result = new QTransform;
	result->translate(offset.x(), offset.y());
	return result;
}

// --------------------------------------------------------
QTransform * SingleHandedPrecisionWidgetController::createSetupTransform()
{
	QTransform * result = new QTransform;
	result->translate(tipPoint().x(), tipPoint().y());
	result->scale(0.0f, 0.0f);
	logger() << "shpw_setupmode" << this << tipPoint() << ratio();
	return result;
}
//// --------------------------------------------------------
//void SingleHandedPrecisionWidgetController::initializeResizer( RelativeValueChanger * changer, float value )
//{
//	changer = new RelativeValueChanger;
//	changer->start_value = _ratio;
//	changer->start_defining_point_pos = definingTouch()->stp->untransformedPos();
//}