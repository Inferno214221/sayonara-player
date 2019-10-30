#ifndef SAYONARATEST_H
#define SAYONARATEST_H

#include <QTest>
#include <QDebug>
#include <QObject>

class SayonaraTest : public QObject
{
	Q_OBJECT

private:
	QString mTmpPath;

public:
	SayonaraTest(const QString& test_name);
	virtual ~SayonaraTest() override;

	QString temp_path() const;
	QString temp_path(const QString& append) const;
};

#endif // SAYONARATEST_H
