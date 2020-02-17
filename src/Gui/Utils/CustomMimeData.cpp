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

#include "Gui/Utils/MimeDataUtils.h"

#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QUrl>

#include <algorithm>

using namespace Gui;

struct CustomMimeData::Private
{
	MetaDataList	tracks;
	int				playlistSourceIndex;
	QString			source;
	QString			coverUrl;
	const void*		ptr;
	//CustomMimeData::AsyncDropHandler* async_drop_handler=nullptr;

	Private(const void* ptr) :
		playlistSourceIndex(-1),
		ptr(ptr)
	{}
};

CustomMimeData::CustomMimeData(const void* ptr) :
	QMimeData()
{
	m = Pimpl::make<Private>(ptr);
}

const void* CustomMimeData::ptr() const
{
	return m->ptr;
}

CustomMimeData::~CustomMimeData() = default;

void CustomMimeData::setMetadata(const MetaDataList& tracks)
{
	m->tracks = tracks;

	QList<QUrl> urls;
	for(const MetaData& md : tracks)
	{
		QString filepath = md.filepath();
		if(Util::File::isUrl(filepath))
		{
			urls << QUrl(filepath);
		}

		else {
			urls << QUrl(QString("file://") + md.filepath());
		}
	}

	this->setUrls(urls);

	if(tracks.isEmpty()){
		this->setText("No tracks");
	}

	else{
		this->setText("tracks");
	}
}

const MetaDataList& CustomMimeData::metadata() const
{
	return m->tracks;
}

bool CustomMimeData::hasMetadata() const
{
	return (m->tracks.size() > 0);
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
