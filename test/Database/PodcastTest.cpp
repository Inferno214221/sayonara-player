#include "test/Common/SayonaraTest.h"

#include "Database/Connector.h"
#include "Database/Podcasts.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Streams/Station.h"
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
		return db->podcastConnector();
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

		auto new_podcasts = all_podcasts();
		QVERIFY(new_podcasts.count() == count);
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

	for(int i=0; i< podcasts.size(); i++)
	{
		Podcast podcast = podcasts[i];
		podcast.setUrl( QString("NeueUrl%1").arg(i) );
		bool b = pod()->updatePodcast(podcast.name(), podcast);
		QVERIFY(b == true);
	}

	{
		Podcast p("asdfasd", "http://newinvalid.url", false);
		bool success = pod()->updatePodcast("asdfkjweroinwe", p);
		QVERIFY(success == false);
	}

	podcasts = all_podcasts();
	for(int i=0; i<podcasts.size(); i++)
	{
		QString url = podcasts[i].url();
		QString name = podcasts[i].name();

		QString exp_name = QString("PodcastName%1").arg(i);
		QString exp_url = QString("NeueUrl%1").arg(i);

		QVERIFY(name == exp_name);
		QVERIFY(url == exp_url);

		pod()->deletePodcast(name);
	}

	podcasts = all_podcasts();
	QVERIFY(podcasts.size() == 0);
}

QTEST_GUILESS_MAIN(PodcastTest)

#include "PodcastTest.moc"

