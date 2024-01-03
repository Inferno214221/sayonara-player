/* MetaDataScanner.h
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

#ifndef DIRECTORYFILESCANNER_H
#define DIRECTORYFILESCANNER_H

#include "Utils/Pimpl.h"

#include <QObject>

class MetaDataList;
class QStringList;
namespace Directory
{
	class MetaDataScanner :
		public QObject
	{
		Q_OBJECT
		PIMPL(MetaDataScanner)

		signals:
			void sigFinished();
			void sigCurrentProcessedPathChanged(const QString& path);

		public:
			explicit MetaDataScanner(const QStringList& files, bool recursive, QObject* parent=nullptr);
			~MetaDataScanner() override;

			MetaDataList metadata() const;
			QStringList files() const;

		public slots:
			void start();
	};
}


#endif // DIRECTORYFILESCANNER_H
