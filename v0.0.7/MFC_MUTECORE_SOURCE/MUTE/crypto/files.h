#ifndef CRYPTOPP_FILES_H
#define CRYPTOPP_FILES_H

#include "cryptlib.h"
#include "filters.h"

#include <iostream>
#include <fstream>

NAMESPACE_BEGIN(CryptoPP)

//! .
class FileStore : public Store
{
public:
	class Err : public BufferedTransformation::Err
	{
	public:
		Err(const std::string &s) : BufferedTransformation::Err(INPUT_ERROR, s) {}
	};
	class OpenErr : public Err {public: OpenErr(const std::string &filename) : Err("FileStore: error opening file for reading: " + filename) {}};
	class ReadErr : public Err {public: ReadErr() : Err("FileStore: error reading file") {}};

	FileStore(std::istream &in);
	FileStore(const char *filename);

	std::istream& GetStream() {return m_in;}

	unsigned long MaxRetrievable() const;
	unsigned int Peek(byte &outByte) const;

	unsigned long TransferTo(BufferedTransformation &target, unsigned long transferMax=ULONG_MAX);
	unsigned long CopyTo(BufferedTransformation &target, unsigned long copyMax=ULONG_MAX) const;

private:
	std::ifstream m_file;
	std::istream &m_in;
	SecByteBlock m_buffer;
};

//! .
class FileSource : public Source
{
public:
	typedef FileStore::Err Err;
	typedef FileStore::OpenErr OpenErr;
	typedef FileStore::ReadErr ReadErr;

	FileSource(std::istream &in, bool pumpAll, BufferedTransformation *outQueue = NULL);
	FileSource(const char *filename, bool pumpAll, BufferedTransformation *outQueue = NULL);

	std::istream& GetStream() {return m_store.GetStream();}

	unsigned long Pump(unsigned long pumpMax=ULONG_MAX)
		{return m_store.TransferTo(*AttachedTransformation(), pumpMax);}
	unsigned int PumpMessages(unsigned int count=UINT_MAX)
		{return m_store.TransferMessagesTo(*AttachedTransformation(), count);}

private:
	FileStore m_store;
};

//! .
class FileSink : public Sink
{
public:
	class Err : public BufferedTransformation::Err
	{
	public:
		Err(const std::string &s) : BufferedTransformation::Err(OUTPUT_ERROR, s) {}
	};
	class OpenErr : public Err {public: OpenErr(const std::string &filename) : Err("FileSink: error opening file for writing: " + filename) {}};
	class WriteErr : public Err {public: WriteErr() : Err("FileSink: error writing file") {}};

	FileSink(std::ostream &out);
	FileSink(const char *filename, bool binary=true);

	std::ostream& GetStream() {return out;}

	void Flush(bool=true, int=-1);
	void MessageEnd(int=-1);
	void Put(byte inByte)
	{
		out.put(inByte);
		if (!out.good())
		  throw WriteErr();
	}

	void Put(const byte *inString, unsigned int length);

private:
	std::ofstream file;
	std::ostream& out;
};

NAMESPACE_END

#endif
