/* AlbumCoverFetchThread.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#ifndef ALBUMCOVERFETCHTHREAD_H
#define ALBUMCOVERFETCHTHREAD_H

#include "Utils/Pimpl.h"

#include <QThread>
#include <QModelIndex>

namespace Cover
{
	class Location;
	class Lookup;
}

namespace Library
{
	/**
	 * @brief This class organizes requests for new Covers for the AlbumCoverView.
	 * When looking for covers, not all requests should be fired simultaneously,
	 * so there should be a buffer assuring that covers are found one by one.
	 * Albums are organized by hashes, each album has a CoverLocation. A new
	 * request is added to the queue by calling add_data(). A new request is
	 * handled BEFORE old requests. The thread is locked until the done() function
	 * is called. The thread emits the signal sig_next(). The cover location
	 * and the hash which should be processed next can be fetched by current_hash()
	 * and current_coverLocation().
	 * @ingroup GuiLibrary
	 */
	class AlbumCoverFetchThread :
		public QThread
	{
		Q_OBJECT
		PIMPL(AlbumCoverFetchThread)

		signals:
			void sigNext();

		public:
			using Hash = QString;
			using HashAlbumPair = QPair<Hash, Album>;
			using HashAlbumList = QList<HashAlbumPair>;
			using HashLocationPair = QPair<Hash, Cover::Location>;
			using HashLocationList = QList<HashLocationPair>;

		protected:
			void run() override;

		public:
			explicit AlbumCoverFetchThread(QObject* parent = nullptr);
			~AlbumCoverFetchThread() override;

			/**
			 * @brief add_data Add a new album request
			 * @param hash hashed album info
			 * @param cl Cover Location of the album
			 */
			void addAlbum(const Album& album);

			/**
			 * @brief check if album is already processed or about
			 * to be processed in the future
			 * @param hashLocationPair
			 * @return
			 */
			bool checkAlbum(const QString& hashLocationPair);

			HashLocationPair takeCurrentLookup();

			/**
			 * @brief stop Stop the thread
			 */
			void stop();
			void clear();
			void removeHash(const Hash& hash);

			static Hash getHash(const Album& album);
	};
}

#endif // ALBUMCOVERFETCHTHREAD_H
