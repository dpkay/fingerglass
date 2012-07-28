#include "touchpointmanager.h"
#include "rigidwidgetcontroller.h"
#include "touchwidgetmanager.h"
#include "ambiguousselectionattachment.h"
#include "toptwomap.h"
#include "globallog.h"

// --------------------------------------------------------
const QMap<int, SceneTouchPoint *> & TouchPointManager::touches() const
{
	return _touches;
}

// --------------------------------------------------------
SceneTouchPoint * TouchPointManager::touch( int id ) const
{
	Q_ASSERT(_touches.contains(id));
	return _touches.value(id);
}

// --------------------------------------------------------
void TouchPointManager::add( SceneTouchPoint * stp )
{
	Q_ASSERT(!_touches.contains(stp->id()));
	_touches[stp->id()] = stp;
}

// --------------------------------------------------------
void TouchPointManager::remove( int id )
{
	Q_ASSERT(_touches.contains(id));
	_touches.remove(id);
}


// --------------------------------------------------------
void TouchPointManager::getFreeTouches(
	QList<SceneTouchPoint *> * new_points,
	QList<SceneTouchPoint *> * existing_points
) const
{
	foreach(SceneTouchPoint * stp, touches())
	{
		// if young, then:
		// pressed/nowidget -> new
		// pressed/widget -> existing
		// notpressed/nowidget -> existing
		// notpressed/widget -> existing

		if(stp->isYoung())
		{
			if(stp->state() == Qt::TouchPointPressed && stp->numPressedWidgets() == 0)
			{
				new_points->push_back(stp);
			}
			else
			{
				if(existing_points != NULL)
				{
					existing_points->push_back(stp);
				}
			}
		}
	}	
}

// --------------------------------------------------------
void TouchPointManager::transferDraggingToWidget( int id, TouchWidgetController * dst )
{
	SceneTouchPoint * stp = touch(id);
	transferDraggingToWidget(stp, dst);
}

// --------------------------------------------------------
void TouchPointManager::transferDraggingToWidget( SceneTouchPoint * stp, TouchWidgetController * dst )
{
	foreach(PointWidgetConnection * pwc, stp->widgetConnections())
	{
		// HACK: halos will get lost in this process. if this will be a problem,
		// then add a check for isPressed and deal with it (only remove if isPressed)
		TouchWidgetController * twc = pwc->widgetController();
		stp->disconnectFromWidget(twc); // HACK: untested if this really works.. removing during iterating?
	}
	Q_ASSERT(stp->numPressedWidgets() == 0);
	connectDraggingToWidget(stp, dst);
}

// --------------------------------------------------------
void TouchPointManager::connectDraggingToWidget( SceneTouchPoint * stp, TouchWidgetController * dst )
{
	PointWidgetConnection * pwc = stp->connectToWidget(dst);
	pwc->setDragging(true);
}

// --------------------------------------------------------
void TouchPointManager::sendReleasedEvent( int id )
{
	SceneTouchPoint * stp = touch(id);
	foreach(PointWidgetConnection * pwc, stp->widgetConnections())
	{
		if(pwc->isDragging())
		{
			TouchWidgetController * twc = pwc->widgetController();
			twc->touchPointReleased(stp);
		}
	}	
}

// --------------------------------------------------------
void TouchPointManager::releaseExpiredHalos(TouchWidgetManager * twm)
{
	// treats those halos which are no longer the closest one to any touch
	foreach(SceneTouchPoint * stp, _touches)
	{
		foreach(PointWidgetConnection * pwc, stp->widgetConnections())
		{
			RigidWidgetController * rwc = dynamic_cast<RigidWidgetController *>(pwc->widgetController());
			if (rwc != NULL && pwc->hasHalo())
			{
				bool remove_halo = stp->closestRigidWidgets().empty() || stp->closestRigidWidgets().front() != rwc;

				if(twm->selectionMode() == IndirectPointSelection && QLineF(rwc->pos(), stp->pos()).length() > rwc->totalRadius())
				{
					remove_halo = true;
				}

				if(remove_halo)
				{
					pwc->setHalo(false);
					if(pwc->isEmpty())
					{
						stp->disconnectFromWidget(rwc);
					}
				}
			}
		}
	}
}

// --------------------------------------------------------
void TouchPointManager::touchPointPressed( SceneTouchPoint * stp, TouchWidgetManager * twm )
{
	// process all forwarding widgets
	Q_ASSERT(twm->widgets().size() >= 1);
	//TouchWidgetMultiMap::iterator tw_it = twm->widgets().end() - 1;
	TouchWidgetMultiMap::iterator tw_it = twm->widgets().end();
	//qDebug() << "end is" << *twm->widgets().end();
	QList<TouchWidgetController *> accepting_widgets;
	do
	{
		--tw_it;
		//qDebug() << "checking" << *tw_it;
		TouchWidgetController * twc = (*tw_it)->controller();
		if(twc->acceptTouchPoint(*stp) && toolsAllowInteraction(*stp, *twc))
		{
			if(twc->forwardTouchPoint(*stp))
			{
				// we accept non-terminating widgets under any condition since
				// they do not mutually exclude each other
				PointWidgetConnection * pwc = stp->connectToWidget(twc);
				pwc->setDragging(true);
				twc->touchPointPressed(stp);
			}
			else
			{
				// terminating widgets should only be accepted if there is only a single
				// candidate. we decide in the end.
				accepting_widgets << twc;
			}
		}
	}
	while(tw_it != twm->widgets().begin());

	// now, send it to exactly one of the remaining widgets
	if(accepting_widgets.size() > 0)
	{
		//TouchWidgetController * twc = bestAcceptingTerminatingWidget(accepting_widgets);
		QList<TouchWidgetController *> topmost_accepting_widgets = topmostAcceptingWidgets(accepting_widgets);
		Q_ASSERT(!topmost_accepting_widgets.empty());

		if(topmost_accepting_widgets.size()==1)
		{
			TouchWidgetController * twc = topmost_accepting_widgets.front();

			twm->pushToFront(twc);
			PointWidgetConnection * pwc = stp->connectToWidget(twc);
			pwc->setDragging(true);

			// this may destroy a widget! thus, do this part in the end
			twc->touchPointPressed(stp);
		}
		else
		{
			foreach(TouchWidgetController * twc, topmost_accepting_widgets)
			{
				RigidWidgetController * rwc = dynamic_cast<RigidWidgetController *>(twc);
				if(rwc != NULL)
				{
					AmbiguousSelectionAttachment * asa = new AmbiguousSelectionAttachment(rwc);
					rwc->attach(asa);
				}
			}
			//qDebug() << "ambiguous selection!";
		}
	}	
}

//
//
//// --------------------------------------------------------
//TouchWidgetController * TouchPointManager::bestAcceptingTerminatingWidget( const QList<TouchWidgetController *> accepting_widgets )
//{
//	QList<TouchWidgetController *> topmost_accepting_widgets = topmostAcceptingWidgets(accepting_widgets);
//
//	Q_ASSERT(!topmost_accepting_widgets.empty());
//	if(topmost_accepting_widgets.size()==1)
//	{
//		return topmost_accepting_widgets.front();
//	}
//	else
//	{
//		return NULL;
//	}	
//}


// --------------------------------------------------------
void TouchPointManager::touchPointMoved( SceneTouchPoint * stp, const TouchWidgetManager * twm )
{
	foreach(PointWidgetConnection * pwc, stp->widgetConnections())
	{
		if(pwc->isDragging())
		{
			TouchWidgetController * twc = pwc->widgetController();
			twc->touchPointMoved(stp);
		}
	}

	if(!twm->selectionModeIsDirect())
	{
		//QTime bubble_cursor_time;
		//bubble_cursor_time.start();
		updateTwoClosestWidgets(stp, twm);
		//qDebug() << "bubble cursor elapsed:" << bubble_cursor_time.elapsed();
	}
}

// --------------------------------------------------------
void TouchPointManager::touchPointReleased( SceneTouchPoint * stp, const TouchWidgetManager * twm )
{
	Q_UNUSED(twm);
	//qDebug() << "released" << stp << stp->id() << stp->pos() << "with connections" << stp->widgetConnections().size();
	foreach(PointWidgetConnection * pwc, stp->widgetConnections())
	{
		TouchWidgetController * twc = pwc->widgetController();
		if(pwc->isDragging())
		{
			// this may destroy a widget! thus, do this part in the end
			twc->touchPointReleased(stp);
		}

		// if the widget is not destroyed yet (in that case, the connection
		// would automatically be removed), kill the connection
		if(stp->widgetConnection(twc) != NULL)
		{
			stp->disconnectFromWidget(twc);
		}
	}
	delete stp;
}

// --------------------------------------------------------
void TouchPointManager::touchEvent( QTouchEvent * touch_event, TouchWidgetManager * twm )
{
	// pre-sort
	QList<QTouchEvent::TouchPoint> pressed_points;
	QList<QTouchEvent::TouchPoint> moved_points;
	QList<QTouchEvent::TouchPoint> released_points;
	foreach(const QTouchEvent::TouchPoint & event_point, touch_event->touchPoints())
	{
		// verify some basic assumptions about input
		Q_ASSERT( event_point.state()==Qt::TouchPointPressed
			|| event_point.state()==Qt::TouchPointMoved
			|| event_point.state()==Qt::TouchPointReleased );

		switch(event_point.state())
		{
			case Qt::TouchPointPressed:		pressed_points << event_point; break;
			case Qt::TouchPointMoved:		moved_points << event_point; break;
			case Qt::TouchPointReleased:	released_points << event_point; break;
			default: Q_ASSERT(false);
		}
	}

	// process pressed points
	foreach(const QTouchEvent::TouchPoint & event_point, pressed_points)
	{
		//GlobalLog::log(QString("press %1").arg(event_point.pos().x()));
		
		//logger() << "press" << "blah";
		//qDebug() << "pressed" << event_point.pos();
		SceneTouchPoint * stp = new SceneTouchPoint(event_point, _parent_size);
		logger() << "touch_press" << *stp;
		this->add(stp);
		this->touchPointPressed(stp, twm);
		//stp->untransform();
		//this->touchPointMoved(stp, twm);
		notifyListeners(stp);
	}

	// process moved points
	foreach(const QTouchEvent::TouchPoint & event_point, moved_points)
	{
		SceneTouchPoint * stp = this->touch(event_point.id());
		Q_ASSERT(stp != NULL);
		logger() << "touch_move" << *stp;
		stp->setPoint(event_point);
		notifyListeners(stp);
		this->touchPointMoved(stp, twm);
	}

	// process released points
	foreach(const QTouchEvent::TouchPoint & event_point, released_points)
	{
		SceneTouchPoint * stp = this->touch(event_point.id());
		Q_ASSERT(stp != NULL);
		logger() << "touch_release" << *stp;
		stp->setPoint(event_point);
		notifyListeners(stp);
		this->touchPointReleased(stp, twm);
		this->remove(event_point.id());
	}

	// take care of new tools
	Q_ASSERT(_tools != NULL);
	foreach(Tool * tool, *_tools)
	{
		tool->updateAfterTouchEvent(touch_event);
	}

	// clean up other things
	releaseExpiredHalos(twm);
}

// --------------------------------------------------------
bool TouchPointManager::toolsAllowInteraction( const SceneTouchPoint & stp, const TouchWidgetController & twc ) const
{
	Q_ASSERT(_tools != NULL);
	const RigidWidgetController * rwc = dynamic_cast<const RigidWidgetController *>(&twc);
	if(rwc == NULL)
	{
		return true;
	}
	else
	{
		foreach(Tool * tool, *_tools)
		{
			if(tool->allowRigidWidgetInteraction(stp, *rwc))
			{
				return true;
			}
		}
		return false;
	}
}

// --------------------------------------------------------
TouchPointManager::TouchPointManager()
{
	_tools = NULL;
	//_update_closest_widgets = false;
}

// --------------------------------------------------------
void TouchPointManager::addTouchListener( TouchListener * tl )
{
	_listeners << tl;
	//qDebug() << "added touch listener" << tl;
}
 
// --------------------------------------------------------
void TouchPointManager::notifyListeners( SceneTouchPoint * stp )
{
	foreach(TouchListener * tl, _listeners)
	{
		switch(stp->state())
		{
		case Qt::TouchPointPressed:
			tl->touchPointPressed(stp);
			break;
		case Qt::TouchPointMoved:
			tl->touchPointMoved(stp);
			break;
		case Qt::TouchPointReleased:
			tl->touchPointReleased(stp);
			break;			
		}
	}
}

// --------------------------------------------------------
void TouchPointManager::removeTouchListener( TouchListener * tl )
{
	_listeners.removeOne(tl);
}

// --------------------------------------------------------
void TouchPointManager::updateTwoClosestWidgets( SceneTouchPoint * stp, const TouchWidgetManager * twm )
{
	stp->closestRigidWidgets().clear();
	if(stp->isFree())
	{
		//QMap<float, RigidWidgetController *> rwc_by_distance;
		TopTwoMap<float, RigidWidgetController *> top_two_rwc;
		//TopTwoMap<float, float>::debug_Test();
		foreach(TouchWidget * tw, twm->widgets())
		{
			RigidWidgetController * rwc = dynamic_cast<RigidWidgetController *>(tw->controller());
			if(rwc != NULL && rwc->isMovable() && toolsAllowInteraction(*stp, *rwc) && !rwc->isMoving())
			{
				float distance = rwc->intersectingDistance(stp->pos());
				//rwc_by_distance[distance] = rwc;
				top_two_rwc.insert(distance, rwc);
			}
		}
		//QMap<float, RigidWidgetController *>::const_iterator rwc_it = rwc_by_distance.begin();

		// if at least one rigid widget is available
		//if(rwc_it != rwc_by_distance.end())
		if(top_two_rwc.hasFirst())
		{
			//stp->closestRigidWidgets() << *rwc_it;
			RigidWidgetController * rwc = top_two_rwc.first();
			stp->closestRigidWidgets() << rwc;
			switch(twm->selectionMode())
			{
			case IndirectPointSelection:
				{
					float distance = QLineF(rwc->pos(), stp->pos()).length();
					//qDebug() << distance << (*rwc_it)->coreRadius();
					if(distance < rwc->coreRadius())
					{
						PointWidgetConnection * pwc = stp->widgetConnection(rwc);
						if(pwc == NULL)
						{
							pwc = stp->connectToWidget(rwc);
						}
						pwc->setHalo(true);
					}
				}
				break;
			case IndirectBubbleSelection:
				{
					// closest widget: add to list of closest widgets and enable halo
					PointWidgetConnection * pwc = stp->widgetConnection(rwc);
					if(pwc == NULL)
					{
						pwc = stp->connectToWidget(rwc);
					}
					pwc->setHalo(true);

					// if more than one rigid widgets is available, add the second-closest one too
					/*++rwc_it;
					if(rwc_it != rwc_by_distance.end())*/
					if(top_two_rwc.hasSecond())
					{
						stp->closestRigidWidgets() << top_two_rwc.second();
					}
				}
				break;
			default:
				Q_ASSERT_X(false, "updateTwoClosestWidgets", "invalid selection mode");
			}
		}
	}
}
//
//// --------------------------------------------------------
//void TouchPointManager::updateAllCoveredWidgets( SceneTouchPoint * stp, const TouchWidgetManager * twm )
//{
//	foreach(TouchWidget * tw, twm->widgets())
//	{
//		RigidWidgetController * rwc = dynamic_cast<RigidWidgetController *>(tw->controller());
//		if(rwc != NULL && rwc->isMovable() && toolsAllowInteraction(*stp, *rwc) && !rwc->isMoving())
//		{
//			float distance = QLineF(rwc->pos(), stp->pos()).length();
//			if(distance < rwc->coreRadius())
//			{
//				PointWidgetConnection * pwc = stp->widgetConnection(rwc);
//				if(pwc == NULL)
//				{
//					pwc = stp->connectToWidget(rwc);
//				}
//				pwc->setHalo(true);
//			}
//			//rwc_by_distance[distance] = rwc;
//		}
//	}
//}

// --------------------------------------------------------
void TouchPointManager::setSize( QSizeF size )
{
	_parent_size = size;
}

// --------------------------------------------------------
QList<TouchWidgetController *> TouchPointManager::topmostAcceptingWidgets(
	const QList<TouchWidgetController *> &accepting_widgets )
{
	Q_ASSERT(!accepting_widgets.empty());

	// sort accepting widget by layer group
	QMultiMap<TouchWidgetType, TouchWidgetController *> twmm;
	foreach(TouchWidgetController * twc, accepting_widgets)
	{
		twmm.insert(twc->type(), twc);
	}

	// if the topmost layer is ambiguous, then complain, otherwise return that one
	return twmm.values((twmm.end()-1).key());
}
