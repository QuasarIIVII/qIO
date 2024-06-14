#ifndef __qIO__
#define __qIO__

#include<iostream>
#include<iomanip>

#include<cstdint>
#include<cstddef>

#include<stdexcept>
#include<new>

#include<thread>
#include<compare>

#include<atomic>

#include<optional>//unused
#include<vector>
#include<list>
#include<array>

#include<chrono>

#include<type_traits>
#include<sstream>

#include<bit>

#include<functional>

namespace qIO{

template<class T>
concept three_way_strong_comparable=requires(T a,T b){
	{a<=>b}->std::convertible_to<std::strong_ordering>;
};
template<class T>
constexpr bool is_three_way_strong_comparable_v=three_way_strong_comparable<T>;

template<class T> concept type3WayStrongComparable = is_three_way_strong_comparable_v<T>;

template<class T> concept typeInt = std::is_integral_v<T>;

template<class T> struct pos2d{T x,y;};
template<class T> struct size2d{T width,height;};

using uchar=unsigned char;

class __layeredOut{
private:
	struct _key{} key;

	using charData=std::array<uint8_t, 10>;
	class charInfo{
	public:
		std::mutex mtx;
		std::array<uint8_t,4> ch;
		uint32_t fgColor, bgColor;
		void* precedence;

		charInfo();
		charInfo(const charInfo&);
		charInfo& operator=(const charInfo&);
	};
	struct Layer{
		bool inUse;
		std::vector<std::vector<charData>> layerData;
		void* precedence;
		pos2d<size_t> pos;
	};

	const static unsigned int nThread;

	size2d<size_t> winSize;

	std::vector<std::vector<charInfo>> finalLayer[3];
	std::list<Layer> layerList;

	int8_t (*orderCompare)(void*,void*);

public:
	template<typeInt T, bool N=false>
	class _order{
		friend class _order<T,false>;

	public:
//	private:
		union{T a; std::list<_order<T,true>*>*orderList;} a;

	private:
		_order(T n){a.a=std::move(n);}

		_order(const _order& a){}
		_order operator=(const _order& a){}
	public:
		_order(){
			if constexpr(N)
				throw std::runtime_error("Bad Access");
			try{
				a.orderList=new std::list<_order<T,true>*>;
			}catch(const std::bad_alloc& e){
				a.orderList=nullptr;
				throw e;
			}
		}
		~_order() noexcept{
			if constexpr(!N){
				for(auto& a:*a.orderList)delete a;
				delete a.orderList;
			}
		}

		void* setOrder(T n,std::nothrow_t _) noexcept{
			_order<T,true>* o=new(std::nothrow) _order<T,true>(n);
			if(o==nullptr)return nullptr;
			try{
				a.orderList->push_back(o);
			}catch(...){
				delete o;
				return nullptr;
			}
			return o;
		}
		void* setOrder(T n){
			_order<T,true>* o=new _order<T,true>(n);
			a.orderList->push_back(o);
			return o;
		}
		static int8_t compare(void*a, void*b){
			union{
				std::strong_ordering o;
				int8_t i;
			} u={.o=static_cast<_order<T,true>*>(a)->a.a<=>static_cast<_order<T,true>*>(b)->a.a};
			return u.i;
		}
	};

	class pairLayerPos2d;

	class layer{
	private:
		std::optional<std::list<Layer>::iterator> a;
		std::ostringstream oss;
	public:
		layer(std::optional<std::list<Layer>::iterator> A,_key&_);
	public:
		bool isValid();
		void setPos(const pos2d<size_t>&);
		void use(const bool&);
		std::optional<std::list<Layer>::iterator> get(); //debug

		layer& operator+();
		layer& operator-();

		__layeredOut::pairLayerPos2d operator()(const pos2d<size_t>& pos);

		template<class T>
		layer& operator<<(const T& a){
			oss<<a;
			return *this;
		}
		layer& operator<<(std::ostream& (*)(std::ostream&));
		layer& putPos(const pos2d<size_t>&);
	};
	class pairLayerPos2d{
	private:
		layer& l;
		pos2d<size_t> pos;
	public:
		pairLayerPos2d(layer&,const pos2d<size_t>&);

		template<class T>
		__layeredOut::layer& operator<<(const T& a){
			l.putPos(pos)<<a;
			return l;
		}
	};

private:
	size2d<size_t> getWinSize() noexcept;
	void init();

public:
	template<typeInt T>
	__layeredOut(_order<T,false>&ord){
		orderCompare=ord.compare;
		init();
	}

//	template<typeInt T, typeInt U=T, typename std::enable_if_t<std::is_convertible_v<U,T>,int> =0>
	template<typeInt T, typeInt U=T> requires std::is_convertible_v<U,T>
	layer makeLayer(size2d<size_t> size, _order<T,false>&ord, U precedence=0) noexcept{
		Layer a;
		try{
			a={
				.inUse=true,
				.layerData=std::move(std::vector<std::vector<charData>>(
					size.width,
					std::vector<charData>(
						size.height,
						charData{0,}
					)
				)),
				.precedence=ord.setOrder(static_cast<T>(precedence)),
				.pos={0,0}
			};

			layerList.push_back(std::move(a));
		}catch(...){
			return std::move(layer(std::nullopt,key));
		}

		return std::move(layer(std::prev(layerList.end()),key));
	}
	int8_t f(layer a,layer b,int8_t (*cmp)(void*,void*));

	__layeredOut(const __layeredOut&)=delete;
	__layeredOut& operator=(const __layeredOut&)=delete;

};

////////////////////////////////////////////////////////////////////////////////////////////////

class layeredOut{
private:
	static constexpr struct key{} K{};

	using charData=std::array<char8_t, 11>;
	using rawListIter=std::array<uchar,sizeof(std::list<int>::iterator)>;
	using rawVectorIter=std::array<uchar,sizeof(std::vector<int>::iterator)>;

	const static unsigned int nThread;

	struct charInfo{
		std::mutex mtx;
		std::array<char8_t,4> ch;
		uint32_t fgColor, bgColor;
		uchar offset;

		charInfo();
		charInfo(const charInfo&);
		charInfo& operator=(const charInfo&);
	};
	struct layerInfo{
		bool inUse;
		std::vector<std::vector<charData>> layerData;
		pos2d<size_t> pos;
		rawListIter orderIter;
	};
	struct colorInfo{uint32_t fg,bg;}; //for operator<<

	std::vector<std::vector<charInfo>> finalLayer[3];
	std::list<layerInfo> layerList;
	std::vector<char> finalStr;

	const uint32_t bgColor; //default background color

	size2d<size_t> winSize;

	void* odr;

	bool isRunning;
	std::thread thrdWorker[2/*nThread*/];

	uint32_t cycle;
	std::atomic<unsigned int> nThreadDone;
	std::chrono::time_point<std::chrono::steady_clock> (*getFrameTimePoint)(bool,size_t,void*&); // call back
	struct{size_t num; void* data;} counter;

	rawListIter (*orderMakeLayer)(void*,void*);
	void (*deleteOdr)(layeredOut&);
	rawVectorIter (*orderGetVectorIter)(void*,size_t,pos2d<size_t>);
	void (*orderSetVectorValue)(rawVectorIter,rawListIter);
	std::strong_ordering (*orderCompare)(rawListIter,rawVectorIter);

	struct sharedData{
		size_t orderType;
		int (*getCharWidth)(const char32_t& ch);
	} shared;

	//builtin classes
	template<type3WayStrongComparable T>
	class order{
	private:
		std::vector<std::vector<T>> finalLayer[3];
		std::list<T> layerList;

	public:
		order(const size2d<size_t>& winSize, const key&);
		static rawListIter makeLayer(void* _inst, void* a);

		static rawVectorIter getVectorIter(void* _inst, size_t plane, pos2d<size_t> p);
		static void setVectorValue(rawVectorIter v, rawListIter a);
		static std::strong_ordering compare(rawListIter a, rawVectorIter b);
	};
	public:class layer;
private:
	class pairLayerPos2d{
	private:
		layer& l;
		pos2d<size_t> pos;
	public:
		pairLayerPos2d(layer&,const pos2d<size_t>&);

		template<class T>
		layer& operator<<(const T& a);
		layer& operator<<(std::ostream& (*)(std::ostream&));

		pairLayerPos2d& operator++();
		template<class T>
		layer& operator<<=(const T& a);
		layer& operator<<=(std::ostream& (*)(std::ostream&));
	};

	class callWhenDestructed{
	private:
		std::function<void()> func;
	public:
		callWhenDestructed(std::function<void()> func);
		~callWhenDestructed();
	};

public:
	class layer{
	private:
		bool validity;
		std::list<layerInfo>::iterator layerIter;
		std::ostringstream oss;
		sharedData* shared;
		pos2d<size_t> cur;
		size_t colBegin;
		colorInfo color;
		std::mutex mtx;

	public:
		layer(std::list<layerInfo>::iterator layerIter, rawListIter orderIter, sharedData* shared, const key&);
		layer(const int&, const key&); //make an invalid layer (validity=false)

		layer(layer&&);

		bool isValid();

		void use(const bool&);
		bool isInUse();

		void setPos(const pos2d<size_t>&);
		pos2d<size_t> getPos();

		template<type3WayStrongComparable T>
		void setPrecedence(const T&);
		template<type3WayStrongComparable T>
		T getPrecedence();

		template<class T>
		layer& operator<<(const T& a);
		layer& operator<<(std::ostream& (*)(std::ostream&));
		layer& operator<<(const pos2d<size_t>& pos);
		layer& operator<<(const colorInfo& a);
		pairLayerPos2d operator()(const pos2d<size_t>& pos);

		layer& putPos(const pos2d<size_t>&, const key&);

		layer& operator++();
		template<class T>
		layer& operator<<=(const T& a);
		layer& operator<<=(std::ostream& (*)(std::ostream&));

		size_t getlen(){return oss.str().size();}
		void print(){
			for(size_t i=0;i<layerIter->layerData[0].size();++i){
				for(size_t j=0;j<layerIter->layerData.size();++j){
					auto& d=layerIter->layerData[j][i];
					std::cout<<std::hex<<std::setw(6)
					<<*reinterpret_cast<uint32_t*>(d.data())<<std::dec
					<<' '<<(*reinterpret_cast<uint32_t*>(d.data()+4)&0xffffff)
					<<' '<<(*reinterpret_cast<uint32_t*>(d.data()+6)>>8)
					<<' '<<static_cast<uint32_t>(d.data()[10])<<" |\t";
				}
				std::cout<<std::endl;
			}
		}

	private:
		inline void delChar(charData&, size_t&);
		inline size_t delChar(charData&, size_t&&);
	};

private:
	size2d<size_t> getWinSize() noexcept;
	static int _getCharWidth(const char32_t& ch);
	void workerPrinter();
	void workerRenderer();
	public:
	static std::chrono::time_point<std::chrono::steady_clock> builtinGetFrameTimePoint(
		const bool isRunning,
		const size_t frameNumber,
		void*& frameData
	);

private:
	std::pair<pos2d<size_t>,size_t> lockCharInfoMtx(pos2d<size_t> pos, size_t length);
	void unlockCharInfoMtx(std::pair<pos2d<size_t>,size_t> range);
	bool initIfOrderHigher(pos2d<size_t> pos, size_t length, rawListIter orderIter);
	void _delChar(const pos2d<size_t>& pos);

public:
	static std::pair<char32_t,uint32_t> cvtUtf8To32(const char8_t* ch);

	explicit layeredOut(
		uint32_t bgColor=0x000000,
		int(*getCharWidth)(const char32_t&)=_getCharWidth
	);
	~layeredOut();

	template<type3WayStrongComparable T>
	void init();

	template<type3WayStrongComparable T>
	static void _deleteOdr(layeredOut&) noexcept;

	template<type3WayStrongComparable T>
	layer makeLayer(size2d<size_t> size, T precedence);

	void begin(std::chrono::time_point<std::chrono::steady_clock> (*const getFrameTimePoint)(bool,size_t,void*&)=nullptr);
	void stop();

	constexpr static pos2d<size_t> pos(const size_t& x, const size_t& y);
	constexpr static colorInfo color(const uint32_t& fg, const uint32_t& bg);

	//debug
	void runWorkerRenderer(){
//		++cycle;
		workerRenderer();
	}
	void runWorkerPrinter(){
		++cycle;
		workerPrinter();
	}
	void print(){
		for(size_t j=20;j<31;++j){
			for(size_t i=120;i<126;++i){
				std::cout<<std::hex<<std::setw(6)
				<<*reinterpret_cast<uint32_t*>(finalLayer[0][i][j].ch.data())<<std::dec
				<<' '<<finalLayer[0][i][j].fgColor
				<<' '<<finalLayer[0][i][j].bgColor
				<<' '<<static_cast<uint32_t>(finalLayer[0][i][j].offset)
				<<" | ";
			}
			std::cout<<std::endl;
		}
	}
};

//Implementation of layeredOut

template<type3WayStrongComparable T>
void layeredOut::init(){
	order<T>* odr = new order<T>(winSize,K);
	shared.orderType=typeid(T).hash_code();

	orderMakeLayer=odr->makeLayer;
	orderGetVectorIter=odr->getVectorIter;
	orderSetVectorValue=odr->setVectorValue;
	orderCompare=odr->compare;

	this->odr=odr;
	deleteOdr=_deleteOdr<T>;

	std::setlocale(LC_ALL, "en_US.utf8");
}

template<type3WayStrongComparable T>
void layeredOut::_deleteOdr(layeredOut& a) noexcept{
	delete static_cast<layeredOut::order<T>*>(a.odr);
}

template<type3WayStrongComparable T>
layeredOut::layer layeredOut::makeLayer(size2d<size_t> size, T precedence){
	if(shared.orderType!=typeid(T).hash_code())throw std::runtime_error("Bad type");
	layerInfo layer;
	rawListIter orderIter;

	try{
		layer={
			.inUse=true,
			.layerData=std::vector(
				size.width,
				std::vector(
					size.height,
					charData{0,}
				)
			),
			.pos={0,0}
		};
		layerList.push_back(std::move(layer));
	}catch(...){
		return std::move(layeredOut::layer(0,K));
	}

	try{
		orderIter=orderMakeLayer(odr, &precedence);
	}catch(...){
		return std::move(layeredOut::layer(0,K));
	}
	return std::move(layeredOut::layer(std::prev(layerList.end()),orderIter,&shared,K));
}

constexpr pos2d<size_t> layeredOut::pos(const size_t& x, const size_t& y){return std::move(pos2d<size_t>(x,y));}

constexpr layeredOut::colorInfo layeredOut::color(const uint32_t& fg, const uint32_t& bg)
{return std::move(colorInfo(fg,bg));}

//Implementation of layeredOut::order

template<type3WayStrongComparable T>
layeredOut::order<T>::order(const size2d<size_t>& winSize, const layeredOut::key&){
	finalLayer[0]=finalLayer[1]=finalLayer[2]
	=std::vector<std::vector<T>>(
		winSize.width,
		std::vector<T>(
			winSize.height
		)
	);
}

template<type3WayStrongComparable T>
layeredOut::rawListIter layeredOut::order<T>::makeLayer(void* _inst, void* a){
	layeredOut::order<T>& inst=*static_cast<layeredOut::order<T>*>(_inst);

	inst.layerList.push_back(*static_cast<T*>(a));

	return std::move(std::bit_cast<rawListIter>(std::prev(inst.layerList.end())));
}

template<type3WayStrongComparable T>
layeredOut::rawVectorIter layeredOut::order<T>::getVectorIter(void* _inst, size_t plane, pos2d<size_t> p){
	auto iter=static_cast<layeredOut::order<T>*>(_inst)->finalLayer[plane][p.x].begin();
	std::advance(iter, p.y);
	return std::bit_cast<rawVectorIter>(iter);
}

template<type3WayStrongComparable T>
void layeredOut::order<T>::setVectorValue(rawVectorIter v, rawListIter a){
	union{
		std::vector<T>::iterator iter;
		rawVectorIter r;
	} uv={.r=v};
	union{
		std::list<T>::iterator iter;
		rawListIter r;
	} ul={.r=a};
	*uv.iter=*ul.iter;
}

template<type3WayStrongComparable T>
std::strong_ordering layeredOut::order<T>::compare(rawListIter a, rawVectorIter b){
	union{
		std::list<T>::iterator iter;
		rawListIter r;
	} ul={.r=a};
	union{
		std::vector<T>::iterator iter;
		rawVectorIter r;
	} uv={.r=b};
	return *ul.iter<=>*uv.iter;
}

//Implementation of layeredOut::layer
template<type3WayStrongComparable T>
void layeredOut::layer::setPrecedence(const T& precedence){
	if(!validity)throw std::runtime_error("Invalid layer");
	if(shared->orderType!=typeid(T).hash_code())throw std::runtime_error("Bad type");
	union{
		std::list<T>::iterator iter;
		rawListIter r;
	}u={.r=layerIter->orderIter};
	*u.iter=precedence;
}

template<type3WayStrongComparable T>
T layeredOut::layer::getPrecedence(){
	if(!validity)throw std::runtime_error("Invalid layer");
	if(shared->orderType!=typeid(T).hash_code())throw std::runtime_error("Bad type");
	union{
		std::list<T>::iterator iter;
		rawListIter r;
	}u={.r=layerIter->orderIter};
	return *u.iter;
}

template<class T>
layeredOut::layer& layeredOut::layer::operator<<(const T& a){
	oss<<a;
	return *this;
}

template<class T>
layeredOut::layer& layeredOut::layer::operator<<=(const T& a){
	std::cout<<"unlock"<<std::endl;
	operator<<(a);
	mtx.unlock();
	return *this;
}

//Implementation of layeredOut::pairLayerPos2d
template<class T>
layeredOut::layer& layeredOut::pairLayerPos2d::operator<<(const T& a){
	l.putPos(pos, K)<<a;
	return l;
}

template<class T>
layeredOut::layer& layeredOut::pairLayerPos2d::operator<<=(const T& a){
	l.putPos(pos, K)<<=a;
	return l;
}

}
#endif//__qIO__
