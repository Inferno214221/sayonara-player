/* PlaylistItemDelegate.h */

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

#ifndef PLAYLISTITEMDELEGATE_H_
#define PLAYLISTITEMDELEGATE_H_

#include "Gui/Utils/Delegates/StyledItemDelegate.h"

namespace Playlist
{
	/**
	 * @brief The PlaylistItemDelegate class
	 * @ingroup GuiPlaylists
	 */
	class Delegate :
		public Gui::StyledItemDelegate
	{
		Q_OBJECT

		public:
			explicit Delegate(QObject* parent);
			~Delegate() override;

			void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

			QWidget*
			createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
			void setEditorData(QWidget* editor, const QModelIndex& index) const override;
			void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

		private slots:
			void deleteEditor(bool save);
	};
}

#endif /* PLAYLISTITEMDELEGATEINTERFACE_H_ */
