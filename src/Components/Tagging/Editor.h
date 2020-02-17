/* TagEdit.h */

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

#ifndef TAGEDIT_H
#define TAGEDIT_H

#include <QThread>
#include <QPixmap>

#include "Utils/Pimpl.h"

class Genre;

namespace Tagging
{
	/**
	 * @brief The TagEdit class
	 * Metadata has to be added using the set_metadata(const MetaDataList&) method.
	 * Use update_track(int idx, const MetaData& md) to stage the changes you made
	 * to the track. commit() starts the thread and writes changes to HDD and the
	 * database. When finished the finished() signal is emitted.
	 * @ingroup Tagging
	 */
	class Editor :
			public QObject
	{
		Q_OBJECT
		PIMPL(Editor)

		signals:
			void sigStarted();
			void sigFinished();
			void sigProgress(int);
			void sigMetadataReceived(const MetaDataList& tracks);

		public:
			explicit Editor(QObject* parent=nullptr);
			explicit Editor(const MetaDataList& tracks, QObject* parent=nullptr);
			~Editor() override;

			enum FailReason
			{
				FileNotWriteable=1,
				FileNotFound,
				TagLibError
			};


			/**
			 * @brief undo changes for a specific track
			 * @param idx track index
			 */
			void undo(int idx);

			/**
			 * @brief undo changes for all tracks
			 */
			void undoAll();


			/**
			 * @brief get the (changed) metadata for a specific index
			 * @param idx track index
			 * @return MetaData object
			 */
			MetaData metadata(int idx) const;


			/**
			 * @brief get all (changed) metadata
			 * @return MetaDataList object
			 */
			MetaDataList metadata() const;


			bool applyRegularExpression(const QString& regex, int idx);

			/**
			 * @brief Add a genre to all (changed) metdata
			 * @param genre the genre name
			 */
			void addGenre(int idx, const Genre& genre);


			void deleteGenre(int idx, const Genre& genre);

			void renameGenre(int idx, const Genre& genre, const Genre& new_genre);

			/**
			 * @brief gets the number of tracks
			 * @return number of tracks
			 */
			int count() const;


			/**
			 * @brief indicates if there are pending changes
			 */
			bool hasChanges() const;


			/**
			 * @brief writes changes to (changed) metadata for a specific track
			 * @param idx track index
			 * @param md new MetaData replacing the old track
			 */
			void updateTrack(int idx, const MetaData& md);

			/**
			 * @brief update the cover for a specific track.
			 * @param idx track index
			 * @param cover new cover image
			 */
			void updateCover(int idx, const QPixmap& cover);

			/**
			 * @brief remove_cover for a specific track
			 * @param idx track index
			 */
		//	void remove_cover(int idx);

			/**
			 * @brief does the user want to replace/add a cover
			 * @param idx track index
			 * @return false, if no new alternative cover is desired
			 */
			bool hasCoverReplacement(int idx) const;


			/**
			 * @brief initializes the TagEdit object with a MetaDataList
			 * @param tracks new MetaDataList
			 */
			void setMetadata(const MetaDataList& tracks);

			bool isCoverSupported(int idx) const;

			bool canLoadEntireAlbum() const;
			void loadEntireAlbum();

			QMap<QString, FailReason> failedFiles() const;

		public slots:

			/**
			 * @brief Commits changes to db
			 */
			void commit();


		private:
			/**
			 * @brief applies the new artists and albums to the original metadata
			 */
			void applyArtistsAndAlbumToMetadata();

		private slots:
			void loadEntireAlbumFinished();
	};
}

#endif // TAGEDIT_H
