/* PlaylistView.h */

/* Copyright (C) 2011-2017 Lucio Carreras
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
 * PlaylistView.h
 *
 *  Created on: Jun 27, 2011
 *      Author: Lucio Carreras
 */

#ifndef PLAYLISTVIEW_H_
#define PLAYLISTVIEW_H_

#include "GUI/Utils/SearchableWidget/SearchableView.h"
#include "GUI/Utils/Widgets/Dragable.h"
#include "GUI/Utils/Widgets/WidgetTemplate.h"

#include "GUI/InfoDialog/InfoDialogContainer.h"

#include "Utils/Playlist/PlaylistFwd.h"
#include "Utils/MetaData/MetaDataFwd.h"
#include "Utils/Pimpl.h"
#include "Utils/Settings/SayonaraClass.h"

class QPoint;
class PlaylistView :
		public SearchableTableView,
		public InfoDialogContainer,
		private Dragable
{
	Q_OBJECT
	PIMPL(PlaylistView)

signals:
	void sig_double_clicked(int row);
	void sig_delete_tracks(const IndexSet& rows);
	void sig_bookmark_pressed(int track_idx, Seconds timestamp);

public:
	explicit PlaylistView(PlaylistPtr pl, QWidget* parent=nullptr);
	virtual ~PlaylistView();

	void goto_row(int row);


	void delete_selected_tracks();

	/**
	 * @brief called from GUI_Playlist when data has not been dropped
	 * directly into the view widget. Insert on first row then
	 * @param event
	 */
	void dropEventFromOutside(QDropEvent* event);


public slots:
	void clear();
	void remove_selected_rows();

private:
	void init_view();
	void init_context_menu();

	// d & d
	int calc_drag_drop_line(QPoint pos);
	void handle_drop(QDropEvent* event);
	void handle_inner_drag_drop(int row, bool copy);

	// overloaded stuff
	void contextMenuEvent(QContextMenuEvent* e) override;

	/**
	 * @brief we start the drag action, all lines has to be cleared
	 * @param the event
	 */
	void dragLeaveEvent(QDragLeaveEvent* event) override;
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dragMoveEvent(QDragMoveEvent* event) override;
	void dropEvent(QDropEvent* event) override;

	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent* event) override;

	void keyPressEvent(QKeyEvent *event) override;

	MD::Interpretation metadata_interpretation() const override;
	MetaDataList info_dialog_data() const override;
	QMimeData* dragable_mimedata() const override;

private slots:
	void refresh();
	void async_drop_finished(bool success, int async_drop_index);
	void rating_changed(Rating rating);
	void sl_columns_changed();

	void move_selected_rows_up();
	void move_selected_rows_down();
	void play_selected_track();


protected:
	// SayonaraSelectionView interface
	int index_by_model_index(const QModelIndex& idx) const override;
	ModelIndexRange model_indexrange_by_index(int idx) const override;
	bool viewportEvent(QEvent *event) override;

	void skin_changed() override;
};

#endif /* PlaylistView_H_ */
