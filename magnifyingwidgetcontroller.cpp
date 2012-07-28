#include "magnifyingwidgetcontroller.h"
#include <QtDebug>
#include "settings.h"
#include "rigidwidgetcontroller.h"
#include "globallog.h"

// --------------------------------------------------------
MagnifyingWidgetController::MagnifyingWidgetController(
	const SettingContainer & tool_setting_container,
	MagnifyingGlass * zoom_glass
) :
	TouchWidgetController(MagnifyingWidgetType),
	_tool_setting_container(tool_setting_container)
{
	_dst_resizing_touch = NULL;
	_mgm = NULL;
	request_update = false;
	TimeSingleton::instance()->subscribe(this);
	_age.start();
	_locking_touch = NULL;
	_just_resized = false;
	_initial_zoom_glass = zoom_glass;
	_force_detached = zoom_glass != NULL;
	//_force_detached = false;
	logger() << "mwc_open" << this;
	_enqueued_pole = NULL;

	_desired_ratio = _tool_setting_container.floatProperty("initial_ratio");
}

// --------------------------------------------------------
MagnifyingWidgetController::~MagnifyingWidgetController()
{
	logger() << "mwc_close"
		<< this
		<< _mgm->singleMagnifyingGlass()->dstCenterT()
		<< _mgm->singleMagnifyingGlass()->dstRadiusT()
		<< _mgm->baseGlass()->dstCenterT()
		<< _mgm->baseGlass()->dstRadiusT();
	TimeSingleton::instance()->unsubscribe(this);
	delete _mgm;
}

// --------------------------------------------------------
const QVector<MagnifyingGlass *> & MagnifyingWidgetController::magnifyingGlasses() const
{
	return _mgm->magnifyingGlasses();
}

// --------------------------------------------------------
void MagnifyingWidgetController::touchPointReleased( SceneTouchPoint * p )
{
	// if one of the two spanning points was released, eliminate the widget
	if(numSpanningPoints()==1 && isSpanningPoint(*p))
	{
		emit createHandleWidgetIfNecessary();
		close();
	}
	else
	{
		if(p == _locking_touch)
		{
			_locking_touch = NULL;
		}
		else
		if(p == _dst_resizing_touch)
		{
			_dst_resizing_touch = NULL;
		}
		else
		if(_owning_magnifying_glasses.contains(p->id()))
		{
			transformTouchPoint(p);
			_owning_magnifying_glasses.remove(p->id());
			_start_positions.remove(p->id());

			RigidWidgetController * rwc = dynamic_cast<RigidWidgetController *>(p->bottomWidgetConnection()->widgetController());
			if(rwc != NULL && rwc->isHaloVisible())
			{
				_active_halos << rwc;
			}
		}
		else
		if(isSpanningPoint(*p) && numSpanningPoints()==2)
		{
			_locking_touch = NULL;
			removeSpanningPoint(touchPointConnections().value(p->id())->point());
		}
		else
		{
			//qDebug() << p->id();
			//qDebug() << spanning_points.first->id();
			//qDebug() << spanning_points.second->id();
			//qDebug() << _owning_magnifying_glasses.keys().first();
			////// HACK: uncomment this
			//Q_ASSERT(false);
		}
		if(_owning_magnifying_glasses.empty())
			_force_detached = false;
	}
}

// --------------------------------------------------------
void MagnifyingWidgetController::touchPointMoved( SceneTouchPoint * p )
{
	const bool allow_modification = !Settings::instance()->mwStabilized() || _owning_magnifying_glasses.empty();
	//const bool allow_modification = _locking_touch == NULL;
	if(!isSpanningPoint(*p))
	{
		//qDebug() << "moved sp";
		if(p == _locking_touch)
		{
			// dont do anything
		}
		else
		if(p == _dst_resizing_touch)
		{
			Q_ASSERT(showGlass());

			Q_ASSERT(Settings::instance()->mwResizable());
			if(allow_modification)
			{
				//MagnifyingGlass * mg = _mgm->singleMagnifyingGlass();
				_mgm->goBackIntoBounds();

				//// compute desired radius of the single magnifying glass
				if(magnifyingGlassDistance() < 0.02f)
				{
					QLineF base_center_to_p(_mgm->magnifyingGlasses().front()->srcCenterT(), p->pos());
					float base_radius = _mgm->magnifyingGlasses().front()->srcRadiusT();
					qreal mag_radius = (base_center_to_p.length() - base_radius) / 2;
					mag_radius = qMax(_mgm->baseGlass()->dstRadiusT(), mag_radius);
					_mgm->magnifyingGlasses().back()->setDstRadiusD(mag_radius);
					//updateMagnifyingGlasses();
					_just_resized = true;
					_desired_ratio = mag_radius/base_radius;
					qDebug() << "updating desired ratioB" << _desired_ratio;
				}
				else
				{
					qreal mag_radius = QLineF(_mgm->magnifyingGlasses().back()->dstCenterT(), p->pos()).length();
					mag_radius = qMax(_mgm->baseGlass()->dstRadius(), mag_radius);
					_mgm->magnifyingGlasses().back()->setDstRadiusD(mag_radius);
					_mgm->magnifyingGlasses().back()->setDstRadius(mag_radius);
					_mgm->goBackIntoBounds();
					_desired_ratio = _mgm->magnifyingGlasses().back()->ratio();
					qDebug() << "updating desired ratio" << _desired_ratio;
				}
				//_mgm->magnifyingGlasses().front()->setDstRadius(mag_radius);

				//// compute center point of the single magnifying glass
				//base_center_to_p.setLength(base_radius+mag_radius);
				//mg->setDstCenter(base_center_to_p.p2());

				//// compute angle
				////qDebug() << "setting angle " <<atan2(-base_center_to_p.dy(), base_center_to_p.dx()) * 180 / 3.1415f;
				//_mgm->setAngle(atan2(-base_center_to_p.dy(), base_center_to_p.dx()) * 180 / 3.1415f);
				////mgm->linearlyArrang eGlasses();
				//request_update = true;

				QPointF diff = p->pos() - _mgm->baseGlass()->dstCenterT();
				_mgm->setDesiredAngle(atan2(-diff.y(), diff.x()) * 180/3.1415f);
				_mgm->setDesiredMaximalVariation(showGlass() ? 30 : 0);
				request_update = true;
				//_just_resized = true;

				// for display
				if(!showGlass())
				{
					Q_ASSERT(!_owning_magnifying_glasses.contains(p->id()));
					_owning_magnifying_glasses.insert(p->id(), _mgm->magnifyingGlasses().back());
					transformTouchPoint(p);
					_owning_magnifying_glasses.remove(p->id());
				}
			}
		}
		else
		if(_owning_magnifying_glasses.contains(p->id()))
		{
			_mgm->setPole(p->untransformedPos());
			//updatePoles();
			transformTouchPoint(p);

			// check if it would be out of glass (in twofinger case)
			const RigidWidgetController * attached_rwc = dynamic_cast<RigidWidgetController *>(p->bottomWidgetConnection()->widgetController());
			float r = attached_rwc != NULL ? attached_rwc->coreRadius() : 0;
			//if(attached_rwc != NULL)
			//{

			if(showGlass())
			{
				QPointF dest_pos = p->pos();

				const MagnifyingGlass * base_glass = _mgm->magnifyingGlasses().front();
				QLineF line(base_glass->dstCenterT(), dest_pos);
				float max_radius = base_glass->dstRadiusT() - r;
				if(line.length() > max_radius)
				{
					dest_pos = line.unitVector().pointAt(max_radius);
				}
				//_attached_widget->setPos(dest_pos);

				MagnifyingGlass * mg = _mgm->magnifyingGlasses().back();

				if(!locked()/* && !(showGlass() && attached_rwc == NULL)*/)
				{
					QPointF shifted_center;
					if(attached_rwc != NULL)
					{
						shifted_center = mg->dstCenterT() + (p->pos()-dest_pos);
					}
					else
					{
						shifted_center = mg->dstCenterT() + (p->pos()-p->startPos());
					}
					if(QLineF(shifted_center, base_glass->dstCenterT()).length() >= base_glass->dstRadiusT() + _mgm->singleMagnifyingGlass()->dstRadiusT())
					{
						mg->setDstCenter(shifted_center);
						_mgm->goBackIntoBounds();
					}
				}
				p->transform(dest_pos, p->startPos(), p->rect().size());
			}
			//}
		}
	}
	else
	{
		//qDebug() << "moved nsp";
		if(allow_modification)
		{
			//updateMagnifyingGlasses();
			request_update = true;
		}
	}

	//qDebug() << "m touches: " << numPressedTouchPoints() << "spanning: " << numSpanningPoints();
} 


// --------------------------------------------------------
bool MagnifyingWidgetController::acceptAsSecondSpanningPoint( const SceneTouchPoint & p ) const
{
	const MagnifyingGlass * mg = _mgm->magnifyingGlassByScreenPos(p.pos());

	// test if one spanning point is missing and whether it should be added again
	// three conditions have to hold:
	return
		// (1) one spanning point must be missing
		numSpanningPoints()==1 &&

		// (2) the touch is not in a magnified glass
		(mg==NULL || mg==_mgm->baseGlass()) &&

		// (3) the touch is reasonably close to the virtual spanning point
		QLineF(p.pos(), virtualSpanningPointPos()).length() < spanningDiameter();
}

// --------------------------------------------------------
void MagnifyingWidgetController::touchPointPressed( SceneTouchPoint * p, bool force_glass )
{
	const MagnifyingGlass * mg = _mgm->singleMagnifyingGlass();

	if(acceptAsSecondSpanningPoint(*p))
	{
		qDebug() << "adding spanning point";
		addSpanningPoint(touchPointConnection(p->id())->point());
		return;
	}

	// otherwise, check if it is inside or should become a resizing point
	if(!isSpanningPoint(*p))
	{
	/*	if(isOnBaseGlassSide(*p) && _locking_touch == NULL)
		{
			_locking_touch = p;
			return;
		}*/
		float r_zoom = QLineF(p->pos(), mg->dstCenterT()).length();
		float r_base = QLineF(p->pos(), _mgm->baseGlass()->dstCenterT()).length();
		if(Settings::instance()->mwResizable() && showGlass())
		{
			float f = fabsf(r_zoom-mg->dstRadiusT());
			if(_dst_resizing_touch == NULL && f<0.03)
			{
				//_dst_resizing_touch = p;
				//qDebug() << "setting resizing point to" << p->id();
				//return;
			}
		}
		if(showGlass())
		{
			if(r_zoom < mg->dstRadiusT() || force_glass)
			{
				qDebug() << "inserting owner for " << p->id() << mg;
				_owning_magnifying_glasses[p->id()] = mg;
				transformTouchPoint(p);
				_start_positions[p->id()] = p->startPos();
				_force_detached = false;
				logger() << "mwc_press inside" << this << p->id();
			}
			else
			{
				if(r_base < _mgm->baseGlass()->dstRadiusT()*4 && _locking_touch == NULL)
				{
					_locking_touch = p;
					logger() << "mwc_press locking" << this << p->id();
				}
				else
				if(_dst_resizing_touch == NULL)
				{
					_dst_resizing_touch = p;
					QPointF diff = p->pos() - _mgm->baseGlass()->dstCenterT();
					_mgm->setDesiredAngle(atan2(-diff.y(), diff.x()) * 180/3.1415f);
					_mgm->setDesiredMaximalVariation(0);
					request_update = true;
					logger() << "mwc_press resizing" << this << p->id();
				}
				else
				{
					; // stupid touch, dont do anything
				}
			}
		}
		else
		{
			// two-stage process: first, do the old transformation and see if we hit an old halo.
			// if yes, then we keep this one. otherwise, we assume that the user wanted a angle change.
			// at this occasion, we can as well just clean up the rwc list.
			bool accept = false;
			QMutableListIterator<RigidWidgetController *> i(_active_halos);
			while (i.hasNext())
			{
				RigidWidgetController * rwc = i.next();
				if (!rwc->isHaloVisible()) {
					i.remove();
				}
				else
				{
					// if we already found an RWC, then we're just interested in cleaning up
					if(accept) continue;

					float dist = QLineF(mg->transform(p->pos()), rwc->pos()).length();

					// HACK: assuming point cursor
					if(dist < rwc->totalRadius())
					{
						// we go for it
						accept = true;
					}
				}
			}

			// if not, shit happens, we change the angle
			if(!accept)
			{
				//QPointF diff = p->pos() - _mgm->baseGlass()->dstCenterT();
				//_mgm->setDesiredAngle(atan2(-diff.y(), diff.x()) * 180/3.1415f);
				//_mgm->setDesiredMaximalVariation(0);

				// HACK: taken from somewhere above (moved)
				QLineF base_center_to_p(_mgm->magnifyingGlasses().front()->srcCenterT(), p->pos());
				float base_radius = _mgm->magnifyingGlasses().front()->srcRadiusT();
				qreal mag_radius = (base_center_to_p.length() - base_radius);
				mag_radius = qMax(_mgm->baseGlass()->dstRadius(), mag_radius);
				_mgm->magnifyingGlasses().back()->setDstCenter(p->pos());
				_mgm->magnifyingGlasses().back()->setDstRadius(mag_radius);
				logger() << "mwc_press invisible_resize" << this << p->id();
				//_mgm->magnifyingGlasses().back()->setDstRadiusD(mag_radius);

				//qDebug() << "desiring" << mag_radius;
				//updateMagnifyingGlasses();
			}

			// do the usual acceptance stuff
			_owning_magnifying_glasses[p->id()] = mg;
			transformTouchPoint(p);
			_start_positions[p->id()] = p->startPos();
			logger() << "mwc_press invisible_inside" << this << p->id();
		}
	}
}

// --------------------------------------------------------
void MagnifyingWidgetController::touchPointPressed( SceneTouchPoint * p )
{
	touchPointPressed(p, false);
}
// --------------------------------------------------------
bool MagnifyingWidgetController::acceptTouchPoint( const SceneTouchPoint & p ) const
{
	Q_ASSERT(numSpanningPoints()>0);

	// if we only have one spanning point right now, we want a second one asap
	if(acceptAsSecondSpanningPoint(p))
	{
		qDebug() << "accepting for spanning: " << &p;
		return true;
	}

	// reject everything in the base glass half-plane
	//QPair<QPointF, QPointF> spanning_positions = spanningPositions();
	//QPointF lower_spanning_point;
	//QPointF higher_spanning_point;
	//if(spanning_positions.first.y() < spanning_positions.second.y())
	//{
	//	lower_spanning_point = spanning_positions.second;
	//	higher_spanning_point = spanning_positions.first;
	//}
	//else
	//{
	//	lower_spanning_point = spanning_positions.first;
	//	higher_spanning_point = spanning_positions.second;
	//}
	//float angle = QLineF(lower_spanning_point, higher_spanning_point).angleTo(QLineF(lower_spanning_point, p.pos()));
	//float angle = QLineF(mg->srcCenterT(), mg->dstCenterT()).angleTo(QLineF(), p.pos()));
	


	//if(showGlass())
	//{
	//	if(isOnBaseGlassSide(p))
	//	{
	//		if(_locking_touch == NULL)
	//		{
	//			return true;
	//		}
	//		return false;
	//	}
	//}



	//// for resizable widgets, we allow one single touch outside any glass for resizing
	//if(Settings::instance()->mwResizable() && showGlass())
	//{
	//	float f = fabsf(QLineF(p.pos(), single_mg->dstCenterT()).length()-single_mg->dstRadiusT());
	//	qDebug() << f;
	//	if(_dst_resizing_touch == NULL && f<0.075)
	//	{
	//		qDebug() << "accepting for resizingG: " << &p << f;
	//		return true;
	//	}
	//}

	//// other than that, we only accept touches inside any magnifying glass
	//const MagnifyingGlass * mg = mgm->magnifyingGlassByScreenPos(p.pos());
	//qDebug() << "maybe accepting for glass: " << &p << mg;
	//return mg != NULL;
	return true;
}

// --------------------------------------------------------
void MagnifyingWidgetController::updateMagnifyingGlasses()
{
	if(numSpanningPoints() == 2)
	{
		updateSpanningDelta();
	}

	QPair<QPointF, QPointF> spanning_positions = spanningPositions();
	
	// update position, angle and radius
	if(_mgm == NULL)
	{
		_mgm = new MagnifyingGlassManager(
			(spanning_positions.first+spanning_positions.second)/2,
			1.0f,
			_tool_setting_container.floatProperty("aspect_ratio"),
			_tool_setting_container.floatProperty("angle"),
			_initial_zoom_glass
		);
		request_update = true;
		if(_enqueued_pole != NULL)
		{
			_mgm->setPole(*_enqueued_pole);
			_enqueued_pole = NULL;
		}
	}

	_mgm->setSrcByPointPair(spanning_positions.first, spanning_positions.second);

	// jun 7
	//float initial_zoom_factor = _tool_setting_container.floatProperty("initial_ratio");
	_mgm->singleMagnifyingGlass()->setDstRadiusD(QLineF(spanning_positions.first, spanning_positions.second).length()*_desired_ratio/2);
	//_mgm->singleMagnifyingGlass()->setDstRadius(QLineF(spanning_positions.first, spanning_positions.second).length()*initial_zoom_factor/2);

	_mgm->updateDstPositions(!detached() && showGlass());

	// for resizable glasses, the dst radius doesn't get updated on
	// every src move operation, so we need to initialize it here.
	if(Settings::instance()->mwResizable())
	{
		// resizable with more than one magnifying glass is not defined yet
		Q_ASSERT(Settings::instance()->mwGlassCount()==1);
		MagnifyingGlass * mw = _mgm->singleMagnifyingGlass();

		// check if dst radius is uninitialized yet
		if(mw->dstRadiusD() == 0.0f)
		{  
			_mgm->singleMagnifyingGlass()->setDstRadiusD(QLineF(spanning_positions.first, spanning_positions.second).length()*_desired_ratio/2);
			_mgm->singleMagnifyingGlass()->setDstRadius(QLineF(spanning_positions.first, spanning_positions.second).length()*_desired_ratio/2);
			_mgm->updateDstPositions();
		}
	}

	logger() << "mwc_update" << this
		<< _mgm->singleMagnifyingGlass()->dstCenterT()
		<< _mgm->singleMagnifyingGlass()->dstRadiusT()
		<< _mgm->baseGlass()->dstCenterT()
		<< _mgm->baseGlass()->dstRadiusT();
}

// --------------------------------------------------------
void MagnifyingWidgetController::addSpanningPoint( SceneTouchPoint * p )
{
	Q_ASSERT(numSpanningPoints()<2);
	logger() << "mwc_press spanning" << this << p->id();
	if(spanning_points.first == NULL)
	{
		spanning_points.first = p;
		p->setVisiblityInMagnification(SceneTouchPoint::NeverVisible);
	}
	else
	{
		spanning_points.second = p;
		p->setVisiblityInMagnification(SceneTouchPoint::NeverVisible);
	}
}

// --------------------------------------------------------
bool MagnifyingWidgetController::isSpanningPoint( const SceneTouchPoint & p ) const
{
	if(spanning_points.first != NULL && p.id() == spanning_points.first->id()) return true;
	if(spanning_points.second != NULL && p.id() == spanning_points.second->id()) return true;
	return false;
}

// --------------------------------------------------------
bool MagnifyingWidgetController::forwardTouchPoint( const SceneTouchPoint & p ) const
{
	return !isSpanningPoint(p) && _dst_resizing_touch != &p;
}

// --------------------------------------------------------
void MagnifyingWidgetController::transformTouchPoint( SceneTouchPoint * p )
{
	Q_ASSERT(_owning_magnifying_glasses.contains(p->id()));
	const MagnifyingGlass * mg = _owning_magnifying_glasses.value(p->id());
	
	// perform size transformation
	QSizeF new_size = p->rect().size() / mg->ratio();

	// perform position transformation (would be nicer with 3x3 matrix)
	//QPointF pos = p->pos() - mg->dstCenter();
	QPointF pos = mg->transform(p->pos());
	QPointF start_pos;
	if(_start_positions.contains(p->id()))
	{
		start_pos = _start_positions.value(p->id());
	}
	else
	{
		start_pos = mg->transform(p->pos());
	}

	//p->point()->setRect(QRectF(QPointF(), new_size)); // gets retranslated anyway by setpos
	//p->point()->setPos(pos);
	//p->point()->setStartPos(start_pos);
	p->transform(pos, start_pos, new_size);

	//	// tra rot sca
	//p->setPos(QPointF(0.5f, 0.5f));
}

// --------------------------------------------------------
QList<int> MagnifyingWidgetController::innerTouchIds() const
{
	return _owning_magnifying_glasses.keys();
}

// --------------------------------------------------------
//float MagnifyingWidgetController::cdRatioOfPoint( int id ) const
//{
//	Q_ASSERT(_owning_magnifying_glasses.contains(id));
//	return _owning_magnifying_glasses.value(id)->ratio();
//}

// --------------------------------------------------------
void MagnifyingWidgetController::timeStep()
{
	TouchWidgetController::timeStep();
	//qDebug() << "updating";
	if(request_update)
	{
		updateMagnifyingGlasses();
		request_update = false;
	}
}

// --------------------------------------------------------
QPair<QPointF, QPointF> MagnifyingWidgetController::spanningPositions() const
{
	QPointF pos1, pos2;
	//Q_ASSERT(spanning_points.first==NULL || spanning_points.first->isPressed());
	//Q_ASSERT(spanning_points.second==NULL || spanning_points.second->isPressed());
	Q_ASSERT(spanning_points.first!=NULL || spanning_points.second!=NULL);
	//if(spanning_points.first!=NULL && spanning_points.second!=NULL)
	//{
	//	pos1 = spanning_points.first->transformedPoint()->pos();
	//	pos2 = spanning_points.second->transformedPoint()->pos();
	//}
	//else
	//{
		if(spanning_points.first!=NULL)
		{
			pos1 = spanning_points.first->pos();
			pos2 = pos1+pos_delta;
		}
		else
		//if(spanning_points.second!=NULL)
		{
			pos2 = spanning_points.second->pos();
			pos1 = pos2-pos_delta;
		}
	//}
	return qMakePair(pos1, pos2);
}

// --------------------------------------------------------
int MagnifyingWidgetController::numSpanningPoints() const
{
	int cnt = 0;
	if(spanning_points.first != NULL) ++cnt;
	if(spanning_points.second != NULL) ++cnt;
	return cnt;
}

// --------------------------------------------------------
float MagnifyingWidgetController::spanningDiameter() const
{
	QPair<QPointF, QPointF> spanning_pos = spanningPositions();
	return QLineF(spanning_pos.first, spanning_pos.second).length();
}

// --------------------------------------------------------
QPointF MagnifyingWidgetController::virtualSpanningPointPos() const
{
	Q_ASSERT(numSpanningPoints() == 1);
	if(spanning_points.first == NULL)
	{
		return spanning_points.second->pos()-pos_delta;
	}
	else
	{
		return spanning_points.first->pos()+pos_delta;
	}
}

// --------------------------------------------------------
void MagnifyingWidgetController::updateSpanningDelta()
{
	Q_ASSERT(numSpanningPoints() == 2);
	QPointF pos1 = spanning_points.first->pos();
	QPointF pos2 = spanning_points.second->pos();
	pos_delta = pos2-pos1;	
}

// --------------------------------------------------------
void MagnifyingWidgetController::removeSpanningPoint( SceneTouchPoint * p )
{
	if(spanning_points.first == p)
	{
		spanning_points.first = NULL;
	}
	else
	{
		Q_ASSERT(spanning_points.second == p);
		spanning_points.second = NULL;
	}
}

// --------------------------------------------------------
const MagnifyingGlass * MagnifyingWidgetController::magnifyingGlassOfPoint( SceneTouchPoint * p )
{
	//qDebug() << "querying" << p << p->id();
	Q_ASSERT(_owning_magnifying_glasses.contains(p->id()));
	return _owning_magnifying_glasses.value(p->id());
}

// --------------------------------------------------------
bool MagnifyingWidgetController::isYoung() const
{
	return _age.elapsed() < 200;
}

// --------------------------------------------------------
bool MagnifyingWidgetController::showGlass() const
{
	return _tool_setting_container.boolProperty("show_glass");
}

// --------------------------------------------------------
bool MagnifyingWidgetController::isOnBaseGlassSide( const SceneTouchPoint &p ) const
{
	MagnifyingGlass * single_mg = _mgm->singleMagnifyingGlass();
	QPointF connector_point = QLineF(single_mg->srcCenterT(), single_mg->dstCenterT()).unitVector().pointAt(single_mg->srcRadiusT());
	QPointF src_to_connector = connector_point - single_mg->srcCenterT();
	QLineF ortho_line(connector_point, connector_point+QPointF(src_to_connector.y(), -src_to_connector.x()));
	float angle = ortho_line.angleTo(QLineF(connector_point, p.pos()));
	//qDebug() << "angle:" << angle;
	return angle < 180;
}

// --------------------------------------------------------
bool MagnifyingWidgetController::detached()
{
	if(_just_resized) {
		_just_resized = false;
		return false;
	}

	if(_force_detached) {
		//_force_detached = false;
		return true;
	}

	if(!showGlass())
	{
		return false;
	}
	else
	{
		//if(_detached)
		//{
		//	return true;
		//}
		//else
		//{
		//	_detached = !innerTouchIds().empty();
		//	return _detached;
		//}
		if(_tool_setting_container.boolProperty("lazy"))
		{
			return
			(
				magnifyingGlassDistance() >= -0.05f
			&&
				(
					_mgm->singleMagnifyingGlass()->dstRadiusT() >= _mgm->singleMagnifyingGlass()->dstRadiusD()
				||
					_mgm->singleMagnifyingGlassHasMaxSize()
				)
			);
		}
		else
		{
			return !innerTouchIds().empty() && magnifyingGlassDistance() >= 0;
		}
		//return false;


		
		//if(!innerTouchIds().empty())
		//{
	
			//||
			//_dst_resizing_touch != NULL;
		//}
		//else
		//{
		//	return false;
		//}
	}
}

// --------------------------------------------------------
float MagnifyingWidgetController::magnifyingGlassDistance() const
{
	float dist = QLineF(_mgm->baseGlass()->dstCenterT(), _mgm->singleMagnifyingGlass()->dstCenterT()).length();
	return dist - (_mgm->singleMagnifyingGlass()->dstRadiusT() + _mgm->baseGlass()->dstRadiusT());
}

// --------------------------------------------------------
bool MagnifyingWidgetController::isInteriorPoint( const SceneTouchPoint * p ) const
{
	return _owning_magnifying_glasses.contains(p->id());
}

//// --------------------------------------------------------
//const MagnifyingGlass * MagnifyingWidgetController::singleMagnifyingGlass()
//{
//	return mgm->singleMagnifyingGlass();
//}
//void MagnifyingWidgetController::transformTouches( QMap<int, TouchPoint> * touches ) const
//{
//	// todo: do magnifying glass stuff
//}

// --------------------------------------------------------
//const QTouchEvent::TouchPoint & MagnifyingWidgetController::firstSpanningPoint() const
//{
//	return getTouches().value(spanning_point_ids.first);
//}
//
//// --------------------------------------------------------
//const QTouchEvent::TouchPoint & MagnifyingWidgetController::secondSpanningPoint() const
//{
//	return getTouches().value(spanning_point_ids.second);
//}