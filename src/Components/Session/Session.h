#ifndef SESSION_H
#define SESSION_H

#include "Utils/Pimpl.h"

#include <QObject>
#include <QDateTime>

class QDateTime;

class Session :
		public QObject
{
	Q_OBJECT
	PIMPL(Session)

public:
	explicit Session(QObject* parent=nullptr);
	~Session();

	static QMap<QDateTime, MetaDataList> get_history(QDateTime beginning=QDateTime());

private slots:
	void track_changed(const MetaData& md);
};

#endif // SESSION_H
