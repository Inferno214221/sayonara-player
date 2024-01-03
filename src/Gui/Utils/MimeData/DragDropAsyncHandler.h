/* DragDropAsyncHandler.h
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

#ifndef DRAGDROPASYNCHANDLER_H
#define DRAGDROPASYNCHANDLER_H

#include <QObject>
#include "Utils/Pimpl.h"

class MetaDataList;

namespace Gui
{
	class AsyncDropHandler : public QObject
	{
		Q_OBJECT
		PIMPL(AsyncDropHandler)

		signals:
			void sigFinished();

		public:
			explicit AsyncDropHandler(QObject* parent=nullptr);
			~AsyncDropHandler() override;

			void setTargetIndex(int index);
			[[nodiscard]] int targetIndex() const;
			[[nodiscard]] virtual MetaDataList tracks() const;

		protected:
			void setTracks(const MetaDataList& tracks);

		public slots:
			virtual void start()=0;
	};
}

#endif // DRAGDROPASYNCHANDLER_H
