#include "fingerglasstool.h"
#include "magnifyingwidgetcontroller.h"
#include "handlewidgetcontroller.h"
#include "magnifyingwidgetrenderer.h"
#include "handlewidgetrenderer.h"
#include "touchwidgetmanager.h"
#include "touchpointmanager.h"

// --------------------------------------------------------
void FingerGlassTool::updateAfterTouchEvent(const QTouchEvent * event)
{
	Q_ASSERT(touchWidgetManager() != NULL && touchPointManager() != NULL);
	if(!allowMagnifyingWidget())
		return;

	if(event->touchPointStates() & (Qt::TouchPointPressed|Qt::TouchPointReleased))
	{
		foreach(const QTouchEvent::TouchPoint & point, event->touchPoints())
		{
			switch(point.state())
			{
			case Qt::TouchPointPressed:
				{
					SceneTouchPoint * stp = touchPointManager()->touch(point.id());

					if(stp->numPressedWidgets() == 0)
					{
						// add to opener
						if(_opener == NULL)
						{
							_opener = new FingerGlassOpener(touchPointManager(), settingContainer().intProperty("initial_delay"));
							connect(_opener, SIGNAL(openerTimedOut(const QList<SceneTouchPoint *> &)),
								this, SLOT(openerTimedOut(const QList<SceneTouchPoint *> &)));
						}
						_opener->addTouchById(point.id());
					}
					break;
				}
			case Qt::TouchPointReleased:
				{
					if(_opener != NULL && _opener->hasTouchById(point.id()))
					{
						// remove from opener
						_opener->removeTouchById(point.id());
						if(!_opener->hasTouches())
						{
							delete _opener;
							_opener = NULL;
						}
					}
					break;
				}
			}
		}
	}
}

// --------------------------------------------------------
FingerGlassTool::FingerGlassTool( SettingContainer * setting_container ) :
	Tool(setting_container)
{
	_opener = NULL;
}

// --------------------------------------------------------
void FingerGlassTool::createMagnifyingWidgetFromHandleWidget()
{
	HandleWidgetController * hwc = dynamic_cast<HandleWidgetController *>(QObject::sender());
	Q_ASSERT(hwc != NULL);
	Q_ASSERT(hwc->absoluteHandleMode() == HandleWidgetController::TwoFingerHandle);

	MagnifyingGlass * zoom_glass = new MagnifyingGlass(*hwc->clutchHandle()->magnifyingGlass());
	zoom_glass->setSpeed(1.0f);
	zoom_glass->restartWarps();

	MagnifyingWidgetController * mwc = new MagnifyingWidgetController(settingContainer(), zoom_glass);
	touchPointManager()->transferDraggingToWidget(hwc->absoluteTouches().first, mwc);
	touchPointManager()->transferDraggingToWidget(hwc->absoluteTouches().second, mwc);
	mwc->addSpanningPoint(hwc->absoluteTouches().first);
	mwc->addSpanningPoint(hwc->absoluteTouches().second);
	if(hwc->clutchTouch() != NULL)
	{
		mwc->enqueuePole(&hwc->clutchTouch()->untransformedPos());
	}
	mwc->updateMagnifyingGlasses();

	// deal with clutch inside magnifying glass
	if(hwc->clutchTouch() != NULL)
	{
		hwc->clutchTouch()->disconnectFromWidget(hwc);
		Q_ASSERT(hwc->clutchTouch()->widgetConnections().empty());
		hwc->clutchTouch()->restart();

		PointWidgetConnection * pwc1 = hwc->clutchTouch()->connectToWidget(mwc);
		pwc1->setDragging(true);
		mwc->touchPointPressed(hwc->clutchTouch(), true);

		RigidWidgetController * rwc = const_cast<RigidWidgetController *>(hwc->attachedWidget());
		rwc->setPos(hwc->clutchTouch()->pos());
		PointWidgetConnection * pwc2 = hwc->clutchTouch()->connectToWidget(rwc);
		pwc2->setDragging(true);
		rwc->touchPointPressed(hwc->clutchTouch());
	}

	// create renderer and add widget
	MagnifyingWidgetRenderer * mwr = new MagnifyingWidgetRenderer(mwc);
	TouchWidget * tw = new TouchWidget(mwc, mwr);
	touchWidgetManager()->add(tw);
	QObject::connect(mwc, SIGNAL(createHandleWidgetIfNecessary()), this, SLOT(createHandleWidgetIfNecessary()));
}

// --------------------------------------------------------
void FingerGlassTool::createHandleWidgetIfNecessary()
{
	Q_ASSERT(touchWidgetManager() != NULL && touchPointManager() != NULL);
	MagnifyingWidgetController * mwc = dynamic_cast<MagnifyingWidgetController *>(QObject::sender());
	Q_ASSERT(mwc != NULL);

	RigidWidgetController * attaching_widget = NULL;
	SceneTouchPoint * attaching_point = NULL;
	//foreach(int id, mwc->innerTouchIds())
	//{

	if(mwc->innerTouchIds().size() == 1)
	{
		int id = mwc->innerTouchIds().front();
		SceneTouchPoint * stp = touchPointManager()->touch(id);
		//qDebug() << "id" << id << "size" << touchPointManager()->touch(id)->owners().size();
		//const QList<PointWidgetConnection *> & connections = stp->widgetConnections();

		// each of these touches is still owned by the magnifying widget.
		// additionally, we want them to be owned by exactly one more widget.
		if(stp->widgetConnections().size() > 1)
		{
			// the following cases are possible:
			//  1) pressed on mwc, halo on rigid
			//  2) pressed on mwc, pressed on rigid
			// either way, at least one has to be pressed, and mwc has to be pressed
			Q_ASSERT(stp->numPressedWidgets()==1 || stp->numPressedWidgets()==2);
			Q_ASSERT(stp->widgetConnections().front()->isDragging());

			// for now, all movable parts are rigidwidgets, so this better be one
			//attaching_widget = dynamic_cast<RigidWidgetController *>(stp->pressedTouchWidgetControllers().back());
			attaching_widget = dynamic_cast<RigidWidgetController *>(stp->widgetConnections().back()->widgetController());
			Q_ASSERT(attaching_widget != NULL);
			attaching_point = touchPointManager()->touch(id);
			//break;
		}
	}
	// if we found a widget to attach, then let's go ahead and create a handlewidget
	if(attaching_widget != NULL)
	{
		Q_ASSERT(attaching_point != NULL);
		const MagnifyingGlass * mg = mwc->magnifyingGlassOfPoint(attaching_point);
		//float clutch_radius = mg->dstRadiusT();
		//float cd_ratio = mwc->cdRatioOfPoint(attaching_point->id());
		touchPointManager()->sendReleasedEvent(attaching_point->id());
		attaching_point->untransform();
		//attaching_point->closestRigidWidgets().clear();
		HandleWidgetController * hwc = new HandleWidgetController(attaching_widget, settingContainer(), *mg);
		//		attaching_point->restart();
		touchPointManager()->transferDraggingToWidget(attaching_point->id(), hwc);
		/*Q_ASSERT(attaching_point->transformedPoint()->pos() == attaching_point->transformedPoint()->startPos());*/
		//hwc->createClutchHandle(attaching_point->transformedPoint()->pos(), cd_ratio);  	
		hwc->createClutchHandle(attaching_point->pos(), mg);  	
		// -> make hwc aware of attaching point (~setSpanningPoints)
		hwc->touchPointPressed(attaching_point);
		//hwc->correctWidgetPosition();
		// TODO -> press touchpoint for handle widget (~hwc->touchPointPressed)
		// TODO -> press touchpoint for widget below (twice?? since we have two handles.. not clear)
		HandleWidgetRenderer * hwr = new HandleWidgetRenderer(hwc);
		TouchWidget * tw = new TouchWidget(hwc, hwr);
		touchWidgetManager()->add(tw);
		QObject::connect(hwc, SIGNAL(createMagnifyingWidget()), this, SLOT(createMagnifyingWidgetFromHandleWidget()));
	}
}

// --------------------------------------------------------
bool FingerGlassTool::allowRigidWidgetInteraction(
	const SceneTouchPoint & stp,
	const RigidWidgetController & rwc ) const
{
	Q_UNUSED(rwc);
	return stp.isTransformed();
	//qDebug() << "allowRigidWidgetInteraction()";
	//return true;
}

// --------------------------------------------------------
bool FingerGlassTool::allowMagnifyingWidget()
{
	foreach(const TouchWidget * tw, touchWidgetManager()->widgets())
	{
		// we only want one magnifying glass at the same time for now
		if(dynamic_cast<const MagnifyingWidgetController *>(tw->controller()) != NULL)
		{
			return false;
		}

		// also, we don't want any magnifying glasses when dealing with handles
		if(dynamic_cast<const HandleWidgetController *>(tw->controller()) != NULL)
		{
			return false;
		}
	}
	return true;
}

// --------------------------------------------------------
void FingerGlassTool::openerTimedOut( const QList<SceneTouchPoint *> & touches )
{
	delete _opener;
	_opener = NULL;

	qDebug() << "touches:" << touches.size();
	Q_ASSERT(touches.size() <= 2);
	if(touches.size() == 2)
	{
		QPair<SceneTouchPoint *, SceneTouchPoint *> spanning_points;
		spanning_points.first = touches.value(0);
		spanning_points.second = touches.value(1);

		// create controller and put touches in the right place
		//touchPointManager()->sendReleasedEvent(spanning_points.first->id());
		//touchPointManager()->sendReleasedEvent(spanning_points.second->id());
		MagnifyingWidgetController * mwc = new MagnifyingWidgetController(settingContainer());
		touchPointManager()->transferDraggingToWidget(spanning_points.first->id(), mwc);
		touchPointManager()->transferDraggingToWidget(spanning_points.second->id(), mwc);
		mwc->addSpanningPoint(spanning_points.first);
		mwc->addSpanningPoint(spanning_points.second);
		mwc->updateMagnifyingGlasses();

		// create renderer and add widget
		MagnifyingWidgetRenderer * mwr = new MagnifyingWidgetRenderer(mwc);
		TouchWidget * tw = new TouchWidget(mwc, mwr);
		touchWidgetManager()->add(tw);
		QObject::connect(mwc, SIGNAL(createHandleWidgetIfNecessary()), this, SLOT(createHandleWidgetIfNecessary()));
	}
}
