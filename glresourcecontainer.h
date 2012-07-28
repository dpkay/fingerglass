#ifndef glresourcecontainer_h__
#define glresourcecontainer_h__

#include <GL/glew.h>
//#include <QtOpenGL>
#include <QGLShaderProgram>

enum GLResourceType {
	GLTexture,
	GLShaderProgram,
	GLFramebufferObject,
	GLVertexBufferObject
};

class GLResourceContainer : public QObject
{
	Q_OBJECT
public:
	GLResourceContainer(const QGLContext * context);

	// query by dictionary
	GLuint texture(const QString & name);
	QGLShaderProgram * shaderProgram(const QString & name);
	QGLFramebufferObject * framebufferObject(const QString & name);
	GLuint vertexBufferObject(const QString & name);

	// query for frequently used objects
	GLuint unitSquareVbo();

	void addTexture(const QString & name, GLuint texture);
	void addShaderProgram(const QString & name, QGLShaderProgram * program);
	void addFramebufferObject(const QString & name, QGLFramebufferObject * fbo);
	void addVertexBufferObject(const QString & name, GLuint vbo);

signals:
	void requestGLResource(GLResourceType type, const QString & name);

private:
	// dictionaries
	QMap<QString, GLuint> _textures;
	QMap<QString, QGLShaderProgram *> _programs;
	QMap<QString, QGLFramebufferObject *> _fbos;
	QMap<QString, GLuint> _vbos;

	// frequently used
	GLuint _unit_square_vbo;

	//QGLContext * _context;
};

inline GLuint GLResourceContainer::unitSquareVbo()
{
	if(_unit_square_vbo == -1)
	{
		//glGenBuffers(1, _unit_square_vbo);
		_unit_square_vbo = vertexBufferObject("unit_square_fbo");
		glBindBuffer( GL_ARRAY_BUFFER, _unit_square_vbo );
		QVector<float> vertices;
		vertices << 0 << 0 << -0.5 << -0.5;
		vertices << 1 << 0 << 0.5 << -0.5;
		vertices << 1 << 1 << 0.5 << 0.5;
		vertices << 0 << 1 << -0.5 << 0.5;
		glBufferData( GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW );
	}
	return _unit_square_vbo;
}

#endif // glresourcecontainer_h__