#ifndef globallog_h__
#define globallog_h__

//#define NO_LOGGING

#include <QObject>
#include <QTime>
#include <QFile>

#include "scenetouchpoint.h"


class GlobalLog
{
public:
	static GlobalLog * instance();
	static int timeElapsed();
	void writeToLog(const QString & string);

private:
	// construction/destruction
	GlobalLog();
	~GlobalLog();

	// singleton
	static GlobalLog * _instance;

	// log management
	QTime * _time;
	QFile * _log_file;
	QTextStream * _log_file_stream;

};


class GlobalLogger
{
	struct Stream {
		Stream() : ts(&buffer, QIODevice::WriteOnly) {}
		QTextStream ts;
		QString buffer;
	} *_stream;

public:
	inline GlobalLogger() { _stream = new Stream; }
	inline ~GlobalLogger()
	{
#ifndef NO_LOGGING
		GlobalLog::instance()->writeToLog(_stream->buffer);
#endif
	}

#ifndef NO_LOGGING
	// primitive types
	inline GlobalLogger &operator<<(const QString & t) { _stream->ts << t << ' '; return *this; }
	inline GlobalLogger &operator<<(const float & t) { _stream->ts << t << ' '; return *this; }
	inline GlobalLogger &operator<<(const qreal & t) { _stream->ts << t << ' '; return *this; }
	inline GlobalLogger &operator<<(const int & t) { _stream->ts << t << ' '; return *this; }

	// complex types
	inline GlobalLogger &operator<<(const SceneTouchPoint & t)
	{
		_stream->ts
			<< t.id() << ' '
			<< t.pos().x() << ' '
			<< t.pos().y() << ' '
			<< t.rect().width() << ' ';
		return *this;
	}
	inline GlobalLogger &operator<<(const QPointF & t)
	{
		_stream->ts
			<< t.x() << ' '
			<< t.y() << ' ';
		return *this;
	}
	
	// pointers
	template<typename T>
	inline GlobalLogger &operator<<(const T * t) { _stream->ts << t << ' '; return *this; }

#else
	template<typename T>
	inline GlobalLogger &operator<<(const T & t) { Q_UNUSED(t); return *this; }
#endif
};

inline GlobalLogger logger() { return GlobalLogger(); }

#endif // globallog_h__