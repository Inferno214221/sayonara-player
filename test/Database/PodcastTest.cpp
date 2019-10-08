#include <QTest>
#include <QObject>
#include <QMap>

#include "Database/Connector.h"
#include "Database/Podcasts.h"
#include "Utils/Utils.h"

class PodcastTest : public QObject
{
	Q_OBJECT

public:
	PodcastTest() : QObject()
	{
		QFile::remove("/tmp/player.db");
	}

	~PodcastTest()
	{
		QFile::remove("/tmp/player.db");
	}

private:
	DB::Podcasts* pod()
	{
		auto* db = DB::Connector::instance_custom("", "/tmp", "player.db");
		return db->podcast_connector();
	}

	QMap<QString, QString> all_podcasts()
	{
		QMap<QString, QString> ret;
		pod()->getAllPodcasts(ret);
		return ret;
	}

	QList<QString> all_podcast_names()
	{
		QList<QString> ret = all_podcasts().keys();
		std::sort(ret.begin(), ret.end(), [=](const QString& s1, const QString& s2){
			return ((s1.size() < s2.size()) || (s1.size() == s2.size() && (s1 < s2)));
		});

		return ret;
	}

private slots:
	void test_insert_and_delete();
	void test_update();
};


void PodcastTest::test_insert_and_delete()
{
	QMap<QString, QString> podcasts;
	podcasts = all_podcasts();
	QVERIFY(podcasts.size() == 0);

	for(int i=0; i<15; i++)
	{
		QString name = QString("PodcastName%1").arg(i);
		QString url = QString("http://podcast_url/%1").arg(i);

		pod()->addPodcast(name, url);
	}

	podcasts = all_podcasts();
	QVERIFY(podcasts.count() == 15);

	bool success = pod()->addPodcast("PodcastName0", QString("http://newinvalid.url"));
	podcasts = all_podcasts();
	QVERIFY(podcasts.count() == 15);

	QVERIFY(success == false);

	QList<QString> names = all_podcast_names();
	for(int i=0; i<names.size(); i++)
	{
		QString name = names[i];
		QString url = podcasts[name];

		QString exp_name = QString("PodcastName%1").arg(i);
		QString exp_url = QString("http://podcast_url/%1").arg(i);
		QVERIFY(name == exp_name);
		QVERIFY(url == exp_url);
	}

	int count = podcasts.count();
	names = all_podcast_names();
	for(const QString& name : names)
	{
		pod()->deletePodcast(name);

		count --;

		podcasts = all_podcasts();
		QVERIFY(podcasts.count() == count);
	}
}

void PodcastTest::test_update()
{
	QMap<QString, QString> podcasts;
	podcasts = all_podcasts();
	QVERIFY(podcasts.size() == 0);

	for(int i=0; i<15; i++)
	{
		QString name = QString("PodcastName%1").arg(i);
		QString url = QString("http://podcast_url/%1").arg(i);

		pod()->addPodcast(name, url);
	}

	podcasts = all_podcasts();
	QVERIFY(podcasts.count() == 15);

	QList<QString> names = all_podcast_names();
	for(int i=0; i<names.size(); i++)
	{
		QString name = names[i];
		pod()->updatePodcastUrl(name, QString("NeueUrl%1").arg(i));
	}

	bool success = pod()->updatePodcastUrl("asdfkjweroinwe", QString("http://newinvalid.url"));
	QVERIFY(success == false);

	podcasts = all_podcasts();
	names = all_podcast_names();
	for(int i=0; i<names.size(); i++)
	{
		QString name = names[i];
		QString url = podcasts[name];

		QString exp_name = QString("PodcastName%1").arg(i);
		QString exp_url = QString("NeueUrl%1").arg(i);
		QVERIFY(name == exp_name);
		QVERIFY(url == exp_url);

		pod()->deletePodcast(name);
	}
}

QTEST_MAIN(PodcastTest)

#include "PodcastTest.moc"

