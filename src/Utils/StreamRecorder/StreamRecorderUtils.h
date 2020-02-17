/* StreamRecorderUtils.h */

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

#ifndef STREAMRECORDERUTILS_H
#define STREAMRECORDERUTILS_H

#include <QPair>
#include <QList>
#include <QString>
#include <QHash>
#include <QDateTime>
#include <QList>

class MetaData;

namespace StreamRecorder
{
    namespace Utils
    {
        enum class ErrorCode : uint8_t
        {
            OK=1,
            BracketError,
            UnknownTag,
            MissingUniqueTag,
            InvalidChars,
            Empty
        };

        // filename, playlistname
        using TargetPaths=QPair<QString, QString>;

        QList<QString> supportedTags();
        QList<QPair<QString, QString>> descriptions();

        ErrorCode validateTemplate(const QString& target_path_template, int* invalid_idx);

        QString targetPathTemplateDefault(bool useSessionPath);

		/**
		 * @brief Get the target path and playlist path of a single recorded audio file
		 * @param sr_path Stream recorder base path
		 * @param path_template template string of path
		 * @param md current MetaData object (used for album, title and artist)
		 * @param d session date
		 * @param t session time
		 * @return tuple of audio filepath and playlist filepath
		 */
        TargetPaths fullTargetPath(const QString& srPath, const QString& path_template, const MetaData& md, const QDate& d, const QTime& t);

        QString parseErrorCode(ErrorCode err);
    }
}

#endif // STREAMRECORDERUTILS_H
