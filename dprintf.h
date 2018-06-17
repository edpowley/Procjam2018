#pragma once

#define DEBUG_LOG_TO_FILE

class DebugLogFileRAII
{
public:
	static FILE* s_file;

	DebugLogFileRAII(const char* filename)
	{
		s_file = fopen(filename, "wt");
	}

	~DebugLogFileRAII()
	{
		if (s_file)
			fclose(s_file);
		s_file = nullptr;
	}
};

//-----------------------------------------------------------------------------
// Purpose: Outputs a set of optional arguments to debugging output, using
//          the printf format setting specified in fmt*.
//-----------------------------------------------------------------------------
inline void dprintf_impl(const char *fmt, ...)
{
	va_list args;
	char buffer[2048];

	va_start(args, fmt);
	vsprintf_s(buffer, fmt, args);
	va_end(args);

	printf("%s", buffer);

	if (DebugLogFileRAII::s_file)
		fprintf(DebugLogFileRAII::s_file, "%s", buffer);

	//OutputDebugStringA(buffer);
}

#define dprintf(FMT, ...) dprintf_impl("[%s:%i] " FMT, __FILE__, __LINE__, __VA_ARGS__  );
