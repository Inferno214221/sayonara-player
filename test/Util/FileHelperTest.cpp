#include <QTest>
#include <QObject>
#include <QRegExp>
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Filepath.h"
#include "Utils/Macros.h"

class FileHelperTest : public QObject
{
	Q_OBJECT

	QString mTmpPath;

public:
	FileHelperTest()
	{
		Q_INIT_RESOURCE(Resources);
		mTmpPath = Util::temp_path("FileHelperTest");
	}

	~FileHelperTest()
	{
		Util::File::delete_files({mTmpPath});
	}

private slots:
	void test();
	void create_and_delete();
	void common_path_test();
	void system_paths_test();
	void resource_path_test();
};


using namespace Util::File;
void FileHelperTest::test()
{
	QString some_path = "/path/./to//my/home/folder/bla.txt";
	QString cleaned = clean_filename(some_path);
	QString extension = calc_file_extension(some_path);
	QString parent = get_parent_directory(some_path);
	QString filename = get_filename_of_path(some_path);

	QString d, f;
	split_filename(some_path, d, f);

	QVERIFY( cleaned == "/path/to/my/home/folder/bla.txt" );
	QVERIFY( extension == "txt" );
	QVERIFY( parent == "/path/to/my/home/folder" );
	QVERIFY( filename == "bla.txt" );
	QVERIFY(d == parent);
	QVERIFY(f == filename);
}

void FileHelperTest::create_and_delete()
{
	bool success;
	QString new_dir, new_file;
	QStringList to_be_deleted;

	/** Absolute **/
	new_dir = Util::temp_path("sayonara/some/absolute/filepath");
	new_file = new_dir + "/file.out";
	to_be_deleted << mTmpPath + "/sayonara";

	QVERIFY(is_absolute(new_file));

	success = create_directories(new_dir);
	QVERIFY(success);
	QVERIFY(check_file(new_dir));

	write_file("Some data", new_file);
	QVERIFY(check_file(new_file));
	delete_files(to_be_deleted);
	QVERIFY( !check_file(mTmpPath + "/sayonara") );

	/** Relative **/
	to_be_deleted.clear();
	new_dir = "." + mTmpPath + "/some/relative/filepath";
	new_file = new_dir + "/file.out";
	to_be_deleted << "." + mTmpPath;

	QVERIFY(!is_absolute(new_file));

	success = create_directories(new_dir);
	QVERIFY(success);
	QVERIFY(check_file(new_dir));

	write_file("Some data", new_file);
	QVERIFY(check_file(new_file));
	delete_files(to_be_deleted);
	QVERIFY( !check_file("." + mTmpPath) );
}

void FileHelperTest::common_path_test()
{
	QString ret;
	QStringList files;

	files << mTmpPath + "/path/to/some/directory/bla.txt";
	files << mTmpPath + "/path/to/some/directory/bla2.txt";

	Util::File::create_directories(mTmpPath + "/path/to/some/directory");
	Util::File::create_directories(mTmpPath + "/other/path/to/somewhere");
	Util::File::create_directories(mTmpPath + "/path/to/some/really/long/directory");

	QFile f1(mTmpPath + "/path/to/some/directory/bla.txt");
	QFile f2(mTmpPath + "/path/to/some/directory/bla2.txt");
	QFile f3(mTmpPath + "/path/to/some/file.txt");

	f1.open(QFile::WriteOnly);
	f1.write("bla");
	f1.close();

	f2.open(QFile::WriteOnly);
	f2.write("bla");
	f2.close();

	f3.open(QFile::WriteOnly);
	f3.write("bla");
	f3.close();

	ret = Util::File::get_common_directory(files);
	QVERIFY(ret.compare(mTmpPath + "/path/to/some/directory") == 0);

	files << mTmpPath + "/path/to/some/file.txt";
	ret = Util::File::get_common_directory(files);
	QVERIFY(ret.compare(mTmpPath + "/path/to/some") == 0);

	files << mTmpPath + "/path/to/some/really/long/directory";
	ret = Util::File::get_common_directory(files);
	QVERIFY(ret.compare(mTmpPath + "/path/to/some") == 0);

	files << mTmpPath + "/other/path/to/somewhere";
	ret = Util::File::get_common_directory(files);
	QVERIFY(ret.compare(mTmpPath) == 0);

	Util::File::delete_files({mTmpPath});
}

void FileHelperTest::system_paths_test()
{
	QString lib_path = Util::lib_path();
	QString share_path = Util::share_path();

	QRegExp re_lib(SAYONARA_INSTALL_PATH "(/[A-Za-z]+)?/lib(64|32)*/sayonara");
	QRegExp re_share(SAYONARA_INSTALL_PATH "(/[A-Za-z]+)?/share/sayonara");

	QVERIFY(re_lib.indexIn(lib_path) == 0);
	QVERIFY(re_share.indexIn(share_path) == 0);
	QVERIFY(re_lib.cap(1) == re_share.cap(1));
}

void FileHelperTest::resource_path_test()
{
	Util::Filepath fp(":/Desktop/sayonara.desktop");

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
		QString fs_path = fp.filesystem_path();
		QVERIFY(fs_path.startsWith(Util::temp_path()));
		QVERIFY(Util::File::exists(fs_path));
		QVERIFY(fs_path != fp.path());

		QFile f(fs_path);
		bool is_open = f.open(QFile::ReadOnly);
		QVERIFY(is_open == true);
		QVERIFY(filesize == f.size());
		f.close();

		Util::File::delete_files({fp.filesystem_path()});
	}
}

QTEST_GUILESS_MAIN(FileHelperTest)

#include "FileHelperTest.moc"
