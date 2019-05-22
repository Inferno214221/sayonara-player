/* MyColumnHeader.h */

/* Copyright 2011-2019  Lucio Carreras
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
 *
 *  Created on: 19.12.2012
 *      Author: luke
 */

#ifndef MYCOLUMNHEADER_H_
#define MYCOLUMNHEADER_H_

#include "Utils/Library/Sortorder.h"
#include "Utils/Pimpl.h"

#include <QObject>

class QAction;
namespace Library
{
	class ColumnHeader : public QObject
	{
		Q_OBJECT
		PIMPL(ColumnHeader)

		public:
			enum HeaderType
			{
				Sharp,
				Artist,
				Album,
				Discnumber,
				Title,
				NumTracks,
				Duration,
				DurationShort,
				Year,
				Rating,
				Bitrate,
				Filesize
			};

		public:
			ColumnHeader(HeaderType type, bool switchable, SortOrder sort_asc, SortOrder sort_desc, int preferred_size, bool stretchable=false);
			virtual ~ColumnHeader();

			bool stretchable() const;
			int default_size() const;
			int preferred_size() const;
			void set_preferred_size(int size);


			SortOrder sortorder_asc() const;
			SortOrder sortorder_desc() const;

			bool is_visible() const;
			bool is_hidden() const;

			void retranslate();

			QAction* action();
			QString	 title() const;
	};

	using ColumnHeaderPtr = std::shared_ptr<ColumnHeader>;
	class ColumnHeaderList :
		public QList<ColumnHeaderPtr>
	{
		public:
			int visible_columns() const;
			int visible_column(int n) const;
	};
}

#endif /* MYCOLUMNHEADER_H_ */
