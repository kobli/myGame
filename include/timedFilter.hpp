#ifndef TIMEDFILTER_HPP_17_06_28_20_18_58
#define TIMEDFILTER_HPP_17_06_28_20_18_58 
#include <functional>

template <typename T>
class TimedFilter
{
	public:
		typedef std::function<T&(T& oldObj, T& newObj)> Chooser;
		typedef std::function<bool(const T&)> Acceptor;
		TimedFilter(float period,
				Chooser choose = [](T& /*oldObj*/, T& newObj)->T&{ return newObj; },
			 	Acceptor accept = [](const T&){ return true; }):
		 	_time{0}, _period{period}, _choose{choose}, _accept{accept}, _updated{false}
		{
		}

		bool filter(T obj)
		{
			if(!_accept(obj))
				return false;
			else {
				T newObj = _choose(_obj, obj);
				if(newObj != _obj) {
					_updated = true;
					_obj = newObj;
				}
				return true;
			}
		}

		bool tick(float delta)
		{
			_time += delta;
			return _time >= _period;
		}

		T objUpdated()
		{
			return _updated;
		}

		T get()
		{
			return _obj;
		}

		T reset()
		{
			_time = 0;
			_updated = false;
			return _obj;
		}

	private:
		T _obj;
		float _time;
		float _period;
		Chooser _choose;
		Acceptor _accept;
		bool _updated;
};

#endif /* TIMEDFILTER_HPP_17_06_28_20_18_58 */
