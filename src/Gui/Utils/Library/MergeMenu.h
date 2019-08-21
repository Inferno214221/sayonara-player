/* Mergable.h */

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



#ifndef MERGABLE_H
#define MERGABLE_H

#include "Utils/Pimpl.h"
#include "Gui/Utils/Widgets/WidgetTemplate.h"

#include <QMap>
#include <QMenu>

class QAction;
class QStringList;

namespace Library
{
	class MergeData;
}

namespace Gui
{
	class MergeMenu :
		public Gui::WidgetTemplate<QMenu>
	{
		Q_OBJECT
		PIMPL(MergeMenu)

		signals:
			void sig_merge_triggered();

		public:
			MergeMenu(QMenu* parent=nullptr);
			virtual ~MergeMenu();

			void set_data(const QMap<Id, QString>& data);

			QAction* action() const;
			bool is_data_valid() const;
			::Library::MergeData mergedata() const;

		protected:
			void language_changed() override;
	};
}

#endif // MERGABLE_H
