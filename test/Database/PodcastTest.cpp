#include "SayonaraTest.h"
#include "Database/Connector.h"
#include "Database/Podcasts.h"

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Streams/Podcast.h"
#include "Utils/Algorithm.h"

#include <QMap>

class PodcastTest :
	public Test::Base
{
	Q_OBJECT

public:
	PodcastTest() :
		Test::Base("PodcastTest")
	{}

	~PodcastTest() override = default;


private:
	DB::Podcasts* pod()
	{
		auto* db = DB::Connector::instance();
		return db->podcast_connector();
	}

	QList<Podcast> all_podcasts()
	{
		QList<Podcast> podcasts;
		pod()->getAllPodcasts(podcasts);
		return podcasts;
	}

	QList<QString> all_podcast_names()
	{
		QList<Podcast> podcasts = all_podcasts();

		QList<QString> ret;
		Util::Algorithm::transform(podcasts, ret, [](const Podcast& p){
			return p.name();
		});

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
	QList<Podcast> podcasts = all_podcasts();
	QVERIFY(podcasts.size() == 0);

	for(int i=0; i<15; i++)
	{
		QString name = QString("PodcastName%1").arg(i);
		QString url = QString("http://podcast_url/%1").arg(i);

		pod()->addPodcast(Podcast(name, url, false));
	}

	podcasts = all_podcasts();
	QVERIFY(podcasts.count() == 15);

	bool success = pod()->addPodcast(Podcast("PodcastName0", QString("http://newinvalid.url"), false));
	podcasts = all_podcasts();
	QVERIFY(podcasts.count() == 15);

	QVERIFY(success == false);

	for(int i=0; i<podcasts.size(); i++)
	{
		QString name = podcasts[i].name();
		QString url = podcasts[i].url();

		QString exp_name = QString("PodcastName%1").arg(i);
		QString exp_url = QString("http://podcast_url/%1").arg(i);
		QVERIFY(name == exp_name);
		QVERIFY(url == exp_url);
	}

	int count = podcasts.count();
	for(auto it=podcasts.begin(); it != podcasts.end(); it++)
	{
		pod()->deletePodcast(it->name());

		count --;

		podcasts = all_podcasts();
		QVERIFY(podcasts.count() == count);
	}
}

void PodcastTest::test_update()
{
	QList<Podcast> podcasts = all_podcasts();
	QVERIFY(podcasts.size() == 0);

	for(int i=0; i<15; i++)
	{
		QString name = QString("PodcastName%1").arg(i);
		QString url = QString("http://podcast_url/%1").arg(i);

		pod()->addPodcast(Podcast(name, url, false));
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
	for(int i=0; i<podcasts.size(); i++)
	{
		QString url = podcasts[i].url();

		QString exp_name = QString("PodcastName%1").arg(i);
		QString exp_url = QString("NeueUrl%1").arg(i);
		//QVERIFY(name == exp_name);
		QVERIFY(url == exp_url);

		//pod()->deletePodcast(name);
	}
}

QTEST_GUILESS_MAIN(PodcastTest)

#include "PodcastTest.moc"

