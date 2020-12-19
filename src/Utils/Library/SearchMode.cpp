/* SearchMode.cpp */

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

#include "SearchMode.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"

#include <QCache>
#include <QMap>
#include <QList>
#include <QString>

#include <array>

using DiacMap = QMap<QString, QString>;

using ConvertCacheEntry = QCache<QString, QString>;
using ConvertCache = std::array<ConvertCacheEntry, +Library::SearchModeMaskSize>;

Q_GLOBAL_STATIC(DiacMap, diacriticChars)
Q_GLOBAL_STATIC(ConvertCache, convertCache)

namespace
{
	void initDiacriticChars()
	{
		diacriticChars->insert(QString::fromUtf8("Š"), "S");
		diacriticChars->insert(QString::fromUtf8("Œ"), "OE");
		diacriticChars->insert(QString::fromUtf8("Ž"), "Z");
		diacriticChars->insert(QString::fromUtf8("š"), "s");
		diacriticChars->insert(QString::fromUtf8("œ"), "oe");
		diacriticChars->insert(QString::fromUtf8("ž"), "z");
		diacriticChars->insert(QString::fromUtf8("Ÿ"), "Y");
		diacriticChars->insert(QString::fromUtf8("¥"), "Y");
		diacriticChars->insert(QString::fromUtf8("µ"), "u");
		diacriticChars->insert(QString::fromUtf8("À"), "A");
		diacriticChars->insert(QString::fromUtf8("Á"), "A");
		diacriticChars->insert(QString::fromUtf8("Â"), "A");
		diacriticChars->insert(QString::fromUtf8("Ä"), "A");
		diacriticChars->insert(QString::fromUtf8("Å"), "A");
		diacriticChars->insert(QString::fromUtf8("Æ"), "AE");
		diacriticChars->insert(QString::fromUtf8("Ç"), "C");
		diacriticChars->insert(QString::fromUtf8("È"), "E");
		diacriticChars->insert(QString::fromUtf8("É"), "E");
		diacriticChars->insert(QString::fromUtf8("Ê"), "E");
		diacriticChars->insert(QString::fromUtf8("Ë"), "E");
		diacriticChars->insert(QString::fromUtf8("Ì"), "I");
		diacriticChars->insert(QString::fromUtf8("Í"), "I");
		diacriticChars->insert(QString::fromUtf8("Î"), "I");
		diacriticChars->insert(QString::fromUtf8("Ï"), "I");
		diacriticChars->insert(QString::fromUtf8("Ð"), "D");
		diacriticChars->insert(QString::fromUtf8("Ñ"), "N");
		diacriticChars->insert(QString::fromUtf8("Ò"), "O");
		diacriticChars->insert(QString::fromUtf8("Ó"), "O");
		diacriticChars->insert(QString::fromUtf8("Ô"), "O");
		diacriticChars->insert(QString::fromUtf8("Õ"), "O");
		diacriticChars->insert(QString::fromUtf8("Ö"), "O");
		diacriticChars->insert(QString::fromUtf8("Ø"), "O");
		diacriticChars->insert(QString::fromUtf8("Ù"), "U");
		diacriticChars->insert(QString::fromUtf8("Ú"), "U");
		diacriticChars->insert(QString::fromUtf8("Û"), "U");
		diacriticChars->insert(QString::fromUtf8("Ü"), "U");
		diacriticChars->insert(QString::fromUtf8("Ý"), "Y");
		diacriticChars->insert(QString::fromUtf8("ß"), "ss");
		diacriticChars->insert(QString::fromUtf8("à"), "a");
		diacriticChars->insert(QString::fromUtf8("á"), "a");
		diacriticChars->insert(QString::fromUtf8("â"), "a");
		diacriticChars->insert(QString::fromUtf8("ã"), "a");
		diacriticChars->insert(QString::fromUtf8("ä"), "a");
		diacriticChars->insert(QString::fromUtf8("å"), "a");
		diacriticChars->insert(QString::fromUtf8("æ"), "ae");
		diacriticChars->insert(QString::fromUtf8("ç"), "c");
		diacriticChars->insert(QString::fromUtf8("è"), "e");
		diacriticChars->insert(QString::fromUtf8("é"), "e");
		diacriticChars->insert(QString::fromUtf8("ê"), "e");
		diacriticChars->insert(QString::fromUtf8("ë"), "e");
		diacriticChars->insert(QString::fromUtf8("ì"), "i");
		diacriticChars->insert(QString::fromUtf8("í"), "i");
		diacriticChars->insert(QString::fromUtf8("î"), "i");
		diacriticChars->insert(QString::fromUtf8("ï"), "i");
		diacriticChars->insert(QString::fromUtf8("ð"), "o");
		diacriticChars->insert(QString::fromUtf8("ñ"), "n");
		diacriticChars->insert(QString::fromUtf8("ò"), "o");
		diacriticChars->insert(QString::fromUtf8("ó"), "o");
		diacriticChars->insert(QString::fromUtf8("ô"), "o");
		diacriticChars->insert(QString::fromUtf8("õ"), "o");
		diacriticChars->insert(QString::fromUtf8("ö"), "o");
		diacriticChars->insert(QString::fromUtf8("ø"), "o");
		diacriticChars->insert(QString::fromUtf8("ù"), "u");
		diacriticChars->insert(QString::fromUtf8("ú"), "u");
		diacriticChars->insert(QString::fromUtf8("û"), "u");
		diacriticChars->insert(QString::fromUtf8("ü"), "u");
		diacriticChars->insert(QString::fromUtf8("ý"), "y");
		diacriticChars->insert(QString::fromUtf8("ÿ"), "y");
	}
}

QString Library::Utils::convertSearchstring(const QString& originalString, Library::SearchModeMask mode,
                                            const QList<QChar>& ignoredChars)
{
	if(mode == Library::SearchMode::None)
	{
		return originalString;
	}

	if(originalString.isEmpty())
	{
		return ::Util::convertNotNull(originalString);
	}

	auto& qCache = convertCache->at(+mode);
	if(qCache.contains(originalString))
	{
		return *(qCache[originalString]);
	}

	if(diacriticChars->isEmpty())
	{
		initDiacriticChars();
	}

	auto convertedString = originalString;
	if(mode & Library::CaseInsensitve)
	{
		convertedString = originalString.toLower();
	}

	if(mode & Library::NoSpecialChars)
	{
		auto withoutSpecialChars = QString {};
		for(const auto& c : convertedString)
		{
			if(ignoredChars.contains(c) || c.isLetterOrNumber())
			{
				withoutSpecialChars.append(c);
			}
		}

		convertedString = withoutSpecialChars;
	}

	if(mode & Library::NoDiacriticChars)
	{
		auto withoutDiacriticChars = QString {};

		for(const auto& c : convertedString)
		{
			const auto seq = QString(c);
			if(diacriticChars->contains(seq))
			{
				const auto& replacement = diacriticChars->value(seq);
				if(mode & Library::CaseInsensitve)
				{
					withoutDiacriticChars.append(replacement.toLower());
				}

				else
				{
					withoutDiacriticChars.append(replacement);
				}
			}

			else
			{
				withoutDiacriticChars.append(c);
			}
		}

		convertedString = withoutDiacriticChars;
	}

	const auto result = ::Util::convertNotNull(convertedString).trimmed();

	if(qCache.isEmpty()){
		qCache.setMaxCost(10000);
	}

	qCache.insert(originalString, new QString(result));

	return result;
}

QString Library::Utils::convertSearchstring(const QString& str, Library::SearchModeMask mode)
{
	return convertSearchstring(str, mode, QList<QChar>());
}

QString Library::Utils::convertSearchstring(const QString& str)
{
	return convertSearchstring(str, GetSetting(Set::Lib_SearchMode));
}
