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



class ObserverMock: public Observer<Msg>
{
	public:
		typedef std::queue<Msg> MessageSequence;
		typedef MessageSequence::container_type MsgSeqCont;
		ObserverMock(MsgSeqCont s = MsgSeqCont());

		ObserverMock(ObserverMock&) = delete;
		ObserverMock(ObserverMock&&) = delete; //unnecessary
		ObserverMock& operator=(ObserverMock&& other);

		~ObserverMock();

		MessageSequence _s;
	private:

		virtual void onObservableAdd(Observable_<Msg>& caller, const Msg& m);
		virtual void onObservableUpdate(Observable_<Msg>& caller, const Msg& m);
		virtual void onObservableRemove(Observable_<Msg>& caller, const Msg& m);
		void onMsg(Observable_<Msg>& caller, const Msg& m);
};

typedef ObserverMock::MsgSeqCont MsgSeq;
#endif /* OBSERVERMOCK_HPP_17_05_25_18_03_23 */
