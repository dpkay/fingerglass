#ifndef toptwomap_h__
#define toptwomap_h__

// smallest value = first
// second-smallest value = second
// everything else is discarded.

template <typename K, typename V>
class TopTwoMap
{
public:
	TopTwoMap()
	{
		_has_first = false;
		_has_second = false;
	}

	void insert(const K & key, V value)
	{
		if(!_has_first)
		{
			_first_key = key;
			_first_value = value;
			_has_first = true;
		}
		else
		{
			if(key < _first_key)
			{
				_second_key = _first_key;
				_second_value = _first_value;
				_first_key = key;
				_first_value = value;
			}
			else
			if(!_has_second || key < _second_key)
			{
				_second_key = key;
				_second_value = value;				
			}
			else
			{
				// not in top two, ignore it
			}
			_has_second = true;
		}
	}
	bool hasFirst() const { return _has_first; }
	bool hasSecond() const { return _has_second; }
	V first() { Q_ASSERT(hasFirst()); return _first_value; }
	V second() { Q_ASSERT(hasSecond()); return _second_value; }

	static void debug_Test()
	{
		TopTwoMap<float, float> ttm;
		Q_ASSERT(!ttm.hasFirst() && !ttm.hasSecond());
		ttm.insert(3, 30);
		Q_ASSERT(ttm.hasFirst() && !ttm.hasSecond());
		ttm.insert(8, 80);
		Q_ASSERT(ttm.hasFirst() && ttm.hasSecond());
		ttm.insert(5, 50);
		ttm.insert(4, 40);
		ttm.insert(2, 20);
		Q_ASSERT(ttm.first() == 20);
		Q_ASSERT(ttm.second() == 30);
	}

private:
	bool _has_first;
	bool _has_second;
	K _first_key;
	V _first_value;
	K _second_key;
	V _second_value;
};

#endif // toptwomap_h__


//TopTwoMap<float, RigidWidgetController *>