/* CustomMimeData.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#include "CustomMimeData.h"

#include "Gui/Utils/MimeData/DragDropAsyncHandler.h"

#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QUrl>

using namespace Gui;

struct CustomMimeData::Private
{
	MetaDataList tracks;
	QString source;
	QString coverUrl;

	AsyncDropHandler* asyncDropHandler = nullptr;
	int playlistSourceIndex;
	const void* data;

	Private(const void* data) :
		playlistSourceIndex(-1),
		data(data) {}
};

CustomMimeData::CustomMimeData(const void* data) :
	QMimeData()
{
	m = Pimpl::make<Private>(data);
}

const void* CustomMimeData::ptr() const
{
	return m->data;
}

CustomMimeData::~CustomMimeData() = default;

void CustomMimeData::setMetadata(const MetaDataList& tracks)
{
	m->tracks = tracks;

	QList<QUrl> urls;
	Util::Algorithm::transform(tracks, urls, [](const auto& track) {
		auto url = QUrl(track.filepath());
		if(url.scheme().isEmpty())
		{
			url.setScheme("file");
		}
		return url;
	});

	this->setUrls(urls);

	const auto text = (tracks.isEmpty())
	                  ? "No tracks"
	                  : "tracks";

	this->setText(text);
}

const MetaDataList& CustomMimeData::metadata() const
{
	return m->tracks;
}

bool CustomMimeData::hasMetadata() const
{
	return (!m->tracks.isEmpty());
}

void CustomMimeData::setPlaylistSourceIndex(int idx)
{
	m->playlistSourceIndex = idx;
}

int CustomMimeData::playlistSourceIndex() const
{
	return m->playlistSourceIndex;
}

QString CustomMimeData::coverUrl() const
{
	return m->coverUrl;
}

void CustomMimeData::setCoverUrl(const QString& url)
{
	m->coverUrl = url;
}

void CustomMimeData::setAsyncDropHandler(AsyncDropHandler* handler)
{
	m->asyncDropHandler = handler;
}

AsyncDropHandler* CustomMimeData::asyncDropHandler() const
{
	return m->asyncDropHandler;
}
