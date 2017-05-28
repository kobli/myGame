#ifndef ITERATEONLY_HPP_17_05_28_11_45_47
template <typename T>
class IterateOnly {
	public:
		typedef typename T::iterator iterator;

		IterateOnly(T& container): _container{container} {
		}

		iterator begin() {
			return _container.begin();
		}

		iterator end() {
			return _container.end();
		}

	private:
		T& _container;
};
#define ITERATEONLY_HPP_17_05_28_11_45_47 
#endif /* ITERATEONLY_HPP_17_05_28_11_45_47 */
