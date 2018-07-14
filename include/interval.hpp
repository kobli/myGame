#ifndef INTERVAL_HPP_17_08_08_18_57_17
#define INTERVAL_HPP_17_08_08_18_57_17 
#include <algorithm>

template <typename T>
class Interval 
{
	public:
		Interval(T a, T b): _begin{std::min(a,b)}, _end{std::max(a,b)} {
		}

		template <typename TT>
			T distanceFrom(TT a) const {
				if(contains(a))
					return 0;
				else
					return std::min(std::abs(a-_begin), std::abs(a-_end));
			}

			T distanceFrom(const Interval<T>& i) const {
				return std::min(distanceFrom(i._begin), distanceFrom(i._end));
			}

			bool contains(T a) const {
				return a >= _begin && a <= _end;
			}

			T length() const {
				return _end-_begin;
			}

	private:
		T _begin;
		T _end;
};
#endif /* INTERVAL_HPP_17_08_08_18_57_17 */
