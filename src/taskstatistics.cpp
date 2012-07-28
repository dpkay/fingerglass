#include "taskstatistics.h"

// --------------------------------------------------------
void TaskStatistics::addStart( int time_elapsed )
{
	_total_start_time += time_elapsed;
	++_total_starts;
}

// --------------------------------------------------------
int TaskStatistics::averageStartTime() const
{
	if(_total_starts > 0)
	{
		return static_cast<float>(_total_start_time)/_total_starts;
	}
	else
	{
		return -1;
	}
}

// --------------------------------------------------------
void TaskStatistics::addAcquisition( int time_elapsed, bool succeeded )
{
	if(succeeded)
	{
		++_succeeded_acquisitions;
		_total_acquisition_time += time_elapsed;
	}
	else
	{
		++_failed_acquisitions;
	}
}

// --------------------------------------------------------
int TaskStatistics::averageAcquisitionTime() const
{
	//int denominator = _succeeded_acquisitions+_failed_acquisitions;
	int denominator = _succeeded_acquisitions;
	if(denominator > 0)
	{
		return static_cast<float>(_total_acquisition_time)/denominator;
	}
	else
	{
		return -1;
	}
}

// --------------------------------------------------------
float TaskStatistics::acquisitionSuccessRate() const
{
	int denominator = _succeeded_acquisitions+_failed_acquisitions;
	if(denominator > 0)
	{
		return static_cast<float>(_succeeded_acquisitions)/denominator;
	}
	else
	{
		return -1;
	}
}

// --------------------------------------------------------
void TaskStatistics::addDrag( int time_elapsed, bool succeeded )
{
	if(succeeded)
	{
		++_succeeded_drags;
		_total_dragging_time += time_elapsed;
	}
	else
	{
		++_failed_drags;
	}
}

// --------------------------------------------------------
int TaskStatistics::averageDraggingTime() const
{
	//int denominator = _succeeded_drags+_failed_drags;
	int denominator = _succeeded_drags;
	if(denominator > 0)
	{
		return static_cast<float>(_total_dragging_time)/denominator;
	}
	else
	{
		return -1;
	}
}

// --------------------------------------------------------
float TaskStatistics::draggingSuccessRate() const
{
	int denominator = _succeeded_drags+_failed_drags;
	if(denominator > 0)
	{
		return static_cast<float>(_succeeded_drags)/denominator;
	}
	else
	{
		return -1;
	}
}

// --------------------------------------------------------
TaskStatistics::TaskStatistics(int num_trials)
{
	this->_num_trials = num_trials;
	this->_total_start_time = 0;
	this->_total_acquisition_time = 0;
	this->_total_dragging_time = 0;
	this->_total_starts = 0;
	this->_succeeded_acquisitions = 0;
	this->_failed_acquisitions = 0;
	this->_succeeded_drags = 0;
	this->_failed_drags = 0;
}

// --------------------------------------------------------
int TaskStatistics::completedTrials() const
{
	//return _succeeded_acquisitions + _failed_acquisitions;
	return _succeeded_drags;
}

// --------------------------------------------------------
int TaskStatistics::totalTrials() const
{
	return _num_trials;
}