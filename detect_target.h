#ifndef __target_sys_H__
#define __target_sys_H__
#include<cstdint>

namespace target_sys{
	constexpr uint32_t target_sys = []{
		uint32_t val = 0;
		#if __has_include(<unistd.h>)
			val |= 0x00'00'00'01;
		#endif
		#if defined(__unix__) || defined(__unix)
			val |= 0x00'00'00'02;
		#endif
		#ifdef __linux__
			val |= 0x00'00'00'04;
		#endif
		#ifdef __gnu_linux__
			val |= 0x00'00'00'08;
		#endif
		#ifdef linux
			val |= 0x00'00'00'10;
		#endif
		#ifdef __linux__
			val |= 0x00'00'00'20;
		#endif
		#ifdef __linux
			val |= 0x00'00'00'40;
		#endif
		#ifdef __ANDROID__
			val |= 0x00'00'00'80;
		#endif
		#ifdef __ANDROID_API__
			val |= 0x00'00'01'00;
		#endif
		#ifdef __FreeBSD__
			val |= 0x00'00'02'00;
		#endif
		#ifdef __NetBSD__
			val |= 0x00'00'04'00;
		#endif
		#ifdef __OpenBSD__
			val |= 0x00'00'08'00;
		#endif
		#ifdef __bsdi__
			val |= 0x00'00'10'00;
		#endif
		#ifdef __DragonFly__
			val |= 0x00'00'20'00;
		#endif
		#ifdef BSD
			val |= 0x00'00'40'00;
		#endif
		#ifdef macintosh
			val |= 0x00'00'80'00;
		#endif
		#ifdef Macintosh
			val |= 0x00'01'00'00;
		#endif
		#ifdef __APPLE__
			val |= 0x00'02'00'00;
		#endif
		#ifdef __MACH__
			val |= 0x00'04'00'00;
		#endif
		#if __has_include(<windows.h>)
			val |= 0x00'08'00'00;
		#endif
		#ifdef _WIN16
			val |= 0x00'10'00'00;
		#endif
		#ifdef _WIN32
			val |= 0x00'20'00'00;
		#endif
		#ifdef _WIN64
			val |= 0x00'40'00'00;
		#endif
		#ifdef __WIN32__
			val |= 0x00'80'00'00;
		#endif
		#ifdef __TOS_WIN__
			val |= 0x01'00'00'00;
		#endif
		#ifdef __WINDOWS__
			val |= 0x02'00'00'00;
		#endif
		#ifdef _WIN32_WCE
			val |= 0x04'00'00'00;
		#endif
		#ifdef MSDOS
			val |= 0x08'00'00'00;
		#endif
		#ifdef __MSDOS__
			val |= 0x10'00'00'00;
		#endif
		#ifdef _MSDOS
			val |= 0x20'00'00'00;
		#endif
		#ifdef __DOS__
			val |= 0x40'00'00'00;
		#endif
		#ifdef __DOS
			val |= 0x80'00'00'00;
		#endif

		return val;
	}();
}
// __unix__ __unix
// __gnu_linux__ __linux__ linux __linux
// __ANDROID__ __ANDROID_API__
// __FreeBSD__ __NetBSD__ __OpenBSD__ __bsdi__ __DragonFly__ BSD
// macintosh Macintosh __APPLE__ __MACH__
// _WIN16 _WIN32 _WIN64 __WIN32__ __TOS_WIN__ __WINDOWS__ _WIN32_WCE
// MSDOS __MSDOS__ _MSDOS __DOS__ __DOS
#endif//__target_sys_H__
