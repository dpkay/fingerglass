#include "magnifyingglass.h"


// --------------------------------------------------------
QPointF MagnifyingGlass::transform( const QPointF & p ) const
{
	QPointF pos = p - dstCenterT();
	pos = invRotationMatrix().map(pos);
	pos.setX(pos.x() * srcSizeT().width() / dstSizeT().width());
	pos.setY(pos.y() * srcSizeT().height() / dstSizeT().height());
	pos = rotationMatrix().map(pos);
	pos += srcCenterT();
	return pos;
}

// --------------------------------------------------------
void MagnifyingGlass::setSpeed( float speed )
{
	Q_ASSERT((0 <= speed && speed <= 1) || speed==FLT_MAX);
	if(speed==FLT_MAX)
	{
		_src_radius_w.setSpeed(1.0f);
		_dst_radius_w.setSpeed(1.0f);
		_src_center_w.setSpeed(1.0f);
		_dst_center_w.setSpeed(1.0f);
	}
	else
	{
		_src_radius_w.setSpeed(0.7f * speed);
		_dst_radius_w.setSpeed(0.7f * speed);
		_src_center_w.setSpeed(0.5f * speed);
		_dst_center_w.setSpeed(0.5f * speed);
	}
	_speed = speed;
}

// --------------------------------------------------------
MagnifyingGlass::MagnifyingGlass( qreal major_minor, qreal dst_radius, qreal ratio, const QPointF & init_center ) :
	_src_radius_w(0, 1.0f),
	_dst_radius_w(0, 1.0f),
	_src_center_w(init_center, 1.0f),
	_dst_center_w(init_center, 1.0f)
{
	//setSrcCenter(init_center);
	//setDstCenter(QPointF());
	setSpeed(1.0f);
	setAngle(0);
	setSrcRadius(dst_radius / ratio, false);
	setDstRadius(dst_radius);
	setDstRadiusD(dst_radius);
	this->_initial_ratio = ratio;
	this->_major_minor = major_minor;
}
// --------------------------------------------------------
//MagnifyingGlass::MagnifyingGlass( QPointF src_center, QPointF dst_center, qreal major_minor, qreal angle, qreal dst_radius, qreal ratio )
//{
//	setSrcCenter(src_center);z
//	setDstCenter(dst_center);
//	//this->_angle = angle;
//	setAngle(angle);
//	this->_src_radius = dst_radius / ratio;
//	this->_dst_radius = dst_radius;
//	this->_major_minor = major_minor;
//}

// --------------------------------------------------------
void MagnifyingGlass::setSrcCenter( const QPointF & src_center )
{
	//this->_src_center = src_center;
	this->_src_center_w.setTarget(src_center);
	//recomputeSrcRect();
}


// --------------------------------------------------------
void MagnifyingGlass::setDstCenter( const QPointF & dst_center )
{
	//this->_dst_center = dst_center;
	this->_dst_center_w.setTarget(dst_center);
	//recomputeDstRect();
}

// --------------------------------------------------------
void MagnifyingGlass::setAngle( qreal angle )
{
	this->_angle = angle;
	_inv_rotation_matrix.reset();
	_inv_rotation_matrix.rotate(angle);
	_rotation_matrix.reset();
	_rotation_matrix.rotate(-angle);
}

// --------------------------------------------------------
const QMatrix & MagnifyingGlass::rotationMatrix() const
{
	return _rotation_matrix;
}

// --------------------------------------------------------
const QMatrix & MagnifyingGlass::invRotationMatrix() const
{
	return _inv_rotation_matrix;
}

// --------------------------------------------------------
QSizeF MagnifyingGlass::srcSize() const
{
	//return _src_rect.size();
	return QRectF(srcCenter().x() - srcRadius() / _major_minor, srcCenter().y() - srcRadius(), 2*srcRadius() / _major_minor, 2*srcRadius()).size();
}

// --------------------------------------------------------
QSizeF MagnifyingGlass::srcSizeT() const
{
	//return _src_rect.size();
	return QRectF(srcCenterT().x() - srcRadiusT() / _major_minor, srcCenterT().y() - srcRadiusT(), 2*srcRadiusT() / _major_minor, 2*srcRadiusT()).size();
}

// --------------------------------------------------------
QSizeF MagnifyingGlass::dstSize() const
{
	//return QSizeF(_dst_rect.right()-_dst_rect.left()
	//recomputeDstRect(); // HACK
	//return _dst_rect.size();
	return QRectF(dstCenter().x() - dstRadius() / _major_minor, dstCenter().y() - dstRadius(), 2*dstRadius() / _major_minor, 2*dstRadius()).size();
}

// --------------------------------------------------------
QSizeF MagnifyingGlass::dstSizeT() const
{
	//return QSizeF(_dst_rect.right()-_dst_rect.left()
	//recomputeDstRect(); // HACK
	//return _dst_rect.size();
	//qDebug() << "radiust" << dstRadiusT() << "radiusd" << dstRadiusD();
	return QRectF(dstCenterT().x() - dstRadiusT() / _major_minor, dstCenterT().y() - dstRadiusT(), 2*dstRadiusT() / _major_minor, 2*dstRadiusT()).size();
}

// --------------------------------------------------------
qreal MagnifyingGlass::srcRadius() const
{
	return _src_radius_w.value();
}

// --------------------------------------------------------
qreal MagnifyingGlass::srcRadiusT() const
{
	return _src_radius_w.targetValue();
}

// --------------------------------------------------------
qreal MagnifyingGlass::dstRadius() const
{
	//return _dst_radius;
	return _dst_radius_w.value();
}

// --------------------------------------------------------
qreal MagnifyingGlass::dstRadiusT() const
{
	//return _dst_radius;
	return _dst_radius_w.targetValue();
}

//// --------------------------------------------------------
//qreal MagnifyingGlass::dstRadiusSq() const
//{
//	return dstRadius() * dstRadius();
//}

// --------------------------------------------------------
qreal MagnifyingGlass::ratio() const
{
	if(dstRadiusT() == 0)
	{
		return _initial_ratio;
	}
	//qDebug() << this << "ratio" << dstRadiusT() << srcRadiusT() << dstRadiusT() / srcRadiusT();
	return dstRadiusT() / srcRadiusT();
}

// --------------------------------------------------------
qreal MagnifyingGlass::angle() const
{
	return _angle;
}

// --------------------------------------------------------
qreal MagnifyingGlass::majorMinor() const
{
	return _major_minor;
}

//// --------------------------------------------------------
//QPointF MagnifyingGlass::srcCenter() const
//{
//	return _src_center;
//}
QPointF MagnifyingGlass::srcCenter() const
{
	return _src_center_w.value();
}

//// --------------------------------------------------------
QPointF MagnifyingGlass::srcCenterT() const
{
	return _src_center_w.targetValue();
}

// --------------------------------------------------------
QPointF MagnifyingGlass::dstCenter() const
{
	return _dst_center_w.value();
}

// --------------------------------------------------------
const QPointF & MagnifyingGlass::dstCenterT() const
{
	return _dst_center_w.targetValue();
}

// --------------------------------------------------------
void MagnifyingGlass::setSrcRadius( qreal radius, bool maintain_ratio )
{
	// ratio should stay constant, dst radious should adapt
	qreal ratio = this->ratio();
	_src_radius_w.setTarget(radius);
	//recomputeSrcRect();
	if(maintain_ratio)
	{
		setDstRadius(srcRadiusT() * ratio);
		//recomputeDstRect();
	}
}
//
//// --------------------------------------------------------
//void MagnifyingGlass::recomputeSrcRect()
//{
//	this->_src_rect = QRectF(_src_center.x() - _src_radius / _major_minor, _src_center.y() - _src_radius, 2*_src_radius / _major_minor, 2*_src_radius);
//}

//// --------------------------------------------------------
//void MagnifyingGlass::recomputeDstRect() const
//{
//	//this->_dst_rect = QRectF(_dst_center.x() - dstRadius() / _major_minor, _dst_center.y() - dstRadius(), 2*dstRadius() / _major_minor, 2*dstRadius());
//}

// --------------------------------------------------------
QPointF MagnifyingGlass::dstRightVector() const
{
	return rotationMatrix().map(QPointF(dstSizeT().width() / 2, 0.0f));
}

// --------------------------------------------------------
void MagnifyingGlass::setDstRadius( qreal radius, bool maintain_ratio )
{
	//qDebug() << "setradius" << radius;
	// src radius should stay constant, ratio should adapt
	//_dst_radius = radius;
	qreal ratio = this->ratio();
	_dst_radius_w.setTarget(radius);
	if(maintain_ratio)
	{
		setSrcRadius(dstRadiusT()/ratio);
	}
	//recomputeDstRect();
}

// --------------------------------------------------------
MagnifyingGlass::~MagnifyingGlass()
{
	_dst_radius_w.destroy();
	_dst_center_w.destroy();
}

// --------------------------------------------------------
void MagnifyingGlass::setDstRadiusD( qreal radius )
{
	//qDebug() << "setradiusD" << radius;
	_dst_radius_d = radius;
}

// --------------------------------------------------------
qreal MagnifyingGlass::dstRadiusD() const
{
	return _dst_radius_d;
}

// --------------------------------------------------------
qreal MagnifyingGlass::speed() const
{
	return _speed;
}

// --------------------------------------------------------
void MagnifyingGlass::setSrcRadiusByRatio( qreal ratio )
{
	Q_ASSERT(ratio != 0);
	setSrcRadius(dstRadiusT()/ratio, false);
}

// --------------------------------------------------------
void MagnifyingGlass::restartWarps()
{
	_src_radius_w.restart();
	_dst_radius_w.restart();
	_src_center_w.restart();
	_dst_center_w.restart();
}

// --------------------------------------------------------
bool MagnifyingGlass::contains( const QPointF & p ) const
{
	return QLineF(dstCenterT(), p).length() < dstRadiusT();
}