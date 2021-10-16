#include "AbstractTaggingTest.h"
#include "Utils/FileUtils.h"

#include <QString>
#include <QByteArray>
#include <QString>
#include <QDir>
#include <QTest>

AbstractTaggingTest::AbstractTaggingTest(const QString& testname) :
	Test::Base(testname)
{}

void AbstractTaggingTest::id3Test()
{
	mFilename = tempPath("sayonara-test.mp3");
	mResourceFilename = ":/test/mp3test.mp3";

	run();
}

void AbstractTaggingTest::xiphTest()
{
	mFilename = tempPath("sayonara-test.ogg");
	mResourceFilename = ":/test/oggtest.ogg";

	run();
}

void AbstractTaggingTest::init()
{
	QByteArray content;
	Util::File::readFileIntoByteArray(mResourceFilename, content);
	Util::File::writeFile(content, mFilename);
}

void AbstractTaggingTest::cleanup()
{
	Util::File::deleteFiles({mFilename});
}

void AbstractTaggingTest::run()
{
	init();
	runTest(mFilename);
	cleanup();
}

