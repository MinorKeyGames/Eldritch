#ifndef MIDI_H
#define MIDI_H

#include "array.h"

class IDataStream;
class MIDIEvent;

class MIDI
{
public:
	MIDI();
	~MIDI();

	void	Load( const IDataStream& Stream );
	void	Save( const IDataStream& Stream ) const;

	void	Log() const;

private:
	struct SMIDIChunkHeader
	{
		SMIDIChunkHeader()
		:	m_ChunkID( 0 )
		,	m_ChunkSize( 0 )
		{
		}

		uint32	m_ChunkID;
		uint32	m_ChunkSize;
	};

	struct SMIDIFileHeader
	{
		SMIDIFileHeader()
		:	m_Header()
		,	m_FormatType( 0 )
		,	m_NumTracks( 0 )
		,	m_TimeDivision( 0 )
		{
		}

		SMIDIChunkHeader	m_Header;		// "MThd", 6
		uint16				m_FormatType;	// 0-2 (0 is everything on a single track, 1 is separate tracks, 2 is separate asynchronous tracks)
		uint16				m_NumTracks;	// 1-65535
		uint16				m_TimeDivision;	// SMPTE stuff, see docs
	};

	struct SMIDITrack
	{
		SMIDITrack()
		:	m_Header()
		,	m_Events()
		{
		}

		SMIDIChunkHeader	m_Header;		// "MTrk", ...
		Array<MIDIEvent*>	m_Events;
	};

	void		LoadTrack( const IDataStream& Stream, SMIDITrack& Track );
	void		SaveTrack( const IDataStream& Stream, const SMIDITrack& Track ) const;
	void		LogTrack( const uint TrackIndex ) const;
	uint32		LoadVariableLengthValue( const IDataStream& Stream ) const;
	void		SaveVariableLengthValue( const IDataStream& Stream, uint32 Value ) const;
	MIDIEvent*	EventFactory( const uint8 EventCode, const IDataStream& Stream ) const;

	SMIDIFileHeader		m_Header;
	Array<SMIDITrack>	m_Tracks;
};

#endif // MIDI_H