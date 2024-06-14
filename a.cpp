//pneumonoconiosis

#include <iostream>
#include"qio.h"
#include<cwchar>
#include<thread>
#include<initializer_list>

template<class T>
struct Point{
	T a,b;
};
template<class T>
void g(Point<int> p){
	std::cout<<p.a<<std::endl;
}
void printColor(std::initializer_list<char> a){
	for(auto& e : a){
		std::cout<<e<<' ';
	}
	std::cout<<std::endl;
}
int main() {
	std::chrono::steady_clock::duration dur(std::chrono::seconds(1));
	std::cout<<++dur<<std::endl;

	return 0;
//	std::setlocale(LC_ALL, "en_US.utf16");

//	wchar_t* ws=L"가a";
//	const wchar_t* ws=L"\x1fa75";
//	wchar_t* ws=L"\uac00";

//	const char* s="\xf0\x9f\xa9\xb5";
//	char* s="가";
//	std::cout<<s<<std::endl;

//	std::wcout<<ws<<std::endl;
//	std::cout<<wcswidth(ws,wcslen(ws))<<std::endl;
//	auto t=qIO::layeredOut::cvtUtf8To32((const char8_t[]){0xf0,0x9f,0xa9,0xb5});
//	std::cout<<std::hex<<static_cast<uint32_t>(t.first)<<' '<<t.second<<std::endl;
//	std::cout<<std::hex<<(uint32_t)s[0]<<' '<<(uint32_t)s[1]<<std::endl;
//	std::cout<<wcwidth(0xac00)<<std::endl;
//	return 0;
	qIO::layeredOut a(0x908080);
	a.init<ptrdiff_t>();
	auto layer0=a.makeLayer({50,6},(ptrdiff_t)3);
	auto layer1=a.makeLayer({50,6},(ptrdiff_t)33);
	layer0.setPos({0,0});
//	std::cout<<layer0.getPrecedence<size_t>()<<std::endl;
	layer0.setPrecedence((ptrdiff_t)32);
//	std::cout<<layer0.getPrecedence<size_t>()<<std::endl;

	++layer0({0,0})<<a.color(0xffeeee,1)<<"abababab"<<1212121212
	<<a.color(0x92cbf3,0xffffff)<<a.pos(0,0)<<"\n가가가가가\n나\n1ñ"<<=std::endl;
	layer0<<std::endl;

	layer1({0,1})<<a.color(0x808080,5)<<"abababab"<<1212121212
	<<a.color(0x92cbf3,0x808080)<<a.pos(0,1)<<"\n가가가가가\n나\n1ñ"<<std::endl;
//	layer0<<(qIO::pos2d<size_t>){3,4};
//	layer0({1,2})<<a.color(3,4)<<"\v\v\v"<<std::flush;
//	layer0<<"asdf"<<std::flush;

	uint32_t fgColor=0xABCDEF;
	printColor({
		static_cast<char>(fgColor/6553600+0x30),
		static_cast<char>(fgColor%6553600/655360+0x30),
		static_cast<char>((fgColor%655360>>16)+0x30),
		';',
		static_cast<char>((fgColor&=0xffff)%65536/25600+0x30),
		static_cast<char>(fgColor%25600/2560+0x30),
		static_cast<char>((fgColor%2560>>8)+0x30),
		';',
		static_cast<char>((fgColor&=0xff)/100+0x30),
		static_cast<char>(fgColor%100/10+0x30),
		static_cast<char>(fgColor%10+0x30),
		'm'
	});

	std::cin.get();

	a.begin();
//	std::cout<<"A"<<std::endl;

	for(size_t i=0;i<10;++i){
		layer0.setPos({i,0});
		layer1.setPos({125-i,0});
		a.runWorkerRenderer();
		a.runWorkerPrinter();
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	std::cin.get();

	a.stop();
	return 0;

	layer0.setPos({3,1});
	layer0.setPrecedence((ptrdiff_t)-1);


	a.runWorkerRenderer();
	a.print();
	std::cout<<std::endl;

//	std::this_thread::sleep_for(std::chrono::seconds(1));

	std::cout<<layer0.getlen()<<std::endl;

//	auto layer0=a.makeLayer<size_t>((qIO::size2d<size_t>){17,20},order,98);
//	layer0<<"b"<<std::flush;

//	auto layer1=a.makeLayer<size_t>((qIO::size2d<size_t>){17,20},order,32);
//	std::cout<<static_cast<int32_t>(a.f(layer0,layer1,order.compare))<<std::endl;
//
//	std::cout<<"\033[?1049h\033[1;1H"<<std::flush;
//	for(int i=0;i<256;i++){
//		std::cout<<"\033[38;2;"<<i<<";255;255mAA\033[0m"<<std::flush;
//	}
//	std::cin.get();
//	std::cout<<"\033[?1049l"<<std::flush;
//
//	std::locale::global(std::locale("en_US.utf-8"));
//	std::cout<<wcwidth(L'A')<<' '<<wcwidth(L'﷽')<<' '<<wcwidth(L'\t')<<std::endl;
}
