#include "handle.h"
#include "magnifyingglassoptimizer.h"

// --------------------------------------------------------
Handle::Handle()
{
	this->_original_pos = NULL;
}

// --------------------------------------------------------
AbsoluteHandle::AbsoluteHandle( const QPointF & pos, const float radius ) :
	_pos_w(pos, 0.5),
	_radius_w(radius, 0.5)
{
	//this->_size = size;
}

// --------------------------------------------------------
ClutchHandle::ClutchHandle( const QPointF & pos, const MagnifyingGlass * mg )
{
	Q_ASSERT(mg != NULL);
 	_mg = new MagnifyingGlass(*mg);
	_mg->setDstCenter(pos);

	//float ratio = _mg->ratio();
	//_mg->setDstRadius(size.width()/2*_mg->ratio());
	//_mg->setSrcRadius(size.width()/2, false);
	_original_dst_radius = mg->dstRadiusT();
	_original_src_radius = mg->srcRadiusT();

	setRadiusMode(SmallRadius);

	// TODO: set speed of mg, and pos, and size
	_mg->setSpeed(0.15f);
	TimeSingleton::instance()->subscribe(this);
}

// --------------------------------------------------------
void ClutchHandle::setRadiusMode( RadiusMode mode )
{
	_radius_mode = mode;
	_mg->setSpeed(0.5f);
	updateSrcRadius();
	updateDstRadius();
}


// --------------------------------------------------------
void ClutchHandle::updateSrcRadius()
{
	float factor;
	if(_radius_mode == OriginalRadius)
	{
		factor = 1.0f;
	}
	else
	{
		factor = 1.0f/3.0f;
	}
	_mg->setSrcRadius(_original_src_radius*factor, false);
}

// --------------------------------------------------------
void ClutchHandle::updateDstRadius()
{
	float factor;
	if(_radius_mode == OriginalRadius)
	{
		factor = 1.0f;
	}
	else
	{
		factor = 1.0f/3.0f;
	}
	_mg->setDstRadius(_original_dst_radius*factor);
}

// --------------------------------------------------------
void Handle::createOriginalPos()
{
	Q_ASSERT(_original_pos == NULL);
	_original_pos = new QPointF(posT());
}

// --------------------------------------------------------
const QPointF & Handle::originalPos() const
{
	Q_ASSERT(_original_pos != NULL);
	return *_original_pos;
}

// --------------------------------------------------------
void Handle::setOriginalPos( const QPointF & pos )
{
	Q_ASSERT(_original_pos != NULL);
	*_original_pos = pos;
}

// --------------------------------------------------------
void Handle::removeOriginalPos()
{
	Q_ASSERT(_original_pos != NULL);
	delete _original_pos;
	_original_pos = NULL;
}

// --------------------------------------------------------
void Handle::resetOriginalPos()
{
	Q_ASSERT(_original_pos != NULL);
	*_original_pos = posT();
}

// --------------------------------------------------------
ClutchHandle::~ClutchHandle()
{
	if(this->_mg != NULL)
	{
		delete this->_mg;
	}
	TimeSingleton::instance()->unsubscribe(this);
}

// --------------------------------------------------------
bool ClutchHandle::finished() const
{
	//qDebug() << "finished?" << _mg->speed();
	//return fabs(_mg->speed()-1.00f)<0.001f;
	return _mg->speed()==FLT_MAX;
}

// --------------------------------------------------------
void ClutchHandle::timeStep()
{
	float new_speed = _mg->speed()+0.02f;
	if(fabs(new_speed-1.0f)<0.02f) new_speed = FLT_MAX;
	_mg->setSpeed(new_speed);
}

// --------------------------------------------------------
void ClutchHandle::setPos( const QPointF & pos )
{
	if(QLineF(_mg->dstCenter(), pos).length()>_mg->dstRadiusT()/2)
	{
		//TimeSingleton::instance()->subscribe(this);
		_mg->setSpeed(0.5f);
	}
	_mg->setDstCenter(pos);
}

// --------------------------------------------------------
void ClutchHandle::goBackIntoBounds(float aspect_ratio)
{
	float scene_margin = (1.0f-(1.0f/aspect_ratio))/2.0f;
	if(posT().x() - _mg->dstRadiusT() < 0.0f)
	{
		setPos(QPointF(_mg->dstRadiusT(), posT().y()));
	}
	if(posT().x() + _mg->dstRadiusT() > 1.0f)
	{
		setPos(QPointF(1.0f - _mg->dstRadiusT(), posT().y()));
	}
	if(posT().y() - _mg->dstRadiusT() < scene_margin)
	{
		setPos(QPointF(posT().x(), scene_margin + _mg->dstRadiusT()));
	}
	if(posT().y() + _mg->dstRadiusT() > 1.0f - scene_margin)
	{
		setPos(QPointF(posT().x(), 1.0f - scene_margin - _mg->dstRadiusT()));
	}
}

// --------------------------------------------------------
void ClutchHandle::optimizePosition(float aspect_ratio)
{
	Q_ASSERT(_radius_mode == OriginalRadius);
	//QPointF c = _clutch_handle->posT();
	//QPointF a = _absolute_handle->posT();

	qDebug() << "pre src pos" << _mg->srcCenterT();
	qDebug() << "pre src radius" << _mg->srcRadiusT();
	qDebug() << "pre dst pos" << _mg->dstCenterT();
	qDebug() << "pre dst radius" << _mg->dstRadiusT();

	QPointF cma = _mg->dstCenterT() - _mg->srcCenterT();
	if(QLineF(_mg->dstCenterT(), _mg->srcCenterT()).length() < _mg->dstRadiusT() + _mg->srcRadiusT())
	{
		qDebug() << "old src pos" << _mg->srcCenterT();
		qDebug() << "old src radius" << _mg->srcRadiusT();
		qDebug() << "old dst pos" << _mg->dstCenterT();
		qDebug() << "old dst radius" << _mg->dstRadiusT();
		MagnifyingGlassOptimizer optimizer(
			_mg,
			aspect_ratio,
			atan2(-cma.y(), cma.x()) * 180 / 3.1415f
			);
		optimizer.optimizeLastGlassPosition();
		qDebug() << "new src pos" << _mg->srcCenterT();
		qDebug() << "new src radius" << _mg->srcRadiusT();
		qDebug() << "new dst pos" << _mg->dstCenterT();
		qDebug() << "new dst radius" << _mg->dstRadiusT();
		//_original_dst_radius = _mg->dstRadiusT(); // is this line required?
	}
}
//
//// --------------------------------------------------------
//float Handle::cdRatio() const
//{
//	//return _cd_ratio;
//	return isAbsolute() ? 1.0f : this->_mg->ratio();
//}
//
//// --------------------------------------------------------
//bool Handle::isAbsolute() const
//{
//	//return _is_absolute;
//	return this->_mg==NULL;
//}
