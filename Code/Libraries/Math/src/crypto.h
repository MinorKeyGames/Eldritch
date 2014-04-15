#ifndef CRYPTO_H
#define CRYPTO_H

// ARC4 stream cipher. This must be kept in sync with the cryptography algorithms on the server.

#include "array.h"

class SimpleString;

namespace Crypto
{
	void	Encrypt( const Array< char >& Plaintext, const Array< char >& Key, Array< char >& OutCiphertext );
	void	Decrypt( const Array< char >& Ciphertext, const Array< char >& Key, Array< char >& OutPlaintext );

	// Helper functions
	void	Encrypt( const SimpleString& Plaintext, const Array< SimpleString >& Keys, Array< char >& OutCiphertext );
	void	Decrypt( const Array< char >& Ciphertext, const Array< SimpleString >& Keys, SimpleString& OutPlaintext );
	void	ConstructKeyFromStrings( const Array< SimpleString >& Keys, Array< char >& OutKey );
}

#endif