#include "core.h"
#include "crypto.h"
#include "simplestring.h"
#include "mathcore.h"
#include "mathfunc.h"

// ARC4 state and methods
char		S[256];
uint		i, j;
const uint	FixedStep = 16;	// Part of the secret shared by both programs
const uint	Overhead = 1;	// One extra character for the nonce

void ARC4Initialize( const Array< char >& Key )
{
	for( uint Index = 0; Index < 256; ++Index )
	{
		S[ Index ] = (char)Index;
	}

	for( i = j = 0; i < 256; ++i )
	{
		j = ( j + Key[ i % Key.Size() ] + S[i] ) & 255;
		Swap( S[i], S[j] );
	}

	i = j = 0;
}

char ARC4Step()
{
	i = ( i + 1 ) & 255;
	j = ( j + S[i] ) & 255;

	Swap( S[i], S[j] );

	return S[ ( S[i] + S[j] ) & 255 ];
}

void Crypto::Encrypt( const Array< char >& Plaintext, const Array< char >& Key, Array< char >& OutCiphertext )
{
	ARC4Initialize( Key );

	uint8 Nonce = (char)Math::Random( 256 );
	for( uint Step = 0; Step < Nonce + FixedStep; ++Step )
	{
		ARC4Step();
	}

	OutCiphertext.Clear();
	OutCiphertext.Resize( Plaintext.Size() + Overhead );

	for( uint Index = 0; Index < Plaintext.Size(); ++Index )
	{
		OutCiphertext[ Index ] = Plaintext[ Index ] ^ ARC4Step();
	}

	// Sign the array with the nonce we chose
	OutCiphertext[ OutCiphertext.Size() - 1 ] = Nonce;
}

void Crypto::Decrypt( const Array< char >& Ciphertext, const Array< char >& Key, Array< char >& OutPlaintext )
{
	ARC4Initialize( Key );

	uint8 Nonce = Ciphertext[ Ciphertext.Size() - 1 ];
	for( uint Step = 0; Step < Nonce + FixedStep; ++Step )
	{
		ARC4Step();
	}

	OutPlaintext.Clear();
	OutPlaintext.Resize( Ciphertext.Size() - Overhead );

	for( uint Index = 0; Index < Ciphertext.Size() - Overhead; ++Index )
	{
		OutPlaintext[ Index ] = Ciphertext[ Index ] ^ ARC4Step();
	}
}

void Crypto::Encrypt( const SimpleString& Plaintext, const Array< SimpleString >& Keys, Array< char >& OutCiphertext )
{
	Array< char > PlaintextArray;
	Array< char > KeyArray;
	Array< char > CiphertextArray;

	Plaintext.FillArray( PlaintextArray );

	ConstructKeyFromStrings( Keys, KeyArray );

	Encrypt( PlaintextArray, KeyArray, OutCiphertext );
}

void Crypto::Decrypt( const Array< char >& Ciphertext, const Array< SimpleString >& Keys, SimpleString& OutPlaintext )
{
	Array< char > KeyArray;
	Array< char > PlaintextArray;

	ConstructKeyFromStrings( Keys, KeyArray );

	Decrypt( Ciphertext, KeyArray, PlaintextArray );

	PlaintextArray.PushBack( '\0' );
	OutPlaintext = PlaintextArray;
}

// Add a bunch of strings together to produce a more robust key
void Crypto::ConstructKeyFromStrings( const Array< SimpleString >& Keys, Array< char >& OutKey )
{
	ASSERT( Keys.Size() );

	Array< Array< char > > KeyArrays;
	uint Length = Keys[0].Length();

	// Push arrays one at a time instead of resizing, so they are constructed and not simply allocated
	for( uint KeyIndex = 0; KeyIndex < Keys.Size(); ++KeyIndex )
	{
		const SimpleString& Key = Keys[ KeyIndex ];
		KeyArrays.PushBack( Array< char >() );
		Key.FillArray( KeyArrays[ KeyIndex ] );
		Length = Min( Length, Key.Length() );
	}

	OutKey.Clear();
	for( uint Index = 0; Index < Length; ++Index )
	{
		OutKey.PushBack( 0 );
		for( uint KeyIndex = 0; KeyIndex < Keys.Size(); ++KeyIndex )
		{
			OutKey[ Index ] = ( OutKey[ Index ] + KeyArrays[ KeyIndex ][ Index ] ) & 255;
		}
	}
}