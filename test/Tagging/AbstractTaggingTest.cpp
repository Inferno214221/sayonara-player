#include "AbstractTaggingTest.h"
#include "Utils/FileUtils.h"

#include <QString>
#include <QByteArray>
#include <QString>
#include <QDir>
#include <QTest>

AbstractTaggingTest::AbstractTaggingTest(const QString& testname) :
	SayonaraTest(testname)
{}

void AbstractTaggingTest::id3_test()
{
	mFilename = temp_path("sayonara-test.mp3");
	mResourceFilename = ":/test/mp3test.mp3";

	run();
}

void AbstractTaggingTest::xiph_test()
{
	mFilename = temp_path("sayonara-test.ogg");
	mResourceFilename = ":/test/oggtest.ogg";

	run();
}

void AbstractTaggingTest::init()
{
	QByteArray content;
	Util::File::read_file_into_byte_arr(mResourceFilename, content);
	Util::File::write_file(content, mFilename);
}

void AbstractTaggingTest::cleanup()
{
	Util::File::delete_files({mFilename});
}

void AbstractTaggingTest::run()
{
	init();
	run_test(mFilename);
	cleanup();
}

