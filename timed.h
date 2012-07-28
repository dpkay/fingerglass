#ifndef timed_h__
#define timed_h__

class Timed
{
public:
	virtual void timeStep() = 0;
	virtual bool finished() const
	{
		return false;
	}
};

#endif // timed_h__