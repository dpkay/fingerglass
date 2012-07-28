/*
 * Copyright (c) 2009 Nokia Corporation.
 */

#include "squareitemwithsidepanelayout.h"
#include "glwidget.h"
#include "sidepane.h"

SquareItemWithSidePaneLayout::SquareItemWithSidePaneLayout(
	QWidget* parent,
    int spacing
)
: QLayout(parent) {
	init(spacing);
}

SquareItemWithSidePaneLayout::SquareItemWithSidePaneLayout(int spacing) {
	init(spacing);
}

SquareItemWithSidePaneLayout::~SquareItemWithSidePaneLayout() {
	delete glwidget_item;
	delete sidepane_item;
	delete lastReceivedRect;
	delete _geometry;
}

void SquareItemWithSidePaneLayout::init(int spacing) {
	glwidget_item = NULL;
	sidepane_item = NULL;
	lastReceivedRect = new QRect(0, 0, 0, 0);
	_geometry = new QRect(0, 0, 0, 0);
	setSpacing(spacing);
}

/* Adds item if place isn't already taken. */
void SquareItemWithSidePaneLayout::add(QLayoutItem* item) {
	if(!hasItemOfType(item)) {
		replaceItem(item);
	}
}

/* Adds item if place isn't already taken. */
void SquareItemWithSidePaneLayout::addItem(QLayoutItem* item) {
	if(!hasItemOfType(item)) {
		replaceItem(item);
	}
}

/* Adds widget if place isn't already taken. */
void SquareItemWithSidePaneLayout::addWidget(QWidget* widget) {
	if(!hasItemOfType(widget)) {
		replaceItem(new QWidgetItem(widget));
	}
}


/* Returns the item pointer and dereferences it here. */
QLayoutItem* SquareItemWithSidePaneLayout::takeAt(int index) {
	QLayoutItem* item = NULL;
	switch(index)
	{
	case 0:
		if(this->glwidget_item != NULL)
		{
			item = this->glwidget_item;
			this->glwidget_item = NULL;
		}
		break;
	case 1:
		if(this->sidepane_item != NULL)
		{
			item = this->sidepane_item;
			this->sidepane_item = NULL;
		}
		break;
	}

	return item;
}

/* Returns the item pointer. */
QLayoutItem* SquareItemWithSidePaneLayout::itemAt(int index) const {
	if(index == 0 && this->glwidget_item!=NULL)
	{
		return this->glwidget_item;
	}
	if(index == 1 && this->sidepane_item!=NULL)
	{
		return this->glwidget_item;
	}
	return NULL;
}

/* Checks if we have an item. */
bool SquareItemWithSidePaneLayout::hasItemOfType(QLayoutItem * item) const {
	Q_ASSERT(item != NULL);
	return hasItemOfType(item->widget());
	//GLWidget * glwidget = dynamic_cast<GLWidget *>(item->widget());
	//SidePane * sidepane = dynamic_cast<SidePane *>(item->widget());
	//Q_ASSERT(glwidget != NULL || sidepane != NULL);
	//if(glwidget != NULL)
	//{
	//	return glwidget_item == NULL;
	//}
	//if(sidepane != NULL)
	//{
	//	return sidepane_item == NULL;
	//}
}

bool SquareItemWithSidePaneLayout::hasItemOfType(QWidget * widget) const {
	Q_ASSERT(widget != NULL);
	GLWidget * glwidget = dynamic_cast<GLWidget *>(widget);
	SidePane * sidepane = dynamic_cast<SidePane *>(widget);
	Q_ASSERT(glwidget != NULL || sidepane != NULL);
	if(glwidget != NULL)
	{
		return glwidget_item != NULL;
	}
	if(sidepane != NULL)
	{
		return sidepane_item != NULL;
	}
	Q_ASSERT(false);
	return false;
}

/* Returns the count of items which can be either 0 or 1. */
int SquareItemWithSidePaneLayout::count() const {
	int returnValue = 0;
	if(glwidget_item != NULL) ++returnValue;
	if(sidepane_item != NULL) ++returnValue;
	return returnValue;
}

/* Replaces the item with the new and returns the old. */
QLayoutItem* SquareItemWithSidePaneLayout::replaceItem(QLayoutItem* item) {
	QLayoutItem* old = 0;
	
	Q_ASSERT(item != NULL);
	GLWidget * glwidget = dynamic_cast<GLWidget *>(item->widget());
	SidePane * sidepane = dynamic_cast<SidePane *>(item->widget());
	Q_ASSERT(glwidget != NULL || sidepane != NULL);
	if(glwidget != NULL)
	{
		old = this->glwidget_item;
		this->glwidget_item = item;
	}
	if(sidepane != NULL)
	{
		old = this->sidepane_item;
		this->sidepane_item = item;
	}

	setGeometry(*this->_geometry);
	return old;
}

/* Tells which way layout expands. */
Qt::Orientations SquareItemWithSidePaneLayout::expandingDirections() const {
	return Qt::Horizontal | Qt::Vertical;
}

/* Tells which size is preferred. */
QSize SquareItemWithSidePaneLayout::sizeHint() const {
	//return this->item->minimumSize();
	//int widgth 
	return minimumSize();
}

/* Tells minimum size. */
QSize SquareItemWithSidePaneLayout::minimumSize() const {
	//return this->item->minimumSize();
	static int width = 0;
	static int height = 0;
	if(this->glwidget_item != NULL && this->sidepane_item != NULL)
	{
		width = this->glwidget_item->minimumSize().width() + this->sidepane_item->minimumSize().width();
		height = qMax(this->glwidget_item->minimumSize().height(), this->sidepane_item->minimumSize().height());
	}
	return QSize(width, height);
}

/*
 * Tells if heightForWidth calculations is handled.
 * It isn't since width isn't enough to calculate
 * proper size.
 */
bool SquareItemWithSidePaneLayout::hasHeightForWidth() const {
	return false;
}

/* Replaces lastReceivedRect. */
void SquareItemWithSidePaneLayout::setLastReceivedRect(const QRect& rect) {
	QRect* oldRect = this->lastReceivedRect;
	this->lastReceivedRect = new QRect(rect.topLeft(), rect.size());
	delete oldRect;
}

/* Returns geometry */
QRect SquareItemWithSidePaneLayout::geometry() {
	return QRect(*this->_geometry);
}

/* Sets geometry to given size. */
void SquareItemWithSidePaneLayout::setGeometry(const QRect& rect) {
	/*
	 * We check if the item is set and
	 * if size is the same previously received.
	 * If either is false nothing is done.
	 */
	if(
		!this->hasBothItems() ||
		areRectsEqual(*this->lastReceivedRect, rect))
	{
		return;
	}
	/* Replace the last received rectangle. */
	setLastReceivedRect(rect);

	// calculate proper size
	const int sidepane_width = dynamic_cast<SidePane *>(sidepane_item->widget())->desiredWidth();
	//const int total_width = rect.width() - sidepane_width;
	
	// calculate all the sizes
	int height;
	QPoint pos;

	float aspect_ratio=1.5;

	if(rect.height()*aspect_ratio < rect.width() - sidepane_width)
	{
		height = rect.height() - this->margin();
		pos.setX(rect.width()/2 - (height*aspect_ratio + sidepane_width)/2);
	}
	else
	{
		height = (rect.width() - sidepane_width - this->margin())/aspect_ratio;
		pos.setY(rect.height()/2 - height/2);
	}
	QSize glwidget_size(height*aspect_ratio, height);
	QSize sidepane_size(sidepane_width, height);
	QSize total_size(height + sidepane_width, height);

	// calculate positions
	QPoint glwidget_pos(pos);
	QPoint sidepane_pos(pos + QPoint(height*aspect_ratio, 0));
	QPoint total_pos(glwidget_pos);

	// backup
	QRect* oldRect = this->_geometry;
	delete oldRect;

	// set geometries
	this->glwidget_item->setGeometry(QRect(glwidget_pos, glwidget_size));
	this->sidepane_item->setGeometry(QRect(sidepane_pos, sidepane_size));
	this->_geometry = new QRect(total_pos, total_size);
	QLayout::setGeometry(*this->_geometry);
}


/* Takes the shortest side and creates QSize
 * with the shortest side as width and height. */
//QSize SingleItemSquareLayout::calculateProperSize(QSize from) const {
//	QSize properSize;
//	if(from.height() < from.width()) {
//		properSize.setHeight(from.height() - this->margin());
//		properSize.setWidth(from.height() - this->margin());
//	}
//	else {
//		properSize.setWidth(from.width() - this->margin());
//		properSize.setHeight(from.width() - this->margin());
//	}
//	return properSize;
//}
//
///* Calculates center location from the given height and width for item size. */
//QPoint SingleItemSquareLayout::calculateCenterLocation(QSize from,
//                                                        QSize itemSize) const {
//	QPoint centerLocation;
//	if(from.width() - from.width()/2 - itemSize.width()/2 > 0) {
//		centerLocation.setX(from.width() -
//		                    from.width()/2 -
//		                    itemSize.width()/2);
//	}
//	if(from.height() - from.height()/2 - itemSize.height()/2 > 0) {
//		centerLocation.setY(from.height() -
//		                    from.height()/2 -
//		                    itemSize.height()/2);
//	}
//	return centerLocation;
//}

/* Compares if two QRects are equal. */
bool SquareItemWithSidePaneLayout::areRectsEqual(const QRect& a,
                                            const QRect& b) const {
	bool result = false;
	if(a.x() == b.x() &&
	   a.y() == b.y() &&
	   a.height() == b.height() &&
	   a.width() == b.width()) {
		result = true;
	}
	return result;
}

// --------------------------------------------------------
bool SquareItemWithSidePaneLayout::hasBothItems() const
{
	return this->glwidget_item!=NULL && this->sidepane_item!=NULL;
}