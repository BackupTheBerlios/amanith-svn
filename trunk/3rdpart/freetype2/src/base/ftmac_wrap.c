// simple wrapping that allows mac support only on Macs
#if defined(__APPLE__) && defined(__GNUC__)
	#include "ftmac.c"
#elif defined(__MACOSX__)
	#include "ftmac.c"
#elif defined(macintosh)
	#include "ftmac.c"
#endif
