/* LocalCoverSearcher.h */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#ifndef LOCALCOVERSEARCHER_H
#define LOCALCOVERSEARCHER_H

class QStringList;
class QString;

namespace Cover
{
    /**
     * @brief Helper Namespace for finding covers in a specific directory
     * @ingroup Covers
     */
    namespace LocalSearcher
    {
        /**
         * @brief Extracts dirname and calls get_local_cover_paths_source_dirname
         * @param filename filename of a file within the same directory. E.g. a mp3 file
         * @return Paths for all covers in that directory
         */
        QStringList cover_paths_from_path_hint(const QString& filename);


    }
}

#endif // LOCALCOVERSEARCHER_H
