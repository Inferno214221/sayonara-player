/* AbstractTaggingTest.h
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

#ifndef ABSTRACTTAGGINGTEST_H
#define ABSTRACTTAGGINGTEST_H

#include "test/Common/SayonaraTest.h"

class AbstractTaggingTest :
	public Test::Base
{
	Q_OBJECT

	public:
		AbstractTaggingTest(const QString& testname);

	private:
		QString mResourceFilename;
		QString mFilename;

	private:
		void init();
		void cleanup();
		void run();

	protected:
		virtual void runTest(const QString& filename) = 0;

	protected slots:
		void id3Test();
		void xiphTest();
};

#endif // ABSTRACTTAGGINGTEST_H
