#ifndef texturedrigidwidgetrenderer_h__
#define texturedrigidwidgetrenderer_h__

#include "touchwidgetrenderer.h"
#include "rigidwidgetcontroller.h"
#include "periodic.h"

class TexturedRigidWidgetRenderer : public TouchWidgetRenderer
{
public:
	TexturedRigidWidgetRenderer(RigidWidgetController * controller, QString texture_name, bool has_active, bool has_highlighted);
	void paint(GLResourceContainer * container, int pass, TouchWidgetRenderer * to_texture_renderer) const;

	const RigidWidgetController * controller() const; 
	bool isSceneDrawer() const { return false; }
	virtual unsigned int numPasses() const { return 3; }

protected:
	void drawAttachments( GLResourceContainer * container, bool * draw_bubble, TouchWidgetRenderer * to_texture_renderer ) const;
	void drawFadingSolidCircleAttachment(GLuint tex_id, WidgetAttachment * wa, float theta, float amplitude, float alpha) const;
	void drawFadingWhiteHollowCircleAttachment(GLResourceContainer * container, WidgetAttachment * wa, float theta, float amplitude, float alpha) const;

private:
	QString _texture_name_normal;
	QString _texture_name_active;
	QString _texture_name_highlighted;

};	

#endif // waypointwidgetrenderer_h__ 