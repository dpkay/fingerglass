#ifndef touchlistener_h__
#define touchlistener_h__

#include "scenetouchpoint.h"

class TouchListener
{
public:
	virtual void touchPointPressed(SceneTouchPoint * stp) = 0;
	virtual void touchPointMoved(SceneTouchPoint * stp) = 0;
	virtual void touchPointReleased(SceneTouchPoint * stp) = 0;
};

#endif // touchlistener_h__