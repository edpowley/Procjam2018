#include "stdafx.h"
#include "dprintf.h"

FILE* DebugLogFileRAII::s_file = nullptr;

#ifdef DEBUG_LOG_TO_FILE

DebugLogFileRAII g_debugLogFileRAII("log.txt");

#endif
