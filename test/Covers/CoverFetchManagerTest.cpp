#include "test/Common/SayonaraTest.h"

#include "Components/Covers/Fetcher/DirectFetcher.h"
#include "Components/Covers/CoverFetchManager.h"
#include "Components/Covers/Fetcher/CoverFetcher.h"
#include "Components/Covers/Fetcher/Website.h"
#include "Components/Covers/Fetcher/Amazon.h"
#include "Components/Covers/Fetcher/CoverFetcherUrl.h"
#include "Utils/Algorithm.h"
#include "Utils/RandomGenerator.h"
#include "Utils/Settings/Settings.h"

// access working directory with Test::Base::tempPath("somefile.txt");

using Cover::Fetcher::Manager;
class CoverFetchManagerTest :
	public Test::Base
{
	Q_OBJECT

	public:
		CoverFetchManagerTest() :
			Test::Base("CoverFetchManagerTest"),
			mWebsiteFetcherIdentifier(Cover::Fetcher::Website().identifier().toLower()),
			mDirectFetcherIdentifier(Cover::Fetcher::DirectFetcher().identifier().toLower()) {}

	private:
		QString mWebsiteFetcherIdentifier;
		QString mDirectFetcherIdentifier;

		void resetAll()
		{
			QStringList allFetchers;
			const auto fetchers = Manager::instance()->coverfetchers();
			Util::Algorithm::transform(fetchers, allFetchers, [](const auto& fetcher) {
				return fetcher->identifier();
			});

			// Don't sort them by first letter
			Util::Algorithm::sort(allFetchers, [](const auto& str1, const auto str2) {
				return (str1[1] < str2[1]);
			});

			SetSetting(Set::Cover_Server, allFetchers);
		}

	private slots:
		void testDisable();
		void testSorting();
		void testIsWebsite();
		void testArtistAddresses();
		void testAlbumAddresses();
		void testSearchAddresses();
		void testSearchAddressesByFetcher();
		void testFetcherByUrl();
		void testWwwSearch();
};

void CoverFetchManagerTest::testDisable()
{
	resetAll();

	auto* manager = Manager::instance();
	{
		const auto fetchers = manager->coverfetchers();
		for(const auto& fetcher : fetchers)
		{
			const auto identifier = fetcher->identifier();
			QVERIFY(manager->isActive(identifier));
			QVERIFY(manager->isActive(fetcher));
		}
	}

	auto allIdentifiers = GetSetting(Set::Cover_Server);
	const auto count = allIdentifiers.count();
	Util::Algorithm::removeIf(allIdentifiers, [](const auto& identifier) {
		return identifier.toLower().contains('a');
	});

	QVERIFY(allIdentifiers.count() < count);
	QVERIFY(allIdentifiers.count() > 0);
	SetSetting(Set::Cover_Server, allIdentifiers);

	{
		const auto fetchers = manager->coverfetchers();
		for(const auto& fetcher : fetchers)
		{
			const auto identifier = fetcher->identifier();
			QVERIFY(manager->isActive(identifier) != identifier.contains('a'));
			QVERIFY(manager->isActive(fetcher) != identifier.contains('a'));
		}
	}
}

void CoverFetchManagerTest::testSorting()
{
	resetAll();

	auto* manager = Manager::instance();
	{
		const auto fetchers = manager->coverfetchers();
		QStringList allFetchers;
		Util::Algorithm::transform(fetchers, allFetchers, [](const auto& fetcher) {
			return fetcher->identifier();
		});

		auto sortedFetchers = allFetchers;
		sortedFetchers.sort();
		SetSetting(Set::Cover_Server, sortedFetchers);
		QVERIFY(sortedFetchers != allFetchers);
	}

	{
		const auto fetchers = manager->coverfetchers();
		QStringList allFetchers;
		Util::Algorithm::transform(fetchers, allFetchers, [](const auto& fetcher) {
			return fetcher->identifier();
		});

		auto sortedFetchers = allFetchers;
		sortedFetchers.sort();
		QVERIFY(sortedFetchers == allFetchers);
	}
}

void CoverFetchManagerTest::testArtistAddresses()
{
	resetAll();

	auto* manager = Manager::instance();
	const auto artistAddresses = manager->artistAddresses("artist");
	const auto fetchers = manager->coverfetchers();

	QStringList identifiers;
	for(const auto& fetcher : fetchers)
	{
		if(!fetcher->artistAddress("some-artist").isEmpty())
		{
			identifiers << fetcher->identifier();
		}
	}

	QVERIFY(identifiers.count() == artistAddresses.count());

	for(const auto& identifier : identifiers)
	{
		const auto contains = Util::Algorithm::contains(artistAddresses, [&](const auto& artistAddress) {
			return (identifier.toLower() == artistAddress.identifier().toLower());
		});

		QVERIFY(contains);
		QVERIFY(identifier.toLower() != mDirectFetcherIdentifier);
		QVERIFY(identifier.toLower() != mWebsiteFetcherIdentifier);
	}
}

void CoverFetchManagerTest::testAlbumAddresses()
{
	resetAll();

	auto* manager = Manager::instance();
	const auto albumAddresses = manager->albumAddresses("artist", "album");
	const auto fetchers = manager->coverfetchers();

	QStringList identifiers;
	for(const auto& fetcher : fetchers)
	{
		if(!fetcher->albumAddress("some-artist", "some-album").isEmpty())
		{
			identifiers << fetcher->identifier();
		}
	}

	QVERIFY(identifiers.count() == albumAddresses.count());

	for(const auto& identifier : identifiers)
	{
		const auto contains = Util::Algorithm::contains(albumAddresses, [&](const auto& albumAddress) {
			return (identifier.toLower() == albumAddress.identifier().toLower());
		});

		QVERIFY(contains);
		QVERIFY(identifier.toLower() != mDirectFetcherIdentifier);
		QVERIFY(identifier.toLower() != mWebsiteFetcherIdentifier);
	}
}

void CoverFetchManagerTest::testSearchAddresses()
{
	resetAll();

	auto* manager = Manager::instance();
	const auto searchAddresses = manager->searchAddresses("search");
	const auto fetchers = manager->coverfetchers();

	QStringList identifiers;
	for(const auto& fetcher : fetchers)
	{
		if(!fetcher->fulltextSearchAddress("search").isEmpty())
		{
			identifiers << fetcher->identifier();
		}
	}

	QVERIFY(identifiers.count() == searchAddresses.count());

	for(const auto& identifier : identifiers)
	{
		const auto contains = Util::Algorithm::contains(searchAddresses, [&](const auto& albumAddress) {
			return (identifier.toLower() == albumAddress.identifier().toLower());
		});

		QVERIFY(contains);
		QVERIFY(identifier.toLower() != mDirectFetcherIdentifier);
		QVERIFY(identifier.toLower() != mWebsiteFetcherIdentifier);
	}
}

void CoverFetchManagerTest::testSearchAddressesByFetcher()
{
	resetAll();

	auto amazonFetcher = Cover::Fetcher::Amazon();
	auto* manager = Manager::instance();
	const auto searchAddresses = manager->searchAddresses("some artist", amazonFetcher.identifier());

	QVERIFY(searchAddresses.size() == 1);
	QVERIFY(searchAddresses.first().identifier() == amazonFetcher.identifier());
}

void CoverFetchManagerTest::testFetcherByUrl()
{
	resetAll();
	auto* manager = Manager::instance();

	{
		auto url = Cover::Fetcher::Url(mDirectFetcherIdentifier, "file:///home/user/image.png");
		auto fetcher = manager->coverfetcher(url);
		QVERIFY(fetcher->identifier().toLower() == mDirectFetcherIdentifier);
	}

	{
		auto url = Cover::Fetcher::Url(mWebsiteFetcherIdentifier, "www.website.de");
		auto fetcher = manager->coverfetcher(url);
		QVERIFY(fetcher->identifier().toLower() == mWebsiteFetcherIdentifier);
	}

	{
		auto url = Cover::Fetcher::Url(Cover::Fetcher::Amazon().identifier(), "www.amazon.de/some-query");
		auto fetcher = manager->coverfetcher(url);
		QVERIFY(fetcher->identifier().toLower() == Cover::Fetcher::Amazon().identifier().toLower());
	}

	{
		auto url = Cover::Fetcher::Url("DoesNotExist", "www.unknown.de/some-query");
		auto fetcher = manager->coverfetcher(url);
		QVERIFY(fetcher == nullptr);
	}
}

void CoverFetchManagerTest::testWwwSearch()
{
	resetAll();
	auto* manager = Manager::instance();
	const auto searchAddresses = manager->searchAddresses("www.website.de");

	QVERIFY(searchAddresses.count() == 1);
	QVERIFY(searchAddresses.first().identifier() == mWebsiteFetcherIdentifier);

	auto fetcher = manager->coverfetcher(searchAddresses.first());
	QVERIFY(fetcher->fulltextSearchAddress("") == "https://www.website.de");
}

void CoverFetchManagerTest::testIsWebsite()
{
	QVERIFY(Manager::isSearchstringWebsite("www.website.de"));
	QVERIFY(Manager::isSearchstringWebsite("www.website.com"));
	QVERIFY(Manager::isSearchstringWebsite("www.website.co.uk"));
	QVERIFY(Manager::isSearchstringWebsite("website.de"));

	QVERIFY(!Manager::isSearchstringWebsite("www.website"));
	QVERIFY(!Manager::isSearchstringWebsite("www.website.dede"));
	QVERIFY(!Manager::isSearchstringWebsite("www.website.d"));
	QVERIFY(!Manager::isSearchstringWebsite("website"));
	QVERIFY(!Manager::isSearchstringWebsite("Some artist"));
	QVERIFY(!Manager::isSearchstringWebsite(QString()));
}

QTEST_GUILESS_MAIN(CoverFetchManagerTest)

#include "CoverFetchManagerTest.moc"
