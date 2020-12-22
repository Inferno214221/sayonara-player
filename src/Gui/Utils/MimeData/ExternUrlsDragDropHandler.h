/* ExternUrlsDragDropHandler.h */
/*
 * Copyright (C) 2011-2020 Michael Lugmair
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
#ifndef SAYONARA_PLAYER_EXTERNURLSDRAGDROPHANDLER_H
#define SAYONARA_PLAYER_EXTERNURLSDRAGDROPHANDLER_H

#include "DragDropAsyncHandler.h"
#include "Utils/Pimpl.h"

	class ExternUrlsDragDropHandler : public Gui::AsyncDropHandler
{
	Q_OBJECT
	PIMPL(ExternUrlsDragDropHandler)

	public:
		explicit ExternUrlsDragDropHandler(const QList<QUrl>& urls, QObject* parent=nullptr);
		virtual ~ExternUrlsDragDropHandler();

	public slots:
		void start() override;

	private slots:
		void metadataScannerFinished();
};
#endif //SAYONARA_PLAYER_EXTERNURLSDRAGDROPHANDLER_H
