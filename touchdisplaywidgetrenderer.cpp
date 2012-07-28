#include "touchdisplaywidgetrenderer.h"


// --------------------------------------------------------
TouchDisplayWidgetRenderer::TouchDisplayWidgetRenderer(
	RigidWidgetController * controller,
	TouchPointManager * tpm,
	TouchWidgetManager * twm,
	const SettingContainer & setting_container
) :
	TouchWidgetRenderer(controller),
	_tool_setting_container(setting_container)
{
	_tpm = tpm;
	_twm = twm;
}

// --------------------------------------------------------
void TouchDisplayWidgetRenderer::paint( GLResourceContainer * container, int pass, TouchWidgetRenderer * to_texture_renderer ) const
{
	Q_UNUSED(to_texture_renderer);
	if(pass==0)
	{
		// draw line
		if(_twm->currentBallTarget() != NULL)
		{
			const RigidWidgetController * ball = _twm->currentBallTarget()->first;
			const RigidWidgetController * target = _twm->currentBallTarget()->second;
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);
			glLineWidth(2);
			//glColor4f(240.0/255,198.0/255,0,1);
			glColor4f(1,1,1,1);
			glBegin(GL_LINE_STRIP);
			glVertex2f(ball->pos().x(), ball->pos().y());
			glVertex2f(target->pos().x(), target->pos().y());
			glEnd();
		}

		foreach(const SceneTouchPoint * stp, _tpm->touches())
		{
			// draw bubble
			//const QList<RigidWidgetController *> & closest_widgets = stp->closestRigidWidgets();
			//if(closest_widgets.size()>=2)
			//{
			//	// labeled as in the bubble cursor 2005 paper
			//	//float con_d_i = QLineF(closest_widgets[0]->pos(), stp->transformedPoint()->pos()).length() + closest_widgets[0]->radius();
			//	//float int_d_j = QLineF(closest_widgets[1]->pos(), stp->transformedPoint()->pos()).length() - closest_widgets[1]->radius();
			//	float con_d_i = closest_widgets[0]->containmentDistance(stp->transformedPoint()->pos());
			//	float int_d_j = closest_widgets[1]->intersectingDistance(stp->transformedPoint()->pos());
			//	float radius = qMin(con_d_i, int_d_j);
			//	drawTexturedQuad(container->texture("halo"), stp->transformedPoint()->pos(), QSizeF(radius, radius)*2, 0, 0.5f);
			//}

			QRectF bubble_rect = stp->bubbleRect();
			if(!bubble_rect.isEmpty())
			{
				Q_ASSERT(QLineF(stp->pos(), bubble_rect.center()).length() < 0.001f);
				drawTexturedQuad(container->texture("halo"), stp->pos(), bubble_rect.size(), 0, 0.3f);
			}

			// we don't want to show all the gory details in magnifying glasses
			if(to_texture_renderer!=NULL)
			{
				//qDebug() << stp->visibilityInMagnification();
				if(stp->visibilityInMagnification() == SceneTouchPoint::NeverVisible)
					continue;
				if(stp->visibilityInMagnification() == SceneTouchPoint::VisibleOnlyWithAreaCursor)
				{
					if(!stp->isFree() || _twm->selectionMode()!=DirectAreaSelection)
					//if(_twm->selectionMode()!=DirectAreaSelection)
						continue;
				}
			}
			else
			{
				if(!stp->visibleInScene())
					continue;
			}

			if(_tool_setting_container.boolProperty("simplified_mg") && to_texture_renderer!=NULL)
				continue;

			// determine texture name
			QString texture_name("touch_bright/touch");
			if(stp->state() == Qt::TouchPointPressed)
			{
				texture_name += "_new";
			}
			else
			{
				texture_name += (stp->numPressedWidgets() == 0) ? "_free" : "_bound";
				// TODO: SETTING "SHOW YOUNG CURSORS"
				//if(stp->isYoung())
				//{
				//	texture_name += "_young";
				//}
			}
			// geometry

			const QRectF & rect = stp->rect();
			
			if(_tool_setting_container.boolProperty("pointy_cursor"))
			{
				drawTexturedQuad(container->texture(texture_name), rect.center(), rect.size(), 0, 0.25f);
				drawHollowCircle(container, rect.center(), rect.size().width()/2*1.2f, 0.5f, Qt::white, 0.0004f, 300);
				drawCross(container, rect.center(), rect.size()*1.2f);
			}
			else
			{
				drawTexturedQuad(container->texture(texture_name), rect.center(), rect.size(), 0, 0.25f);
				drawHollowCircle(container, rect.center(), rect.size().width()/2*1.2f, 1.0f, Qt::white, 0.0005f, 300);
			}
		}
	}
}


//// --------------------------------------------------------
//void TouchDisplayWidgetRenderer::setScreenSize( const QSize & screen_size )
//{
//	_screen_size = screen_size;
//}