#include "core.h"
#include "midi.h"
#include "idatastream.h"
#include "endian.h"

#include "midinoteoffevent.h"
#include "midinoteonevent.h"
#include "midicontrollerevent.h"
#include "midiprogramchangeevent.h"
#include "midipitchbendevent.h"

MIDI::MIDI()
:	m_Header()
,	m_Tracks()
{
}

MIDI::~MIDI()
{
	for( uint TrackIndex = 0; TrackIndex < m_Tracks.Size(); ++TrackIndex )
	{
		for( uint EventIndex = 0; EventIndex < m_Tracks[ TrackIndex ].m_Events.Size(); ++EventIndex )
		{
			SafeDelete( m_Tracks[ TrackIndex ].m_Events[ EventIndex ] );
		}
	}
}

void MIDI::Load( const IDataStream& Stream )
{
	m_Tracks.Clear();

	Stream.Read( 14, &m_Header );	// sizeof( SMIDIFileHeader ) is rounded to word size, so we can't use that

	Endian::SwapInPlace( m_Header.m_Header.m_ChunkID );
	Endian::SwapInPlace( m_Header.m_Header.m_ChunkSize );
	Endian::SwapInPlace( m_Header.m_FormatType );
	Endian::SwapInPlace( m_Header.m_NumTracks );
	Endian::SwapInPlace( m_Header.m_TimeDivision );

	ASSERT( m_Header.m_Header.m_ChunkID == 'MThd' );
	ASSERT( m_Header.m_Header.m_ChunkSize == 6 );
	ASSERT( m_Header.m_FormatType <= 2 );

	for( uint TrackIndex = 0; TrackIndex < m_Header.m_NumTracks; ++TrackIndex )
	{
		//PRINTF( "======== Loading track %d ========\n", TrackIndex );
		m_Tracks.PushBack( SMIDITrack() );
		LoadTrack( Stream, m_Tracks[ TrackIndex ] );
	}
}

void MIDI::LoadTrack( const IDataStream& Stream, SMIDITrack& Track )
{
	Stream.Read( 8, &Track.m_Header );

	Endian::SwapInPlace( Track.m_Header.m_ChunkID );
	Endian::SwapInPlace( Track.m_Header.m_ChunkSize );

	ASSERT( Track.m_Header.m_ChunkID == 'MTrk' );

	const int StartPos = Stream.GetPos();
	uint8 RunningEventCode = 0;
	while( ( uint )( Stream.GetPos() - StartPos ) < Track.m_Header.m_ChunkSize )
	{
		// We expect 1-4 bytes of a variable length delta time for each event
		const uint32 DeltaTime = LoadVariableLengthValue( Stream );

		// Followed by an event code byte (which also includes the channel, for MIDI events)
		const uint8 EventCode = Stream.ReadUInt8();
		MIDIEvent* pEvent = NULL;

		if( EventCode < 0x80 )
		{
			// Running status, rewind the stream and create another of the last MIDI event
			Stream.SetPos( Stream.GetPos() - 1 );
			pEvent = EventFactory( RunningEventCode, Stream );
		}
		else
		{
			RunningEventCode = EventCode;
			pEvent = EventFactory( EventCode, Stream );
		}

		//PRINTF( "Time: %d, Code: 0x%02X\n", DeltaTime, EventCode );

		if( pEvent )
		{
			pEvent->Initialize( DeltaTime, EventCode );
			pEvent->Load( Stream );

			Track.m_Events.PushBack( pEvent );
		}
	}
}

uint32 MIDI::LoadVariableLengthValue( const IDataStream& Stream ) const
{
	// We expect 1-4 bytes of a variable length delta time for each event
	uint32 VariableLengthValue = 0;
	uint8 NextChar = 0;
	do
	{
		NextChar = Stream.ReadUInt8();
		VariableLengthValue <<= 7;
		VariableLengthValue |= ( NextChar & 0x7F );
	}
	while( NextChar & 0x80 );

	return VariableLengthValue;
}

MIDIEvent* MIDI::EventFactory( const uint8 EventCode, const IDataStream& Stream ) const
{
	const uint8 MIDIEventType = ( EventCode & 0xF0 ) >> 4;

	if( MIDIEventType == 0x08 )
	{
		// Note off
		return new MIDINoteOffEvent();
	}
	else if( MIDIEventType == 0x09 )
	{
		// Note on
		return new MIDINoteOnEvent();
	}
	else if( MIDIEventType == 0x0A )
	{
		// Note aftertouch
	}
	else if( MIDIEventType == 0x0B )
	{
		// Controller
		return new MIDIControllerEvent();
	}
	else if( MIDIEventType == 0x0C )
	{
		// Program change
		return new MIDIProgramChangeEvent();
	}
	else if( MIDIEventType == 0x0D )
	{
		// Channel aftertouch
	}
	else if( MIDIEventType == 0x0E )
	{
		// Pitch bend
		return new MIDIPitchBendEvent();
	}
	else if( EventCode == 0xF0 || EventCode == 0xF7 )
	{
		// SysEx event

		// For now, skip SysEx events because I don't care (TODO: I'll need to load them to resave them, though)
		const uint SysExLength = LoadVariableLengthValue( Stream );
		Stream.Skip( SysExLength );

		//PRINTF( "Skipped SysEx of length %d\n", SysExLength );

		return NULL;
	}
	else if( EventCode == 0xFF )
	{
		// Meta event
		const uint8 MetaType = Stream.ReadUInt8();
		Unused( MetaType );

		// For now, skip meta events because I don't care (TODO: I'll need to load them to resave them, though)
		const uint MetaLength = LoadVariableLengthValue( Stream );
		//Stream.Skip( MetaLength );
		Array<uint8> MetaString;
		MetaString.Resize( MetaLength );
		Stream.Read( MetaLength, MetaString.GetData() );
		MetaString.PushBack( '\0' );

		//PRINTF( "Skipped meta (0x%02X) of length %d\n", MetaType, MetaLength );
		//PRINTF( "%s\n", MetaString.GetData() );

		return NULL;
	}
	else
	{
		// Unknown event
		WARN;
		return NULL;
	}

	// Not yet handled
	WARN;
	return NULL;
}

void MIDI::Save( const IDataStream& Stream ) const
{
	ASSERT( m_Header.m_Header.m_ChunkID == 'MThd' );
	ASSERT( m_Header.m_Header.m_ChunkSize == 6 );
	ASSERT( m_Header.m_FormatType <= 2 );

	SMIDIFileHeader HeaderCopy = m_Header;

	Endian::SwapInPlace( HeaderCopy.m_Header.m_ChunkID );
	Endian::SwapInPlace( HeaderCopy.m_Header.m_ChunkSize );
	Endian::SwapInPlace( HeaderCopy.m_FormatType );
	Endian::SwapInPlace( HeaderCopy.m_NumTracks );
	Endian::SwapInPlace( HeaderCopy.m_TimeDivision );

	Stream.Write( 14, &HeaderCopy );

	for( uint TrackIndex = 0; TrackIndex < m_Header.m_NumTracks; ++TrackIndex )
	{
		//PRINTF( "======== Saving track %d ========\n", TrackIndex );
		SaveTrack( Stream, m_Tracks[ TrackIndex ] );
	}
}

// NOTE: This doesn't currently set up the chunk length, it just assumes it has been filled in properly already.
void MIDI::SaveTrack( const IDataStream& Stream, const SMIDITrack& Track ) const
{
	SMIDIChunkHeader TrackHeaderCopy = Track.m_Header;

	ASSERT( TrackHeaderCopy.m_ChunkID == 'MTrk' );

	Endian::SwapInPlace( TrackHeaderCopy.m_ChunkID );
	Endian::SwapInPlace( TrackHeaderCopy.m_ChunkSize );

	Stream.Write( 8, &TrackHeaderCopy );

	for( uint EventIndex = 0; EventIndex < Track.m_Events.Size(); ++EventIndex )
	{
		const MIDIEvent* const pEvent = Track.m_Events[ EventIndex ];

		SaveVariableLengthValue( Stream, pEvent->GetDeltaTime() );

		// TODO: Save running event codes properly. Could be tricky.

		pEvent->Save( Stream );
	}
}

void MIDI::SaveVariableLengthValue( const IDataStream& Stream, uint32 Value ) const
{
	while( Value > 0x7F )
	{
		Stream.WriteUInt8( ( uint8 )( 0x80 | ( Value & 0x7F ) ) );
		Value >>= 7;
	}

	Stream.WriteUInt8( ( uint8 )Value );
}

void MIDI::Log() const
{
	PRINTF( "MIDI log\n\n" );

	PRINTF( "Header:\n" );
	PRINTF( "Chunk ID: 0x%08X\n", m_Header.m_Header.m_ChunkID );
	PRINTF( "Chunk size: %d\n", m_Header.m_Header.m_ChunkSize );
	PRINTF( "Format: %d\n", m_Header.m_FormatType );
	PRINTF( "# Tracks: %d\n", m_Header.m_NumTracks );
	PRINTF( "Time Division: %d\n", m_Header.m_TimeDivision );

	for( uint TrackIndex = 0; TrackIndex < m_Header.m_NumTracks; ++TrackIndex )
	{
		LogTrack( TrackIndex );
	}
}

void MIDI::LogTrack( const uint TrackIndex ) const
{
	const SMIDITrack& Track = m_Tracks[ TrackIndex ];

	PRINTF( "Track %d:\n", TrackIndex );

	// TODO
	Unused( Track );
}