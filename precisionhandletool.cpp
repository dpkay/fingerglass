#include "precisionhandletool.h"
#include "precisionhandlewidgetrenderer.h"
#include "touchpointmanager.h"
#include "touchwidgetmanager.h"
#include "touchwidget.h"

// --------------------------------------------------------
void PrecisionHandleTool::updateAfterTouchEvent( const QTouchEvent * event )
{
	if(event->touchPointStates() & Qt::TouchPointPressed)
	{
		Q_ASSERT(touchWidgetManager() != NULL && touchPointManager() != NULL);

		foreach(const TouchWidget * tw, touchWidgetManager()->widgets())
		{
			// we only want one dfs tool at the same time for now
			if(dynamic_cast<const PrecisionHandleWidgetController *>(tw->controller()) != NULL)
			{
				return;
			}
		}

		// create one-finger widget
		SceneTouchPoint * stp = getFreeTouch();
		if(stp != NULL)
		{
			// create controller and put touches in the right place
			PrecisionHandleWidgetController * phwc = new PrecisionHandleWidgetController(settingContainer());
			//touchPointManager()->connectDraggingToWidget(stp, phwc);
			PointWidgetConnection * pwc = stp->connectToWidget(phwc);
			pwc->setDragging(true);
			phwc->setDefiningTouch(stp);

			// create renderer and add widget
			PrecisionHandleWidgetRenderer * phwr = new PrecisionHandleWidgetRenderer(phwc);
			TouchWidget * tw = new TouchWidget(phwc, phwr);
			touchWidgetManager()->add(tw);
		}
	}
}

// --------------------------------------------------------
bool PrecisionHandleTool::allowRigidWidgetInteraction( const SceneTouchPoint & stp, const RigidWidgetController & rwc ) const
{
	//Q_UNUSED(stp);
	Q_UNUSED(rwc);
	//return true;
	return stp.isTransformed();
}