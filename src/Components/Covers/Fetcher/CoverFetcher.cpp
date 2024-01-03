/* CoverFetcherInterface.cpp */

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

#include "CoverFetcher.h"

#include <QString>
#include <QRegExp>
#include <QUrl>

namespace
{
	QString convertStringToSearchString(const QString& str)
	{
		QString result;
		QChar previouseChar;
		for(const auto& c: str)
		{
			if(c.isLetterOrNumber())
			{
				result.append(c);
				previouseChar = c;
			}

			else if(previouseChar != ' ')
			{
				result.append(' ');
			}
		}

		return result.trimmed();
	}

	QString convertUrlToSearchString(const QString& url)
	{
		const auto host = QUrl(url).host();
		auto ipRegex = QRegExp("[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+");
		return (ipRegex.indexIn(host) >= 0)
		       ? QString()
		       : convertStringToSearchString(host);
	}
}

Cover::Fetcher::Base::Base() = default;

Cover::Fetcher::Base::~Base() = default;

QString Cover::Fetcher::Base::identifier() const
{
	return this->privateIdentifier().toLower();
}

QString Cover::Fetcher::Base::artistAddress([[maybe_unused]] const QString& artist) const
{
	return QString();
}

QString
Cover::Fetcher::Base::albumAddress([[maybe_unused]] const QString& artist, [[maybe_unused]] const QString& album) const
{
	return QString();
}

QString Cover::Fetcher::Base::fulltextSearchAddress([[maybe_unused]] const QString& str) const
{
	return QString();
}

QString Cover::Fetcher::Base::radioSearchAddress([[maybe_unused]] const QString& stationName,
                                                 [[maybe_unused]] const QString& stationUrl) const
{
	return QString();
}

bool Cover::Fetcher::Base::isWebserviceFetcher() const
{
	return true;
}

QString Cover::Fetcher::Base::searchStringFromRadioStation(const QString& stationName, const QString& stationUrl) const
{
	const auto convertedUrl = convertUrlToSearchString(stationUrl);
	const auto convertedStation = convertStringToSearchString(stationName);
	const auto searchString = QString("%1 %2").arg(stationName).arg(convertedUrl);

	return searchString.trimmed();
}
