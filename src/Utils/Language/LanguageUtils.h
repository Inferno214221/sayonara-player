/* LanguageUtils.h */

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

#ifndef LANGUAGEUTILS_H
#define LANGUAGEUTILS_H

#include "Utils/Macros.h"

class QString;
class QStringList;
class QLocale;

template<typename A, typename B>
class QMap;

namespace Util::Language
{
	QString getSharePath(const QString& fourLetter);

	QString getHttpPath(const QString& fourLetter);

	QString getChecksumHttpPath();

	QString getHomeTargetPath(const QString& fourLetter);

	QString getUsedLanguageFile(const QString& fourLetter);

	QString getIconPath(const QString& fourLetter);

	QString extractLanguageCode(const QString& languageFile);

	QString getChecksum(const QString& fourLetter);

	void updateLanguageVersion(const QString& fourLetter);

	bool isOutdated(const QString& fourLetter);

	QString getSimilarLanguage4(const QString& fourLetter);

	QLocale getCurrentLocale();

	QStringList getCurrentQtTranslationPaths();

	bool importLanguageFile(const QString& filename);

	QString convertOldLanguage(const QString& languageCode);

	QMap<QString, QLocale> availableLanguages();

#ifdef SAYONARA_WITH_TESTS
	void setLanguageSettingFilename(const QString& filename);
	void setLanguageVersion(const QString& fourLetter, const QString& version);
#endif
}

#endif // LANGUAGEUTILS_H
