/* Audioscrobbler.cpp */

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

#include "Audioscrobbler.h"
#include "Utils/Logger/Logger.h"
#include "Components/Streaming/LastFM/LFMGlobals.h"

#include <QStringList>
#include <QDomDocument>
#include <QMap>
#include <QUrl>

using Cover::Fetcher::Audioscrobbler;

bool Audioscrobbler::canFetchCoverDirectly() const
{
	return false;
}

static
QDomNode findArtistNode(const QDomNode& node, const QString& prefix)
{
	if((node.nodeName().compare("artist", Qt::CaseInsensitive) == 0) ||
	   (node.nodeName().compare("album", Qt::CaseInsensitive) == 0))
	{
		return node;
	}

	if(node.hasChildNodes())
	{
		return findArtistNode(node.firstChild(), prefix + "  ");
	}

	else if(!node.nextSibling().isNull())
	{
		return findArtistNode(node.nextSibling(), prefix);
	}

	else
	{
		return QDomNode();
	}
}

QStringList Audioscrobbler::parseAddresses(const QByteArray& website) const
{
	QDomDocument doc("LastFM Cover");
	doc.setContent(website);

	QDomNode rootNode = doc.firstChild();
	QDomNode artistNode = findArtistNode(rootNode, "");

	if(artistNode.isNull())
	{
		return QStringList();
	}

	const QDomNodeList nodes = artistNode.childNodes();
	if(nodes.isEmpty())
	{
		return QStringList();
	}

	const QStringList attributes
		{
			"mega",
			"extralarge",
			"large",
			""
		};

	QMap<QString, QString> lfmCovers;

	for(int i = 0; i < nodes.size(); i++)
	{
		QDomNode node = nodes.item(i);
		QString name = node.toElement().tagName();
		if(name.compare("image", Qt::CaseInsensitive) == 0)
		{
			QDomNode attrNode = node.attributes().namedItem("size");
			QString sizeAttr = attrNode.nodeValue();

			QString url = node.toElement().text();

			lfmCovers[sizeAttr] = url;
		}
	}

	QStringList ret(lfmCovers.values());
	ret.removeAll("");

	spLog(Log::Debug, this) << "Got " << ret.size() << " addresses";

	return ret;
}

QString Audioscrobbler::albumAddress(const QString& artist, const QString& album) const
{
	const QString str = QString("http://ws.audioscrobbler.com/2.0/?method=album.getinfo&artist=" +
	                      QUrl::toPercentEncoding(artist) +
	                      "&album=" +
	                      QUrl::toPercentEncoding(album) +
	                      "&api_key=") + LFM_API_KEY;

	return str;
}

int Audioscrobbler::estimatedSize() const
{
	return 300;
}

QString Audioscrobbler::privateIdentifier() const
{
	return "audioscrobbler";
}
