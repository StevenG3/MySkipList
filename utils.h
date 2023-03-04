#ifndef __UTILS_H__
#define __UTILS_H__

/* TRACE_CMH: 格式化追踪宏 */
#define __output(...) \
	printf(__VA_ARGS__);

#define __format(__fmt__) "%s(%d)-<%s>: " __fmt__ "\n"

#define TRACE_CMH(__fmt__, ...) \
	__output(__format(__fmt__), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);

#endif