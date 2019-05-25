/* MimeDataUtils.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#include "Components/Directories/DirectoryReader.h"

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

	if(data->hasUrls())
	{
		MetaDataList v_md;
		DirectoryReader reader;
		reader.set_filter(::Util::soundfile_extensions());

		const QList<QUrl> urls = data->urls();
		for(const QUrl& url : urls)
		{
			if(url.isLocalFile())
			{
				QStringList file_list;
				file_list << url.toLocalFile();
				MetaDataList v_md_tmp = reader.scan_metadata(file_list);
				v_md << std::move(v_md_tmp);
			}
		}

		return v_md;
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
		if(::Util::File::is_playlistfile(url.toString())){
			www_playlists << url.toString();
		}
	}

	return www_playlists;
}

QString MimeData::cover_url(const QMimeData* data)
{
	if(!data){
		return QString();
	}

	QString cover_url;
	const CustomMimeData* cmd = custom_mimedata(data);
	if(cmd){
		cover_url = cmd->cover_url();
	}

	if(cover_url.isEmpty()){
		cover_url = data->property("cover_url").toString();
	}

	return cover_url;
}

void MimeData::set_cover_url(QMimeData* data, const QString &url)
{
	if(!data){
		return;
	}

	CustomMimeData* cmd = custom_mimedata(data);
	if(cmd){
		cmd->set_cover_url(url);
	}

	data->setProperty("cover_url", url);
}

CustomMimeData* MimeData::custom_mimedata(QMimeData* data)
{
	return dynamic_cast<CustomMimeData*>(data);
}

const CustomMimeData* MimeData::custom_mimedata(const QMimeData* data)
{
	return dynamic_cast<const CustomMimeData*>(data);
}


bool MimeData::is_inner_drag_drop(const QMimeData* data, int target_playlist_idx)
{
	const CustomMimeData* cmd = custom_mimedata(data);
	if(!cmd){
		return false;
	}

	int source_idx = cmd->playlist_source_index();
	if(source_idx == -1){
		return false;
	}

	return (source_idx == target_playlist_idx);
}

bool MimeData::is_drag_from_playlist(const QMimeData* data)
{
	const CustomMimeData* cmd = custom_mimedata(data);
	if(!cmd){
		return false;
	}

	int source_idx = cmd->playlist_source_index();
	return (source_idx != -1);
}

bool MimeData::is_player_drag(const QMimeData* data)
{
	return (custom_mimedata(data) != nullptr);
}
