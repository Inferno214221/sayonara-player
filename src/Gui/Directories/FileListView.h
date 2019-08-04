/* FileListView.h */

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

#ifndef FILELISTVIEW_H
#define FILELISTVIEW_H

#include "Gui/Utils/SearchableWidget/SearchableView.h"
#include "Gui/Utils/Widgets/Dragable.h"

#include "Utils/Pimpl.h"

class FileListModel;
class LibraryContextMenu;

class FileListView :
		public SearchableListView,
		private Gui::Dragable
{
	Q_OBJECT
	PIMPL(FileListView)

signals:
	void sig_info_clicked();
	void sig_edit_clicked();
	void sig_lyrics_clicked();
	void sig_delete_clicked();
	void sig_play_clicked();
	void sig_play_new_tab_clicked();
	void sig_play_next_clicked();
	void sig_append_clicked();
	void sig_enter_pressed();
	void sig_import_requested(LibraryId lib_id, const QStringList& files, const QString& target_dir);

public:
	explicit FileListView(QWidget* parent=nullptr);
	virtual ~FileListView();

	QModelIndexList selected_rows() const;
	MetaDataList selected_metadata() const;

	QStringList selected_paths() const;

	void set_parent_directory(LibraryId id, const QString& dir);
	QString parent_directory() const;

	void set_search_filter(const QString& search_string);

	QMimeData* dragable_mimedata() const override;


protected:
	void keyPressEvent(QKeyEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;

	void dragEnterEvent(QDragEnterEvent* event) override;
	void dragMoveEvent(QDragMoveEvent* event) override;
	void dropEvent(QDropEvent* event) override;

	void language_changed() override;
	void skin_changed() override;

	// SayonaraSelectionView
	int index_by_model_index(const QModelIndex& idx) const override;
	ModelIndexRange model_indexrange_by_index(int idx) const override;

private:
	void init_context_menu();

private slots:
	void rename_file_clicked();

};

#endif // FILELISTVIEW_H
