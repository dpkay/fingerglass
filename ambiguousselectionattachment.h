#ifndef ambiguousselectionattachment_h__
#define ambiguousselectionattachment_h__

#include "widgetattachment.h"

class AmbiguousSelectionAttachment : public WidgetAttachment
{
	Q_OBJECT
public:
	AmbiguousSelectionAttachment(QObject * parent) :
		WidgetAttachment(parent, 1000)
	{
		//startClosingTimer(1000);
	}

};

#endif // ambiguousselectionattachment_h__