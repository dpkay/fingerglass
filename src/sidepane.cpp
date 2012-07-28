#include "sidepane.h"
#include "sidepaneevent.h"
#include "globallog.h"
#include <QtGui>

// --------------------------------------------------------
void SidePane::sidePaneEvent( SidePaneEvent * event )
{
	_lw_log->addItem(event->timedScreenLogString());
	//_lw_log->selec
	_lw_log->scrollToBottom();
	taskCreationEvent(event);
	roundCompletedEvent(event);
}

// --------------------------------------------------------
SidePane::SidePane( QWidget * parent /*= 0*/ ) :
	QWidget(parent)
{
	QGridLayout * sidepane_layout = new QGridLayout;
	this->setLayout(sidepane_layout);

	// diro
	QDir dir("studies");
	dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
	dir.setNameFilters(QStringList("*.xml"));
	dir.setSorting(QDir::Name);
	QStringList xml_list = dir.entryList();
	xml_list.replaceInStrings(QRegExp("^(.*)\\.xml"), "\\1");

	// cbx
	_study_cbx = new QComboBox(this);
	QFont fnt("Myriad Pro", 16, QFont::Normal);
	_study_cbx->setFont(fnt);
	_study_cbx->addItems(xml_list);
	sidepane_layout->addWidget(_study_cbx);
	connect(_study_cbx, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeStudySlot(QString)));

	for(int i=0; i<6; ++i)
	{
		QLabel * lbl = new QLabel(this);
		lbl->setFont(fnt);
		lbl->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
		sidepane_layout->addWidget(lbl);
		lbl->hide();
		_statistic_labels << lbl;
	}

	// initialize log list widget
	_lw_log = new QListWidget(this);
	//_lw_log->setEnabled(false);
	_lw_log->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sidepane_layout->addWidget(_lw_log);

	// button
	QPushButton * btn = new QPushButton("SHOW CURRENT BALL", this);
	btn->setMinimumHeight(100);
	btn->setMaximumHeight(100);
	btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	sidepane_layout->addWidget(btn);
	connect(btn, SIGNAL(pressed()), this, SLOT(highlightCurrentBallSlot()));

	//// label
	//QLabel * lbl = new QLabel("empty", this);
	//QFont fnt("Myriad Pro", 24, QFont::Bold);
	//lbl->setFont(fnt);
	//lbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	//sidepane_layout->addWidget(lbl);

	// initialize fields
	_log_file = NULL;
	_log_file_stream = NULL;
	_task_statistics = NULL;
}

// --------------------------------------------------------
SidePane::~SidePane()
{
	delete _lw_log;
	if(_log_file != NULL)
	{
		_log_file->close();
		delete _log_file;
		delete _log_file_stream;
	}
}

// --------------------------------------------------------
void SidePane::taskCreationEvent( SidePaneEvent * event )
{
	// check if this is the right event
	TaskCreationEvent * tce = dynamic_cast<TaskCreationEvent *>(event);
	if(tce == NULL)
	{
		return;
	}

	// deal with old file if there is any
	if(_log_file != NULL)
	{
		_log_file->close();
		delete _log_file;
		delete _log_file_stream;
	}

	// open new log file
  QDir("log").mkpath(".");
	QString file_name = QString("log/%1_%2.txt").arg(tce->prefix()).arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
	_log_file = new QFile(file_name);
	if(!_log_file->open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qCritical() << "cannot open file" << file_name << "! leaving...";
		exit(1);
	}
	_log_file_stream = new QTextStream(_log_file);

	// store reference to statistics
	_task_statistics = tce->taskStatistics();
}

// --------------------------------------------------------
void SidePane::roundCompletedEvent( SidePaneEvent * event )
{
	// check if this is the right event
	RoundCompletedEvent * sce = dynamic_cast<RoundCompletedEvent *>(event);
	if(sce == NULL)
	{
		return;
	}

	Q_ASSERT(_log_file->isOpen());
	*_log_file_stream << GlobalLog::timeElapsed() << " " << sce->fileLogString() << "\n";
	_log_file_stream->flush();

	updateTaskStatistics();
}

// --------------------------------------------------------
void SidePane::highlightCurrentBallSlot()
{
	emit highlightCurrentBall();
}

// --------------------------------------------------------
void SidePane::changeStudySlot( QString name )
{
	emit changeStudy(name);
}

// --------------------------------------------------------
void SidePane::selectStudy( const QString & name )
{
	_study_cbx->setEditText(name);
}

// --------------------------------------------------------
QString SidePane::selectedStudy() const
{
	return _study_cbx->currentText();
}

// --------------------------------------------------------
void SidePane::updateTaskStatistics()
{
	Q_ASSERT(_task_statistics != NULL);

	updateStatisticsLabel(0,
		QString("Trials completed: <b>%1</b>/%2").arg(_task_statistics->completedTrials()).arg(_task_statistics->totalTrials()),
		true);

	updateStatisticsLabel(1,
		QString("Avg Start Time: <b>%1</b> ms").arg(_task_statistics->averageStartTime()),
		_task_statistics->averageStartTime() != -1);

	updateStatisticsLabel(2,
		QString("Avg Acquisition Time: <b>%1</b> ms").arg(_task_statistics->averageAcquisitionTime()),
		_task_statistics->averageAcquisitionTime() != -1);

	updateStatisticsLabel(3,
		QString("Avg Dragging Time: <b>%1</b> ms").arg(_task_statistics->averageDraggingTime()),
		_task_statistics->averageDraggingTime() != -1);

	updateStatisticsLabel(4,
		QString("Acquisition Succ. Rate: <b>%1</b>%").arg(_task_statistics->acquisitionSuccessRate()*100, 0, 'f', 1),
		_task_statistics->acquisitionSuccessRate() != -1);

	updateStatisticsLabel(5,
		QString("Dragging Succ. Rate: <b>%1</b>%").arg(_task_statistics->draggingSuccessRate()*100, 0, 'f', 1),
		_task_statistics->draggingSuccessRate() != -1);

	//_statistic_labels.value(0)->setText(QString("Avg Start Time: <b>%1</b> ms").arg(_task_statistics->averageStartTime()));
	//_statistic_labels.value(1)->setText(QString("Avg Acquisition Time: <b>%1</b> ms").arg(_task_statistics->averageAcquisitionTime()));
	//_statistic_labels.value(2)->setText(QString("Avg Dragging Time: <b>%1</b> ms").arg(_task_statistics->averageDraggingTime()));
	//_statistic_labels.value(3)->setText(QString("Acquisition Success Rate: <b>%1</b>%").arg(_task_statistics->acquisitionSuccessRate()/100, 0, 'f', 1));
	//_statistic_labels.value(4)->setText(QString("Dragging Success Rate: <b>%1</b>%").arg(_task_statistics->draggingSuccessRate()/100, 0, 'f', 1));
}

// --------------------------------------------------------
void SidePane::updateStatisticsLabel( int index, const QString text, bool visible )
{
	_statistic_labels.value(index)->setText(text);
	if(_statistic_labels.value(index)->isVisible() != visible)
	{
		_statistic_labels.value(index)->setVisible(visible);
	}
}
//// --------------------------------------------------------
//void SidePane::statisticReportEvent( SidePaneEvent * event )
//{
//	// check if this is the right event
//	StatisticReportEvent * sre = dynamic_cast<StatisticReportEvent *>(event);
//	if(sre == NULL)
//	{
//		return;
//	}
//
//	_statistic_labels.value(0)->setText("Avg Start Time: <b>534</b> ms");
//	_statistic_labels.value(1)->setText("Avg Acquisition Time: <b>534</b> ms");
//	_statistic_labels.value(2)->setText("Avg Dragging Time: <b>534</b> ms");
//	_statistic_labels.value(3)->setText("Acquisition Success Rate: <b>534</b> ms");
//	_statistic_labels.value(4)->setText("Dragging Success Rate: <b>534</b> ms");
//}
