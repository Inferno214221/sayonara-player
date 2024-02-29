/* SayonaraTest.h
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

#ifndef SAYONARA_TEST_H
#define SAYONARA_TEST_H

#include <QTest>
#include <QDebug>
#include <QObject>

namespace Test
{
	class Base :
		public QObject
	{
		Q_OBJECT

		public:
			explicit Base(const QString& testName);
			~Base() override;

			[[nodiscard]] QString tempPath() const;
			[[nodiscard]] QString tempPath(const QString& append) const;

		private:
			QString m_localPath;
			QString m_oldHome;
	};
}

#endif // SAYONARA_TEST_H
