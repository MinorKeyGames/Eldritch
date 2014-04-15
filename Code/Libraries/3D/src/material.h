#ifndef MATERIAL_H
#define MATERIAL_H

#include "3d.h"
#include "vector4.h"
#include "simplestring.h"
#include "array.h"
#include "vector2.h"
#include "vector.h"
#include "renderstates.h"

class ITexture;
class IShaderProgram;
class ShaderDataProvider;
class IVertexDeclaration;

#define DEFAULT_NEWMATERIAL	"Material_Min"

// TODO: Replace with a data-driven flag system for better per-project support
#define MAT_NONE			0x00000000
#define MAT_SHADOW_CASTER	0x00000001
#define MAT_SHADOW_RECEIVER	0x00000002
#define MAT_ALPHA			0x00000004
#define MAT_ALPHATEST		0x00000008
#define MAT_HUD				0x00000010
#define MAT_WORLD			0x00000020	// i.e., not HUD
#define MAT_SKY				0x00000040
#define MAT_POSTFX			0x00000080
#define MAT_BLOOM			0x00000100
#define MAT_ANIMATED		0x00000200
#define MAT_FOREGROUND		0x00000400
#define MAT_INWORLDHUD		0x00000800
#define MAT_DYNAMIC			0x00001000

#define MAT_DEBUG_WORLD		0x00010000
#define MAT_DEBUG_HUD		0x00020000

#if BUILD_DEV
#define MAT_DEBUG_ALWAYS	0x00040000	// Always draw this, even if frustum-culled, etc.
#endif

#define MAT_ALWAYS			0x00080000	// Always draw this, even if frustum-culled, etc.

#define MAT_STEREO_LEFT		0x00100000
#define MAT_STEREO_RIGHT	0x00200000

// This sort of thing shouldn't be a material flag, it should be part of a whole separate scene with separate buckets.
// But as I'm not in a place to refactor my renderer to support that concept at the moment, here it is.
// HACK for Eldritch to render character in mirror RT.
#define MAT_OFFSCREEN_0		0x00400000
// HACK for Eldritch to render to minimap RT.
#define MAT_OFFSCREEN_1		0x00800000

#define MAT_ALL				0xffffffff

class Material
{
public:
	Material();
	~Material();

	void	SetDefinition( const SimpleString& DefinitionName, IRenderer* const pRenderer, IVertexDeclaration* const pVertexDeclaration );
	void	SetSamplerDefinition( const uint SamplerStage, const SimpleString& SamplerDefinitionName );
	void	SetTexture( const uint SamplerStage, ITexture* const pTexture );

	IShaderProgram*			GetShaderProgram() const { return m_ShaderProgram; }
	ITexture*				GetTexture( const uint SamplerStage ) const;
	ShaderDataProvider*		GetSDP() const { return m_SDP; }
	const SRenderState&		GetRenderState() const { return m_RenderState; }
	const SSamplerState&	GetSamplerState( const uint SamplerStage ) const;
	uint					GetNumSamplers() const { return m_NumSamplers; }

	uint					GetFlags() const { return m_Flags; }
	void					SetFlags( const uint Flags, const uint Mask = MAT_ALL );
	void					SetFlag( const uint Flag, const bool Set );

private:
	IShaderProgram*		m_ShaderProgram;
	ShaderDataProvider*	m_SDP;
	uint				m_Flags;	// TODO: Move to a data-driven/project-specific system for declaring flags

	SRenderState		m_RenderState;
	SSamplerState		m_SamplerStates[ MAX_TEXTURE_STAGES ];
	uint				m_NumSamplers;
};

#endif // MATERIAL_H