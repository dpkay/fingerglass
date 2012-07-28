#include "dualfingerstretchtool.h"
#include "dualfingerstretchwidgetcontroller.h"
#include "dualfingerstretchwidgetrenderer.h"
#include "touchpointmanager.h"
#include "touchwidget.h"
#include "touchwidgetmanager.h"

// --------------------------------------------------------
void DualFingerStretchTool::updateAfterTouchEvent( const QTouchEvent * event )
{
	if(event->touchPointStates() & Qt::TouchPointPressed)
	{
		Q_ASSERT(touchWidgetManager() != NULL && touchPointManager() != NULL);

		foreach(const TouchWidget * tw, touchWidgetManager()->widgets())
		{
			// we only want one dfs tool at the same time for now
			if(dynamic_cast<const DualFingerStretchWidgetController *>(tw->controller()) != NULL)
			{
				return;
			}
		}

		// create one-finger widget
		SceneTouchPoint * stp = getFreeTouch();
		if(stp != NULL)
		{
			// create controller and put touches in the right place
			DualFingerStretchWidgetController * dfswc = new DualFingerStretchWidgetController(settingContainer());
			//touchPointManager()->connectDraggingToWidget(stp, dfswc);
			PointWidgetConnection * pwc = stp->connectToWidget(dfswc);
			pwc->setDragging(true);
			stp->pushToFront(pwc);
			dfswc->setDefiningTouch(stp);
			dfswc->touchPointPressed(stp);

			// create renderer and add widget
			DualFingerStretchWidgetRenderer * swr = new DualFingerStretchWidgetRenderer(dfswc);
			TouchWidget * tw = new TouchWidget(dfswc, swr);
			touchWidgetManager()->add(tw);
		}
	}
}

// --------------------------------------------------------
bool DualFingerStretchTool::allowRigidWidgetInteraction( const SceneTouchPoint & stp, const RigidWidgetController & rwc ) const
{
	Q_UNUSED(stp);
	Q_UNUSED(rwc);
	return true;
}

