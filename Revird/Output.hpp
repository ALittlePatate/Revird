#pragma once

#ifdef _DEBUG
	#define DbgPrint_(msg, ...) DbgPrint(msg, __VA_ARGS__)
#else
	#define DbgPrint_(msg, ...) (void)0
#endif
