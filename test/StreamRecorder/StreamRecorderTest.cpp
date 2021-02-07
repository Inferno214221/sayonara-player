#include "SayonaraTest.h"
#include "TestPlayManager.h"

#include "Components/Engine/StreamRecorder/StreamRecorder.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Settings/SettingRegistry.h"
#include "Utils/StreamRecorder/StreamRecorderUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/FileUtils.h"
#include "Utils/Utils.h"

#include <QDateTime>
#include <QFile>

namespace SR = StreamRecorder;

class StreamRecorderTest :
	public Test::Base
{
	Q_OBJECT

		StreamRecorder::StreamRecorder* mStreamRecorder = nullptr;
		PlayManager* mPlayManager;

	public:
		StreamRecorderTest();
		~StreamRecorderTest() noexcept;

	private slots:
		void target_path_template_test();
		void www_test();
		void file_test();
};

StreamRecorderTest::StreamRecorderTest() :
	Test::Base("StreamRecorderTest")
{
	SetSetting(Set::Engine_SR_Path, tempPath());
	SetSetting(SetNoDB::MP3enc_found, true);
	SetSetting(Set::Engine_SR_Active, true);
	SetSetting(Set::Engine_SR_SessionPath, true);
	SetSetting(Set::Engine_SR_SessionPathTemplate, QString("<y><m><d>/<tn> - <t>"));

	mPlayManager = new TestPlayManager();
	mStreamRecorder = new SR::StreamRecorder(mPlayManager, this);
}

StreamRecorderTest::~StreamRecorderTest() noexcept
{
	delete mStreamRecorder;
	delete mPlayManager;
}

void StreamRecorderTest::target_path_template_test()
{
	int idx;
	Settings* s = Settings::instance();
	QString tpt = s->get<Set::Engine_SR_SessionPathTemplate>();
	SR::Utils::ErrorCode err = SR::Utils::validateTemplate(tpt, &idx);

	QVERIFY(err == SR::Utils::ErrorCode::OK);
}

void StreamRecorderTest::www_test()
{
	QDate d = QDate::currentDate();
	mStreamRecorder->record(true);

	int track_num = 1;
	for(int i = 1; i < 100; i++, track_num++)
	{
		QString filepath = QString("http://path%1.com")
			.arg(i);

		QVERIFY(Util::File::isWWW(filepath) == true);

		MetaData md;
		md.setTitle(QString("title%1").arg(i));
		md.setArtist(QString("artist%1").arg(i));
		md.setFilepath(filepath);

		QString filename = mStreamRecorder->changeTrack(md);

		QString should_filename =
			tempPath() + "/" +
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

		else
		{
			// no mp3 file ->
			// StreamRecorder::save() will fail the next time
			// index will not be incremented by StreamRecorder
			track_num--;
		}

		QVERIFY(filename == should_filename);
		QVERIFY(mStreamRecorder->isRecording());
	}

	Util::File::deleteFiles({tempPath()});
}

void StreamRecorderTest::file_test()
{
	mStreamRecorder->record(true);

	for(int i = 1; i < 100; i++)
	{
		QString filepath = tempPath
			(
				QString("path%1.mp3").arg(i)
			);

		QVERIFY(Util::File::isWWW(filepath) == false);

		MetaData md;
		md.setTitle(QString("title%1").arg(i));
		md.setArtist(QString("artist%1").arg(i));
		md.setFilepath(filepath);

		QString filename = mStreamRecorder->changeTrack(md);

		QVERIFY(filename.isEmpty());
		QVERIFY(!mStreamRecorder->isRecording());
	}
}

QTEST_GUILESS_MAIN(StreamRecorderTest)

#include "StreamRecorderTest.moc"

