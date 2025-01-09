#ifndef __qIO_H__
#define __qIO_H__

#include <iostream>
#include <iomanip>

#include <cstdint>

#include <array>
#include <vector>
#include <list>

#include <mutex>

#include "target_sys.h"

#if __has_include(<unistd.h>) && __has_include(<sys/ioctl.h>)
#include<sys/ioctl.h>
#endif

namespace qIO{

template<class T>
concept three_way_strong_comparable=requires(T a,T b){
	{a<=>b}->std::convertible_to<std::strong_ordering>;
};

template<class T>
constexpr bool is_three_way_strong_comparable_v=three_way_strong_comparable<T>;

template<class T> concept threeWayStrongComparable_t = is_three_way_strong_comparable_v<T>;


template<class T>
struct point2d : public std::array<T, 2>{
	T &x = (*this)[0];
	T &y = (*this)[1];
};
template<class T>
struct size2d : public std::array<T, 2>{
	T &w = (*this)[0];
	T &h = (*this)[1];

	size2d() = default;

	size2d(T w, T h) noexcept
	:	std::array<T, 2>{w, h}
	{}

	template<class U, typename = std::enable_if_t<std::is_convertible_v<U, T>>>
	size2d(const size2d<U> &other) noexcept
	:	std::array<T, 2>(other.array)
	{}

	template<class U, typename = std::enable_if_t<std::is_convertible_v<U, T>>>
	size2d(size2d<U> &&other):std::array<T, 2>(std::move(other.array)){std::cout<<"move"<<std::endl;}

	size2d& operator=(const size2d &other) noexcept{
		this->array::operator=(other);
		return *this;
	}

	size2d& operator=(size2d &&other) noexcept{
		if(this!=&other)
			this->array::operator=(std::move(other));
		return *this;
	}
};

class layeredOut{

private: // section private types
	static constexpr struct key_t{} key{};

	using uchar = unsigned char;
	using state_t = uint32_t;
	using rawListIter = std::array<uchar,sizeof(std::list<int>::iterator)>;
	using rawVectorIter = std::array<uchar,sizeof(std::vector<int>::iterator)>;

#if __has_include(<unistd.h>)
	static_assert(sizeof(winsize::ws_col)==sizeof(winsize::ws_row));
	using winSize_t = size2d<decltype(winsize::ws_col)>;
#endif

	struct shared_t;

	class spinlock{
	private:
		std::atomic_flag lock_flag;

	public:
		spinlock() noexcept;

		spinlock(const spinlock&) = delete;
		spinlock& operator=(const spinlock&) = delete;

		void lock() noexcept;
		bool try_lock() noexcept;
		void unlock() noexcept;
	};

	struct charColorData{
		uint32_t fg, bg;

		charColorData() noexcept = default;

		charColorData(charColorData&) noexcept;
		charColorData(const charColorData&) noexcept;
		charColorData(charColorData&&) noexcept;

		charColorData& operator=(charColorData&) noexcept;
		charColorData& operator=(const charColorData&) noexcept;
		charColorData& operator=(charColorData&&) noexcept;
	};

	struct charData{
		spinlock mtx;
		std::array<char8_t, 4> ch = {0, };
		charColorData color;
		uchar offset;

		charData() noexcept = default;

		charData(charData&) noexcept;
		charData(const charData&) noexcept;
		charData(charData&&) noexcept;

		charData& operator=(charData&) noexcept;
		charData& operator=(const charData&) noexcept;
		charData& operator=(charData&&) noexcept;
	};
	using charBin=std::array<char8_t, sizeof(charData)>;

	struct layerData{
		bool inUse;
		std::vector<std::vector<charData>> layerData;
		point2d<size_t> pos;
		rawListIter orderIter;
	};

	template<threeWayStrongComparable_t T>
	class order_t{
	private:
		std::array<std::vector<std::vector<T>>,3> oLayer;
		std::list<T> orderList;

	public:
		order_t(state_t &, const winSize_t &) noexcept;
		~order_t() noexcept = default;
	};

	struct shared_t{
		size_t hash_order_type;
	};

public: // section public types
	class layer{
	private:
		bool isValid;
		std::list<layerData>::iterator iter;
		std::ostringstream oss;

	public:
		layer(int, key_t) noexcept; // create an invalid layer
		layer(const layer&) = delete;
		layer(layer&&) noexcept;
	};

private: // section private variables
	winSize_t winSize_v;
	/*
	 * state_v & n
	 * 0x01: runable
	 * 0x02: running
	 * 0x8000'0000: error occured
	 */
	state_t state_v;

	std::array<std::vector<std::vector<charData>>, 3> oLayer; // output layer
	void *order;

	shared_t shared;

private: // section private function pointers
	void (*deleteOrder)(layeredOut&) noexcept;

private: // section private static function declarations
	static winSize_t winSize_f();
	static int _charWidth(const char32_t&) noexcept;

	template<threeWayStrongComparable_t T>
	static void _deleteOrder(layeredOut&) noexcept;

public: // section public constructor and destructor declarations
	template<threeWayStrongComparable_t T = int32_t>
	explicit layeredOut(
		uint32_t bgColor=0xff'000000
	) noexcept;
	~layeredOut() noexcept;

public: // section public function declarations
	winSize_t winSize() const noexcept;
	state_t state() const noexcept;

	template<threeWayStrongComparable_t T>
	layer createLayer(size2d<size_t> size, T order) noexcept;

public: // section debug function declarations
	void debug();
}; // scope end : class layeredOut

// Implementation of qIO::layeredOut

template<threeWayStrongComparable_t order_type>
void layeredOut::_deleteOrder(layeredOut &lo) noexcept{
	delete static_cast<order_t<order_type>*>(lo.order);
}

template<threeWayStrongComparable_t order_type>
layeredOut::layeredOut(
	uint32_t bgColor
) noexcept
:	winSize_v(winSize_f())
,	state_v(0)
{
	static const size_t hash_order_type = typeid(order_type).hash_code();
	shared.hash_order_type = hash_order_type;

	std::cout<<winSize_v.w<<'x'<<winSize_v.h<<std::endl;
	if(!winSize_v.w){
		// state_v &= ~ 0x01;
		return;
	}

	try{
		oLayer[0] = oLayer[1] = oLayer[2]
		= std::vector<std::vector<charData>>(
			winSize_v.h,
			std::vector<charData>(
				winSize_v.w,
				charData()
			)
		);
	}catch(std::exception &e){
//		std::cerr<<e.what()<<std::endl;
		// state_v &= ~ 0x01;
		return;
	}

	// noexcept
	order_t<order_type> *order = new(std::nothrow) order_t<order_type>(state_v, winSize_v);
	if(!order || (state_v & 0x8000'0000) ){
		// state_v &= ~ 0x01;
		return;
	}

	this->order = order;
	deleteOrder = _deleteOrder<order_type>;

	std::setlocale(LC_ALL, "en_US.utf8");

	state_v |= 0x01;
}

// Implementation of qIO::layeredOut::order_t

template<threeWayStrongComparable_t T>
layeredOut::order_t<T>::order_t(state_t &state_v, const winSize_t &winSize) noexcept{
	try{
		oLayer[0] = oLayer[1] = oLayer[2]
		= std::vector<std::vector<T>>(
			winSize.h,
			std::vector<T>(winSize.w)
		);
	}catch(std::exception &e){
//		std::cerr<<e.what()<<std::endl;
		state_v |= 0x8000'0000;
		return;
	}
}

} // scope end : namespace qIO

#endif//__qIO_H__
