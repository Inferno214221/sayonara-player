/* LFMSimArtistsParser.cpp */

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

#include "LFMSimArtistsParser.h"
#include "ArtistMatch.h"
#include "Utils/Logger/Logger.h"

#include <QFile>
#include <QMap>
#include <QString>

#include <QDomDocument>
#include <QDomNode>
#include <QDomElement>

using namespace LastFM;

struct SimArtistsParser::Private
{
	ArtistMatch artistMatch;
	QString     artistName;
	QByteArray  data;

	Private(const QString& artistName, const QByteArray& data) :
		artistName(artistName),
		data(data)
	{}
};


SimArtistsParser::SimArtistsParser(const QString& artistName, const QByteArray& data)
{
	m = Pimpl::make<Private>(artistName, data);

	parseDocument();
}

SimArtistsParser::SimArtistsParser(const QString& artistName, const QString& filename)
{
	QByteArray data;

	QFile f(filename);
	if(f.open(QFile::ReadOnly)){
		data = f.readAll();
		f.close();
	}

	m = Pimpl::make<Private>(artistName, data);

	parseDocument();
}

SimArtistsParser::~SimArtistsParser() {}

void SimArtistsParser::parseDocument()
{
	bool success;
	QDomDocument doc("similar_artists");
	success = doc.setContent(m->data);

	if(!success)
	{
		m->artistMatch = ArtistMatch();
		spLog(Log::Warning, this) << "Cannot parse similar artists document";
		return;
	}

	m->artistMatch = ArtistMatch(m->artistName);

	QDomElement docElement = doc.documentElement();
	QDomNode similar_artists = docElement.firstChild();			// similarartists

	if(!similar_artists.hasChildNodes()) {
		return;
	}

	QString artistName, mbid;
	double match = -1.0;

	QDomNodeList child_nodes = similar_artists.childNodes();
	for(int idx_artist=0; idx_artist < child_nodes.size(); idx_artist++)
	{
		QDomNode artist = child_nodes.item(idx_artist);
		QString node_name = artist.nodeName();

		if(node_name.compare("artist", Qt::CaseInsensitive) != 0) {
			continue;
		}

		if(!artist.hasChildNodes()) {
			continue;
		}

		QDomNodeList artist_child_nodes = artist.childNodes();
		for(int idx_content = 0; idx_content <artist_child_nodes.size(); idx_content++)
		{
			QDomNode content = artist_child_nodes.item(idx_content);
			QString node_name = content.nodeName().toLower();
			QDomElement e = content.toElement();

			if(node_name.compare("name") == 0)
			{
				if(!e.isNull()) {
					artistName = e.text();
				}
			}

			else if(node_name.compare("match") == 0)
			{
				if(!e.isNull()) {
					match = e.text().toDouble();
				}
			}

			else if(node_name.compare("mbid") == 0)
			{
				if(!e.isNull()) {
					mbid = e.text();
				}
			}

			if(!artistName.isEmpty() && match > 0 && !mbid.isEmpty())
			{
				ArtistMatch::ArtistDesc artist_desc(artistName, mbid);
				m->artistMatch.add(artist_desc, match);
				artistName = "";
				match = -1.0;
				mbid = "";
				break;
			}
		}
	}
}


LastFM::ArtistMatch SimArtistsParser::artistMatch() const
{
	return m->artistMatch;
}
