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

	QList<Podcast> allPodcasts()
	{
		QList<Podcast> podcasts;
		pod()->getAllPodcasts(podcasts);
		return podcasts;
	}

	QList<QString> allPodcastNames()
	{
		QList<Podcast> podcasts = allPodcasts();

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
	void testInsertAndDelete();
	void testUpdate();
};

void PodcastTest::testInsertAndDelete()
{
	QList<Podcast> podcasts = allPodcasts();
	QVERIFY(podcasts.size() == 0);

	for(int i=0; i<15; i++)
	{
		const QString name = QString("PodcastName%1").arg(i);
		const QString url = QString("http://podcast_url/%1").arg(i);
		bool reversed = ((i % 2) == 0);

		bool success = pod()->addPodcast(Podcast(name, url, reversed));
		QVERIFY(success == true);
	}

	podcasts = allPodcasts();
	QVERIFY(podcasts.count() == 15);

	bool success = pod()->addPodcast(Podcast("PodcastName0", QString("http://newinvalid.url"), false));
	QVERIFY(success == false);

	podcasts = allPodcasts();
	QVERIFY(podcasts.count() == 15);

	for(int i=0; i<podcasts.size(); i++)
	{
		QString name = podcasts[i].name();
		QString url = podcasts[i].url();
		bool reversed = podcasts[i].reversed();

		QString expectedName = QString("PodcastName%1").arg(i);
		QString expectedUrl = QString("http://podcast_url/%1").arg(i);
		bool expectedReversed = ((i % 2) == 0);

		QVERIFY(name == expectedName);
		QVERIFY(url == expectedUrl);
		QVERIFY(reversed == expectedReversed);
	}

	int count = podcasts.count();
	for(auto it=podcasts.begin(); it != podcasts.end(); it++)
	{
		bool success = pod()->deletePodcast(it->name());
		QVERIFY(success == true);

		count --;

		const QList<Podcast> podcastsTmp = allPodcasts();
		QVERIFY(podcastsTmp.count() == count);
	}
}

void PodcastTest::testUpdate()
{
	QList<Podcast> podcasts = allPodcasts();
	QVERIFY(podcasts.size() == 0);

	for(int i=0; i<15; i++)
	{
		const QString name = QString("PodcastName%1").arg(i);
		const QString url = QString("http://podcast_url/%1").arg(i);
		bool reversed = ((i % 2) == 1);

		bool success = pod()->addPodcast(Podcast(name, url, reversed));
		QVERIFY(success == true);
	}

	podcasts = allPodcasts();
	QVERIFY(podcasts.count() == 15);

	QList<QString> names = allPodcastNames();
	for(int i=0; i<names.size(); i++)
	{
		const QString name = names[i];
		pod()->updatePodcastUrl(name, QString("NeueUrl%1").arg(i));
	}

	bool success = pod()->updatePodcastUrl("asdfkjweroinwe", QString("http://newinvalid.url"));
	QVERIFY(success == false);

	podcasts = allPodcasts();
	names = allPodcastNames();
	for(int i=0; i<podcasts.size(); i++)
	{
		const QString name = podcasts[i].name();
		const QString url = podcasts[i].url();
		bool reversed = podcasts[i].reversed();

		const QString expectedName = QString("PodcastName%1").arg(i);
		const QString expectedUrl = QString("NeueUrl%1").arg(i);
		bool expectedReversed = ((i % 2) == 1);

		QVERIFY(name == expectedName);
		QVERIFY(url == expectedUrl);
		QVERIFY(reversed == expectedReversed);

		bool success = pod()->deletePodcast(name);
		QVERIFY(success == true);
	}
}

QTEST_GUILESS_MAIN(PodcastTest)

#include "PodcastTest.moc"

