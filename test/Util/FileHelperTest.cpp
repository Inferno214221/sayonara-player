#include "test/Common/SayonaraTest.h"

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Filepath.h"
#include "Utils/Macros.h"
#include "Utils/StandardPaths.h"

#include <QRegExp>

class FileHelperTest : public Test::Base
{
	Q_OBJECT	

public:
	FileHelperTest() :
		Test::Base("FileHelperTest")
	{}

	~FileHelperTest() override = default;

private slots:
	void test();
	void parentDirectoryTest();
	void createAndDelete();
	void commonPathTest();
	void systemPathsTest();
	void resourcePathTest();
	void splitDirectoriesTest();
	void subDirAndSameFilenameTest();
	void isUrlTest();
	void filenameOfPathTest();
};


using namespace Util::File;
void FileHelperTest::test()
{
	{
		const QString somePath = "/path/./to//my/home/folder/bla.txt";
		const QString cleaned = cleanFilename(somePath);
		const QString extension = getFileExtension(somePath);
		const QString parent = getParentDirectory(somePath);
		const QString filename = getFilenameOfPath(somePath);

		auto [d, f] = splitFilename(somePath);

		QVERIFY( cleaned == "/path/to/my/home/folder/bla.txt" );
		QVERIFY( extension == "txt" );
		QVERIFY( parent == "/path/to/my/home/folder" );
		QVERIFY( filename == "bla.txt" );
		QVERIFY(d == parent);
		QVERIFY(f == filename);
	}

	{
		const QString somePath = "/path/./to//my/home/folder/bla";
		const QString cleaned = cleanFilename(somePath);
		const QString extension = getFileExtension(somePath);
		const QString parent = getParentDirectory(somePath);
		const QString filename = getFilenameOfPath(somePath);

		auto [d, f] = splitFilename(somePath);

		QVERIFY( cleaned == "/path/to/my/home/folder/bla" );
		QVERIFY( extension.isEmpty() );
		QVERIFY( parent == "/path/to/my/home/folder" );
		QVERIFY( filename == "bla" );
		QVERIFY(d == parent);
		QVERIFY(f == filename);
	}

	{
		const QString somePath = "/path/./to//my/home/folder/bla/";
		const QString cleaned = cleanFilename(somePath);
		const QString extension = getFileExtension(somePath);
		const QString parent = getParentDirectory(somePath);
		const QString filename = getFilenameOfPath(somePath);

		auto [d, f] = splitFilename(somePath);

		QVERIFY( cleaned == "/path/to/my/home/folder/bla" );
		QVERIFY( extension.isEmpty() );
		QVERIFY( parent == "/path/to/my/home/folder" );
		QVERIFY( filename == "bla" );
		QVERIFY(d == parent);
		QVERIFY(f == filename);
	}

	{
		const QString somePath = "/path/./to//my/home/folder/bla.dir///";
		const QString cleaned = cleanFilename(somePath);
		const QString extension = getFileExtension(somePath);
		const QString parent = getParentDirectory(somePath);
		const QString filename = getFilenameOfPath(somePath);

		auto [d, f] = splitFilename(somePath);

		QVERIFY( cleaned == "/path/to/my/home/folder/bla.dir" );
		QVERIFY( extension.isEmpty() );
		QVERIFY( parent == "/path/to/my/home/folder" );
		QVERIFY( filename == "bla.dir" );
		QVERIFY(d == parent);
		QVERIFY(f == filename);
	}
}

void FileHelperTest::parentDirectoryTest()
{
	QVERIFY(getParentDirectory("") == QDir::rootPath());
	QVERIFY(getParentDirectory("/") == QDir::rootPath());
	QVERIFY(getParentDirectory("/hallo") == QDir::rootPath());
	QVERIFY(getParentDirectory("/hallo/du") == "/hallo");
	QVERIFY(getParentDirectory("/hallo/du/") == "/hallo");
}

void FileHelperTest::createAndDelete()
{
	bool success;
	QString newDir, newFile;
	QStringList to_be_deleted;

	/** Absolute **/
	newDir = tempPath("some/absolute/filepath");
	newFile = newDir + "/file.out";
	to_be_deleted << tempPath("some");

	QVERIFY(isAbsolute(newFile));

	success = createDirectories(newDir);
	QVERIFY(success);
	QVERIFY(checkFile(newDir));

	writeFile("Some data", newFile);
	QVERIFY(checkFile(newFile));
	deleteFiles(to_be_deleted);
	QVERIFY( !checkFile(tempPath("some")) );

	/** Relative **/
	to_be_deleted.clear();
	newDir = "." + tempPath("some/relative/filepath");
	newFile = newDir + "/file.out";
	to_be_deleted << "." + tempPath("some");

	QVERIFY(!isAbsolute(newFile));

	success = createDirectories(newDir);
	QVERIFY(success);
	QVERIFY(checkFile(newDir));

	writeFile("Some data", newFile);
	QVERIFY(checkFile(newFile));
	deleteFiles(to_be_deleted);
	QVERIFY( !checkFile("." +  tempPath("some")) );
}

void FileHelperTest::commonPathTest()
{
	QString ret;
	QStringList files;

	files << tempPath("path/to/some/directory/bla.txt");
	files << tempPath("path/to/some/directory/bla2.txt");

	Util::File::createDirectories(tempPath("path/to/some/directory"));
	Util::File::createDirectories(tempPath("other/path/to/somewhere"));
	Util::File::createDirectories(tempPath("path/to/some/really/long/directory"));

	QFile f1(tempPath("path/to/some/directory/bla.txt"));
	QFile f2(tempPath("path/to/some/directory/bla2.txt"));
	QFile f3(tempPath("path/to/some/file.txt"));

	f1.open(QFile::WriteOnly);
	f1.write("bla");
	f1.close();

	f2.open(QFile::WriteOnly);
	f2.write("bla");
	f2.close();

	f3.open(QFile::WriteOnly);
	f3.write("bla");
	f3.close();

	ret = Util::File::getCommonDirectory(files);
	QVERIFY(ret.compare(tempPath("path/to/some/directory")) == 0);

	files << tempPath("path/to/some/file.txt");
	ret = Util::File::getCommonDirectory(files);
	QVERIFY(ret.compare(tempPath("path/to/some")) == 0);

	files << tempPath("path/to/some/really/long/directory");
	ret = Util::File::getCommonDirectory(files);
	QVERIFY(ret.compare(tempPath("path/to/some")) == 0);

	files << tempPath("other/path/to/somewhere");
	ret = Util::File::getCommonDirectory(files);
	QVERIFY(ret.compare(tempPath()) == 0);

	Util::File::deleteFiles({tempPath()});
}

void FileHelperTest::systemPathsTest()
{
	QString share_path = Util::sharePath();
	QRegExp re_share(SAYONARA_INSTALL_PATH "(/[A-Za-z]+)?/share/sayonara");

	QVERIFY(re_share.indexIn(share_path) == 0);
}

void FileHelperTest::resourcePathTest()
{
	Util::Filepath fp(":/Desktop/com.sayonara-player.Sayonara.desktop");

	qint64 filesize=0;

	{ // check if exists
		QFile f(fp.path());

		bool is_open = f.open(QFile::ReadOnly);
		QVERIFY(is_open == true);
		filesize = f.size();

		QVERIFY(filesize > 10);
		f.close();
	}

	{
		QString fs_path = fp.fileystemPath();
		QVERIFY(fs_path.startsWith(Util::tempPath()));
		QVERIFY(Util::File::exists(fs_path));
		QVERIFY(fs_path != fp.path());

		QFile f(fs_path);
		bool is_open = f.open(QFile::ReadOnly);
		QVERIFY(is_open == true);
		QVERIFY(filesize == f.size());
		f.close();

		Util::File::deleteFiles({fp.fileystemPath()});
	}
}

void FileHelperTest::splitDirectoriesTest()
{
	QStringList ret;
	QStringList expected;

	ret = Util::File::splitDirectories("/path/to/somewhere");
	expected.clear();
	expected << "path" << "to" << "somewhere";
	QVERIFY(ret == expected);

	ret = Util::File::splitDirectories("/path/to/a/file.mp3");
	expected.clear();
	expected << "path" << "to" << "a" << "file.mp3";
	QVERIFY(ret == expected);

	ret = Util::File::splitDirectories("///a//very/strange///path");
	expected.clear();
	expected << "a" << "very" << "strange" << "path";
	QVERIFY(ret == expected);

	ret = Util::File::splitDirectories("///a//very/strange///path/////");
	expected.clear();
	expected << "a" << "very" << "strange" << "path";
	QVERIFY(ret == expected);

	ret = Util::File::splitDirectories("///a//very/strange///path/to/some/file.mp3");
	expected.clear();
	expected << "a" << "very" << "strange" << "path" << "to" << "some" << "file.mp3";
	QVERIFY(ret == expected);

	ret = Util::File::splitDirectories("/root");
	expected.clear();
	expected << "root";
	QVERIFY(ret == expected);

	ret = Util::File::splitDirectories("///");
	QVERIFY(ret.isEmpty());

	ret = Util::File::splitDirectories("/");
	QVERIFY(ret.isEmpty());

	ret = Util::File::splitDirectories("");
	QVERIFY(ret.isEmpty());
}

void FileHelperTest::subDirAndSameFilenameTest()
{
	QString f1 = "/path/to/some/non/existing/dir/file.ogg";
	QString f2 = "/path/to/some/non/existing/dir2/file.mp3";

	QString d1 = "/path/to/some/non/existing/dir";
	QString d2 = "/path/to/some/non/existing/dir/";
	QString d3 = "/path/to/some/non/existing/dir/";
	QString d4 = "/path/to/./some/non/existing/dir";

	bool b;
	b = Util::File::isSamePath(d1, d2);
	QVERIFY(b == true);

	b = Util::File::isSamePath(d2, d3);
	QVERIFY(b == true);

	b = Util::File::isSamePath(d3, d4);
	QVERIFY(b == true);

	b = Util::File::isSamePath(d1, d4);
	QVERIFY(b == true);

	b = Util::File::isSubdir(d1, d2);
	QVERIFY(b == false);

	b = Util::File::isSubdir(d1, d4);
	QVERIFY(b == false);

	b = Util::File::isSubdir(f1, d1);
	QVERIFY(b == true);

	b = Util::File::isSubdir(f1, d2);
	QVERIFY(b == true);

	b = Util::File::isSubdir(f1, d3);
	QVERIFY(b == true);

	b = Util::File::isSubdir(f2, d4);
	QVERIFY(b == false);

	b = Util::File::isSubdir(f2, d2);
	QVERIFY(b == false);

	b = Util::File::isSubdir(f1, QDir::root().absolutePath());
	QVERIFY(b == true);

	b = Util::File::isSubdir(d1, QDir::root().absolutePath());
	QVERIFY(b == true);

	b = Util::File::isSubdir(f1, "");
	QVERIFY(b == false);

	b = Util::File::isSubdir(d1, "");
	QVERIFY(b == false);
}

void FileHelperTest::isUrlTest()
{
	QVERIFY(Util::File::isUrl("file:///path/to/bla") == true);
	QVERIFY(Util::File::isUrl("/path/to/bla") == false);
	QVERIFY(Util::File::isUrl("http://www.domain.org/path/to/bla") == true);
	QVERIFY(Util::File::isUrl("www.domain.org/path/to/bla") == false);
}

void FileHelperTest::filenameOfPathTest()
{
	QVERIFY(Util::File::getFilenameOfPath("/root.txt") == "root.txt");
	QVERIFY(Util::File::getFilenameOfPath("/path/to/bla.txt") == "bla.txt");
	QVERIFY(Util::File::getFilenameOfPath("/path/to/bla") == "bla");
	QVERIFY(Util::File::getFilenameOfPath("/path/to/dir/") == "dir");
	QVERIFY(Util::File::getFilenameOfPath("/").isEmpty());
	QVERIFY(Util::File::getFilenameOfPath("//").isEmpty());
	QVERIFY(Util::File::getFilenameOfPath("").isEmpty());
}

QTEST_GUILESS_MAIN(FileHelperTest)

#include "FileHelperTest.moc"
