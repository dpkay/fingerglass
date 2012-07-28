#include "touchwidgetrenderer.h"
#include "..\src\opengl\qglframebufferobject.h"

//GLuint TouchWidgetRenderer::_unit_square_vbo = -1;
GLuint TouchWidgetRenderer::_current_texture_id = -1;

// --------------------------------------------------------
void TouchWidgetRenderer::drawUnitQuad(float alpha) const
{
	glColor4f(1,1,1,alpha);
	//glBegin(GL_QUADS);
	//glTexCoord2f(0,0); glVertex2f(-0.5,-0.5);
	//glTexCoord2f(1,0); glVertex2f(0.5, -0.5);
	//glTexCoord2f(1,1); glVertex2f(0.5, 0.5);
	//glTexCoord2f(0,1); glVertex2f(-0.5, 0.5);
	//glEnd();

	// on first call, initialize vertex buffer object
	static GLuint unit_square_vbo = -1;
	if(unit_square_vbo == -1)
	{
		qDebug() << "generating";
		glGenBuffers(1, &unit_square_vbo);
		glBindBuffer( GL_ARRAY_BUFFER, unit_square_vbo );
		QVector<float> vertices;
		vertices << -0.5 << -0.5 << 0 << 0;
		vertices << 0.5 << -0.5 << 1 << 0 ;
		vertices << 0.5 << 0.5 << 1 << 1;
		vertices << -0.5 << 0.5 << 0 << 1;
		glBufferData( GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW );
	}

	// draw object from vertex buffer object
	glBindBuffer( GL_ARRAY_BUFFER, unit_square_vbo );
	glVertexPointer(2, GL_FLOAT, 4*sizeof(float), (char*)0);
	glTexCoordPointer(2, GL_FLOAT, 4*sizeof(float), (char*)(2*sizeof(float)));
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

// --------------------------------------------------------
void TouchWidgetRenderer::drawTexturedUnitQuad( GLuint tex_id, float alpha ) const
{
	glActiveTexture(GL_TEXTURE0);
	//if(_current_texture_id != tex_id)
	//{
		glBindTexture(GL_TEXTURE_2D, tex_id);
		//_current_texture_id = tex_id;
		//glBindTexture(GL_TEXTURE_2D, 0);
	//}
	drawUnitQuad(alpha);
}

// --------------------------------------------------------
void TouchWidgetRenderer::drawTexturedQuad( GLuint tex_id,
	const QPointF & pos, const QSizeF & size, float rotation,
	float alpha) const
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(pos.x(), pos.y(), 0);
	glRotatef(rotation, 0, 0, 1);
	glScalef(size.width(), size.height(), 0);
	drawTexturedUnitQuad(tex_id, this->alpha() * alpha);
	glPopMatrix();
}

// --------------------------------------------------------
void TouchWidgetRenderer::drawQuad( const QPointF & pos, const QSizeF & size, float rotation, float alpha ) const
{

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(pos.x(), pos.y(), 0);
	glRotatef(rotation, 0, 0, 1);
	glScalef(size.width(), size.height(), 0);
	drawUnitQuad(alpha);
	glPopMatrix();
}

// --------------------------------------------------------
void TouchWidgetRenderer::drawSceneToFbo( QGLFramebufferObject * fbo, const QPointF & pos, const QSizeF & size, float rotation ) const
{
	glViewport(0,0,fbo->size().width(), fbo->size().height());
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-0.5, 0.5, -0.5, 0.5, -1.0, 1.0);
	glScalef(1/size.width(), 1/size.height(),0.0f);
	glRotatef(rotation, 0,0, 1);
	glTranslatef(-pos.x(), -pos.y(), 0.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	fbo->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	emit drawScene(); // TODO: insert layer number
	fbo->release();

	// draw to scene
	emit setupViewport();
}

// --------------------------------------------------------
void TouchWidgetRenderer::drawMagnifyingGlass(
	GLResourceContainer * container,
	const MagnifyingGlass * mg,
	float alpha ) const
{
	// draw contents to fbo
	QGLFramebufferObject * fbo = container->framebufferObject("mg_fbo");
	drawSceneToFbo(fbo,	mg->srcCenter(), mg->srcSize(), mg->angle());

	//qDebug() << "drawing mg " << mg << mg->dstCenter() << mg->dstSize();

	// draw magnifying glass including contents
	QGLShaderProgram * shader = container->shaderProgram("mglass");
	shader->bind();
	shader->setUniformValue("mglass_tex", 0);
	shader->setUniformValue("content_tex", 1);
	shader->setUniformValue("alpha", this->alpha()*alpha);
	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, container->texture("mglass"));
	glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, fbo->texture());
	drawQuad(mg->dstCenter(), mg->dstSize(), -mg->angle(), this->alpha()*alpha);
	shader->release();
}

// --------------------------------------------------------
float TouchWidgetRenderer::alpha() const
{
	return _alpha.value();
}

// --------------------------------------------------------
TouchWidgetRenderer::TouchWidgetRenderer( TouchWidgetController * controller, float alpha ) :
	_alpha(alpha, 0.4f)
{
	_alpha.setTarget(1.0f);
	_controller = controller;
}

// --------------------------------------------------------
TouchWidgetRenderer::~TouchWidgetRenderer()
{
	_alpha.destroy();
}

// --------------------------------------------------------
void TouchWidgetRenderer::drawPrecisionHandle(
	GLResourceContainer * container,
	const QPointF & grip,
	const QPointF & tip,
	float aspect_ratio,
	float alpha ) const
{
	// compute difference
	QLineF diff(grip, tip);
	float length = diff.length();

	// compute affine transformation
	QPointF pos = (grip+tip)/2;
	QSizeF handle_size(length, length * aspect_ratio);
	QSizeF grip_size(length * aspect_ratio * 1.125f, length * aspect_ratio * 1.125f);
	float rotation = atan2f(-diff.dy(),-diff.dx()) / 3.1415f * 180;

	// draw
	drawTexturedQuad(container->texture("precision_handle"), pos, handle_size, rotation, alpha);
	drawTexturedQuad(container->texture("precision_handle_grip"), grip, grip_size, rotation, 1.0f);
}

// --------------------------------------------------------
void TouchWidgetRenderer::drawCross( GLResourceContainer * container, const QPointF & pos, const QSizeF size ) const
{
	//static const ;
	drawTexturedQuad(container->texture("precision_cross"), pos, size);
}

// --------------------------------------------------------
void TouchWidgetRenderer::drawHollowCircle( GLResourceContainer * container, const QPointF & pos, qreal radius, qreal alpha, const QColor & color, qreal thickness, qreal blur_multiplier ) const
{
	QSizeF size(2*radius, 2*radius);
	QGLShaderProgram * shader = container->shaderProgram("circle");
	shader->bind();
	shader->setUniformValue("thickness", 1.0f/(GLfloat)size.width()*(float)thickness);
	shader->setUniformValue("blur", (float) thickness*(float)blur_multiplier);
	//shader->setUniformValue("blur", (float) radius);
	shader->setUniformValue("alpha", (float) (this->alpha()*alpha));
	shader->setUniformValue("color", color);
	drawQuad(pos, size, 0, 1.0f /* ignored by shader anyway */);
	shader->release();
}

// --------------------------------------------------------
void TouchWidgetRenderer::drawResizers(
	GLResourceContainer * container,
	const SceneTouchPoint * resizing_point,
	const QPointF & circle_center,
	float circle_radius,
	const QPointF & base_target,
	QList<float> resizer_angles ) const
{
	static Warping<qreal> resizer_activeness(0.0f, 0.2f);
	static bool resizer_active = false;
	static QPointF last_resizer_pos;
	static float touch_angle_target = 0;

	QPointF base_pos = QLineF(circle_center, base_target).unitVector().pointAt(circle_radius);
	float base_angle = QLineF(base_target, circle_center).angleTo(QLineF(0,0,1,0));

	// trigger the warp towards 1 or 0 if the status just changed and evaluate it
	if((resizing_point != NULL) != resizer_active)
	{
		resizer_active = (resizing_point != NULL);
		resizer_activeness.setTarget(resizer_active ? 1.0f : 0.0f);
	}
	float activeness = resizer_activeness.value();

	// update resizer position and angle based on current touch
	if(resizer_active)
	{
		last_resizer_pos = resizing_point->pos();
		touch_angle_target = QLineF(last_resizer_pos, circle_center).angleTo(QLineF(0,0,1,0));;

		touch_angle_target += 60;
		while(touch_angle_target>120) touch_angle_target-=120;
		while(touch_angle_target<-120) touch_angle_target+=120;
		touch_angle_target -= 60;
	}

	// compute resizer size
	const float resizer_diameter = 0.3f * circle_radius * (activeness*0.5+1);
	QSizeF resizer_size(resizer_diameter, resizer_diameter);

	// draw individual angles
	foreach(float resizer_angle, resizer_angles)
	{
		// compute desired angle
		float blended_angle = (touch_angle_target+resizer_angle)*activeness + resizer_angle*(1-activeness);

		// compute resizer handle location
		QTransform transform;
		transform.translate(circle_center.x(), circle_center.y());
		transform.rotate(blended_angle);
		transform.translate(-circle_center.x(), -circle_center.y());
		QPointF resizer_pos = transform.map(base_pos);

		// draw resizer handle
		if(activeness < 1)
		{
			drawTexturedQuad(container->texture("mglass_resizer"), resizer_pos, resizer_size, blended_angle+base_angle+180);
		}
		if(activeness > 0)
		{
			drawTexturedQuad(container->texture("mglass_resizer_active"), resizer_pos, resizer_size, blended_angle+base_angle+180, activeness);
		}
	}
}


//void TouchWidgetRenderer::drawResizers(
//	GLResourceContainer * container,
//	const SceneTouchPoint * resizing_point,
//	const QPointF & circle_center,
//	float circle_radius,
//	const QPointF & base_target,
//	QList<float> resizer_angles ) const
//{
//	static Warping<qreal> resizer_activeness(0.0f, 0.2f);
//	static bool resizer_active = false;
//	static QPointF last_resizer_pos;
//
//	QPointF base_pos = QLineF(circle_center, base_target).unitVector().pointAt(circle_radius);
//	float base_angle = QLineF(circle_center, base_target).angleTo(QLineF(0,0,1,0));
//
//	// trigger the warp towards 1 or 0 if the status just changed and evaluate it
//	if((resizing_point != NULL) != resizer_active)
//	{
//		resizer_active = (resizing_point != NULL);
//		resizer_activeness.setTarget(resizer_active ? 1.0f : 0.0f);
//	}
//	float activeness = resizer_activeness.value();
//
//	// update resizer position and angle based on current touch
//	if(resizer_active)
//	{
//		last_resizer_pos = resizing_point->pos();
//	}
//	float touch_angle = QLineF(last_resizer_pos, circle_center).angleTo(QLineF(0,0,1,0));
//
//	// compute resizer size
//	const float resizer_diameter = 0.3f * circle_radius * (activeness+1);
//	QSizeF resizer_size(resizer_diameter, resizer_diameter);
//
//	// draw individual angles
//	foreach(float resizer_angle, resizer_angles)
//	{
//		// compute desired angle
//		float blended_angle = touch_angle*activeness + resizer_angle*(1-activeness);
//
//		// compute resizer handle location
//		QTransform transform;
//		transform.translate(circle_center.x(), circle_center.y());
//		transform.rotate(blended_angle);
//		transform.translate(-circle_center.x(), -circle_center.y());
//		QPointF resizer_pos = transform.map(base_pos);
//
//		// draw resizer handle
//		if(activeness < 1)
//		{
//			drawTexturedQuad(container->texture("mglass_resizer"), resizer_pos, resizer_size, blended_angle+base_angle);
//		}
//		if(activeness > 0)
//		{
//			drawTexturedQuad(container->texture("mglass_resizer_active"), resizer_pos, resizer_size, blended_angle+base_angle, activeness);
//		}
//	}
//}


