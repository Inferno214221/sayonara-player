/* DragDropAsyncHandler.cpp
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

#include "DragDropAsyncHandler.h"
#include "Utils/MetaData/MetaDataList.h"

using Gui::AsyncDropHandler;

struct Gui::AsyncDropHandler::Private
{
	MetaDataList tracks;
	int targetIndex;

	Private() :
		targetIndex(-1) {}
};

AsyncDropHandler::AsyncDropHandler(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();
}

AsyncDropHandler::~AsyncDropHandler() = default;

void AsyncDropHandler::setTargetIndex(int index)
{
	m->targetIndex = index;
}

int AsyncDropHandler::targetIndex() const
{
	return m->targetIndex;
}

void Gui::AsyncDropHandler::setTracks(const MetaDataList& tracks)
{
	m->tracks = tracks;
	emit sigFinished();
}

MetaDataList Gui::AsyncDropHandler::tracks() const
{
	return m->tracks;
}
