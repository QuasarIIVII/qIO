#include "qio.h"

using qIO::layeredOut;
using qIO::size2d;

// Implementation of qIO::layeredOut::charColorData

layeredOut::charColorData::charColorData(charColorData& other) noexcept{
	if constexpr(alignof(charColorData)==alignof(uint64_t))
		*reinterpret_cast<uint64_t*>(this) = reinterpret_cast<uint64_t&>(other);
	else
		this->fg=other.fg, this->bg=other.bg;
}

layeredOut::charColorData::charColorData(const charColorData& other) noexcept{
	if constexpr(alignof(charColorData)==alignof(uint64_t))
		*reinterpret_cast<uint64_t*>(this) = reinterpret_cast<const uint64_t&>(other);
	else
		this->fg=other.fg, this->bg=other.bg;
}

layeredOut::charColorData::charColorData(charColorData&& other) noexcept{
	if constexpr(alignof(charColorData)==alignof(uint64_t))
		*reinterpret_cast<uint64_t*>(this) = reinterpret_cast<uint64_t&&>(other);
	else
		this->fg=other.fg, this->bg=other.bg;
//		this->fg=std::move(other.fg), this->bg=std::move(other.bg);
}

layeredOut::charColorData& layeredOut::charColorData::operator=(charColorData& other) noexcept{
	if constexpr(alignof(charColorData)==alignof(uint64_t))
		*reinterpret_cast<uint64_t*>(this) = reinterpret_cast<uint64_t&>(other);
	else
		this->fg=other.fg, this->bg=other.bg;

	return *this;
}

layeredOut::charColorData& layeredOut::charColorData::operator=(
	const charColorData& other
) noexcept
{
	if constexpr(alignof(charColorData)==alignof(uint64_t))
		*reinterpret_cast<uint64_t*>(this) = reinterpret_cast<const uint64_t&>(other);
	else
		this->fg=other.fg, this->bg=other.bg;

	return *this;
}

layeredOut::charColorData& layeredOut::charColorData::operator=(
	charColorData&& other
) noexcept
{
	if constexpr(alignof(charColorData)==alignof(uint64_t))
		*reinterpret_cast<uint64_t*>(this) = reinterpret_cast<uint64_t&&>(other);
	else
		this->fg=other.fg, this->bg=other.bg;
//		this->fg=std::move(other.fg), this->bg=std::move(other.bg);

	return *this;
}

// Implementation of qIO::layeredOut::charData

layeredOut::charData::charData(charData& other) noexcept:mtx(){
	std::lock_guard lock(other.mtx);
	ch=other.ch;
	color=other.color;
	offset=other.offset;
}

layeredOut::charData::charData(const charData& other) noexcept:mtx(){
	ch=other.ch;
	color=other.color;
	offset=other.offset;
}

layeredOut::charData::charData(charData&& other) noexcept:mtx(){
	std::lock_guard lock(other.mtx);
	ch=std::move(other.ch);
	color=std::move(other.color);
	offset=other.offset;
}

layeredOut::charData& layeredOut::charData::operator=(charData& other) noexcept{
	std::lock(mtx, other.mtx);
	std::lock_guard lock0(mtx, std::adopt_lock);
	std::lock_guard lock1(other.mtx, std::adopt_lock);
	ch=other.ch;
	color=other.color;
	offset=other.offset;
	return *this;
}

layeredOut::charData& layeredOut::charData::operator=(const charData& other) noexcept{
	ch=other.ch;
	color=other.color;
	offset=other.offset;
	return *this;
}

layeredOut::charData& layeredOut::charData::operator=(charData&& other) noexcept{
	std::lock(mtx, other.mtx);
	std::lock_guard lock0(mtx, std::adopt_lock);
	std::lock_guard lock1(other.mtx, std::adopt_lock);
	ch=std::move(other.ch);
	color=std::move(other.color);
	offset=other.offset;
	return *this;
}

// Implementation of qIO::layeredOut::layer

layeredOut::layer::layer(std::list<layerData>::iterator iter, shared_t *shared, key_t) noexcept
:	iter(iter), cursor_v({0,0}), shared(shared)
{}

layeredOut::layer::layer(int, key_t) noexcept
:	shared(nullptr)
{}

// Implementation of qIO::layeredOut::layerStream
layeredOut::layerStream::layerStream(layer& layer_v, key_t) noexcept
:	layer_v(layer_v)
{
	std::cout<<"layerStream::layerStream(layer&, key_t)"<<std::endl;
}

layeredOut::layerStream::layerStream(layerStream&& other) noexcept
:	layer_v(other.layer_v), oss(std::move(other.oss))
{
	std::cout<<"layerStream::layerStream(layerStream&&)"<<std::endl;
}

layeredOut::layerStream::~layerStream() noexcept{
	std::cout<<"layerStream::~layerStream()"<<std::endl;
}

// Implementation of qIO::layeredOut::spinlock

layeredOut::spinlock::spinlock() noexcept
:	lock_flag(ATOMIC_FLAG_INIT)
{}

void layeredOut::spinlock::lock() noexcept{
	while (lock_flag.test_and_set(std::memory_order_acquire))
		std::this_thread::yield();
}

bool layeredOut::spinlock::try_lock() noexcept{
	return !lock_flag.test_and_set(std::memory_order_acquire);
}

void layeredOut::spinlock::unlock() noexcept{
	lock_flag.clear(std::memory_order_release);
}

void layeredOut::debug(){
	std::cout<<sizeof(charData)<<std::endl;
#if __has_include(<unistd.h>)
	std::cout<<"posix"<<std::endl;
#else
	std::cout<<"not posix"<<std::endl;
#endif

#ifdef __linux__
	std::cout<<"linux"<<std::endl;
#elif __APPLE__
	std::cout<<"macOS"<<std::endl;
#endif

#ifdef __unix__
	std::cout<<"unix"<<std::endl;
#else
	std::cout<<"not unix"<<std::endl;
#endif
}

// Implementation of qIO::layeredOut

#if __has_include(<unistd.h>) && __has_include(<sys/ioctl.h>)
#include<unistd.h>
#include<sys/ioctl.h>
#endif
layeredOut::winSize_t layeredOut::winSize_f() noexcept{
	if constexpr (target_sys::target_sys & 1)
	{
		winsize w;
		if(ioctl(0, TIOCGWINSZ, &w))
			return winSize_t{0, 0};
		return winSize_t{w.ws_col, w.ws_row};
	}
	else
		return winSize_t{0, 0};
}

layeredOut::~layeredOut() noexcept{
	deleteOrder(*this);
}

layeredOut::state_t layeredOut::state() const noexcept{
	return state_v;
}
