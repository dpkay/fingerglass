#include "touchaccepter.h"

void testTouchAccepter()
{
	TouchAccepter * accepter = new HandleWidgetFirstAbsoluteAccepter();
	accepter->setForward(true);

	if(dynamic_cast<HandleWidgetFirstAbsoluteAccepter *>(accepter) != NULL)
	{
		// ...
	}
	else
	if(dynamic_cast<HandleWidgetSecondAbsoluteAccepter *>(accepter) != NULL)
	{
		// ...
	}
}


/*
	TouchAccepter * acceptTouchPoint() const;
	void touchPointPressed(SceneTouchPoint * stp, const TouchAccepter * ta);
	void touchPointMoved(SceneTouchPoint * stp);
	void touchPointReleased(SceneTouchPoint * stp);

	...

	foreach(TouchWidgetController * twc, pressed_widgets)
	{
		TouchAccepter * accepter = twc->acceptTouchPoint(stp);
		if(accepter != NULL)
		{
			touchPointPressed(SceneTouchPoint * stp);
			if(!accepter->forward())
			{
				continue;
			}
			delete accepter;
		}
	}
*/
	
	