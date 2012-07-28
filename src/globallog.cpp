#include "globallog.h"

GlobalLog * GlobalLog::_instance = 0;

// --------------------------------------------------------
GlobalLog * GlobalLog::instance()
{
	if(_instance == 0)
	{
		_instance = new GlobalLog;
	}

	return _instance;
}

// --------------------------------------------------------
GlobalLog::GlobalLog()
{
	_time = new QTime;
	_time->start();

	// open new log file
  QDir("log").mkpath(".");
	QString file_name = QString("log/full_%1.txt").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
	_log_file = new QFile(file_name);
	if(!_log_file->open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qCritical() << "cannot open file" << file_name << "! leaving...";
		exit(1);
	}
	_log_file_stream = new QTextStream(_log_file);
}

// --------------------------------------------------------
GlobalLog::~GlobalLog()
{
	if(_log_file != NULL)
	{
		_log_file->close();
		delete _log_file;
		delete _log_file_stream;
	}
}

// --------------------------------------------------------
void GlobalLog::writeToLog( const QString & string )
{
	Q_ASSERT(_log_file->isOpen());
	*_log_file_stream << _time->elapsed() << " " << string << "\n";
	_log_file_stream->flush();
}

// --------------------------------------------------------
int GlobalLog::timeElapsed()
{
	return instance()->_time->elapsed();
}