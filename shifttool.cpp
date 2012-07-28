#include "shifttool.h"
#include "shiftpluswidgetcontroller.h"
#include "shiftpluswidgetrenderer.h"
#include "touchpointmanager.h"
#include "touchwidget.h"
#include "touchwidgetmanager.h"

// --------------------------------------------------------
void ShiftTool::updateAfterTouchEvent( const QTouchEvent * event )
{
	if(event->touchPointStates() & Qt::TouchPointPressed)
	{
		Q_ASSERT(touchWidgetManager() != NULL && touchPointManager() != NULL);

		if(touchWidgetManager()->hasWidgetOfType<ShiftPlusWidgetController>()) return;

		// create one-finger widget
		SceneTouchPoint * stp = getFreeTouch();
		if(stp != NULL)
		{
			// create controller and put touches in the right place
			ShiftPlusWidgetController * swc = new ShiftPlusWidgetController(settingContainer());
			touchPointManager()->connectDraggingToWidget(stp, swc);
			swc->setDefiningTouch(stp);
			//swc->updateMagnifyingGlasses();

			// create renderer and add widget
			ShiftPlusWidgetRenderer * swr = new ShiftPlusWidgetRenderer(swc);
			TouchWidget * tw = new TouchWidget(swc, swr);
			touchWidgetManager()->add(tw);
		}
	}
}

// --------------------------------------------------------
bool ShiftTool::allowRigidWidgetInteraction( const SceneTouchPoint & stp, const RigidWidgetController & rwc ) const
{
	Q_UNUSED(stp);
	Q_UNUSED(rwc);
	//return touchWidgetManager()->hasWidgetOfType<ShiftPlusWidgetController>();
	return true;
}

