#ifndef OBSERVERMOCK_HPP_17_05_25_18_03_23
#define OBSERVERMOCK_HPP_17_05_25_18_03_23 
#include <observer.hpp>
#include <queue>

class Msg
{
	public:
		Msg(int data = 0, bool add=false, bool rem=false);

		bool operator==(const Msg& other) const;

		int _data;
		bool _add;
		bool _rem;
};

std::ostream& operator<<(std::ostream& o, const Msg& m);


template <typename MsgT>
class ObserverMock_: public Observer<MsgT>
{
	public:
		typedef std::queue<MsgT> MessageSequence;
		typedef typename MessageSequence::container_type MsgSeqCont;

		ObserverMock_(MsgSeqCont s = MsgSeqCont()) : _s{s} {
		}

		ObserverMock_& operator=(ObserverMock_&& other) {
			Observer<MsgT>::operator=(std::move(other));
			std::swap(_s, other._s);
			return *this;
		}

		~ObserverMock_() {
			EXPECT_EQ(_s.empty(), true);
		}

		MessageSequence _s;
	private:
		virtual void onObservableAdd(const MsgT& m) {
			onMsg(m);
			//std::cout << "add\n";
		}

		virtual void onObservableUpdate(const MsgT& m) {
			onMsg(m);
		}

		virtual void onObservableRemove(const MsgT& m) {
			onMsg(m);
			//std::cout << "remove\n";
		}

		virtual void onMsg(const MsgT& m) {
			//std::cout << m << std::endl;
			EXPECT_EQ(_s.empty(), false);
			EXPECT_EQ(m, _s.front());
			_s.pop();
		}
};
#endif /* OBSERVERMOCK_HPP_17_05_25_18_03_23 */
