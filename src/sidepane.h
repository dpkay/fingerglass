#ifndef sidepane_h__
#define sidepane_h__

#include <QListWidget>
#include <QLabel>
#include <QFile>
#include <QComboBox>
#include "sidepaneevent.h"

class SidePane : public QWidget
{
	Q_OBJECT
public:
	SidePane(QWidget * parent = 0);
	~SidePane();
	const int desiredWidth() const { return 300; }
	void selectStudy(const QString & name);
	QString selectedStudy() const;

public slots:
	void sidePaneEvent(SidePaneEvent * event);

private slots:
	void highlightCurrentBallSlot();
	void changeStudySlot(QString name);

signals:
	void highlightCurrentBall();
	void changeStudy(QString name);

protected:
	void taskCreationEvent(SidePaneEvent * event);
	void roundCompletedEvent(SidePaneEvent * event);
	void updateTaskStatistics();
	void updateStatisticsLabel(int index, const QString text, bool visible);
	//void statisticReportEvent(SidePaneEvent * event);

private:
	QListWidget * _lw_log;
	QFile * _log_file;
	QTextStream * _log_file_stream;
	QComboBox * _study_cbx;
	QList<QLabel *> _statistic_labels;
	const TaskStatistics * _task_statistics;

};

#endif // sidepane_h__
