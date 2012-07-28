#ifndef magnifyingglass_h__
#define magnifyingglass_h__

#include <QRectF>
#include <QMatrix>
#include "Warping.h"

class MagnifyingGlass
{
public:
	// construction
	MagnifyingGlass(qreal major_minor, qreal dst_radius, qreal ratio, const QPointF & init_center);
	~MagnifyingGlass();
	//MagnifyingGlass(QPointF src_center, QPointF dst_center, qreal major_minor, qreal angle, qreal dst_radius, qreal ratio);

	// mapping
	QPointF transform(const QPointF & p) const;

	// warping
	void setSpeed(float speed);
	qreal speed() const;
	void restartWarps();

	// src related
	void setSrcCenter(const QPointF & src_center);
	//void recomputeSrcRect();
	void setSrcRadius(qreal radius, bool maintain_ratio = false);
	void setSrcRadiusByRatio(qreal ratio);
	QSizeF srcSize() const;
	QSizeF srcSizeT() const;
	qreal majorMinor() const;
	QPointF srcCenter() const;
	QPointF srcCenterT() const;
	qreal srcRadius() const;
	qreal srcRadiusT() const;
	qreal ratio() const;

	// dst related
	void setDstCenter(const QPointF & dst_center);
	//void recomputeDstRect() const;
	void setDstRadius(qreal radius, bool maintain_ratio = false);
	void setDstRadiusD(qreal radius); // desired
	QSizeF dstSize() const;
	QSizeF dstSizeT() const; // target
	QPointF dstCenter() const;
	const QPointF & dstCenterT() const;
	qreal dstRadius() const;
	qreal dstRadiusT() const;
	qreal dstRadiusD() const;
	//qreal dstRadiusSq() const;
	QPointF dstRightVector() const;

	// angle related
	void setAngle(qreal angle);
	const QMatrix & rotationMatrix() const;
	const QMatrix & invRotationMatrix() const;
	qreal angle() const;

	// various
	bool contains(const QPointF & p) const;

protected:

private:
	qreal _major_minor;
	qreal _speed;
	//qreal _src_radius;
	//qreal _dst_radius;
	//QPointF _src_center;
	//QPointF _dst_center;
	//QRectF _src_rect;
	//mutable QRectF _dst_rect;
	qreal _angle;
	QMatrix _rotation_matrix;
	QMatrix _inv_rotation_matrix;
	qreal _initial_ratio;

	Warping<qreal> _src_radius_w;
	Warping<qreal> _dst_radius_w;
	Warping<QPointF> _src_center_w;
	Warping<QPointF> _dst_center_w;

	qreal _dst_radius_d; // before optimization
};

#endif // magnifyingglass_h__