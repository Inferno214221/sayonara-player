/* MetaDataChangeNotifier.h */

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

#ifndef METADATACHANGENOTIFIER_H
#define METADATACHANGENOTIFIER_H

#include "Utils/Singleton.h"
#include "Utils/Pimpl.h"

#include <QObject>
#include <QPair>

class MetaDataList;

namespace Tagging
{
    /**
     * @brief The MetaDataChangeNotifier class (Singleton)
     * @ingroup Tagging
     */
    class ChangeNotifier : public QObject
    {
        Q_OBJECT

        SINGLETON_QOBJECT(ChangeNotifier)
        PIMPL(ChangeNotifier)

    public:
		/**
		 * @brief change_metadata Tell the MetaDataChangeNotifier that MetaData has been changed
		 * @param v_md_old The original Metadata used for comparison
		 * @param v_md_new The actualized Metadata
		 */
		void change_metadata(const MetaDataList& v_md_old, const MetaDataList& v_md_new);

        void delete_metadata(const MetaDataList& v_md_deleted);

        QPair<MetaDataList, MetaDataList> changed_metadata() const;
        MetaDataList deleted_metadata() const;

    signals:
        /**
         * @brief sig_metadata_changed Signal emitted when change_metadata was called
         * @param v_md_oldThe original Metadata used for comparison
         * @param v_md_new The actualized Metadata
         */
        void sig_metadata_changed();

        void sig_metadata_deleted();
    };
}

#endif // METADATACHANGENOTIFIER_H
