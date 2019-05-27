#include <QTest>
#include <QObject>
#include "Utils/FileUtils.h"

class FileHelperTest : public QObject
{
	Q_OBJECT

private slots:
	void test();
	void create_and_delete();
	void common_path_test();
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
	new_dir = "/tmp/sayonara/some/absolute/filepath";
	new_file = new_dir + "/file.out";
	to_be_deleted << "/tmp/sayonara";

	QVERIFY(is_absolute(new_file));

	success = create_directories(new_dir);
	QVERIFY(success);
	QVERIFY(check_file(new_dir));

	write_file("Some data", new_file);
	QVERIFY(check_file(new_file));
	delete_files(to_be_deleted);
	QVERIFY( !check_file("/tmp/sayonara") );

	/** Relative **/
	to_be_deleted.clear();
	new_dir = "./tmp/sayonara/some/relative/filepath";
	new_file = new_dir + "/file.out";
	to_be_deleted << "./tmp";

	QVERIFY(!is_absolute(new_file));

	success = create_directories(new_dir);
	QVERIFY(success);
	QVERIFY(check_file(new_dir));

	write_file("Some data", new_file);
	QVERIFY(check_file(new_file));
	delete_files(to_be_deleted);
	QVERIFY( !check_file("./tmp") );
}

void FileHelperTest::common_path_test()
{
	QString ret;
	QStringList files;

	files << "/tmp/path/to/some/directory/bla.txt";
	files << "/tmp/path/to/some/directory/bla2.txt";

	Util::File::create_directories("/tmp/path/to/some/directory");
	Util::File::create_directories("/tmp/other/path/to/somewhere");
	Util::File::create_directories("/tmp/path/to/some/really/long/directory");

	QFile f1("/tmp/path/to/some/directory/bla.txt");
	QFile f2("/tmp/path/to/some/directory/bla2.txt");
	QFile f3("/tmp/path/to/some/file.txt");

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
	QVERIFY(ret.compare("/tmp/path/to/some/directory") == 0);

	files << "/tmp/path/to/some/file.txt";
	ret = Util::File::get_common_directory(files);
	QVERIFY(ret.compare("/tmp/path/to/some") == 0);

	files << "/tmp/path/to/some/really/long/directory";
	ret = Util::File::get_common_directory(files);
	QVERIFY(ret.compare("/tmp/path/to/some") == 0);

	files << "/tmp/other/path/to/somewhere";
	ret = Util::File::get_common_directory(files);
	QVERIFY(ret.compare("/tmp") == 0);

	Util::File::delete_files({"/tmp/path", "/tmp/other"});
}

QTEST_MAIN(FileHelperTest)

#include "FileHelperTest.moc"
