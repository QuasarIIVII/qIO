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

private:
	static constexpr struct key_t{} key{};

	using uchar = unsigned char;
	using rawListIter = std::array<uchar,sizeof(std::list<int>::iterator)>;
	using rawVectorIter = std::array<uchar,sizeof(std::vector<int>::iterator)>;
	using winSize_t = size2d<decltype(winsize)>;

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

		charColorData() = default;

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

		charData() = default;

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
	class order{
	private:
		std::array<std::vector<std::vector<T>>,3> oLayer;
		std::list<T> orderList;
	};

public:
	class layer{
	private:
		bool isValid;
		std::list<layerData>::iterator iter;
		std::ostringstream oss;

	public:
		layer(int, key_t); // create an invalid layer
		layer(const layer&) = delete;
		layer(layer&&);
	};

private:
	winSize_t winSize_v;
	/*
	 * state_v & n
	 * 0x01: runable
	 * 0x02: running
	 */
	uint8_t state_v;

	std::array<std::vector<std::vector<charData>>, 3> oLayer; // output layer

private:
	static winSize_t winSize_f();
	static int _charWidth(const char32_t&);

public:
	winSize_t winSize() const;

public:
	template<threeWayStrongComparable_t T = int32_t>
	explicit layeredOut(
		uint32_t bgColor=0xff'000000
	);
	~layeredOut();

public:
	decltype(state_v) state() const;

	template<threeWayStrongComparable_t T>
	layer createLayer(size2d<size_t> size, T order);

public:
	void debug();
}; // scope end : class layeredOut

// Implementation of qIO::layeredOut

template<threeWayStrongComparable_t order_t>
layeredOut::layeredOut(
	uint32_t bgColor
)
:	winSize_v(winSize_f())
,	state_v(0)
{
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
		std::cerr<<e.what()<<std::endl;
		// state_v &= ~ 0x01;
		return;
	}

	state_v |= 0x01;
}

}

#endif//__qIO_H__
