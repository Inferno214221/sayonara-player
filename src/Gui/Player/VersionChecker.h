#ifndef VERSIONCHECKER_H
#define VERSIONCHECKER_H

#include <QObject>
#include "Utils/Pimpl.h"

class VersionChecker : public QObject
{
	Q_OBJECT

signals:
	void sig_finished();

public:
	explicit VersionChecker(QObject* parent);
	~VersionChecker();

private slots:
	void version_check_finished();
};

#endif // VERSIONCHECKER_H
