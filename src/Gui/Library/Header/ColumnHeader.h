/* MyColumnHeader.h */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#include "ColumnIndex.h"
#include "Utils/Library/Sortorder.h"
#include "Utils/Pimpl.h"

#include <QObject>

class QAction;
namespace Library
{
	/**
	 * @brief The ColumnHeader class
	 * @ingroup GuiLibrary
	 */
	class ColumnHeader : public QObject
	{
		Q_OBJECT
		PIMPL(ColumnHeader)

		protected:
			ColumnHeader(ColumnIndex::IntegerType columnIndex, bool switchable, SortOrder sortAsc, SortOrder sortDesc, int preferredWidth, bool isStretchable=false);
			virtual QString hashPrefix() const=0;

		public:
			virtual ~ColumnHeader();

			virtual QString title() const=0;

			QString hash() const;

			bool isStretchable() const;
			bool isSwitchable() const;
			int defaultSize() const;

			int preferredSize() const;
			void setPreferredSize(int size);

			SortOrder sortorder(Qt::SortOrder sortOrder) const;

			ColumnIndex::IntegerType columnIndex() const;
	};


	class ColumnHeaderTrack : public ColumnHeader
	{
		Q_OBJECT

		public:
			ColumnHeaderTrack(ColumnIndex::Track columnIndex, bool switchable, SortOrder sortAsc, SortOrder sortDesc, int preferredWidth, bool isStretchable=false);
			QString title() const override;

		protected:
			QString hashPrefix() const override;
	};

	class ColumnHeaderAlbum : public ColumnHeader
	{
		Q_OBJECT

		public:
			ColumnHeaderAlbum(ColumnIndex::Album columnIndex, bool switchable, SortOrder sortAsc, SortOrder sortDesc, int preferredWidth, bool isStretchable=false);
			QString title() const override;

		protected:
			QString hashPrefix() const override;
	};

	class ColumnHeaderArtist : public ColumnHeader
	{
		Q_OBJECT

		public:
			ColumnHeaderArtist(ColumnIndex::Artist columnIndex, bool switchable, SortOrder sortAsc, SortOrder sortDesc, int preferredWidth, bool isStretchable=false);
			QString title() const override;

		protected:
			QString hashPrefix() const override;
	};

	using ColumnHeaderPtr = std::shared_ptr<ColumnHeader>;
	using ColumnHeaderList = QList<ColumnHeaderPtr>;
}

#endif /* MYCOLUMNHEADER_H_ */
