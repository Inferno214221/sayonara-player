#ifndef SESSION_H
#define SESSION_H

#include "Utils/Settings/SayonaraClass.h"
#include "Utils/Pimpl.h"

#include <QObject>

class MetaData;
class Session :
		public QObject,
		public SayonaraClass
{
	Q_OBJECT
	PIMPL(Session)

public:
	explicit Session(QObject* parent=nullptr);
	~Session();

private slots:
	void track_changed(const MetaData& md);
};

#endif // SESSION_H
