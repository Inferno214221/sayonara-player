/* PlaylistItemDelegate.h */

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

#ifndef PLAYLISTITEMDELEGATE_H_
#define PLAYLISTITEMDELEGATE_H_

#include "Gui/Utils/Delegates/StyledItemDelegate.h"
#include "Utils/Pimpl.h"

class QTableView;

/**
 * @brief The PlaylistItemDelegate class
 * @ingroup GuiPlaylists
 */
class PlaylistItemDelegate :
		public Gui::StyledItemDelegate
{
	Q_OBJECT
	PIMPL(PlaylistItemDelegate)

	public:
		explicit PlaylistItemDelegate(QTableView* parent);
		~PlaylistItemDelegate() override;

		void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

		QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
		void setEditorData(QWidget* editor, const QModelIndex& index) const override;
		void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

	private slots:
		void sl_look_changed();
		void sl_show_rating_changed();

		void destroy_editor(bool save);
};



#endif /* PLAYLISTITEMDELEGATEINTERFACE_H_ */
