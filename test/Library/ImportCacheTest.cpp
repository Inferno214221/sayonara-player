#include "SayonaraTest.h"
#include "Components/Library/Importer/ImportCache.h"

class ImportCacheTest : public
	Test::Base
{
	Q_OBJECT

public:
	ImportCacheTest() :
		Test::Base("ImportCacheTest")
	{}

private slots:
	void test();
};


void ImportCacheTest::test()
{
	const QString library_path("/path/to/my/library");
	const QStringList paths
	{
		"/some/path/to/be/imported/cover.jpg",
		"/some/path/to/be/imported/subfolder/playlist.pls"
	};

	Library::ImportCache cache1(library_path);
	Library::ImportCache cache2(library_path);
	Library::ImportCache cache3(library_path);

	for(const QString& path : paths)
	{
		cache1.add_file(path, "/some/path/to/be/imported");
	}

	for(const QString& path : paths)
	{
		cache2.add_file(path, "/some/path/to");
	}

	QString p11 = cache1.target_filename(paths[0], "");
	QVERIFY(p11 == "/path/to/my/library/cover.jpg");

	QString p12 = cache1.target_filename(paths[1], "");
	QVERIFY(p12 == "/path/to/my/library/subfolder/playlist.pls");

	QString p21 = cache2.target_filename(paths[0], "deeper");
	QVERIFY(p21 == "/path/to/my/library/deeper/be/imported/cover.jpg");

	QString p22 = cache2.target_filename(paths[1], "deeper");
	QVERIFY(p22 == "/path/to/my/library/deeper/be/imported/subfolder/playlist.pls");
}

QTEST_GUILESS_MAIN(ImportCacheTest)

#include "ImportCacheTest.moc"
