/* Lyrics.h */

/* Copyright (C) 2011-2017  Lucio Carreras
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



#ifndef LYRICS_H
#define LYRICS_H

#include "Utils/Pimpl.h"

#include <QObject>

class Lyrics :
		public QObject
{
	Q_OBJECT
	PIMPL(Lyrics)

signals:
	void sig_lyrics_fetched();

public:
	explicit Lyrics(QObject* parent=nullptr);
	~Lyrics();

	QStringList servers() const;
	void set_metadata(const MetaData& md);
	bool fetch_lyrics(const QString& artist, const QString& title, int server_index);
	bool save_lyrics(const QString& plain_text);

	QString artist() const;
	QString title() const;
	QString lyric_header() const;
	QString local_lyric_header() const;
	QString lyrics() const;
	QString local_lyrics() const;

	bool is_lyric_valid() const;
	bool is_lyric_tag_available() const;
	bool is_lyric_tag_supported() const;

private slots:
	void lyrics_fetched();


};

#endif // LYRICS_H
