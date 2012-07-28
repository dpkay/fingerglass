#include "glresourcecontainer.h"


// --------------------------------------------------------
GLuint GLResourceContainer::texture( const QString & name )
{
	if(!_textures.contains(name))
	{
		emit requestGLResource(GLTexture, name);
		Q_ASSERT(_textures.contains(name));
	}
	return _textures.value(name);
}

// --------------------------------------------------------
GLuint GLResourceContainer::vertexBufferObject( const QString & name )
{
	if(!_vbos.contains(name))
	{
		emit requestGLResource(GLVertexBufferObject, name);
		Q_ASSERT(_vbos.contains(name));
	}
	return _vbos.value(name);
}

// --------------------------------------------------------
QGLShaderProgram * GLResourceContainer::shaderProgram( const QString & name )
{
	if(!_programs.contains(name))
	{
		emit requestGLResource(GLShaderProgram, name);
		Q_ASSERT(_programs.contains(name));
	}
	return _programs.value(name);
}

// --------------------------------------------------------
GLResourceContainer::GLResourceContainer( const QGLContext * context )
{
	// this is a silly Qt feature, exporting the context only as const!
	// HACK
	//_context = const_cast<QGLContext *>(context);
	_unit_square_vbo = -1;
}

// --------------------------------------------------------
QGLFramebufferObject * GLResourceContainer::framebufferObject( const QString & name )
{
	if(!_fbos.contains(name))
	{
		 emit requestGLResource(GLFramebufferObject, name);
		 Q_ASSERT(_fbos.contains(name));
		//QGLFramebufferObject * fbo 
		
	}
	return _fbos[name];
}

// --------------------------------------------------------
void GLResourceContainer::addTexture( const QString & name, GLuint texture )
{
	_textures[name] = texture;
}

// --------------------------------------------------------
void GLResourceContainer::addShaderProgram( const QString & name, QGLShaderProgram * program )
{
	_programs[name] = program;
}

// --------------------------------------------------------
void GLResourceContainer::addFramebufferObject( const QString & name, QGLFramebufferObject * fbo )
{
	_fbos[name] = fbo;
}

// --------------------------------------------------------
void GLResourceContainer::addVertexBufferObject( const QString & name, GLuint vbo )
{
	_vbos[name] = vbo;
}