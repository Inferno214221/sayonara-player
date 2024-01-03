/* AbstractTaggingTest.cpp
 *
 * Copyright (C) 2011-2024 Michael Lugmair
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

