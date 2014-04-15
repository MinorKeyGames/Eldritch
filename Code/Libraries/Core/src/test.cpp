#include "core.h"
#include "test.h"
#include "filestream.h"

#include <string.h>

Test* Test::m_Instance = NULL;

Test::Test( const char* Filename ) : m_TotalTests(0), m_SuccessfulTests(0)
{
	m_Stream = new FileStream( Filename, FileStream::EFM_Write );
}

Test::~Test()
{
	ASSERT( m_SuccessfulTests <= m_TotalTests );
	CloseReport();
	delete m_Stream;
}

Test* Test::GetInstance( const char* Filename )
{
	if( !m_Instance )
	{
		m_Instance = new Test( Filename );
	}
	return m_Instance;
}

void Test::DeleteInstance()
{
	SafeDelete( m_Instance );
}

void Test::IncrementTotal()
{
	++m_TotalTests;
}

void Test::IncrementSuccessful()
{
	++m_SuccessfulTests;
}

int Test::GetNumFailed()
{
	return m_TotalTests - m_SuccessfulTests;
}

void Test::AddToReport( const char* Expression, bool Successful )
{
	if( Successful )
	{
		m_Stream->Write( 11, "Succeeded: " );
	}
	else
	{
		m_Stream->Write( 11, "Failed:    " );
	}
	m_Stream->Write( (int)strlen( Expression ), Expression );
	m_Stream->WriteInt8( '\n' );
}

void Test::CloseReport()
{
	m_Stream->PrintF( "\n%d tests, %d succeeded, %d failed.", m_TotalTests, m_SuccessfulTests, m_TotalTests - m_SuccessfulTests );
}