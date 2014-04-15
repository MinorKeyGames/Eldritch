#include "core.h"
#include "gl2vertexshader.h"
#include "idatastream.h"
#include "memorystream.h"

GL2VertexShader::GL2VertexShader()
:	m_VertexShader( 0 )
{
}

GL2VertexShader::~GL2VertexShader()
{
	if( m_VertexShader != 0 )
	{
		glDeleteShader( m_VertexShader );
	}
}

void GL2VertexShader::Initialize( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const int Length	= Stream.Size();
	byte* pBuffer		= new byte[ Length ];
	Stream.Read( Length, pBuffer );

	m_VertexShader = glCreateShader( GL_VERTEX_SHADER );
	ASSERT( m_VertexShader != 0 );

	// Copy the GLSL source
	const GLsizei	NumStrings		= 1;
	const GLchar*	Strings[]		= { reinterpret_cast<GLchar*>( pBuffer ) };
	const GLint		StringLengths[]	= { Length };	// I don't trust this file to be null-terminated, so explicitly declare the length.
	glShaderSource( m_VertexShader, NumStrings, Strings, StringLengths );

	// Compile the shader
	glCompileShader( m_VertexShader );
	GLERRORCHECK;

	GLint CompileStatus;
	glGetShaderiv( m_VertexShader, GL_COMPILE_STATUS, &CompileStatus );

	if( CompileStatus != GL_TRUE )
	{
		GLint LogLength;
		glGetShaderiv( m_VertexShader, GL_INFO_LOG_LENGTH, &LogLength );
		Array<GLchar>	Log;
		Log.Resize( LogLength );
		glGetShaderInfoLog( m_VertexShader, LogLength, NULL, Log.GetData() );
		if( LogLength > 0 )
		{
			PRINTF( "GLSL vertex shader compile failed:\n" );
			PRINTF( Log.GetData() );
		}
		WARNDESC( "GLSL vertex shader compile failed" );
	}

	SafeDeleteArray( pBuffer );
}

/*virtual*/ bool GL2VertexShader::GetRegister( const HashedString& Parameter, uint& Register ) const
{
	Unused( Parameter );
	Unused( Register );

	// Shouldn't be called. Handled by GL2ShaderProgram.
	WARN;
	return false;
}