#include "Utils/MetaData/MetaData.h"
#include "Utils/StreamRecorder/StreamRecorderUtils.h"

#include <QObject>
#include <QTest>
#include <QDateTime>
#include <QDebug>

using namespace StreamRecorder;

class StreamRecorderUtilsTest : public QObject
{
	Q_OBJECT

private slots:

	void tag_mismatch_test();
	void filename_test();
	void invalid_tag_test();
	void invalid_chars_test();
	void unique_tag_test();
};


void StreamRecorderUtilsTest::tag_mismatch_test()
{
	int invalid_idx;
	QString templ  = "<ar> - <t> - <<m>/bla.mp3";
	Utils::ErrorCode e = Utils::validateTemplate(templ, &invalid_idx);

	qDebug() << __FUNCTION__ << (int) e;

	QVERIFY(e == Utils::ErrorCode::BracketError);
}

void StreamRecorderUtilsTest::filename_test()
{
	MetaData md;
	md.setTitle("Bli");
	md.setArtist("bla");

	QString sr_path("/path/to/nowhere");

	QString templ = "<t> <m>_<y>-/<ds>+df3-<min>_<t>-<d>-/<dl>df<ar>";

	QDate d = QDate::currentDate();
	QTime t = QTime::currentTime();

	int idx;
	Utils::ErrorCode err = Utils::validateTemplate(templ, &idx);
	QVERIFY(err == Utils::ErrorCode::OK);

	Utils::TargetPaths path = Utils::fullTargetPath(sr_path, templ, md, d, t);

	QLocale loc;
	QString other_path = sr_path + "/" + QString("%1 %2_%3-/%4+df3-%5_%6-%7-/%8df%9.mp3")
			.arg("Bli")
			.arg(d.month(), 2, 10, QChar('0'))
			.arg(d.year(), 4, 10, QChar('0'))
			.arg(loc.dayName(d.dayOfWeek(), QLocale::ShortFormat))
			.arg(t.minute(), 2, 10, QChar('0'))
			.arg("Bli")
			.arg(d.day(), 2, 10, QChar('0'))
			.arg(loc.dayName(d.dayOfWeek(), QLocale::LongFormat))
			.arg("bla");

	qDebug() << "Path1: " << path.first;
	qDebug() << "Path2: " << other_path;
	QVERIFY(path.first.compare(other_path) == 0);
}

void StreamRecorderUtilsTest::invalid_tag_test()
{
	int invalid_idx;
	QString templ = "<t> <m>_<y>-/<fs>+df3-<min>_<t>-<d>-/<dl>df<a>";
	Utils::ErrorCode e = Utils::validateTemplate(templ, &invalid_idx);

	QVERIFY(e == Utils::ErrorCode::UnknownTag);
}

void StreamRecorderUtilsTest::invalid_chars_test()
{
	int invalid_idx;
	QString templ = "<m>_<y>-/<ds>:df3-<min>_<tn><t>-<d>-/<dl>df<ar>";
	Utils::ErrorCode e = Utils::validateTemplate(templ, &invalid_idx);

	QVERIFY(e == Utils::ErrorCode::InvalidChars);
}

void StreamRecorderUtilsTest::unique_tag_test()
{
	int idx;

	QString templ1 = "<tn><t> <m>_<y>-/<ds>+df3-<min>_-<d>-/<dl>df<ar>";
	QString templ2 = "<t> <m>_<y>-/<ds>+df3-<min>-<d>-/<dl>df<ar>";
	QString templ3 = "<tn> <m>_<y>-/<ds>+df3-<min>-<d>-/<dl>df<ar>";
	QString templ4 = "<m>_<y>-/<ds>+df3-<min>-<d>-/<dl>df<ar>";

	Utils::ErrorCode e1, e2, e3, e4;

	e1 = Utils::validateTemplate(templ1, &idx);
	e2 = Utils::validateTemplate(templ2, &idx);
	e3 = Utils::validateTemplate(templ3, &idx);
	e4 = Utils::validateTemplate(templ4, &idx);

	QVERIFY(e1 == Utils::ErrorCode::OK);
	QVERIFY(e2 == Utils::ErrorCode::OK);
	QVERIFY(e3 == Utils::ErrorCode::OK);
	QVERIFY(e4 == Utils::ErrorCode::MissingUniqueTag);
}

QTEST_GUILESS_MAIN(StreamRecorderUtilsTest)

#include "StreamRecorderUtilsTest.moc"
