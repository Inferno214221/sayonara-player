/* CoverView.h */

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

#ifndef COVERVIEW_H
#define COVERVIEW_H

#include "Gui/Library/ItemView.h"
#include "Gui/Library/Header/ActionPair.h"
#include "Utils/Library/Sortorder.h"

class LocalLibrary;
class QAction;

namespace Library
{
	class MergeData;
	class ActionPair;
	class CoverView :
			public ItemView
	{
		Q_OBJECT
		PIMPL(CoverView)

	public:
		explicit CoverView(QWidget* parent=nullptr);
		virtual ~CoverView() override;

		void init(LocalLibrary* library);
		AbstractLibrary* library() const override;

		// QAbstractItemView
		QStyleOptionViewItem viewOptions() const override;

		//SayonaraSelectionView
		int index_by_model_index(const QModelIndex& idx) const override;
		ModelIndexRange model_indexrange_by_index(int idx) const override;

		void change_zoom(int zoom=-1);
		void change_sortorder(SortOrder so);

		static QList<ActionPair> sorting_actions();
		static QStringList zoom_actions();

	public slots:
		void reload();
		void clear_cache();

	protected:
		void fill() override;
		void init_context_menu() override;

		void language_changed() override;
		void wheelEvent(QWheelEvent* e) override;
		void resizeEvent(QResizeEvent* e) override;
		void hideEvent(QHideEvent* e) override;

		// ItemView
		bool is_mergeable() const override;
		MD::Interpretation metadata_interpretation() const override;

		int sizeHintForColumn(int) const override;

	private:
		void resize_sections();

		// Library::ItemView
		void play_clicked() override;
		void play_new_tab_clicked() override;
		void play_next_clicked() override;
		void append_clicked() override;
		void selection_changed(const IndexSet& indexes) override;
		void refresh_clicked() override;
		void run_merge_operation(const Library::MergeData& mergedata) override;
	};
}

#endif // COVERVIEW_H
