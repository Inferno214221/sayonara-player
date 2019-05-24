#include "Components/Engine/StreamRecorder.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Settings/SettingRegistry.h"
#include "Utils/StreamRecorder/StreamRecorderUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/FileUtils.h"

#include <QTest>
#include <QDateTime>
#include <QFile>

namespace SR=StreamRecorder;

class StreamRecorderTest :
		public QObject
{
	Q_OBJECT

	StreamRecorder::StreamRecorder* sr=nullptr;

public:
	StreamRecorderTest();

private:
	QString sr_path() const;

private slots:
	void target_path_template_test();
	void www_test();
	void file_test();

};

StreamRecorderTest::StreamRecorderTest()
{
	Settings* s = Settings::instance();
	s->check_settings();

	Util::File::remove_files_in_directory(sr_path());
	Util::File::delete_files({sr_path()});

	SetSetting(Set::Engine_SR_Path, sr_path());
	SetSetting(SetNoDB::MP3enc_found, true);
	SetSetting(Set::Engine_SR_Active, true);
	SetSetting(Set::Engine_SR_SessionPath, true);
	SetSetting(Set::Engine_SR_SessionPathTemplate, QString("<y><m><d>/<tn> - <t>"));

	sr = new SR::StreamRecorder(this);
}

QString StreamRecorderTest::sr_path() const
{
	return "/tmp/sayonara_sr";
}

void StreamRecorderTest::target_path_template_test()
{
	int idx;
	Settings* s = Settings::instance();
	QString tpt = s->get<Set::Engine_SR_SessionPathTemplate>();
	SR::Utils::ErrorCode err = SR::Utils::validate_template(tpt, &idx);

	QVERIFY(err == SR::Utils::ErrorCode::OK);
}

void StreamRecorderTest::www_test()
{
	QDate d = QDate::currentDate();
	sr->record(true);

	int track_num = 1;
	for(int i=1; i<100; i++, track_num++)
	{
		QString filepath = QString("http://path%1.com")
				.arg(i);

		QVERIFY(Util::File::is_www(filepath) == true);

		MetaData md;
		md.set_title(QString("title%1").arg(i));
		md.set_artist( QString("artist%1").arg(i));
		md.set_filepath( filepath);

		QString filename = sr->change_track(md);

		QString should_filename =
				sr_path() + "/" +
				QString("%1%2%3")
					.arg(d.year())
					.arg(d.month(), 2, 10, QChar('0'))
					.arg(d.day(), 2, 10, QChar('0')) +
				QString("/%1 - %2.mp3")
					.arg(track_num, 4, 10, QChar('0'))
					.arg(md.title());

		if(i % 2 == 1)
		{
			// fake some mp3 file usually delivered by engine
			QFile f(should_filename);
			f.open(QFile::WriteOnly);
			QByteArray data(25000, 'x');
			f.write(data);
			f.close();
		}

		else {
			// no mp3 file ->
			// StreamRecorder::save() will fail the next time
			// index will not be incremented by StreamRecorder
			track_num--;
		}

		QVERIFY(filename == should_filename);
		QVERIFY(sr->is_recording());
	}

	Util::File::remove_files_in_directory(sr_path());
	Util::File::delete_files({sr_path()});
}


void StreamRecorderTest::file_test()
{
	sr->record(true);

	for(int i=1; i<100; i++)
	{
		QString filepath = QString("/tmp/path%1.mp3")
				.arg(i);

		QVERIFY(Util::File::is_www(filepath) == false);

		MetaData md;
		md.set_title(QString("title%1").arg(i));
		md.set_artist(QString("artist%1").arg(i));
		md.set_filepath(filepath);

		QString filename = sr->change_track(md);

		QVERIFY(filename.isEmpty());
		QVERIFY(!sr->is_recording());
	}
}

QTEST_MAIN(StreamRecorderTest)

#include "StreamRecorderTest.moc"

