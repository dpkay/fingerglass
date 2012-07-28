#include "afterglowbubble.h"
#include <QLineF>

int AfterGlowBubble::_life_span = 0;

// --------------------------------------------------------
const QRectF & AfterGlowBubble::bubbleRect() const
{
	Q_ASSERT(hasBubble());
	return *_bubble_rect;
}

// --------------------------------------------------------
const QTime & AfterGlowBubble::time() const
{
	return _time;
}

// --------------------------------------------------------
AfterGlowBubble::AfterGlowBubble( QRectF * bubble_rect )
{
	if(bubble_rect != NULL)
	{
		this->_bubble_rect = new QRectF(*bubble_rect);
	}
	else
	{
		this->_bubble_rect = NULL;
	}
	this->_time.start();
}

// --------------------------------------------------------
bool AfterGlowBubble::expired() const
{
	Q_ASSERT(_life_span != 0);
	return _time.elapsed() > _life_span;
}

// --------------------------------------------------------
bool AfterGlowBubble::intersectsCircle( const QPointF & p, float radius ) const
{
	// assume round circle
	Q_ASSERT(hasBubble());
	Q_ASSERT(bubbleRect().width() == bubbleRect().height());
	return QLineF(bubbleRect().center(), p).length() < bubbleRect().width()/2 + radius;
}

// --------------------------------------------------------
void AfterGlowBubble::moveTo( const QPointF & p)
{
	Q_ASSERT(hasBubble());
	this->_bubble_rect->moveCenter(p);
}

// --------------------------------------------------------
void AfterGlowBubble::restartTimer()
{
	this->_time.start();
}

// --------------------------------------------------------
bool AfterGlowBubble::hasBubble() const
{
	return this->_bubble_rect != NULL;
}

// --------------------------------------------------------
AfterGlowBubble::~AfterGlowBubble()
{
	delete _bubble_rect;
}