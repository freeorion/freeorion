#include <log4cpp/Manipulator.hh>
using namespace std;
namespace log4cpp {
	ostream& operator<< (ostream& os, const width& w) {
		if  (os.good()) {
			os.width(w.size);
		}
		return os;
	}
	ostream& operator<< (ostream& os, const tab& t) {
		if  (os.good()) {
			for(size_t no = 0; no < t.size; no++) os.put(os.widen('\t'));
		}
		return os;
	}
};
