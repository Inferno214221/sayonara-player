/* GenreFetcher.h */

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



#ifndef GENREFETCHER_H
#define GENREFETCHER_H

#include <QObject>
#include "Utils/Pimpl.h"
#include "Utils/SetFwd.h"

class QStringList;
class MetaDataList;
class LocalLibrary;
class Genre;

namespace Tagging
{
	class Editor;
}

class GenreFetcher :
		public QObject
{
	Q_OBJECT
	PIMPL(GenreFetcher)

signals:
	void sig_genres_fetched();
	void sig_progress(int progress);
	void sig_finished();

private:
	Tagging::Editor* tag_edit();

public:
	explicit GenreFetcher(QObject* parent=nullptr);
	~GenreFetcher();

	SP::Set<Genre> genres() const;

	void add_genre_to_md(const MetaDataList& v_md, const Genre& genre);

	void create_genre(const Genre& genre);
	void delete_genre(const Genre& genre);
	void rename_genre(const Genre& old_genre, const Genre& new_genre);

	void set_local_library(LocalLibrary* local_library);

public slots:
	void reload_genres();

private slots:
	void metadata_changed(const MetaDataList& v_md_old, const MetaDataList& v_md_new);
	void metadata_deleted(const MetaDataList& v_md_deleted);

	void tag_edit_finished();
};


#endif // GENREFETCHER_H
