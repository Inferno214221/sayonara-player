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

#include "Gui/Utils/MimeData/CustomMimeData.h"
#include "Gui/Utils/MimeData/ExternUrlsDragDropHandler.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"

#include <QStringList>
#include <QUrl>

#include <algorithm>

using namespace Gui;

MetaDataList MimeData::metadata(const QMimeData* data)
{
	const auto* cmd = dynamic_cast<const CustomMimeData*>(data);
	return (cmd != nullptr) ? cmd->metadata() : MetaDataList();
}

bool MimeData::hasMetadata(const QMimeData* data)
{
	const auto* cmd = dynamic_cast<const CustomMimeData*>(data);
	return (cmd && cmd->hasMetadata());
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
	QStringList wwwPlaylists;

	for(const QUrl& url : urls)
	{
		if(::Util::File::isPlaylistFile(url.toString())){
			wwwPlaylists << url.toString();
		}
	}

	return wwwPlaylists;
}

QString MimeData::coverUrl(const QMimeData* data)
{
	if(!data){
		return QString();
	}

	QString coverUrl;
	const CustomMimeData* cmd = customMimedata(data);
	if(cmd){
		coverUrl = cmd->coverUrl();
	}

	if(coverUrl.isEmpty()){
		coverUrl = data->property("cover_url").toString();
	}

	return coverUrl;
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

bool MimeData::isInnerDragDrop(const QMimeData* data, int targetPlaylistIdx)
{
	const CustomMimeData* cmd = customMimedata(data);
	if(!cmd){
		return false;
	}

	int sourceIdx = cmd->playlistSourceIndex();
	if(sourceIdx == -1){
		return false;
	}

	return (sourceIdx == targetPlaylistIdx);
}

bool MimeData::isDragFromPlaylist(const QMimeData* data)
{
	const CustomMimeData* cmd = customMimedata(data);
	if(!cmd){
		return false;
	}

	int sourceIdx = cmd->playlistSourceIndex();
	return (sourceIdx != -1);
}

bool MimeData::isPlayerDrag(const QMimeData* data)
{
	return (customMimedata(data) != nullptr);
}

AsyncDropHandler* MimeData::asyncDropHandler(const QMimeData* data)
{
	const auto* cmd = customMimedata(data);
	if(cmd) {
		return cmd->asyncDropHandler();
	}

	if(data->hasUrls()) {
		return new ExternUrlsDragDropHandler(data->urls());
	}

	return nullptr;
}
