#include "core.h"
#include "d3d9constanttable.h"
#include "idatastream.h"

// See documentation for D3DXSHADER_CONSTANTTABLE
#define VERTEXSHADERMASK	0xfffe0000
#define PIXELSHADERMASK		0xffff0000
#define COMMENTMASK			0x0000fffe
#define CONSTANTTABLEID		0x42415443	// CTAB
#define HEADEROFFSET		0x0000000c

// Mirrors D3DXSHADER_CONSTANTTABLE, see documentation for details
struct SConstantTable
{
	uint	m_Size;
	uint	m_CreatorOffset;
	uint	m_Version;
	uint	m_NumConstants;
	uint	m_ConstantsOffset;
	uint	m_Flags;
	uint	m_TargetOffset;
};

// Mirrors D3DXSHADER_CONSTANTINFO
struct SConstantInfo
{
	uint	m_NameOffset;			// Contrary to documentation, this is the offset from the constant table, *not* from this structure
	uint16	m_RegisterSet;
	uint16	m_RegisterIndex;
	uint16	m_RegisterCount;
	uint16	m_Padding;
	uint	m_TypeInfoOffset;		// Probably don't need this.
	uint	m_DefaultValueOffset;	// Might need this, if defaults aren't automagically initialized.
};

// Mirrors D3DXREGISTER_SET
// Which type of register is the constant bound to
enum ERegisterSet
{ 
	ERS_Bool,
	ERS_Int4,
	ERS_Float4,
	ERS_Sampler,
};

void D3D9ConstantTable::ParseConstantTable( const IDataStream& Stream, Map<HashedString, uint>& OutTable )
{
	const uint Version = Stream.ReadUInt32();
	// If I need it, version number (major, minor) is in low bytes.
	DEVASSERT(
		( Version & VERTEXSHADERMASK ) == VERTEXSHADERMASK ||
		( Version & PIXELSHADERMASK ) == PIXELSHADERMASK );
	Unused( Version );

	const uint Token = Stream.ReadUInt32();
	DEVASSERT( ( Token & COMMENTMASK ) == COMMENTMASK );
	Unused( Token );

	const uint ID = Stream.ReadUInt32();
	DEVASSERT( ID == CONSTANTTABLEID );
	Unused( ID );

	SConstantTable ConstantTable;
	Stream.Read( sizeof( SConstantTable ), &ConstantTable );
	DEVASSERT( ConstantTable.m_Size == sizeof( SConstantTable ) );
	DEVASSERT( ConstantTable.m_NumConstants == 0 || ConstantTable.m_ConstantsOffset == ConstantTable.m_Size );

	for( uint ConstantIndex = 0; ConstantIndex < ConstantTable.m_NumConstants; ++ConstantIndex )
	{
		SConstantInfo ConstantInfo;
		Stream.Read( sizeof( SConstantInfo ), &ConstantInfo );

		const int Pos = Stream.GetPos();

		Stream.SetPos( ConstantInfo.m_NameOffset + HEADEROFFSET );
		const SimpleString ConstantName = Stream.ReadCString();
		Stream.SetPos( Pos );

		DEBUGASSERT( OutTable.Search( ConstantName ).IsNull() );
		OutTable.Insert( ConstantName, ConstantInfo.m_RegisterIndex );
	}
}