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

private slots:
	void data_received();
	void remote_settings_changed();

public slots:
	void set_active(bool b);
};

#endif // UDPSOCKET_H
