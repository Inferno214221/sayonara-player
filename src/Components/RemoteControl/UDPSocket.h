#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include <QObject>
#include "Utils/Pimpl.h"

class RemoteUDPSocket : public QObject
{
	Q_OBJECT
	PIMPL(RemoteUDPSocket)

public:
	explicit RemoteUDPSocket(QObject* parent=nullptr);
	~RemoteUDPSocket() override;

public slots:
	void setActive(bool b);

private slots:
	void dataReceived();
	void remoteSettingsChanged();
};

#endif // UDPSOCKET_H
