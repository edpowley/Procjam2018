#pragma once

class Exception : public std::exception
{
public:
	Exception() {}
	Exception(const Exception& other)
		: m_whatStream(other.m_whatStream.str())
	{
	}

	const char* what() const override 
	{
		m_whatString = m_whatStream.str();
		return m_whatString.c_str();
	}

	template<typename T>
	Exception& operator<<(const T& value)
	{
		m_whatStream << value;
		return *this;
	}

private:
	mutable std::string m_whatString;
	std::ostringstream m_whatStream;
};

#define THROW(EXCEPTION) { \
	Exception exception = (EXCEPTION); \
	dprintf("----- Exception at %s line %d -----\n", __FILE__, __LINE__); \
	dprintf("%s\n", exception.what()); \
	dprintf("----------------\n"); \
	throw exception; \
}
