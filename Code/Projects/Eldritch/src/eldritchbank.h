#ifndef ELDRITCHBANK_H
#define ELDRITCHBANK_H

#include "iwbeventobserver.h"

class IDataStream;

class EldritchBank : public IWBEventObserver
{
public:
	EldritchBank();
	~EldritchBank();

	// IWBEventObserver
	virtual void	HandleEvent( const WBEvent& Event );

	void			RegisterForEvents();

	uint			GetBankMoney() const;
	void			SetBankMoney( const uint Money );

	bool			HasBankMoney( uint Money ) { return Money <= GetBankMoney(); }
	void			AddBankMoney( uint Money );
	void			RemoveBankMoney( uint Money );

private:
	void			TryBankTransaction( const int Amount );
	void			PublishToHUD() const;
};

#endif // ELDRITCHBANK_H