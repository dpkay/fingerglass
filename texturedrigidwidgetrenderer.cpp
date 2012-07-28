#include "texturedrigidwidgetrenderer.h"
#include "widgetattachment.h"
#include "ambiguousselectionattachment.h"
#include "variousattachments.h"

// --------------------------------------------------------
TexturedRigidWidgetRenderer::TexturedRigidWidgetRenderer(
	RigidWidgetController * controller,
	QString texture_name,
	bool has_active,
	bool has_highlighted
) :
	TouchWidgetRenderer(controller, 1.0f)
	//_highlight_opacity(0.25f, 0.30f, 0.05f)
{
	_texture_name_normal = has_active || has_highlighted ? QString("%1_normal").arg(texture_name) : texture_name;
	_texture_name_active = has_active ? QString("%1_active").arg(texture_name) : _texture_name_normal;
	_texture_name_highlighted = has_highlighted ? QString("%1_highlighted").arg(texture_name) : _texture_name_normal;
}

// --------------------------------------------------------
void TexturedRigidWidgetRenderer::paint( GLResourceContainer * container, int pass, TouchWidgetRenderer * to_texture_renderer ) const
{
	Q_UNUSED(to_texture_renderer);
	if(pass == 0)
	{
		bool draw_bubble = true; // HACK: somewhat of a hack at least.. this should not really be decided here.

		// draw halo
		if(controller()->isHaloVisible())
		{
			drawTexturedQuad(container->texture("halo"), controller()->pos(), controller()->totalSize(), 0, 0.3f);
		}

		// draw attachments
		drawAttachments(container, &draw_bubble, to_texture_renderer);

		// draw afterglow bubble
		const AfterGlowBubble * agb = controller()->afterGlowBubble();
		if(agb != NULL && agb->hasBubble() && draw_bubble)
		{
			drawTexturedQuad(container->texture("halo"), agb->bubbleRect().center(), agb->bubbleRect().size(), 0, 0.3f);
		}
	}
	else if(pass == 1)
	{
		// draw selection ring
		if(controller()->isMoving())
		{
			drawTexturedQuad(container->texture("touch_bright/touch_bound"), controller()->pos(), controller()->coreSize()*1.3f,0,1);
		}

		// draw actual widget
		QString texture_name = controller()->isHighlighted() ? _texture_name_highlighted : _texture_name_normal;
		texture_name = controller()->isMoving() ? _texture_name_active : texture_name;
		drawTexturedQuad(container->texture(texture_name), controller()->pos(), controller()->coreSize());
	}
	else if(pass == 2)
	{
		// draw connection to current touch
		if(!controller()->touchPointConnections().empty() && (*controller()->touchPointConnections().begin())->isDragging())
		{
			const QPointF & touch_pos = controller()->singlePressedTouchPointConnection()->point()->pos();
			//drawPrecisionHandle(container, touch_pos, controller()->pos(), 0.3f);
		}
	}
}

// --------------------------------------------------------
void TexturedRigidWidgetRenderer::drawFadingSolidCircleAttachment(GLuint tex_id, WidgetAttachment * wa, float theta, float amplitude, float alpha) const
{
	// this is a normalized gamma function with alpha=2, soon peaking with f(theta)=1, then slowly decaying
	float t = wa->lifeTime();
	float normalization = expf(1)/theta;
	float radius = t*expf(-t/theta) * normalization;

	drawTexturedQuad( tex_id, controller()->pos(), controller()->coreSize()*amplitude*radius, 0,	(1-wa->lifeTime())*alpha );
}

// --------------------------------------------------------
const RigidWidgetController * TexturedRigidWidgetRenderer::controller() const
{
	const RigidWidgetController * rwc = dynamic_cast<const RigidWidgetController *>(TouchWidgetRenderer::controller());
	Q_ASSERT(rwc != NULL);
	return rwc;
}

// --------------------------------------------------------
void TexturedRigidWidgetRenderer::drawFadingWhiteHollowCircleAttachment( GLResourceContainer * container, WidgetAttachment * wa, float theta, float amplitude, float alpha ) const
{
	// this is a normalized gamma function with alpha=2, soon peaking with f(theta)=1, then slowly decaying
	float t = wa->lifeTime();
	float normalization = expf(1)/theta;
	float radius = t*expf(-t/theta) * normalization;

	drawHollowCircle( container, controller()->pos(), controller()->coreRadius()*amplitude*radius, (1-wa->lifeTime())*alpha);
}

// --------------------------------------------------------
void TexturedRigidWidgetRenderer::drawAttachments( GLResourceContainer * container, bool * draw_bubble, TouchWidgetRenderer * to_texture_renderer ) const
{
	foreach(WidgetAttachment * attachment, controller()->attachments())
	{
		NewWidgetAttachment * nba = dynamic_cast<NewWidgetAttachment *>(attachment);
		if(nba != NULL)
		{
			QString texture_name;
			switch(nba->type())
			{
			case NewWidgetAttachment::NewBall:
				texture_name = "halo";
				break;
			case NewWidgetAttachment::NewTarget:
				texture_name = "destination_highlight";
				break;
			default:
				Q_ASSERT(false);
			}
			drawFadingSolidCircleAttachment(container->texture(texture_name), nba, 0.08f, 250, 0.5f);
			continue;
		}
		AmbiguousSelectionAttachment * asa = dynamic_cast<AmbiguousSelectionAttachment *>(attachment);
		if(asa != NULL)
		{
			drawFadingSolidCircleAttachment(container->texture("alert_ambiguous"), asa, 0.08f, 10, 0.5f);
			continue;
		}
		InteractionFinishedAttachment * ifa = dynamic_cast<InteractionFinishedAttachment *>(attachment);
		if(ifa != NULL)
		{
			*draw_bubble = false;
			drawFadingWhiteHollowCircleAttachment(container, ifa, 0.08f, 10, 1.0f);
			if(to_texture_renderer == NULL && ifa->magnifyingGlass() != NULL)
			{
				//drawWhiteCircle(container, ifa->magnifyingGlass()->dstCenter(), ifa->magnifyingGlass()->dstRadius());
				//qDebug() << "drawing mg with radius" << ifa->magnifyingGlass()->dstRadius();
				drawMagnifyingGlass(container, ifa->magnifyingGlass(), qMax(0.0f,1 - ifa->lifeTime()*3));
			}
			continue;
		}
	}
}
//
//// --------------------------------------------------------
//void TexturedRigidWidgetRenderer::setTexture( QString name )
//{
//	_texture_name = name;
//}