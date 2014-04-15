#ifndef ELDRITCHFACTIONS_H
#define ELDRITCHFACTIONS_H

class HashedString;

namespace EldritchFactions
{
	enum EFactionCon
	{
		EFR_Hostile,
		EFR_Neutral,
		EFR_Friendly,
	};

	// For managing statically allocated memory
	void		AddRef();
	void		Release();

	EFactionCon	GetCon( const HashedString& FactionA, const HashedString& FactionB );
}

#endif // ELDRITCHFACTIONS_H