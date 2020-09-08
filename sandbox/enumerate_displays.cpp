#include "wtypes.h"

#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>

template<typename T1, typename T2> std::ostream& operator<<(std::ostream& os, const std::pair<T1, T2>& p)
{
	os << "{ first: " << p.first << ", second: " << p.second << "}";
	return os;
}


std::ostream& operator<<(std::ostream& os, const RECT& r)
{
	os << "{ top: " << r.top << ", left: " << r.left << ", bottom: " << r.bottom << ", right:" << r.right << "}";
	return os;
}

template<typename T> std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
{
	os << "[";
	auto last = std::rbegin(v);
	for (auto e = std::begin(v); e != std::end(v); ++e) {
		os << *e;
		if (&*e != &*last) {
			os << ", ";
		}
	}
	os << "]";
	return os;
}


std::vector<RECT> monitors;

BOOL Monitorenumproc(
	HMONITOR Arg1,
	HDC Arg2,
	LPRECT monitor_rect,
	LPARAM Arg4
)
{
	//...

	RECT r;
	std::memcpy(&r, monitor_rect, sizeof(RECT));
	monitors.push_back(r);

	return TRUE;
}

int main()
{
	// this is not an async function, even if the callback would suggest it...
	auto status = EnumDisplayMonitors(NULL, NULL, Monitorenumproc, NULL);
	if (!status) {
		std::cerr << "EnumDisplayMonitors failed!" << std::endl;
		return -1;
	}

	std::cout << monitors << std::endl;
	std::vector<std::pair<unsigned, unsigned>> sizes;
	std::transform(std::begin(monitors), std::end(monitors), std::back_inserter(sizes), 
				   [](auto& r) { return std::make_pair(r.right-r.left, r.bottom-r.top); });
	std::cout << sizes << std::endl;

	return 0;
}
