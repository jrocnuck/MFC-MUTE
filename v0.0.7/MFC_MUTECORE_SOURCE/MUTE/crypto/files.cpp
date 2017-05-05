/*
 * Modificaton History
 *
 * 2002-August-5   Jason Rohrer
 * Fixed a typo that caused Win32 compiler to complain.
 */
 
 
 
// files.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "files.h"

NAMESPACE_BEGIN(CryptoPP)


/*
 * 2002-June-26   Jason Rohrer
 * Fixed this consistency bug.
 */
USING_NAMESPACE( std )
//using namespace std;


static const unsigned int BUFFER_SIZE = 1024;

FileStore::FileStore(istream &i)
	: m_in(i)
{
}

FileStore::FileStore(const char *filename)
	: m_file(filename, ios::in | ios::binary), m_in(m_file)
{
	if (!m_file)
		throw OpenErr(filename);
}

unsigned long FileStore::MaxRetrievable() const
{
	streampos current = m_in.tellg();
	streampos end = m_in.rdbuf()->pubseekoff(0, ios::end, ios::in);
	m_in.rdbuf()->pubseekpos(current);
	return end-current;
}

unsigned int FileStore::Peek(byte &outByte) const
{
	int result = m_in.peek();
	if (result == EOF)	// GCC workaround: 2.95.2 doesn't have char_traits<char>::eof()
		return 0;
	else
	{
		outByte = byte(result);
		return 1;
	}
}

unsigned long FileStore::TransferTo(BufferedTransformation &target, unsigned long size)
{
	m_buffer.Resize(BUFFER_SIZE);
	unsigned long total=0;

	while (size && m_in.good())
	{
		m_in.read((char *)m_buffer.ptr, STDMIN(size, (unsigned long)BUFFER_SIZE));
		unsigned int l = m_in.gcount();
		target.Put(m_buffer, l);
		size -= l;
		total += l;
	}

	if (!m_in.good() && !m_in.eof())
		throw ReadErr();

	return total;
}

unsigned long FileStore::CopyTo(BufferedTransformation &target, unsigned long copyMax) const
{
	unsigned long total = const_cast<FileStore *>(this)->TransferTo(target, copyMax);
	m_in.clear();
	m_in.seekg(-std::streamoff(total), ios::cur);	// GCC workaround: 2.95.2 doesn't have istream::off_type
	return total;
}

FileSource::FileSource (istream &i, bool pumpAll, BufferedTransformation *outQueue)
	: Source(outQueue), m_store(i)
{
	if (pumpAll)
		PumpAll();
}

FileSource::FileSource (const char *filename, bool pumpAll, BufferedTransformation *outQueue)
	: Source(outQueue), m_store(filename)
{
	if (pumpAll)
		PumpAll();
}

FileSink::FileSink(ostream &o)
	: out(o)
{
}

FileSink::FileSink(const char *filename, bool binary)
	: file(filename, ios::out | (binary ? ios::binary : ios::openmode(0)) | ios::trunc), out(file)
{
	if (!file)
		throw OpenErr(filename);
}

void FileSink::Flush(bool, int)
{
	out.flush();
	if (!out.good())
	  throw WriteErr();
}

void FileSink::MessageEnd(int)
{
	Flush(true);
}

void FileSink::Put(const byte *inString, unsigned int length)
{
	out.write((const char *)inString, length);
	if (!out.good())
	  throw WriteErr();
}

NAMESPACE_END
