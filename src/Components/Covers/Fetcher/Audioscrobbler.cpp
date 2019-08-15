/* Audioscrobbler.cpp */

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

#include "Audioscrobbler.h"
#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Components/Streaming/LastFM/LFMGlobals.h"

#include <QStringList>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QMap>
#include <QUrl>

namespace Algorithm=Util::Algorithm;
using Cover::Fetcher::Audioscrobbler;

bool Audioscrobbler::can_fetch_cover_directly() const
{
	return false;
}

QDomNode find_artist_node(const QDomNode& node, const QString& prefix)
{
	if(node.nodeName().compare("artist", Qt::CaseInsensitive) == 0 ||
	   node.nodeName().compare("album", Qt::CaseInsensitive) == 0){
		return node;
	}

	if(node.hasChildNodes()){
		return find_artist_node(node.firstChild(), prefix + "  ");
	}

	else if(!node.nextSibling().isNull()){
		return find_artist_node(node.nextSibling(), prefix);
	}

	else
	{
		return QDomNode();
	}
}

QStringList Audioscrobbler::parse_addresses(const QByteArray& website) const
{
	QDomDocument doc("LastFM Cover");
	doc.setContent(website);

	QDomNode root_node = doc.firstChild();
	QDomNode artist_node = find_artist_node(root_node, "");

	if(artist_node.isNull()){
		return QStringList();
	}

	const QDomNodeList nodes = artist_node.childNodes();
	if(nodes.isEmpty()){
		return QStringList();
	}

	const QStringList attributes
	{
		"mega",
		"extralarge",
		"large",
		""
	};

	QMap<QString, QString> lfm_covers;

	for(int i=0; i<nodes.size(); i++)
	{
		QDomNode node = nodes.item(i);
		QString name = node.toElement().tagName();
		if(name.compare("image", Qt::CaseInsensitive) == 0)
		{
			QDomNode attr_node = node.attributes().namedItem("size");
			QString size_attr = attr_node.nodeValue();

			QString url = node.toElement().text();

			lfm_covers[size_attr] = url;
		}
	}

	QStringList ret;
	for(const QString& attr : attributes)
	{
		QString url = lfm_covers[attr];
		if(!url.isEmpty()){
			ret << url;
		}
	}

	sp_log(Log::Debug, this) << "Got " << ret.size() << " addresses";
	return ret;
}

QString Audioscrobbler::album_address(const QString& artist, const QString& album) const
{
	QString str = QString("http://ws.audioscrobbler.com/2.0/?method=album.getinfo&artist=" +
						  QUrl::toPercentEncoding(artist) +
						  "&album=" +
						  QUrl::toPercentEncoding(album) +
						  "&api_key=") + LFM_API_KEY;

	return str;
}


int Audioscrobbler::estimated_size() const
{
	return 300;
}

QString Audioscrobbler::priv_identifier() const
{
	return "audioscrobbler";
}
