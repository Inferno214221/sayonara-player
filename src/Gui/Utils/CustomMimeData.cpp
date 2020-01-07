/* CustomMimeData.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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
	MetaDataList	v_md;
	int				playlist_source_index;
	QString			source;
	QString			cover_url;
	const void*		ptr;
	//CustomMimeData::AsyncDropHandler* async_drop_handler=nullptr;

	Private(const void* ptr) :
		playlist_source_index(-1),
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

void CustomMimeData::set_metadata(const MetaDataList& v_md)
{
	m->v_md = v_md;

	QList<QUrl> urls;
	for(const MetaData& md : v_md)
	{
		QString filepath = md.filepath();
		if(Util::File::is_url(filepath))
		{
			urls << QUrl(filepath);
		}

		else {
			urls << QUrl(QString("file://") + md.filepath());
		}
	}

	this->setUrls(urls);

	if(v_md.isEmpty()){
		this->setText("No tracks");
	}

	else{
		this->setText("tracks");
	}
}

const MetaDataList& CustomMimeData::metadata() const
{
	return m->v_md;
}

bool CustomMimeData::has_metadata() const
{
	return (m->v_md.size() > 0);
}

void CustomMimeData::set_playlist_source_index(int idx)
{
	m->playlist_source_index = idx;
}

int CustomMimeData::playlist_source_index() const
{
	return m->playlist_source_index;
}

QString CustomMimeData::cover_url() const
{
	return m->cover_url;
}

void CustomMimeData::set_cover_url(const QString& url)
{
	m->cover_url = url;
}

//void CustomMimeData::set_drop_handler(CustomMimeData::AsyncDropHandler* handler)
//{
//	m->async_drop_handler = handler;
//}

//CustomMimeData::AsyncDropHandler* CustomMimeData::drop_handler() const
//{
//	return m->async_drop_handler;
//}

