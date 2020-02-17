/* MimeDataUtils.cpp */

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

#include "MimeDataUtils.h"

#include "Gui/Utils/CustomMimeData.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"

#include <QStringList>
#include <QUrl>

#include <algorithm>

using namespace Gui;

MetaDataList MimeData::metadata(const QMimeData* data)
{
	if(!data){
		return MetaDataList();
	}

	const CustomMimeData* cmd = dynamic_cast<const CustomMimeData*>(data);
	if(cmd)
	{
		return cmd->metadata();
	}

	return MetaDataList();
}

QStringList MimeData::playlists(const QMimeData* data)
{
	if(!data){
		return QStringList();
	}

	if(!data->hasUrls()){
		return QStringList();
	}

	const QList<QUrl> urls = data->urls();
	QStringList www_playlists;

	for(const QUrl& url : urls)
	{
		if(::Util::File::isPlaylistFile(url.toString())){
			www_playlists << url.toString();
		}
	}

	return www_playlists;
}

QString MimeData::coverUrl(const QMimeData* data)
{
	if(!data){
		return QString();
	}

	QString cover_url;
	const CustomMimeData* cmd = customMimedata(data);
	if(cmd){
		cover_url = cmd->coverUrl();
	}

	if(cover_url.isEmpty()){
		cover_url = data->property("cover_url").toString();
	}

	return cover_url;
}

void MimeData::setCoverUrl(QMimeData* data, const QString& url)
{
	if(!data){
		return;
	}

	CustomMimeData* cmd = customMimedata(data);
	if(cmd){
		cmd->setCoverUrl(url);
	}

	data->setProperty("cover_url", url);
}

CustomMimeData* MimeData::customMimedata(QMimeData* data)
{
	return dynamic_cast<CustomMimeData*>(data);
}

const CustomMimeData* MimeData::customMimedata(const QMimeData* data)
{
	return dynamic_cast<const CustomMimeData*>(data);
}


bool MimeData::isInnerDragDrop(const QMimeData* data, int target_playlist_idx)
{
	const CustomMimeData* cmd = customMimedata(data);
	if(!cmd){
		return false;
	}

	int source_idx = cmd->playlistSourceIndex();
	if(source_idx == -1){
		return false;
	}

	return (source_idx == target_playlist_idx);
}

bool MimeData::isDragFromPlaylist(const QMimeData* data)
{
	const CustomMimeData* cmd = customMimedata(data);
	if(!cmd){
		return false;
	}

	int source_idx = cmd->playlistSourceIndex();
	return (source_idx != -1);
}

bool MimeData::isPlayerDrag(const QMimeData* data)
{
	return (customMimedata(data) != nullptr);
}
