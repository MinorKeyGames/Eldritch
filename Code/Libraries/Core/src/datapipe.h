#ifndef DATAPIPE_H
#define DATAPIPE_H

// Shuttles data from one IDataStream to another

#include "idatastream.h"
#include "simplestring.h"

class DataPipe
{
public:
	DataPipe( const IDataStream& InStream, const IDataStream& OutStream );
	~DataPipe();

	void	Pipe( int NumBytes ) const;

	inline uint8		PipeUInt8() const { uint8 i = m_InStream.ReadUInt8(); m_OutStream.WriteUInt8(i); return i; }
	inline uint16		PipeUInt16() const { uint16 i = m_InStream.ReadUInt16(); m_OutStream.WriteUInt16(i); return i; }
	inline uint32		PipeUInt32() const { uint32 i = m_InStream.ReadUInt32(); m_OutStream.WriteUInt32(i); return i; }
	inline int8			PipeInt8() const { int8 i = m_InStream.ReadInt8(); m_OutStream.WriteInt8(i); return i; }
	inline int16		PipeInt16() const { int16 i = m_InStream.ReadInt16(); m_OutStream.WriteInt16(i); return i; }
	inline int32		PipeInt32() const { int32 i = m_InStream.ReadInt32(); m_OutStream.WriteInt32(i); return i; }
	inline float		PipeFloat() const { float f = m_InStream.ReadFloat(); m_OutStream.WriteFloat(f); return f; }
	inline bool			PipeBool() const { bool b = m_InStream.ReadBool(); m_OutStream.WriteBool(b); return b; }
	inline SimpleString	PipeString() const { SimpleString s = m_InStream.ReadString(); m_OutStream.WriteString(s); return s; }

private:
	DataPipe& operator=( const DataPipe& Pipe );

	const IDataStream&	m_InStream;
	const IDataStream&	m_OutStream;
};

#endif // DATAPIPE_H