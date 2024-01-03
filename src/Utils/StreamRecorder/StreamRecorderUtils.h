/* StreamRecorderUtils.h */

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

#ifndef STREAMRECORDERUTILS_H
#define STREAMRECORDERUTILS_H

#include <QList>
#include <QPair>
#include <QString>

#include <cstdint>

class MetaData;
class QStringList;
class QDate;
class QTime;

namespace StreamRecorder::Utils
{
	enum class ErrorCode :
		std::uint8_t
	{
		OK = 1,
		BracketError,
		UnknownTag,
		MissingUniqueTag,
		InvalidChars,
		Empty
	};

	struct TargetPath
	{
		QString filename;
		QString playlistName;
	};

	QStringList supportedTags();
	QList<QPair<QString, QString>> descriptions();

	ErrorCode validateTemplate(const QString& targetPathTemplate, int* invalidIndex);

	QString targetPathTemplateDefault(bool useSessionPath);

	/**
	 * @brief Get the target path and playlist path of a single recorded audio file
	 * @return tuple of audio filepath and playlist filepath
	 */
	TargetPath
	fullTargetPath(const QString& streamRecorderPath, const QString& pathTemplate, const MetaData& track,
	               const QDate& d, const QTime& t);

	QString parseErrorCode(ErrorCode err);
}

#endif // STREAMRECORDERUTILS_H
