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
#include "Utils/Logger/Logger.h"
#include "Utils/Utils.h"

#include <QRegExp>
#include <QMap>
#include <QList>
#include <QString>

using DiacMap=QMap<QString, QString>;

Q_GLOBAL_STATIC(DiacMap, diacritic_chars)

static void init_diacritic_chars()
{
	diacritic_chars->insert(QString::fromUtf8("Š"), "S");
	diacritic_chars->insert(QString::fromUtf8("Œ"), "OE");
	diacritic_chars->insert(QString::fromUtf8("Ž"), "Z");
	diacritic_chars->insert(QString::fromUtf8("š"), "s");
	diacritic_chars->insert(QString::fromUtf8("œ"), "oe");
	diacritic_chars->insert(QString::fromUtf8("ž"), "z");
	diacritic_chars->insert(QString::fromUtf8("Ÿ"), "Y");
	diacritic_chars->insert(QString::fromUtf8("¥"), "Y");
	diacritic_chars->insert(QString::fromUtf8("µ"), "u");
	diacritic_chars->insert(QString::fromUtf8("À"), "A");
	diacritic_chars->insert(QString::fromUtf8("Á"), "A");
	diacritic_chars->insert(QString::fromUtf8("Â"), "A");
	diacritic_chars->insert(QString::fromUtf8("Ä"), "A");
	diacritic_chars->insert(QString::fromUtf8("Å"), "A");
	diacritic_chars->insert(QString::fromUtf8("Æ"), "AE");
	diacritic_chars->insert(QString::fromUtf8("Ç"), "C");
	diacritic_chars->insert(QString::fromUtf8("È"), "E");
	diacritic_chars->insert(QString::fromUtf8("É"), "E");
	diacritic_chars->insert(QString::fromUtf8("Ê"), "E ");
	diacritic_chars->insert(QString::fromUtf8("Ë"), "E");
	diacritic_chars->insert(QString::fromUtf8("Ì"), "I");
	diacritic_chars->insert(QString::fromUtf8("Í"), "I");
	diacritic_chars->insert(QString::fromUtf8("Î"), "I");
	diacritic_chars->insert(QString::fromUtf8("Ï"), "I");
	diacritic_chars->insert(QString::fromUtf8("Ð"), "D");
	diacritic_chars->insert(QString::fromUtf8("Ñ"), "N");
	diacritic_chars->insert(QString::fromUtf8("Ò"), "O");
	diacritic_chars->insert(QString::fromUtf8("Ó"), "O");
	diacritic_chars->insert(QString::fromUtf8("Ô"), "O");
	diacritic_chars->insert(QString::fromUtf8("Õ"), "O");
	diacritic_chars->insert(QString::fromUtf8("Ö"), "O");
	diacritic_chars->insert(QString::fromUtf8("Ø"), "O");
	diacritic_chars->insert(QString::fromUtf8("Ù"), "U");
	diacritic_chars->insert(QString::fromUtf8("Ú"), "U");
	diacritic_chars->insert(QString::fromUtf8("Û"), "U");
	diacritic_chars->insert(QString::fromUtf8("Ü"), "U");
	diacritic_chars->insert(QString::fromUtf8("Ý"), "Y");
	diacritic_chars->insert(QString::fromUtf8("ß"), "ss");
	diacritic_chars->insert(QString::fromUtf8("à"), "a");
	diacritic_chars->insert(QString::fromUtf8("á"), "a");
	diacritic_chars->insert(QString::fromUtf8("â"), "a");
	diacritic_chars->insert(QString::fromUtf8("ã"), "a");
	diacritic_chars->insert(QString::fromUtf8("ä"), "a");
	diacritic_chars->insert(QString::fromUtf8("å"), "a");
	diacritic_chars->insert(QString::fromUtf8("æ"), "ae");
	diacritic_chars->insert(QString::fromUtf8("ç"), "c");
	diacritic_chars->insert(QString::fromUtf8("è"), "e");
	diacritic_chars->insert(QString::fromUtf8("é"), "e");
	diacritic_chars->insert(QString::fromUtf8("ê"), "e");
	diacritic_chars->insert(QString::fromUtf8("ë"), "e");
	diacritic_chars->insert(QString::fromUtf8("ì"), "i");
	diacritic_chars->insert(QString::fromUtf8("í"), "i");
	diacritic_chars->insert(QString::fromUtf8("î"), "i");
	diacritic_chars->insert(QString::fromUtf8("ï"), "i");
	diacritic_chars->insert(QString::fromUtf8("ð"), "o");
	diacritic_chars->insert(QString::fromUtf8("ñ"), "n");
	diacritic_chars->insert(QString::fromUtf8("ò"), "o");
	diacritic_chars->insert(QString::fromUtf8("ó"), "o");
	diacritic_chars->insert(QString::fromUtf8("ô"), "o");
	diacritic_chars->insert(QString::fromUtf8("õ"), "o");
	diacritic_chars->insert(QString::fromUtf8("ö"), "o");
	diacritic_chars->insert(QString::fromUtf8("ø"), "o");
	diacritic_chars->insert(QString::fromUtf8("ù"), "u");
	diacritic_chars->insert(QString::fromUtf8("ú"), "u");
	diacritic_chars->insert(QString::fromUtf8("û"), "u");
	diacritic_chars->insert(QString::fromUtf8("ü"), "u");
	diacritic_chars->insert(QString::fromUtf8("ý"), "y");
	diacritic_chars->insert(QString::fromUtf8("ÿ"), "y");
}

QString Library::Utils::convertSearchstring(const QString& originalString, Library::SearchModeMask mode, const QList<QChar>& ignoredChars)
{
	if(diacritic_chars->isEmpty()){
		init_diacritic_chars();
	}

	QString convertedString(originalString);
	if(mode & Library::CaseInsensitve)
	{
		convertedString = originalString.toLower();
	}

	if(mode & Library::NoSpecialChars)
	{
		QString convertedStringWithoutSpecialChars(convertedString);

		for(QChar c : convertedStringWithoutSpecialChars)
		{
			if(ignoredChars.contains(c)){
				continue;
			}

			if(!c.isLetterOrNumber()) {
				convertedStringWithoutSpecialChars.remove(c);
			}
		}

		convertedString = convertedStringWithoutSpecialChars;
	}

	if(mode & Library::NoDiacriticChars)
	{
		QString cleanedString;

		for (int i = 0; i < convertedString.length(); i++)
		{
			const QString c = QString(convertedString[i]);

			if(diacritic_chars->contains(c))
			{
				QString replacement = diacritic_chars->value(c);
				if(mode & Library::CaseInsensitve) {
					cleanedString.append(replacement);
				}

				else {
					cleanedString.append(replacement.toLower());
				}
			}

			else
			{
				cleanedString.append(c);
			}
		}

		convertedString = cleanedString;
	}

	return ::Util::convertNotNull(convertedString);
}


QString Library::Utils::convertSearchstring(const QString& str, Library::SearchModeMask mode)
{
	return convertSearchstring(str, mode, QList<QChar>());
}
