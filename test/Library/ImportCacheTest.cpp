#include "test/Common/SayonaraTest.h"

#include "Components/Library/Importer/ImportCache.h"
#include "Utils/Tagging/TagReader.h"

class ImportCacheTest :
	public Test::Base
{
	Q_OBJECT

	public:
		ImportCacheTest() :
			Test::Base("ImportCacheTest") {}

	private slots:
		void test();
};

void ImportCacheTest::test()
{
	const QString libraryPath("/path/to/my/library");
	const QStringList paths
		{
			"/some/path/to/be/imported/cover.jpg",
			"/some/path/to/be/imported/subfolder/playlist.pls"
		};

	const auto tagReader = Tagging::TagReader::create();
	Library::ImportCache cache1(libraryPath, tagReader);
	Library::ImportCache cache2(libraryPath, tagReader);
	Library::ImportCache cache3(libraryPath, tagReader);

	for(const QString& path: paths)
	{
		cache1.addFile(path, "/some/path/to/be/imported");
	}

	for(const QString& path: paths)
	{
		cache2.addFile(path, "/some/path/to");
	}

	const QString& cache3Path("/files/from/somewhere/else");
	cache3.addFile(cache3Path, "/files/from/somewhere/else");

	QString p11 = cache1.targetFilename(paths[0], "");
	QVERIFY(p11 == "/path/to/my/library/cover.jpg");

	QString p12 = cache1.targetFilename(paths[1], "");
	QVERIFY(p12 == "/path/to/my/library/subfolder/playlist.pls");

	QString p21 = cache2.targetFilename(paths[0], "deeper");
	QVERIFY(p21 == "/path/to/my/library/deeper/be/imported/cover.jpg");

	QString p22 = cache2.targetFilename(paths[1], "deeper");
	QVERIFY(p22 == "/path/to/my/library/deeper/be/imported/subfolder/playlist.pls");

	QString p3 = cache3.targetFilename(cache3Path, "");
	QVERIFY(p3 == "/path/to/my/library");
}

QTEST_GUILESS_MAIN(ImportCacheTest)

#include "ImportCacheTest.moc"
