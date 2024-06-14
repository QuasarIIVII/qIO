#include"qio.h"
#include<typeinfo>
#include<limits>
#include<cstring>
//
//#include<iostream>
//
//
//template<typeInt precedenceType>
//const unsigned int __layeredOut<precedenceType>::nThread=std::thread::hardware_concurrency();
//
//
//template<typeInt precedenceType>
//int __layeredOut<precedenceType>::begin(){
//	return 0;
//}
//
//template<typeInt precedenceType>
//__layeredOut<precedenceType>::layer __layeredOut<precedenceType>::makeLayer(
//__layeredOut<precedenceType>::size2d size,precedenceType precedence){
//	std::vector<std::vector<std::atomic<charData>>> v;
//	try{
//		v=std::vector<std::vector<std::atomic<charData>>>(
//			size.width,
//			std::vector<std::atomic<charData>>(
//				size.height,
//				std::vector<uint32_t>()
//			)
//		);
//	}catch(const std::bad_alloc& e){
//	}catch(...){
//	}
////	layerList.push_back(std::move((Layer){}));
//}
//
//template<typeInt precedenceType>
//__layeredOut<precedenceType>::layer::layer(std::list<Layer>::iterator iterLayer, __layeredOut::_key&_){
//	
//}

#include<thread>

using namespace qIO;

//definition of layeredOut

#ifndef _WIN32
#include<unistd.h>
#include<sys/ioctl.h>

size2d<size_t> __layeredOut::getWinSize() noexcept{
	struct winsize winsz;
	if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsz))
		return std::move((size2d<size_t>){0,0});
	return std::move((size2d<size_t>){winsz.ws_col,winsz.ws_row});
}

#else
#include<windows.h>
size2d<size_t> __layeredOut::getWinSize() noexcept{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if(GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)){
		return std::move((size2d<size_t>){
			csbi.srWindow.Right - csbi.srWindow.Left + 1,
			csbi.srWindow.Bottom - csbi.srWindow.Top + 1});
	}
	return std::move((size2d<size_t>){0,0});
}
#endif

const unsigned int __layeredOut::nThread=std::thread::hardware_concurrency();

__layeredOut::charInfo::charInfo():mtx(){}
__layeredOut::charInfo::charInfo(const __layeredOut::charInfo& a):mtx(){}
__layeredOut::charInfo& __layeredOut::charInfo::operator=(const __layeredOut::charInfo& a){return *this;}

void __layeredOut::init(){
	winSize=getWinSize();
	std::cout<<winSize.width<<' '<<winSize.height<<std::endl;
	if(winSize.width==0){
		throw std::runtime_error("Failed to get screen size");
	}
	finalLayer[0]=std::vector<std::vector<__layeredOut::charInfo>>(
		winSize.width,
		std::vector<__layeredOut::charInfo>(
			winSize.height,
			__layeredOut::charInfo()
		)
	);
}

//definition of __layeredOut::layer

__layeredOut::layer::layer(std::optional<std::list<__layeredOut::Layer>::iterator> A,__layeredOut::_key&_):a(A){}

bool __layeredOut::layer::isValid(){
	return a.has_value();
}

void __layeredOut::layer::setPos(const pos2d<size_t>& pos){
	if(!a.has_value())throw std::runtime_error("Invalid iterator");
	(*a)->pos=pos;
}

void __layeredOut::layer::use(const bool& inUse){
	if(!a.has_value())throw std::runtime_error("Invalid iterator");
	(*a)->inUse=inUse;
}

__layeredOut::pairLayerPos2d __layeredOut::layer::operator()(const pos2d<size_t>& pos){
	return std::move(__layeredOut::pairLayerPos2d(*this,pos));
}

__layeredOut::layer& __layeredOut::layer::operator<<(std::ostream& (*f)(std::ostream&)){
	static uintptr_t intEndl = reinterpret_cast<uintptr_t>(&std::endl<char,std::char_traits<char>>);
	static uintptr_t intEnds = reinterpret_cast<uintptr_t>(&std::ends<char,std::char_traits<char>>);
	static uintptr_t intFlush = reinterpret_cast<uintptr_t>(&std::flush<char,std::char_traits<char>>);
	auto t=reinterpret_cast<uintptr_t>(f);
	if(t==intFlush){
		std::cout<<"flush"<<std::endl;
		oss.clear();
		oss.str("");
	}else if(t==intEndl){
		std::cout<<"endl"<<std::endl;
	}else if(t==intEnds){
		std::cout<<"ends"<<std::endl;
	}
	return *this;
}

__layeredOut::layer& __layeredOut::layer::putPos(const pos2d<size_t>& a){
	oss.write("\xff\x00",2);
	oss.write(reinterpret_cast<const char*>(&a),sizeof(size_t)<<1);
	return *this;
}

std::optional<std::list<__layeredOut::Layer>::iterator> __layeredOut::layer::get(){ //debug
	return a;
}
int8_t __layeredOut::f(__layeredOut::layer a,__layeredOut::layer b,int8_t (*cmp)(void*,void*)){ //debug
	return cmp((*a.get())->precedence,(*b.get())->precedence);
}

__layeredOut::pairLayerPos2d::pairLayerPos2d(__layeredOut::layer& a, const pos2d<size_t>& b):l(a),pos(b){}

////////////////////////////////////////////////////////////////////////////////////////////////

//Implementation of layeredOut

const unsigned int layeredOut::nThread=std::thread::hardware_concurrency();

#ifndef _WIN32
#include<unistd.h>
#include<sys/ioctl.h>

size2d<size_t> layeredOut::getWinSize() noexcept{
	struct winsize winsz;
	if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsz))
		return std::move((size2d<size_t>){0,0});
	return std::move((size2d<size_t>){winsz.ws_col,winsz.ws_row});
}

int layeredOut::_getCharWidth(const char32_t& ch){
//	if(ch==45208)return 3;
	using wcWidthParamType=wchar_t;
	if constexpr(sizeof(char32_t)<=sizeof(wcWidthParamType)){
		return wcwidth(ch);
	}else{
		constexpr char32_t wcWidthMax
		=sizeof(char32_t)<=sizeof(wcWidthParamType)
		? 0 : ((char32_t)1<<(sizeof(wcWidthParamType)*std::numeric_limits<unsigned char>::digits))-1;
		return ch<=wcWidthMax ? wcwidth(ch) : -1;
	}
}

std::pair<char32_t,uint32_t> layeredOut::cvtUtf8To32(const char8_t* ch){
	if((ch[0]&0xe0)!=0xe0){
		if((ch[0]&0x80)==0){
			return std::pair(ch[0]&0xff,1);
		}else{
			return std::pair(
				static_cast<char32_t>(ch[0]&0x1f)<<6|
				static_cast<char32_t>(ch[1]&0x3f),
				2);
		}
	}else{
		if((ch[0]&0xf0)==0xe0){
			return std::pair(
				static_cast<char32_t>(ch[0]&0x0f)<<12|
				static_cast<char32_t>(ch[1]&0x3f)<<6|
				static_cast<char32_t>(ch[2]&0x3f),
				3);
		}else{
			return std::pair(
				static_cast<char32_t>(ch[0]&0x0f)<<18|
				static_cast<char32_t>(ch[1]&0x3f)<<12|
				static_cast<char32_t>(ch[2]&0x3f)<<6|
				static_cast<char32_t>(ch[3]&0x3f),
				4);
		}
	}
}

#else
#include<windows.h>
size2d<size_t> layeredOut::getWinSize() noexcept{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if(GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)){
		return std::move((size2d<size_t>){
			csbi.srWindow.Right - csbi.srWindow.Left + 1,
			csbi.srWindow.Bottom - csbi.srWindow.Top + 1});
	}
	return std::move((size2d<size_t>){0,0});
}
#endif

layeredOut::layeredOut(uint32_t bgColor, int(*getCharWidth)(const char32_t&))
	: bgColor(bgColor), shared({.getCharWidth=getCharWidth}){
	winSize=getWinSize();
	if(winSize.width==0)
		throw std::runtime_error("Failed to get screen size");

	finalLayer[0]=finalLayer[1]=finalLayer[2]
	=std::vector<std::vector<layeredOut::charInfo>>(
		winSize.width,
		std::vector<layeredOut::charInfo>(
			winSize.height,
			layeredOut::charInfo()
		)
	);
}

layeredOut::~layeredOut(){
	deleteOdr(*this);
}

void layeredOut::begin(std::chrono::time_point<std::chrono::steady_clock> (*const getFrameTimePoint)(bool,size_t,void*&)){
	finalStr.reserve((42*winSize.width+1)*winSize.height+4 +1/*spare space*/);
	finalStr.clear();
	nThreadDone = cycle = 0;
	std::cout<<"\033[?1049h\033[?25l\033[;H\033[38;2;255;255;255m\033[48;2;255;255;255m";
	counter={0,nullptr};
	(this->getFrameTimePoint = getFrameTimePoint ? getFrameTimePoint : builtinGetFrameTimePoint)(
		true,0,counter.data
	);
	thrdWorker[0]=std::thread([this]{this->workerPrinter();});
	thrdWorker[1]=std::thread([this]{this->workerRenderer();});
}

void layeredOut::stop(){
	std::cout<<"\033[?25h\033[?1049l"<<std::flush;
}

void layeredOut::workerPrinter(){
	size_t i, j;
	bool loopFlag;
	uint32_t fgColor=0xffffff, bgColor=0xffffff;
	static std::initializer_list<char> prefixFgColor{'\033','[','3','8',';','2',';'};
	static std::initializer_list<char> prefixBgColor{'\033','[','4','8',';','2',';'};

	while(1){
		finalStr.insert(finalStr.end(),{'\033','[',';','H'});

		for(j=0;j<winSize.height;++j){
			for(loopFlag=true, i=0;i<winSize.width && loopFlag;++i){
				switch(finalLayer[(cycle+1)%2][i][j].ch[0]){
				case 0xff:
					if(fgColor!=finalLayer[(cycle+1)%2][i][j].fgColor){
						uint32_t color=fgColor=finalLayer[(cycle+1)%2][i][j].fgColor;
						finalStr.insert(finalStr.end(),prefixFgColor);
						finalStr.insert(finalStr.end(),{
							static_cast<char>(color/6553600+0x30),
							static_cast<char>(color%6553600/655360+0x30),
							static_cast<char>((color%655360>>16)+0x30),
							';',
							static_cast<char>((color&=0xffff)%65536/25600+0x30),
							static_cast<char>(color%25600/2560+0x30),
							static_cast<char>((color%2560>>8)+0x30),
							';',
							static_cast<char>((color&=0xff)/100+0x30),
							static_cast<char>(color%100/10+0x30),
							static_cast<char>(color%10+0x30),
							'm'
						});
					}
					if(bgColor!=finalLayer[(cycle+1)%2][i][j].bgColor){
						uint32_t color=bgColor=finalLayer[(cycle+1)%2][i][j].bgColor;
						finalStr.insert(finalStr.end(),prefixBgColor);
						finalStr.insert(finalStr.end(),{
							static_cast<char>(color/6553600+0x30),
							static_cast<char>(color%6553600/655360+0x30),
							static_cast<char>((color%655360>>16)+0x30),
							';',
							static_cast<char>((color&=0xffff)%65536/25600+0x30),
							static_cast<char>(color%25600/2560+0x30),
							static_cast<char>((color%2560>>8)+0x30),
							';',
							static_cast<char>((color&=0xff)/100+0x30),
							static_cast<char>(color%100/10+0x30),
							static_cast<char>(color%10+0x30),
							'm'
						});
					}
					loopFlag=false;
					--i;
					break;
				case 0x00:
					if(bgColor!=this->bgColor){
						uint32_t color=bgColor=this->bgColor;
						finalStr.insert(finalStr.end(),prefixBgColor);
						finalStr.insert(finalStr.end(),{
							static_cast<char>(color/6553600+0x30),
							static_cast<char>(color%6553600/655360+0x30),
							static_cast<char>((color%655360>>16)+0x30),
							';',
							static_cast<char>((color&=0xffff)%65536/25600+0x30),
							static_cast<char>(color%25600/2560+0x30),
							static_cast<char>((color%2560>>8)+0x30),
							';',
							static_cast<char>((color&=0xff)/100+0x30),
							static_cast<char>(color%100/10+0x30),
							static_cast<char>(color%10+0x30),
							'm'
						});
					}
					finalStr.push_back(0x20);
				case 0xfe:
					break;
				default:
					if(fgColor!=finalLayer[(cycle+1)%2][i][j].fgColor){
						uint32_t color=fgColor=finalLayer[(cycle+1)%2][i][j].fgColor;
						finalStr.insert(finalStr.end(),prefixFgColor);
						finalStr.insert(finalStr.end(),{
							static_cast<char>(color/6553600+0x30),
							static_cast<char>(color%6553600/655360+0x30),
							static_cast<char>((color%655360>>16)+0x30),
							';',
							static_cast<char>((color&=0xffff)%65536/25600+0x30),
							static_cast<char>(color%25600/2560+0x30),
							static_cast<char>((color%2560>>8)+0x30),
							';',
							static_cast<char>((color&=0xff)/100+0x30),
							static_cast<char>(color%100/10+0x30),
							static_cast<char>(color%10+0x30),
							'm'
						});
					}
					if(bgColor!=finalLayer[(cycle+1)%2][i][j].bgColor){
						uint32_t color=bgColor=finalLayer[(cycle+1)%2][i][j].bgColor;
						finalStr.insert(finalStr.end(),prefixBgColor);
						finalStr.insert(finalStr.end(),{
							static_cast<char>(color/6553600+0x30),
							static_cast<char>(color%6553600/655360+0x30),
							static_cast<char>((color%655360>>16)+0x30),
							';',
							static_cast<char>((color&=0xffff)%65536/25600+0x30),
							static_cast<char>(color%25600/2560+0x30),
							static_cast<char>((color%2560>>8)+0x30),
							';',
							static_cast<char>((color&=0xff)/100+0x30),
							static_cast<char>(color%100/10+0x30),
							static_cast<char>(color%10+0x30),
							'm'
						});
					}
					for(size_t k=0;k<cvtUtf8To32(finalLayer[(cycle+1)%2][i][j].ch.data()).second;++k)
						finalStr.push_back(finalLayer[(cycle+1)%2][i][j].ch[k]);
				}
			}
			for(;i<winSize.width;++i)finalStr.push_back(0x20);
			finalStr.push_back('\n');
		}
		finalStr[finalStr.size()-1]=0;

		std::cout<<finalStr.3ata()<<std::flush;
		finalStr.clear();
		for(auto& v : finalLayer[(cycle+1)%2])for(auto& ch : v)ch.ch[0]=0;

		const std::chrono::time_point<std::chrono::steady_clock> frameTimePoint
			= getFrameTimePoint(true,counter.num,counter.data);
		std::this_thread::sleep_for(
			std::chrono::steady_clock::now()-frameTimePoint-std::chrono::milliseconds(3)
		);
		while(std::chrono::steady_clock::now()<frameTimePoint)std::this_thread::yield();
	}
//	std::cout<<"cycle\t"<<(cycle+1)%2<<std::endl;
//	for(size_t i=0;i<400;++i){
//		std::cout<<std::dec<<std::setw(3)<<i<<" : "<<std::hex<<std::setw(2)<<static_cast<uint32_t>(finalStr[i])<<", ";
//	}
//	std::cout<<std::dec<<std::flush;
}

void layeredOut::workerRenderer(){
	while(1){
		for(auto& layer : layerList){
			if(!layer.inUse)continue;
			size_t i, j;
			for(i=layer.layerData.size()-1;i+1;--i)for(j=layer.layerData[i].size()-1;j+1;--j){
				if(layer.pos.x+i>=winSize.width || layer.pos.y+j>=winSize.height)continue;
	//			if(orderCompare(layer.orderIter,vIter)>=0)continue;
	//			orderSetVectorValue(vIter,layer.orderIter); //Set order

				switch(layer.layerData[i][j][0]){
				case 0:
				case 0xfe:
					break;
				default:{
					auto lockRange=lockCharInfoMtx(
						{layer.pos.x+i,layer.pos.y+j},
						winSize.width-(layer.pos.x+i)
					);
					callWhenDestructed unlocker([this, &lockRange]{unlockCharInfoMtx(lockRange);});

					if(layer.pos.x+i+layer.layerData[i][j][10]>winSize.width){
						if(!initIfOrderHigher(
							{layer.pos.x+i,layer.pos.y+j},
							winSize.width-(layer.pos.x+i),
							layer.orderIter
						)) break;

						charInfo& ch=finalLayer[cycle%2][layer.pos.x+i][layer.pos.y+j];
						ch.fgColor=*reinterpret_cast<uint32_t*>(layer.layerData[i][j].data()+4)&0xffffff;
						ch.bgColor=*reinterpret_cast<uint32_t*>(layer.layerData[i][j].data()+7)&0xffffff;
						ch.offset=winSize.width-(layer.pos.x+i);
						ch.ch[0]=0xff;

						for(size_t k=ch.offset-1;k;--k){
							finalLayer[cycle%2][layer.pos.x+i+k][layer.pos.y+j].ch[0]=0xfe;
							finalLayer[cycle%2][layer.pos.x+i+k][layer.pos.y+j].offset=k;
						}
						break;
					}
					if(!initIfOrderHigher(
						{layer.pos.x+i,layer.pos.y+j},
						layer.layerData[i][j][10],
						layer.orderIter
					)) break;

					charInfo& ch=finalLayer[cycle%2][layer.pos.x+i][layer.pos.y+j];
	//				ch.mtx;
					ch.fgColor=*reinterpret_cast<uint32_t*>(layer.layerData[i][j].data()+4)&0xffffff;
					ch.bgColor=*reinterpret_cast<uint32_t*>(layer.layerData[i][j].data()+7)&0xffffff;
					ch.offset=layer.layerData[i][j][10];
					std::memcpy(
						ch.ch.data(),
						layer.layerData[i][j].data(),
						4
					);
					for(size_t k=ch.offset-1;k;--k){
						finalLayer[cycle%2][layer.pos.x+i+k][layer.pos.y+j].ch[0]=0xfe;
						finalLayer[cycle%2][layer.pos.x+i+k][layer.pos.y+j].offset=k;
					}
				}
				}
			}
		}

		const std::chrono::time_point<std::chrono::steady_clock> frameTimePoint
			= getFrameTimePoint(true,counter.num,counter.data);
		std::this_thread::sleep_for(
			std::chrono::steady_clock::now()-frameTimePoint-std::chrono::milliseconds(3)
		);
		while(std::chrono::steady_clock::now()<frameTimePoint)std::this_thread::yield();
	}
}

std::chrono::time_point<std::chrono::steady_clock> layeredOut::builtinGetFrameTimePoint(
	const bool isRunning,
	const size_t frameNumber,
	void*& frameData){
	// 60fps

	constexpr std::chrono::steady_clock::duration unitDur(std::chrono::seconds(1));
	struct S{
		std::chrono::time_point<std::chrono::steady_clock> startTime;
	};

	if(!frameData){ // isRunning==true && frameData==nullptr
		frameData=new S;
		reinterpret_cast<S*>(frameData)->startTime=std::chrono::steady_clock::now();
	}
	if(!isRunning){ // isRunning==false
		delete reinterpret_cast<S*>(frameData);
		frameData=nullptr;
		return std::chrono::time_point<std::chrono::steady_clock>();
	}
	return reinterpret_cast<S*>(frameData)->startTime + unitDur*(frameNumber/60) + unitDur*(frameNumber%60)/60;
}

std::pair<pos2d<size_t>,size_t> layeredOut::lockCharInfoMtx(pos2d<size_t> pos, size_t length){
	std::pair<pos2d<size_t>,size_t> range(pos, pos.x+length);
	if(finalLayer[cycle%2][pos.x][pos.y].ch[0]==0xfe)
		range.first.x-=finalLayer[cycle%2][pos.x][pos.y].offset;

	size_t i=pos.x;
	for(;i<range.second;i+=finalLayer[cycle%2][i][pos.y].offset);
	range.second=i;

	for(i-=range.first.x;i;--i)
		finalLayer[cycle%2][i+range.first.x-1][pos.y].mtx.lock();

	return range;
}

void layeredOut::unlockCharInfoMtx(std::pair<pos2d<size_t>,size_t> range){
	for(size_t i=range.second-range.first.x;i;--i)
		finalLayer[cycle%2][i+range.first.x-1][range.first.y].mtx.unlock();
}

bool layeredOut::initIfOrderHigher(pos2d<size_t> pos, size_t length, layeredOut::rawListIter orderIter){
	const size_t end=pos.x+length, origin=pos.x;
	if(finalLayer[cycle%2][pos.x][pos.y].ch[0]==0xfe)
		pos.x-=finalLayer[cycle%2][pos.x][pos.y].offset;

	const size_t start=pos.x;
	for(;pos.x<end;pos.x+=finalLayer[cycle%2][pos.x][pos.y].offset)
		if( finalLayer[cycle%2][pos.x][pos.y].ch[0] &&
			orderCompare(orderIter, orderGetVectorIter(odr, cycle%2, pos))>=0)break;
	if(pos.x<end)return false;

	for(pos.x=start;pos.x<end;pos.x+=finalLayer[cycle%2][pos.x][pos.y].offset)
		_delChar(pos);

	orderSetVectorValue(orderGetVectorIter(odr,cycle%2,{origin,pos.y}), orderIter);
	return true;
}

void layeredOut::_delChar(const pos2d<size_t>& pos){
	for(size_t i=finalLayer[cycle%2][pos.x][pos.y].offset;i;--i)
		finalLayer[cycle%2][pos.x+i-1][pos.y].ch[0]=0;
}

//Implementation of layeredOut::layer

layeredOut::layer::layer(
	std::list<layeredOut::layerInfo>::iterator layerIter,
	layeredOut::rawListIter orderIter,
	sharedData* shared,
	const layeredOut::key&)
	:validity(true), shared(shared)
	,cur(0,0), colBegin(0), color(0xffffff,0){
	this->layerIter=layerIter;
	this->layerIter->orderIter=orderIter;
}

layeredOut::layer::layer(const int&, const layeredOut::key&):validity(false){}

layeredOut::layer::layer(layeredOut::layer&& a)
	:validity(std::move(a.validity))
	,layerIter(std::move(a.layerIter))
	,oss(std::move(a.oss))
	,shared(std::move(a.shared))
	,cur(std::move(a.cur))
	,colBegin(std::move(a.colBegin))
	,color(std::move(a.color))
	,mtx()
{}

bool layeredOut::layer::isValid(){return validity;}

void layeredOut::layer::use(const bool& inUse){
	if(!validity)throw std::runtime_error("Invalid layer");
	layerIter->inUse=inUse;
}
bool layeredOut::layer::isInUse(){
	if(!validity)throw std::runtime_error("Invalid layer");
	return layerIter->inUse;
}

void layeredOut::layer::setPos(const pos2d<size_t>& pos){
	if(!validity)throw std::runtime_error("Invalid layer");
	layerIter->pos=pos;
}
pos2d<size_t> layeredOut::layer::getPos(){
	if(!validity)throw std::runtime_error("Invalid layer");
	return layerIter->pos;
}

layeredOut::layer& layeredOut::layer::operator<<(std::ostream& (*f)(std::ostream&)){
	static uintptr_t intEndl = reinterpret_cast<uintptr_t>(&std::endl<char,std::char_traits<char>>);
	static uintptr_t intFlush = reinterpret_cast<uintptr_t>(&std::flush<char,std::char_traits<char>>);
	auto t=reinterpret_cast<uintptr_t>(f);

	if(t==intEndl){
		oss.put('\n');
		t=intFlush;
	}
	if(t==intFlush){
		std::string str=oss.str();
		const char8_t*s=reinterpret_cast<const char8_t*>(str.data());

		for(size_t i=0;s[i];){
			switch(s[i]){
			case 0xff:
				switch(s[i+1]){
				case 0: //Change cursor
					cur=*reinterpret_cast<const pos2d<size_t>*>(s+i+2);
					colBegin=cur.x;
					i+=2+sizeof(pos2d<size_t>);
					break;
				case 1: //Change color
					color=*reinterpret_cast<const colorInfo*>(s+i+2);
					i+=2+sizeof(colorInfo);
					break;
				}
				break;

			case 0x0b:{ // \v
				if(cur.x >= layerIter->layerData.size() || cur.y >= layerIter->layerData[0].size())break;

				charData d;
				*reinterpret_cast<uint32_t*>(d.data()+4)=color.fg;
				*reinterpret_cast<uint32_t*>(d.data()+7)=color.bg;
				delChar(d,layerIter->layerData.size());

				++cur.x;
				++i;
				break;
			}
			case 0x08:{ // \b
				if(cur.x >= layerIter->layerData.size() || cur.y >= layerIter->layerData[0].size())break;

				charData d;
				*reinterpret_cast<uint32_t*>(d.data()+4)=color.fg;
				*reinterpret_cast<uint32_t*>(d.data()+7)=color.bg;
				cur.x=delChar(d,layerIter->layerData.size());
				++i;
				break;
			}
			case 0x0a: // \n
				++cur.y;

			case 0x0d: // \r
				cur.x=colBegin;
				++i;
				break;

			default:{
				auto c=cvtUtf8To32(s+i);

				if(cur.x >= layerIter->layerData.size() || cur.y >= layerIter->layerData[0].size()){
					i+=c.second;
					break;
				}

				int chWidth=shared->getCharWidth(c.first);
				if(chWidth==-1){
					i+=c.second;
					break;
				}

				charData d;

				*reinterpret_cast<uint32_t*>(d.data()+4)=color.fg;
				*reinterpret_cast<uint32_t*>(d.data()+7)=color.bg;

				size_t colSize = layerIter->layerData.size();

				if(cur.x+chWidth-1>=colSize){
					delChar(d,colSize);

					*reinterpret_cast<uint32_t*>(d.data())=0xff; //space
					d[10]=colSize-cur.x;
					layerIter->layerData[cur.x++][cur.y]=d;
					*reinterpret_cast<uint32_t*>(d.data())=0xfe; //space
					for(d[10]=1;cur.x<colSize;++cur.x,++d[10])layerIter->layerData[cur.x][cur.y]=d;

					i+=c.second;
					break;
				}

				delChar(d,colSize);

				for(size_t j=c.second-1;j+1;--j)d[j]=s[i+j];
				d[10]=chWidth;
				layerIter->layerData[cur.x][cur.y]=d;

				*reinterpret_cast<uint32_t*>(d.data())=0xfe;
				d[10]=chWidth-1;
				for(size_t j=chWidth-1;j;--j,--d[10])layerIter->layerData[j+cur.x][cur.y]=d;

				cur.x+=chWidth;
				i+=c.second;
				break;
			}
			}
		}
		oss.clear();
		oss.str(std::string());
	}
	return *this;
}

layeredOut::layer& layeredOut::layer::operator<<(const pos2d<size_t>& pos){return putPos(pos,K);}

layeredOut::layer& layeredOut::layer::operator<<(const colorInfo& a){
	oss.write(reinterpret_cast<const char*>((const uchar[]){0xff,0x01}),2);
	oss.write(reinterpret_cast<const char*>(&a),sizeof(colorInfo));
	return *this;
}

layeredOut::pairLayerPos2d layeredOut::layer::operator()(const pos2d<size_t>& pos){
	return std::move(layeredOut::pairLayerPos2d(*this,pos));
}

layeredOut::layer& layeredOut::layer::putPos(const pos2d<size_t>& a, const key&){
	oss.write(reinterpret_cast<const char*>((const uchar[]){0xff,0x00}),2);
	oss.write(reinterpret_cast<const char*>(&a),sizeof(pos2d<size_t>));
	return *this;
}

layeredOut::layer& layeredOut::layer::operator++(){
	std::cout<<"lock"<<std::endl;
	mtx.lock();
	return *this;
}

layeredOut::layer& layeredOut::layer::operator<<=(std::ostream& (*f)(std::ostream&)){
	std::cout<<"unlock f"<<std::endl;
	operator<<(f);
	mtx.unlock();
	return *this;
}

inline void layeredOut::layer::delChar(charData& d, size_t& colSize){
	*reinterpret_cast<uint32_t*>(d.data())=0;
	d[10]=0;

	size_t tcur=cur.x, i;
	if(layerIter->layerData[cur.x][cur.y][0]==0xfe)
		tcur-=layerIter->layerData[cur.x][cur.y].data()[10];

	for(i=layerIter->layerData[tcur][cur.y].data()[10];i;--i)
		layerIter->layerData[tcur++][cur.y]=d;
}

inline size_t layeredOut::layer::delChar(charData& d, size_t&& colSize){
	*reinterpret_cast<uint32_t*>(d.data())=0;
	d[10]=0;

	size_t tcur=cur.x, i;
	if(layerIter->layerData[cur.x][cur.y][0]==0xfe)
		tcur-=layerIter->layerData[cur.x][cur.y].data()[10];

	for(i=layerIter->layerData[tcur][cur.y].data()[10];i;--i)
		layerIter->layerData[tcur++][cur.y]=d;
	return tcur;
}

//Implementation of layeredOut::pairLayerPos2d
layeredOut::pairLayerPos2d::pairLayerPos2d(layeredOut::layer& a, const pos2d<size_t>& b):l(a),pos(b){}

layeredOut::pairLayerPos2d& layeredOut::pairLayerPos2d::operator++(){
	std::cout<<"lock"<<std::endl;
	++l;
	return *this;
}

layeredOut::layer& layeredOut::pairLayerPos2d::operator<<(std::ostream& (*f)(std::ostream&)){
	l.putPos(pos, K)<<f;
	return l;
}

layeredOut::layer& layeredOut::pairLayerPos2d::operator<<=(std::ostream& (*f)(std::ostream&)){
	l.putPos(pos, K)<<=f;
	return l;
}

//Implementation of others

layeredOut::callWhenDestructed::callWhenDestructed(std::function<void()> func):func(func){}
layeredOut::callWhenDestructed::~callWhenDestructed(){func();}

layeredOut::charInfo::charInfo():mtx(),ch({0,}),offset(1){}
layeredOut::charInfo::charInfo(const layeredOut::charInfo& a):mtx(),ch({0,}),offset(1){}
layeredOut::charInfo& layeredOut::charInfo::operator=(const layeredOut::charInfo& a){return *this;}
