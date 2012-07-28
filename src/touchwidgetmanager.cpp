#include "touchwidgetmanager.h"
#include "touchdisplaywidgetrenderer.h"
#include "magnifyingwidgetcontroller.h"

// --------------------------------------------------------
void TouchWidgetManager::add(TouchWidget * tw)
{
	touch_widgets.insert(tw->type(), tw);

	QObject::connect(tw->controller(), SIGNAL(closed()), this, SLOT(remove()));

	QObject::connect(tw->renderer(), SIGNAL(setupViewport()), glwidget, SLOT(setupViewport()));
	QObject::connect(tw->renderer(), SIGNAL(drawScene()), glwidget, SLOT(drawWidgets()));

	qDebug() << "added" << tw->controller() << "#touches" << tw->controller()->touchPointConnections().size();
}

// --------------------------------------------------------
void TouchWidgetManager::remove(TouchWidgetController * param_twc)
{
	// this function may be called regularly or as a slot. so the widget
	// is either given by the parameter or the signal sender. exactly
	// one of them should contain the widget, the other one zero. in 
	// any other case, there is something wrong.
	QObject * sender = QObject::sender();
	TouchWidgetController * sender_twc = NULL;
	if(sender != NULL)
	{
		sender_twc = dynamic_cast<TouchWidgetController *>(sender);
	}
	Q_ASSERT((param_twc == NULL) ^ (sender_twc == NULL));
	TouchWidgetController * twc = (param_twc == NULL) ? sender_twc : param_twc; 

	// remove touch widget indicated by twc from the list
	TouchWidgetMultiMap::iterator tw_it = touch_widgets.begin();
	while(tw_it != touch_widgets.end())
	{
		if((*tw_it)->controller() == twc) break;
		++tw_it;
	}
	Q_ASSERT(tw_it != touch_widgets.end());
	remove(tw_it);
}

// --------------------------------------------------------
void TouchWidgetManager::remove( const TouchWidgetMultiMap::iterator & tw_it )
{
	Q_ASSERT(*tw_it != NULL);
	TouchWidgetController * twc = (*tw_it)->controller();
	TouchWidgetRenderer * twr = (*tw_it)->renderer();
	delete *tw_it;
	touch_widgets.erase(tw_it);

	// disconnect signals
	twc->disconnect();
	twr->disconnect();

	// disconnect touchpoints
	foreach(PointWidgetConnection * pwc, twc->touchPointConnections())
	{
		pwc->point()->disconnectFromWidget(twc);
	}

	qDebug() << "deleted controller" << twc;

	// kill them
	delete twc;
	delete twr;
}
// --------------------------------------------------------
const TouchWidgetMultiMap & TouchWidgetManager::widgets() const
{
	return touch_widgets;
}

// --------------------------------------------------------
TouchWidgetManager::TouchWidgetManager( QObject * glwidget )
{
	//this->tpm = tpm;
	this->glwidget = glwidget;
	this->_selection_mode = DirectPointSelection;
	this->_current_ball_target = NULL;
}

// --------------------------------------------------------
void TouchWidgetManager::clear()
{
	TouchWidgetMultiMap::iterator tw_it = touch_widgets.begin();
	while(tw_it != touch_widgets.end())
	{
		TouchWidgetMultiMap::iterator prev_tw_it = tw_it;
		++tw_it;
		remove(prev_tw_it);
	}

	//QMutableMapIterator<TouchWidgetType, TouchWidget *> it(touch_widgets);
	//while (it.hasNext()) {
	//	it.next();
	//	it.remove();
	//}
}

#include "shiftpluswidgetcontroller.h"

// --------------------------------------------------------
void TouchWidgetManager::paintAll( GLResourceContainer * resource_container, TouchWidgetRenderer * to_texture_renderer )
{
	//qDebug() << "drawing";
	TouchWidgetMultiMap::const_iterator tw_it = widgets().begin();
	TouchWidgetType current_type = NullType;
	TouchWidgetMultiMap::const_iterator current_type_start;
	int current_type_passes = -1;
	while(tw_it != widgets().end())
	{
		if(tw_it.key() != current_type)
		{
			current_type = tw_it.key();
			current_type_start = tw_it;
			current_type_passes = (*tw_it)->renderer()->numPasses();
		}
		Q_ASSERT(current_type_passes != -1);
		
		for(int pass = 0; pass < current_type_passes; ++pass)
		{
			tw_it = current_type_start;
			while(tw_it != widgets().end() && tw_it.key() == current_type)
			{
				//qDebug() << "pass" << pass << "type" << current_type << "widget" << *tw_it;

				// avoid infinite recursion in a render-to-texture mode
				if( to_texture_renderer != NULL && // we are in render-to-texture mode
					(*tw_it)->renderer()->isSceneDrawer() ) // renderer would like to do render-to-texture stuff
				{
					//break;
				}
				else
				{
					(*tw_it)->renderer()->paint(resource_container, pass, to_texture_renderer);

					//if(dynamic_cast<ShiftPlusWidgetController *>((*tw_it)->controller())!=NULL)
					//	qDebug() << "drawing shift widget";

				}
				++tw_it;
			}
		}
	}

	
}

// --------------------------------------------------------
void TouchWidgetManager::pushToFront( const TouchWidgetController * twc )
{
	TouchWidgetMultiMap::iterator tw_it = touch_widgets.begin();
	while(tw_it != touch_widgets.end())
	{
		if((*tw_it)->controller() == twc) break;
		++tw_it;
	}
	Q_ASSERT(tw_it != touch_widgets.end());

	TouchWidget * tw = *tw_it;
	QList<TouchWidget *> others = touch_widgets.values(tw->type());
	touch_widgets.remove(twc->type());
	touch_widgets.insert(twc->type(), tw);
	foreach(TouchWidget * other_tw, others)
	{
		if(other_tw != tw)
		{
			touch_widgets.insert(other_tw->type(), other_tw);
		}
	}
}

// --------------------------------------------------------
void TouchWidgetManager::setSelectionMode( const QString & selection_mode )
{
	if(selection_mode=="directpoint")
	{
		_selection_mode = DirectPointSelection;
	}
	else if(selection_mode=="directarea")
	{
		_selection_mode = DirectAreaSelection;
	}
	else if(selection_mode=="indirectpoint")
	{
		_selection_mode = IndirectPointSelection;
	}
	else if(selection_mode=="indirectbubble")
	{
		_selection_mode = IndirectBubbleSelection;
	}
	else
	{
		Q_ASSERT_X(false, "setSelectionMode", "invalid selection mode");
	}
}

// --------------------------------------------------------
bool TouchWidgetManager::selectionModeIsDirect() const
{
	return _selection_mode == DirectAreaSelection || _selection_mode == DirectPointSelection;
}

// --------------------------------------------------------
TouchWidgetManager::~TouchWidgetManager()
{
	if(_current_ball_target != NULL)
	{
		delete _current_ball_target;
	}
}

// --------------------------------------------------------
void TouchWidgetManager::setCurrentBallTarget( const RigidWidgetController * ball, const RigidWidgetController * target )
{
	if(ball == NULL && target == NULL)
	{
		// delete
		if(_current_ball_target != NULL)
		{
			delete _current_ball_target;
			_current_ball_target = NULL;
		}
	}
	else
	{
		if(_current_ball_target == NULL)
		{
			_current_ball_target = new QPair<const RigidWidgetController *, const RigidWidgetController *>;
		}
		_current_ball_target->first = ball;
		_current_ball_target->second = target;

	}
}
// --------------------------------------------------------
//TouchWidgetRenderer * TouchWidgetManager::rendererOfController( TouchWidgetController * twc )
//{
//	foreach(TouchWidget * tw, _widgets)
//	{
//	}
//}

//// --------------------------------------------------------
//TouchWidgetMultiMap::iterator TouchWidgetManager::lastNonMagnifyingWidgetIterator()
//{
//	TouchWidgetMultiMap::iterator it = touch_widgets.begin();
//	while(it.key() != MagnifyingWidgetType && it != touch_widgets.end()) ++it;
//	return --it;
//}

//// --------------------------------------------------------
//void TouchWidgetManager::setScreenSize( const QSize & screen_size )
//{
//	foreach(TouchWidget * tw, touch_widgets)
//	{
//		TouchDisplayWidgetRenderer * tdwr =
//			dynamic_cast<TouchDisplayWidgetRenderer *>(tw->renderer());
//		if(tdwr != NULL)
//		{
//			tdwr->setScreenSize(screen_size);
//		}
//	}
//}

//// --------------------------------------------------------
//void TouchWidgetManager::addHalo( RigidWidgetController * rwc, SceneTouchPoint * stp )
//{
//	_halo_rigid_widget_controllers[qMakePair(rwc, stp)] = QTime::currentTime();
//}
//
//// --------------------------------------------------------
//void TouchWidgetManager::releaseExpiredHalos()
//{
//	HaloMap::iterator it = _halo_rigid_widget_controllers.begin();
//	while(it != _halo_rigid_widget_controllers.end())
//	{
//		RigidWidgetController * rwc = it.key().first;
//		SceneTouchPoint * stp = it.key().second;
//		const QTime & time = it.value();
//		if(!rwc->needsHalo(*stp->transformedPoint()))
//		{
//			// HACK: does removing during iteration really work?
//			rwc->setHaloVisible(false);
//			it = _halo_rigid_widget_controllers.erase(it);			
//		}
//		else
//		{
//			++it;
//		}
//	}
//}
//
//// --------------------------------------------------------
//void TouchWidgetManager::removeHalos( SceneTouchPoint * stp )
//{
//	HaloMap::iterator it = _halo_rigid_widget_controllers.begin();
//	while(it != _halo_rigid_widget_controllers.end())
//	{
//		if(it.key().second == stp)
//		{
//			it.key().first->setHaloVisible(false);
//			it = _halo_rigid_widget_controllers.erase(it);
//		}
//		else
//		{
//			++it;
//		}
//	}
//}
//
//// --------------------------------------------------------
//void TouchWidgetManager::removeHalos( RigidWidgetController * rwc )
//{
//	HaloMap::iterator it = _halo_rigid_widget_controllers.begin();
//	while(it != _halo_rigid_widget_controllers.end())
//	{
//		if(it.key().first == rwc)
//		{
//			rwc->setHaloVisible(false);
//			_halo_rigid_widget_controllers.erase(it);
//		}
//		else
//		{
//			++it;
//		}
//	}
//
//}