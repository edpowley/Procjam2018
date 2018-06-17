#include "stdafx.h"
#include "BinaryFileView.h"

BinaryFileView::BinaryFileView()
{

}

BinaryFileView::BinaryFileView(const std::string& path, int64_t offset, int64_t length)
{
	open(path, offset, length);
}

BinaryFileView::~BinaryFileView()
{
	close();
}

void BinaryFileView::open(const std::string& path, int64_t offset, int64_t length)
{
	if (m_file)
		close();

	m_path = path;
	m_offset = offset;
	m_length = length;
	m_currentPosition = 0;

	m_file.open(m_path, std::ios::binary);
	m_file.seekg(m_offset);
}

void BinaryFileView::close()
{
	m_path = "";
	m_file.close();
}

bool BinaryFileView::isEOF()
{
	return m_file.eof() || m_currentPosition >= m_length;
}

void BinaryFileView::seek(int64_t pos)
{
	if (pos != m_currentPosition)
	{
		m_file.seekg(m_offset + pos);
		m_currentPosition = pos;
	}
}

void BinaryFileView::readRaw(void *buf, int64_t numBytes)
{
	m_file.read((char*)buf, numBytes);
	m_currentPosition += numBytes;
}
