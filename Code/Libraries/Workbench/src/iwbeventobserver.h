#ifndef IWBEVENTOBSERVER_H
#define IWBEVENTOBSERVER_H

class WBEvent;

class IWBEventObserver
{
public:
	virtual ~IWBEventObserver() {}
	virtual void HandleEvent( const WBEvent& Event ) = 0;

	// For serialization of recipients in event queue.
	// Currently, only entities can be serialized, but that should be fine.
	// No need to implement this for non-entity observers.
	virtual uint GetEntityUID() const { return 0; }
};

#endif // IWBEVENTOBSERVER_H