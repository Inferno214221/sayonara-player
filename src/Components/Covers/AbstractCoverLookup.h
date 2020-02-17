/* AbstractCoverLookup.h */

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



#ifndef ABSTRACTCOVERLOOKUP_H
#define ABSTRACTCOVERLOOKUP_H

#include <QObject>
#include <QPixmap>
#include "Utils/Pimpl.h"

namespace Cover
{
	class Location;
	/**
	 * @brief The CoverLookupInterface class
	 * @ingroup Covers
	 */
	class LookupBase :
			public QObject
	{
		Q_OBJECT
		PIMPL(LookupBase)

	signals:
		void sigCoverFound(const QPixmap& pm);
		void sigFinished(bool success);
		void sigStarted();

	public slots:
		virtual void stop()=0;

	public:
		explicit LookupBase(const Location& cl, QObject* parent=nullptr);
		virtual ~LookupBase();

		Location coverLocation() const;
		void setCoverLocation(const Location& cl);
	};
}

#endif // ABSTRACTCOVERLOOKUP_H
