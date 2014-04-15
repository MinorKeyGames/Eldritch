#ifndef CHECKFORUPDATES_H
#define CHECKFORUPDATES_H

// Project should include ContentSyncer.exe alongside, with the proper configuration.
// Project should also provide these strings for the UI:
// CouldNotConnect
// Connecting
// CouldNotReceive
// UpToDate
// ShouldUpdate
// CouldNotRunContentSyncer

#include "array.h"
#include "httpsocket.h"

class UIManagerCommon;

class CheckForUpdates
{
public:
	CheckForUpdates( UIManagerCommon* pUIManager );
	~CheckForUpdates();

	void					Tick();
	void					Check( bool Verbose, bool PausesGame );

private:
	bool					IsRequestFinished();
	void					InitSocket( HTTPSocket::SSocketInit& SocketInit );

	void					OnReceive();
	static void				TryRunContentSyncer( void* pUIElement, void* pVoid );

	UIManagerCommon*		m_UIManager;

	HTTPSocket				m_Socket;
	Array< char >			m_ResultBuffer;

	bool					m_WaitingToConnect;
	bool					m_WaitingToReceive;
	bool					m_Verbose;
	bool					m_PausesGame;
};

#endif // CHECKFORUPDATES_H