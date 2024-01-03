/* SomaFMAsyncDropHandler.h
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

#ifndef SOMAFMASYNCDROPHANDLER_H
#define SOMAFMASYNCDROPHANDLER_H

#include "Utils/MimeData/DragDropAsyncHandler.h"

#include <QList>
class QUrl;

namespace SomaFM
{
	class Station;
	class AsyncDropHandler : public Gui::AsyncDropHandler
	{
		Q_OBJECT
		PIMPL(AsyncDropHandler)

		public:
			AsyncDropHandler(const SomaFM::Station& station, QObject* parent);
			~AsyncDropHandler() override;

		public slots:
			void start() override;

		private slots:
			void streamParserFinished(bool success);
	};
}

#endif // SOMAFMASYNCDROPHANDLER_H
