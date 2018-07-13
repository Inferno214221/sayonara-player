/* DirectoryTreeView.h */

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

#ifndef DIRECTORYTREEVIEW_H
#define DIRECTORYTREEVIEW_H

#include "DirectoryModel.h"
#include "GUI/Utils/SearchableWidget/SearchableView.h"
#include "GUI/Utils/Widgets/WidgetTemplate.h"
#include "GUI/Utils/Widgets/Dragable.h"
#include "Utils/Pimpl.h"

#include <QTreeView>
#include <QModelIndexList>
#include <QTreeView>

class LibraryContextMenu;
class DirectoryModel;
class MetaDataList;
class IconProvider;
class QStringList;
class CustomMimeData;

namespace Library
{
	class Info;
}

using SearchableTreeView=Gui::WidgetTemplate<SearchableView<QTreeView, DirectoryModel>>;
class DirectoryTreeView :
		public SearchableTreeView,
		protected Dragable
{
	Q_OBJECT
	PIMPL(DirectoryTreeView)

signals:
	void sig_info_clicked();
	void sig_edit_clicked();
	void sig_lyrics_clicked();
	void sig_delete_clicked();
	void sig_play_clicked();
	void sig_play_new_tab_clicked();
	void sig_play_next_clicked();
	void sig_append_clicked();
	void sig_directory_loaded(const QModelIndex& index);
	void sig_enter_pressed();
	void sig_import_requested(LibraryId lib_id, const QStringList& v_md, const QString& target_dir);

	void sig_copy_started();
	void sig_copy_finished();

public:
	explicit DirectoryTreeView(QWidget* parent=nullptr);
	virtual ~DirectoryTreeView();

	QModelIndex		search(const QString& search_term);
	QString			directory_name(const QModelIndex& index);
	QString			directory_name_origin(const QModelIndex& index);

	QModelIndexList	selected_indexes() const;
	MetaDataList	selected_metadata() const;
	QStringList		selected_paths() const;

	QMimeData*		dragable_mimedata() const override;
	LibraryId		library_id(const QModelIndex& index) const;

private:
	enum class DropAction
	{
		Copy,
		Move,
		Cancel
	};

	void init_context_menu();
	DropAction show_drop_menu(const QPoint& pos);

private slots:
	void drag_move_timer_finished();
	void create_dir_clicked();
	void rename_dir_clicked();
	void copy_started();
	void copy_finished();

protected:
	void keyPressEvent(QKeyEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent *event) override;

	void dragEnterEvent(QDragEnterEvent *event) override;
	void dragLeaveEvent(QDragLeaveEvent* event) override;
	void dragMoveEvent(QDragMoveEvent *event) override;
	void dropEvent(QDropEvent *event) override;


	// SayonaraSelectionView
	int index_by_model_index(const QModelIndex& idx) const override;
	ModelIndexRange model_indexrange_by_index(int idx) const override;

	void select_match(const QString& str, SearchDirection direction) override;

	// Dragable
	bool has_drag_label() const override;
	QString drag_label() const override;

	void skin_changed() override;
	void language_changed() override;

	void handle_sayonara_drop(const CustomMimeData* mimedata, const QString& target_dir);


};

#endif // DIRECTORYTREEVIEW_H
