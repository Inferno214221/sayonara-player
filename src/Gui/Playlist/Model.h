/* PlaylistItemModel.h */

/* Copyright (C) 2011-2019 Lucio Carreras
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


/*
 * PlaylistItemModel.h
 *
 *  Created on: Apr 8, 2011
 *      Author: Lucio Carreras
 */

#ifndef PLAYLISTITEMMODEL_H_
#define PLAYLISTITEMMODEL_H_

#include "Gui/Utils/SearchableWidget/SearchableModel.h"
#include "Utils/Playlist/PlaylistFwd.h"
#include "Utils/Pimpl.h"

class CustomMimeData;

class PlaylistItemModel :
		public SearchableTableModel
{
	Q_OBJECT
	PIMPL(PlaylistItemModel)

	using SearchableModelInterface::ExtraTriggerMap;

signals:
	void sig_data_ready();

public:
	enum ColumnName
	{
		TrackNumber=0,
		Cover,
		Description,
		Time,
		NumColumns
	};

	explicit PlaylistItemModel(PlaylistPtr pl, QObject* parent=nullptr);
	virtual ~PlaylistItemModel();

	int rowCount(const QModelIndex& parent=QModelIndex()) const override;
	int columnCount(const QModelIndex& parent=QModelIndex()) const override;

	Qt::ItemFlags	flags(const QModelIndex& index=QModelIndex()) const override;
	QVariant		data(const QModelIndex& index, int role=Qt::DisplayRole) const override;
	bool			setData(const QModelIndex &index, const QVariant &value, int role) override;


	SearchableModelInterface::ExtraTriggerMap	getExtraTriggers() override;
	QModelIndex		getRowIndexOf(const QString& substr, int row, bool is_forward);
	QModelIndexList	search_results(const QString& substr) override;

	void		clear();
	void		remove_rows(const IndexSet& rows);
	IndexSet	move_rows(const IndexSet& rows, int target_index);
	IndexSet	move_rows_up(const IndexSet& rows);
	IndexSet	move_rows_down(const IndexSet& rows);
	IndexSet	copy_rows(const IndexSet& rows, int target_index);
	void		change_rating(const IndexSet& rows, Rating rating);
	void		insert_tracks(const MetaDataList& v_md, int row);

	void		set_current_track(int row);
	int			current_track() const;

	MetaData		metadata(int row) const;
	MetaDataList	metadata(const IndexSet& rows) const;

	QMimeData*		mimeData(const QModelIndexList& indexes) const override;

	bool			has_local_media(const IndexSet& rows) const;
	void			set_drag_index(int drag_index);
	void			set_row_height(int row_height);

public slots:
	void			refresh_data();

private:
	void			look_changed();

private slots:
	void			playlist_changed(int pl_idx);
};

#endif /* PLAYLISTITEMMODEL_H_ */
