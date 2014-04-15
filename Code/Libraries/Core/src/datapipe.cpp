#include "core.h"
#include "datapipe.h"
#include "idatastream.h"

DataPipe::DataPipe( const IDataStream& InStream, const IDataStream& OutStream )
:	m_InStream( InStream )
,	m_OutStream( OutStream )
{
}

DataPipe::~DataPipe()
{
}

DataPipe& DataPipe::operator=( const DataPipe& Pipe )
{
	Unused( Pipe );
	return *this;
}

void DataPipe::Pipe( int NumBytes ) const
{
	byte* pBuffer = new byte[ NumBytes ];
	m_InStream.Read( NumBytes, pBuffer );
	m_OutStream.Write( NumBytes, pBuffer );
	SafeDeleteArray( pBuffer );
}