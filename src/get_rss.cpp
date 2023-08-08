#include <get_rss.hpp>

#include <cstdio>
#include <cstdlib>
#ifdef _MSC_VER
#include <psapi.h>
#endif

std::size_t alloc_test::get_rss() {
#ifdef _MSC_VER
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS pmc;
	hProcess = GetCurrentProcess();
	BOOL ok = GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc));
    CloseHandle( hProcess );
    if ( ok )
		return pmc.PagefileUsage >> 12; // note: we may also be interested in 'PeakPagefileUsage'
	else
		return 0;
#else
	// see http://man7.org/linux/man-pages/man5/proc.5.html for details
    std::FILE* fstats = std::fopen( "/proc/self/statm", "rb" );
	constexpr std::size_t buffsz = 0x1000;
	char buff[buffsz];
	buff[buffsz-1] = 0;
    std::fread( buff, 1, buffsz-1, fstats);
    std::fclose( fstats);
	const char* pos = buff;
	while ( *pos && *pos == ' ' ) ++pos;
	while ( *pos && *pos != ' ' ) ++pos;
	return std::atol( pos );
#endif
}
