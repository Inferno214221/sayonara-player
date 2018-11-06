/* TagEdit.h */

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
			public QThread
	{
		Q_OBJECT
		PIMPL(Editor)

	signals:
		void sig_finished();
		void sig_progress(int);
		void sig_metadata_received(const MetaDataList& v_md);


	public:
		explicit Editor(QObject* parent=nullptr);
		explicit Editor(const MetaDataList& v_md, QObject* parent=nullptr);
		~Editor();


		/**
		 * @brief undo changes for a specific track
		 * @param idx track index
		 */
		void undo(int idx);

		/**
		 * @brief undo changes for all tracks
		 */
		void undo_all();


		/**
		 * @brief get the (changed) metadata for a specific index
		 * @param idx track index
		 * @return MetaData object
		 */
		const MetaData& metadata(int idx) const;


		/**
		 * @brief get all (changed) metadata
		 * @return MetaDataList object
		 */
		const MetaDataList& metadata() const;


		bool apply_regex(const QString& regex, int idx);


		/**
		 * @brief Add a genre to all (changed) metdata
		 * @param genre the genre name
		 */
		void add_genre(int idx, const Genre& genre);


		void delete_genre(int idx, const Genre& genre);

		void rename_genre(int idx, const Genre& genre, const Genre& new_genre);



		/**
		 * @brief gets the number of tracks
		 * @return number of tracks
		 */
		int count() const;


		/**
		 * @brief indicates if there are pending changes
		 */
		bool has_changes() const;


		/**
		 * @brief writes changes to (changed) metadata for a specific track
		 * @param idx track index
		 * @param md new MetaData replacing the old track
		 */
		void update_track(int idx, const MetaData& md);

		/**
		 * @brief update the cover for a specific track.
		 * @param idx track index
		 * @param cover new cover image
		 */
		void update_cover(int idx, const QPixmap& cover);

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
		bool has_cover_replacement(int idx) const;


		/**
		 * @brief initializes the TagEdit object with a MetaDataList
		 * @param v_md new MetaDataList
		 */
		void set_metadata(const MetaDataList& v_md);

		bool is_cover_supported(int idx) const;

		bool can_load_entire_album() const;
		void load_entire_album();



	public slots:

		/**
		 * @brief Commits changes to db
		 */
		void commit();


	private:
		/**
		 * @brief applies the new artists and albums to the original metadata
		 */
		void apply_artists_and_albums_to_md();

		void run() override;

	private slots:
		void thread_finished();
	};
}

#endif // TAGEDIT_H
