#ifndef IINPUTSYSTEMOBSERVER_H
#define IINPUTSYSTEMOBSERVER_H

class IInputSystemObserver
{
public:
	virtual ~IInputSystemObserver()
	{
	}

	virtual void OnInputContextsChanged() = 0;
};

#endif // IINPUTSYSTEMOBSERVER_H