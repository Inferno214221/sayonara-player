#include "test/Common/SayonaraTest.h"

#include "Utils/FileUtils.h"
#include "Utils/RandomGenerator.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"

#include <algorithm>

namespace
{
	MetaDataList createTracks(const int min, const int max)
	{
		auto tracks = MetaDataList {};
		for(auto i = min; i < max; i++)
		{
			auto track = MetaData {};
			track.setId(i);
			track.setFilepath(QString("/path/to/%1.mp3").arg(i));

			tracks << std::move(track);
		}

		return tracks;
	}

	IndexSet listToSet(const QList<int>& list)
	{
		auto result = IndexSet {};
		for(const auto item: list)
		{
			result << item;
		}

		return result;
	}

	bool uniqueIdsAreEqual(QList<UniqueId> lst1, QList<UniqueId> lst2)
	{
		std::sort(lst1.begin(), lst1.end());
		std::sort(lst2.begin(), lst2.end());

		return lst1 == lst2;
	}
}

class MetaDataListTest :
	public Test::Base
{
	Q_OBJECT

	public:
		MetaDataListTest() :
			Test::Base("MetaDataListTest") {}

		~MetaDataListTest() override = default;

	private slots:
		[[maybe_unused]] void testCopyAndAssignment();
		[[maybe_unused]] void testInsert();
		[[maybe_unused]] void testMoveTracks();
		[[maybe_unused]] void testCopyTracks();
		[[maybe_unused]] void testRemoveByIndexSet();
		[[maybe_unused]] void testRemoveByIndexRange();
		[[maybe_unused]] void testAppendUnique();
		[[maybe_unused]] void testPushBack();
};

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
[[maybe_unused]] void MetaDataListTest::testCopyAndAssignment()
{
	constexpr const auto MaxIndex = 5000;
	{ // copy constructor
		const auto originalTracks = createTracks(0, MaxIndex);
		const auto originalUniqueIds = Util::uniqueIds(originalTracks);
		const auto originalTrackIds = Util::trackIds(originalTracks);

		const auto tracks = MetaDataList(originalTracks);
		QVERIFY(originalUniqueIds != Util::uniqueIds(tracks));
		QVERIFY(originalTrackIds == Util::trackIds(tracks));
	}

	{ // copy assignment
		const auto originalTracks = createTracks(0, MaxIndex);
		const auto originalUniqueIds = Util::uniqueIds(originalTracks);
		const auto originalTrackIds = Util::trackIds(originalTracks);

		const auto tracks = originalTracks; // NOLINT(performance-unnecessary-copy-initialization)
		QVERIFY(originalUniqueIds != Util::uniqueIds(tracks));
		QVERIFY(originalTrackIds == Util::trackIds(tracks));
	}

	{ // move constructor
		auto originalTracks = createTracks(0, MaxIndex);
		const auto originalUniqueIds = Util::uniqueIds(originalTracks);
		const auto originalTrackIds = Util::trackIds(originalTracks);

		const auto tracks = MetaDataList(std::move(originalTracks));
		QVERIFY(originalUniqueIds == Util::uniqueIds(tracks));
		QVERIFY(originalTrackIds == Util::trackIds(tracks));
	}

	{ // move assignment
		auto originalTracks = createTracks(0, MaxIndex);
		const auto originalUniqueIds = Util::uniqueIds(originalTracks);
		const auto originalTrackIds = Util::trackIds(originalTracks);

		const auto tracks = std::move(originalTracks);
		QVERIFY(originalUniqueIds == Util::uniqueIds(tracks));
		QVERIFY(originalTrackIds == Util::trackIds(tracks));
	}
}

[[maybe_unused]] void MetaDataListTest::testInsert() // NOLINT(readability-convert-member-functions-to-static)
{
	struct TestCase
	{
		MetaDataList originalTracks;
		MetaDataList tracksToInsert;
		int insertIndex;
		QList<int> expctedIds;
	};

	const auto testCases = {
		TestCase {createTracks(0, 8), createTracks(100, 103), 0, {100, 101, 102, 0, 1, 2, 3, 4, 5, 6, 7}},
		TestCase {createTracks(0, 8), createTracks(100, 103), 5, {0, 1, 2, 3, 4, 100, 101, 102, 5, 6, 7}},
		TestCase {createTracks(0, 8), createTracks(100, 103), 8, {0, 1, 2, 3, 4, 5, 6, 7, 100, 101, 102}},
		TestCase {createTracks(0, 8), createTracks(100, 103), -3, {100, 101, 102, 0, 1, 2, 3, 4, 5, 6, 7}},
		TestCase {createTracks(0, 8), {}, 3, {0, 1, 2, 3, 4, 5, 6, 7}},
		TestCase {{}, createTracks(100, 103), 5, {100, 101, 102}},
	};

	for(const auto& testCase: testCases)
	{
		auto tracks = testCase.originalTracks;
		tracks.insertTracks(testCase.tracksToInsert, testCase.insertIndex);
		QVERIFY(testCase.expctedIds == Util::trackIds(tracks));
	}
}

[[maybe_unused]] void MetaDataListTest::testRemoveByIndexSet() // NOLINT(readability-convert-member-functions-to-static)
{
	struct TestCase
	{
		MetaDataList originalTracks;
		IndexSet indexesToDelete;
		QList<int> expctedIds;
	};

	const auto testCases = {
		TestCase {createTracks(0, 8), listToSet({0, 1, 2, 3, 4, 5, 6, 7}), {}},
		TestCase {createTracks(0, 8), listToSet({0, 2, 4, 6}), {1, 3, 5, 7}},
		TestCase {createTracks(0, 8), listToSet({-1, -2, -3, 9, 10, 11}), {0, 1, 2, 3, 4, 5, 6, 7}},
		TestCase {{}, listToSet({-1, -2, -3, 9, 10, 11}), {}},
	};

	for(const auto& testCase: testCases)
	{
		auto tracks = testCase.originalTracks;
		tracks.removeTracks(testCase.indexesToDelete);
		QVERIFY(testCase.expctedIds == Util::trackIds(tracks));
	}
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
[[maybe_unused]] void MetaDataListTest::testRemoveByIndexRange()
{
	struct TestCase
	{
		MetaDataList originalTracks;
		std::pair<int, int> rangeToDelete;
		QList<int> expctedIds;
	};

	const auto testCases = {
		TestCase {createTracks(0, 8), {0, 7}, {}},
		TestCase {createTracks(0, 8), {0, 4}, {5, 6, 7}},
		TestCase {createTracks(0, 8), {-5, 4}, {5, 6, 7}},
		TestCase {createTracks(0, 8), {5, 8}, {0, 1, 2, 3, 4}},
		TestCase {createTracks(0, 8), {5, 20}, {0, 1, 2, 3, 4}},
		TestCase {{},
		          {0, 4},
		          {}}
	};

	for(const auto& testCase: testCases)
	{
		auto tracks = testCase.originalTracks;
		tracks.removeTracks(testCase.rangeToDelete.first, testCase.rangeToDelete.second);
		QVERIFY(testCase.expctedIds == Util::trackIds(tracks));
	}
}

[[maybe_unused]] void MetaDataListTest::testAppendUnique() // NOLINT(readability-convert-member-functions-to-static)
{
	struct TestCase
	{
		MetaDataList tracks;
		MetaDataList tracksToAdd;
		QList<int> expectedIds;
	};

	const auto testCases = {
		TestCase {createTracks(0, 4), createTracks(10, 14), {0, 1, 2, 3, 10, 11, 12, 13}},
		TestCase {createTracks(0, 4), createTracks(2, 8), {0, 1, 2, 3, 4, 5, 6, 7}},
		TestCase {createTracks(0, 4), createTracks(0, 4), {0, 1, 2, 3}},
		TestCase {{}, createTracks(0, 4), {0, 1, 2, 3}}
	};

	for(const auto& testCase: testCases)
	{
		auto tracks = testCase.tracks;
		tracks.appendUnique(testCase.tracksToAdd);

		QVERIFY(Util::trackIds(tracks) == testCase.expectedIds);
	}
}

[[maybe_unused]] void MetaDataListTest::testMoveTracks() // NOLINT(readability-convert-member-functions-to-static)
{
	struct TestCase
	{
		MetaDataList tracks;
		IndexSet indexes;
		int targetIndex;
		QList<int> expectedIds;
	};

	const auto testCases = {
		TestCase {createTracks(0, 8), listToSet({0, 1, 2}), 0, {0, 1, 2, 3, 4, 5, 6, 7}},
		TestCase {createTracks(0, 8), listToSet({0, 1, 2}), 5, {3, 4, 0, 1, 2, 5, 6, 7}},
		TestCase {createTracks(0, 8), listToSet({2, 4, 6}), 3, {0, 1, 2, 4, 6, 3, 5, 7}},
		TestCase {createTracks(0, 8), listToSet({5, 6, 7}), 1, {0, 5, 6, 7, 1, 2, 3, 4}},
		TestCase {createTracks(0, 8), listToSet({1, 3, 7}), 4, {0, 2, 1, 3, 7, 4, 5, 6}},
		TestCase {createTracks(0, 8), listToSet({-1, 4, 10}), 2, {0, 1, 4, 2, 3, 5, 6, 7}},
		TestCase {createTracks(0, 8), listToSet({}), 2, {0, 1, 2, 3, 4, 5, 6, 7}},
		TestCase {{}, listToSet({0, 1, 2}), 2, {}}
	};

	for(const auto& testCase: testCases)
	{
		auto tracks = testCase.tracks;
		const auto uniqueIds = Util::uniqueIds(tracks);
		tracks.moveTracks(testCase.indexes, testCase.targetIndex);

		QVERIFY(Util::trackIds(tracks) == testCase.expectedIds);
		QVERIFY(uniqueIdsAreEqual(Util::uniqueIds(tracks), uniqueIds));
	}
}

[[maybe_unused]] void MetaDataListTest::testCopyTracks() // NOLINT(readability-convert-member-functions-to-static)
{
	struct TestCase
	{
		MetaDataList tracks;
		IndexSet indexes;
		int targetIndex;
		QList<int> expectedIds;
	};

	const auto testCases = {
		TestCase {createTracks(0, 8), listToSet({0, 1, 2}), 0, {0, 1, 2, 0, 1, 2, 3, 4, 5, 6, 7}},
		TestCase {createTracks(0, 8), listToSet({0, 1, 2}), 5, {0, 1, 2, 3, 4, 0, 1, 2, 5, 6, 7}},
		TestCase {createTracks(0, 8), listToSet({2, 4, 6}), 3, {0, 1, 2, 2, 4, 6, 3, 4, 5, 6, 7}},
		TestCase {createTracks(0, 8), listToSet({5, 6, 7}), 1, {0, 5, 6, 7, 1, 2, 3, 4, 5, 6, 7}},
		TestCase {createTracks(0, 8), listToSet({1, 3, 7}), 4, {0, 1, 2, 3, 1, 3, 7, 4, 5, 6, 7}},
		TestCase {createTracks(0, 8), listToSet({-1, 4, 10}), 2, {0, 1, 4, 2, 3, 4, 5, 6, 7}},
		TestCase {createTracks(0, 8), {}, 2, {0, 1, 2, 3, 4, 5, 6, 7}},
		TestCase {{}, listToSet({0, 1, 2}), 2, {}}
	};

	for(const auto& testCase: testCases)
	{
		auto tracks = testCase.tracks;
		const auto uniqueIds = Util::uniqueIds(tracks);
		tracks.copyTracks(testCase.indexes, testCase.targetIndex);

		QVERIFY(Util::trackIds(tracks) == testCase.expectedIds);
	}
}

[[maybe_unused]] void MetaDataListTest::testPushBack()
{
	constexpr const auto MaxIndex = 10000;

	struct TestCase
	{
		MetaDataList tracks1;
		MetaDataList tracks2;
	};

	const auto testCases = std::array {
		TestCase {createTracks(0, MaxIndex), createTracks(MaxIndex + 1, MaxIndex + MaxIndex)},
		TestCase {createTracks(0, MaxIndex), MetaDataList {}},
		TestCase {MetaDataList {}, createTracks(0, MaxIndex)},
		TestCase {MetaDataList {}, MetaDataList {}},
	};

	for(const auto& testCase: testCases)
	{
		auto tracks1 = testCase.tracks1;
		auto tracks2 = testCase.tracks2;
		auto tracks = MetaDataList {};
		tracks << std::move(tracks1);
		tracks << std::move(tracks2);

		QVERIFY(tracks.count() == testCase.tracks1.count() + testCase.tracks2.count());
	}

	for(const auto& testCase: testCases)
	{
		auto tracks1 = testCase.tracks1;
		auto tracks2 = testCase.tracks2;
		auto tracks = MetaDataList() << tracks1 << tracks2;
		QVERIFY(tracks.count() == testCase.tracks1.count() + testCase.tracks2.count());
	}
}

QTEST_GUILESS_MAIN(MetaDataListTest)

#include "MetaDataListTest.moc"
