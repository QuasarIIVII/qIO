#include<iostream>
#include"qio.h"
#include<shared_mutex>
#include<condition_variable>
#include"target_sys.h"
#include<unistd.h>
#include<sys/ioctl.h>

int main() {
	qIO::layeredOut lo;
	qIO::layeredOut::layer l = lo.createLayer(qIO::layeredOut::winSize_t{16, 16}, 0);
	l << 1 << 2 << 3 << 4 << 5;
	std::cout<<static_cast<uint16_t>(lo.state())<<std::endl;
	lo.debug();
	std::cout<<sizeof(std::unique_lock<std::mutex>)<<std::endl;
	std::cout << "Hello, World!" << std::endl;
	std::cout<<std::hex<<target_sys::target_sys<<std::endl;
	std::cout<<sizeof(winsize)<<std::endl;
	return 0;
}
