#ifndef variousattachments_h__
#define variousattachments_h__

#include "widgetattachment.h"
#include "magnifyingglass.h"

class InteractionFinishedAttachment : public WidgetAttachment
{
public:
	InteractionFinishedAttachment(QObject * parent, MagnifyingGlass * mg = NULL) :
		WidgetAttachment(parent, 1000)
	{
		_mg = mg;
	}

	~InteractionFinishedAttachment()
	{
		if(_mg != NULL)
		{
			delete _mg;
		}
	}

	const MagnifyingGlass * magnifyingGlass() { return _mg; }

private:
	MagnifyingGlass * _mg;

};

class NewWidgetAttachment : public WidgetAttachment
{
public:
	enum NewWidgetType
	{
		NewBall,
		NewTarget
	};

	NewWidgetAttachment(QObject * parent, NewWidgetType new_widget_type) :
		WidgetAttachment(parent, 750)
	{
		_new_widget_type = new_widget_type;
	}

	NewWidgetType type() const { return _new_widget_type; }

private:
	NewWidgetType _new_widget_type;

};

#endif // interactionfinishedattachment_h__