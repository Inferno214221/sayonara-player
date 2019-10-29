/* GenreFetcher.h */

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



#ifndef GENREFETCHER_H
#define GENREFETCHER_H

#include <QObject>
#include "Utils/Pimpl.h"
#include "Utils/SetFwd.h"
#include "Database/ConnectorProvider.h"

class LocalLibrary;
class Genre;

namespace Tagging
{
	class Editor;
	class UserOperations;
}

class GenreFetcher :
		public QObject,
		public DB::ConnectorConsumer
{
	Q_OBJECT
	PIMPL(GenreFetcher)

signals:
	void sig_genres_fetched();
	void sig_progress(int progress);
	void sig_finished();

private:
	Tagging::UserOperations* init_tagging();

public:
	explicit GenreFetcher(QObject* parent=nullptr);
	~GenreFetcher() override;

	Util::Set<Genre> genres() const;

	void add_genre_to_md(const MetaDataList& v_md, const Genre& genre);
	void create_genre(const Genre& genre);
	void delete_genre(const Genre& genre);
	void delete_genres(const Util::Set<Genre>& genres);
	void rename_genre(const Genre& old_genre, const Genre& new_genre);

	void set_local_library(LocalLibrary* local_library);

public slots:
	void reload_genres();
};

#endif // GENREFETCHER_H
