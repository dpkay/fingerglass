#ifndef taskstatistics_h__
#define taskstatistics_h__

struct TaskStatistics
{
	TaskStatistics(int num_trials);

	void addStart(int time_elapsed);
	int averageStartTime() const;

	void addAcquisition(int time_elapsed, bool succeeded);
	int averageAcquisitionTime() const;
	float acquisitionSuccessRate() const;

	void addDrag(int time_elapsed, bool succeeded);
	int averageDraggingTime() const;
	float draggingSuccessRate() const;

	int completedTrials() const;
	int totalTrials() const;

private:
	int _num_trials;
	int _total_start_time;
	int _total_acquisition_time;
	int _total_dragging_time;
	int _total_starts;
	int _succeeded_acquisitions;
	int _failed_acquisitions;
	int _succeeded_drags;
	int _failed_drags;

};

#endif // taskstatistics_h__