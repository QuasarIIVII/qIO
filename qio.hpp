#ifndef __qIO_H__
#define __qIO_H__

#include <iostream>
#include <iomanip>

#include <cstdint>

#include <cstring>

#include <array>
#include <vector>
#include <list>

#include <optional>

#include <compare>

#include <mutex>

#include<cassert>

#include "target_sys.h"

#if __has_include(<unistd.h>) && __has_include(<sys/ioctl.h>)
#include<sys/ioctl.h>
#endif

/*
 * `small size` means its size is small enough to be copied by a few instructions. (no move needed)
 * - e.g. <= 16 bytes
 */

namespace qIO{

template<class T>
concept three_way_strong_comparable = std::three_way_comparable<T, std::strong_ordering>;

template<class T>
constexpr bool is_three_way_strong_comparable_v = three_way_strong_comparable<T>;

template<class T>
concept threeWayStrongComparable_t = is_three_way_strong_comparable_v<T>;

template<class T>
struct moveOnly{
	T a;
	moveOnly() = default;
	moveOnly(const moveOnly&) = delete;
	moveOnly(moveOnly&&) = default;

	moveOnly& operator=(const moveOnly&) = delete;
	moveOnly& operator=(moveOnly&&) = default;

	moveOnly(T &&a) : a(std::move(a)) {}
};

template<class T>
struct typeId{
private:
	static constexpr struct{} s{};
public:
	static constexpr const void *value = &s;
};

template<class T, std::size_t N>
struct array : public std::array<T, N>{
	constexpr array() noexcept = default;

	template<class U = T>
	constexpr array(const array<U, N>& other)
	requires std::is_convertible_v<U, T> && std::copyable<T>
	{
		std::copy(other.begin(), other.end(), this->begin());
	}

	constexpr array(std::initializer_list<T> init) noexcept
	requires std::copyable<T> && std::default_initializable<T>
	{
		if(init.size() <= N)
			std::copy(init.begin(), init.end(), this->begin());
		else
			std::copy(init.begin(), init.begin()+N, this->begin());
	}
};

template<class T>
struct point2d : public std::array<T, 2>{
	T &x = (*this)[0];
	T &y = (*this)[1];
};

template<class T>
struct size2d : public qIO::array<T, 2>{
	T &w = (*this)[0];
	T &h = (*this)[1];

	size2d() = default;

	template<class... U>
	requires (std::is_convertible_v<U, T> && ...)
	size2d(U... args) noexcept
	:	qIO::array<T, 2>{static_cast<T>(args)...}
	{}

	size2d(const size2d&) noexcept = default;

	template<class U = T>
	requires std::is_convertible_v<U, T>
	size2d(const size2d<U> &other) noexcept
	:	qIO::array<T, 2>(other)
	{}

	size2d& operator=(const size2d &other) noexcept{
		this->qIO::array<T, 2>::operator=(other);
		return *this;
	}
};

struct pair_unicode_len{
	union{
		uint64_t i64;
		std::array<uint32_t, 2> i32;
	} data;
	uint32_t &unicode = data.i32[0];
	uint32_t &len = data.i32[1];
};

class layeredOut{

private: // section private type aliases
	static constexpr struct key_t{} key{};

	using uchar = unsigned char;
	using state_t = uint32_t;
	using rawListIter = std::array<uchar,sizeof(std::list<int>::iterator)>;
	using rawVectorIter = std::array<uchar,sizeof(std::vector<int>::iterator)>;

public: // section public type aliases
#if __has_include(<unistd.h>)
	static_assert(sizeof(winsize::ws_col)==sizeof(winsize::ws_row));
	using winSize_t = size2d<decltype(winsize::ws_col)>;	// will be small size
	using winPoint_t = point2d<decltype(winsize::ws_col)>;	// will be small size
#endif

private: // section private type declarations
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
		bool visible;
		std::vector<std::vector<charData>> data;
		winPoint_t pos;
		rawListIter orderIter;
	};

	template<threeWayStrongComparable_t T>
	class order_t{
	private:
		std::array<std::vector<std::vector<T>>,3> oLayer;
		std::list<T> layerList; // list of layer order

	public:
		order_t(state_t &, const winSize_t &) noexcept;
		~order_t() noexcept = default;

		static std::optional<rawListIter> createLayer(
			void *inst_p, const void *order_p
		) noexcept;
	};

	struct shared_t{
		const void* hash_order_type;
	};

public: // section public types
	class layerStream;

	class layer{
	private: // section private variables
		std::list<layerData>::iterator iter;
		winPoint_t cursor_v;

		shared_t *shared; // layer (*this) is invalid if shared == nullptr

	public: // section public constructor and destructor declarations
		layer(std::list<layerData>::iterator, shared_t *, key_t) noexcept;
		layer(int, key_t) noexcept; // create an invalid layer (shared = nullptr)
		layer(const layer&) = delete;
		layer(layer&&) noexcept = default;

	public: // section layeredOut internal function declarations
		layerData& data(key_t) noexcept;

	public:
		template<class T>
		layerStream operator<<(const T&) noexcept;

		// getters and setters
		winPoint_t cursor() const noexcept;
		void cursor(const winPoint_t&) noexcept;
	}; // scope end : class layer

	class layerStream{
	private:
		/*
		 * state_v
		 * 0x01: bad bit
		 */
		uint8_t state_v;
		layer &layer_v;
		winPoint_t cursor_v;
		std::ostringstream oss; // default constructor

	public:
		layerStream(layer&, winPoint_t, key_t) noexcept;

		layerStream(const layerStream&) = delete;
		layerStream(layerStream&&) noexcept = default;

		layerStream(int, const layerStream&) noexcept; // bad constructor

		~layerStream() noexcept;

	public:
		template<class T>
		layerStream&& operator<<(const T&) noexcept;

		std::ostringstream& getOss(){return oss;}
	}; // scope end : class layerStream

private: // section private variables
	/*
	 * state_v
	 * 0x01: runable
	 * 0x02: running
	 * 0x8000'0000: error occured
	 */
	state_t state_v;

	winSize_t winSize_v;

	std::array<std::vector<std::vector<charData>>, 3> oLayer; // output layer
	void *order;

	shared_t shared;

	std::list<layerData> layerList;

	const void *orderTypeId;

private: // section private function pointers
	void (*deleteOrder)(layeredOut&) noexcept;
	std::optional<rawListIter> (*orderCreateLayer)(void *inst, const void *order) noexcept;

	layer (*createLayer_p)(layeredOut *const, winSize_t size, const void *order) noexcept;

private: // section private static function declarations
	static winSize_t winSize_f() noexcept;
	static int _charWidth(const char32_t&) noexcept;
//	static 

	template<threeWayStrongComparable_t T>
	static void _deleteOrder(layeredOut&) noexcept;

	template<threeWayStrongComparable_t T>
	static layer _createLayer(layeredOut *const, winSize_t size, const void *order) noexcept;

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
	layer createLayer(winSize_t size, const T& order) noexcept;

public: // section debug function declarations
	void debug();
}; // scope end : class layeredOut

#include"qio.inl"

} // scope end : namespace qIO

#endif//__qIO_H__
