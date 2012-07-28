#ifndef touchaccepter_h__
#define touchaccepter_h__

class TouchAccepter
{
public:
	TouchAccepter()
	{
		_forward = false;
	}

	virtual ~TouchAccepter() {}

	bool forward() const
	{
		return _forward;
	}

	void setForward(bool forward)
	{
		_forward = forward;
	}

private:
	bool _forward;

};

class HandleWidgetTouchAccepter : public TouchAccepter {};

class HandleWidgetFirstAbsoluteAccepter : public HandleWidgetTouchAccepter {};
class HandleWidgetSecondAbsoluteAccepter : public HandleWidgetTouchAccepter {};

#endif // touchaccepter_h__