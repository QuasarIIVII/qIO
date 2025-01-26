#include "qio.hpp"

using qIO::layeredOut;
using qIO::size2d;

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

layeredOut::layerData& layeredOut::layer::data(key_t) noexcept{
	return *iter;
}

// Implementation of qIO::layeredOut::layerStream

layeredOut::layerStream::layerStream(layer& layer_v, wPoint2d cursor_v, key_t) noexcept
:	state_v(0)
,	layer_v(layer_v)
,	cursor_v(cursor_v)
{
	std::cout<<"layerStream::layerStream(layer&, key_t)"<<std::endl;
}

/*
layeredOut::layerStream::layerStream(layerStream&& other) noexcept
:	layer_v(other.layer_v), oss(std::move(other.oss))
{
	std::cout<<"layerStream::layerStream(layerStream&&)"<<std::endl;
}
*/

layeredOut::layerStream::~layerStream() noexcept{
	std::cout<<"layerStream::~layerStream()"<<std::endl;

	static constexpr auto delChar
	= [](const charData *p, wSize len){
		charData *q;

		q = p->ch[0] == 0xff /* trail byte */
		?	const_cast<charData*>(p)
		:	const_cast<charData*>(p) - p->offset;

		while(q-p < len){
			auto e = q + q->offset;

			for(; q != e; ++q){
				reinterpret_cast<decltype(charData::ch)*>(
					reinterpret_cast<char*>(&*q)+offsetof(charData, ch)
				) -> fill(0);
				*reinterpret_cast<decltype(charData::offset)*>(
					reinterpret_cast<char*>(&*q)+offsetof(charData, offset)
				) = 0;
			}
		}
	};

	decltype(layerData::data) &data = layer_v.data(key).data;

	std::string_view sv = oss.view();
	std::string_view::const_pointer p = sv.data();
	std::string_view::const_pointer e = p + sv.size();

	wSize2d lsize = {data[0].size(), data.size()}; // layer size

	while(p < e){
		switch(*reinterpret_cast<const uint8_t*>(p)){
		default:{
			auto c = cvtUtf8To32(p);
			std::cout<<c.unicode<<' ';

			if(cursor_v >= lsize){ // out of range
				p += c.len;
				break;
			}

			int cw = charWidth_f(c.unicode);

			if(cw <= 0){ // invalid character
				p += c.len;
				break;
			}

			if(cursor_v.x + cw > lsize.w){ // out of range
			}

			break;
		}
		}
	}
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
layeredOut::wSize2d layeredOut::winSize_f() noexcept{
	if constexpr (target_sys::target_sys & 1)
	{
		winsize w;
		if(ioctl(0, TIOCGWINSZ, &w))
			return wSize2d{0, 0};
		return wSize2d{w.ws_col, w.ws_row};
	}
	else
		return wSize2d{0, 0};
}

int layeredOut::charWidth_f(const char32_t ch) noexcept{
	using wcwidth_arg_t = wchar_t;
	if constexpr(sizeof(char32_t)<=sizeof(wcwidth_arg_t)){
		return wcwidth(static_cast<wcwidth_arg_t>(ch));
	}else{
		return ch <= static_cast<const char32_t>(std::numeric_limits<wchar_t>::max())
		?	wcwidth(ch)
		:	-1;
	}
}

constexpr qIO::pair_unicode_len layeredOut::cvtUtf8To32(const char *c) noexcept{
	if((c[0]&0xe0)!=0xe0){
		if((c[0]&0x80)==0){
			return pair_unicode_len{{
				.i32={
					c[0]&0xffu,
					1
				}
			}};
		}else{
			return pair_unicode_len{{
				.i32={
					static_cast<char32_t>(c[0]&0x1fu)<<6|
					static_cast<char32_t>(c[1]&0x3fu),
					2
				}
			}};
		}
	}else{
		if((c[0]&0xf0)==0xe0){
			return pair_unicode_len{{
				.i32={
					static_cast<char32_t>(c[0]&0x0fu)<<12|
					static_cast<char32_t>(c[1]&0x3fu)<<6|
					static_cast<char32_t>(c[2]&0x3fu),
					3
				}
			}};
		}else{
			return pair_unicode_len{{
				.i32={
					static_cast<char32_t>(c[0]&0x0fu)<<18|
					static_cast<char32_t>(c[1]&0x3fu)<<12|
					static_cast<char32_t>(c[2]&0x3fu)<<6|
					static_cast<char32_t>(c[3]&0x3fu),
					4
				}
			}};
		}
	}
}

layeredOut::~layeredOut() noexcept{
	deleteOrder(*this);
}

layeredOut::state_t layeredOut::state() const noexcept{
	return state_v;
}
