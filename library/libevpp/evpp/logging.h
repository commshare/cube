#pragma once

#include "evpp/platform_config.h"
#include <vector>
#ifdef __cplusplus

#define DLOG_TRACE std::cout << __FILE__ << ":" << __LINE__ << " "
#define DLOG_DEBUG std::cout << __FILE__ << ":" << __LINE__ << " "
#define DLOG_INFO  std::cout << __FILE__ << ":" << __LINE__ << " "
#define DLOG_WARN  std::cout << __FILE__ << ":" << __LINE__ << " "
#define DLOG_ERROR std::cout << __FILE__ << ":" << __LINE__ << " "
#define DLOG_FATAL std::cout << __FILE__ << ":" << __LINE__ << " "
#define LOG_TRACE std::cout << __FILE__ << ":" << __LINE__ << " "
#define LOG_DEBUG std::cout << __FILE__ << ":" << __LINE__ << " "
#define LOG_INFO  std::cout << __FILE__ << ":" << __LINE__ << " "
#define LOG_WARN  std::cout << __FILE__ << ":" << __LINE__ << " "
#define LOG_ERROR std::cout << __FILE__ << ":" << __LINE__ << " "
#define LOG_FATAL std::cout << __FILE__ << ":" << __LINE__ << " "
#define CHECK_NOTnullptr(val) LOG_ERROR << "'" #val "' Must be non nullptr";

#endif // end of define __cplusplus