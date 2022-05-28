/* StreamParser.h */

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

#ifndef STREAMPARSER_H
#define STREAMPARSER_H

#include "Utils/Pimpl.h"
#include <QObject>

class WebClientFactory;
class IcyWebAccess;
class StreamParser :
	public QObject
{
	Q_OBJECT
	PIMPL(StreamParser)

	signals:
		void sigFinished(bool success);
		void sigStopped();
		void sigUrlCountExceeded(int urlCount, int maxUrlCount);

	public:
		explicit StreamParser(const std::shared_ptr<WebClientFactory>& webClientFactory, QObject* parent = nullptr);
		~StreamParser() override;

		// NOLINTNEXTLINE(readability-magic-numbers)
		void parse(const QString& stationName, const QString& stationUrl, int timeout = 5000);
		void parse(const QStringList& urls, int timeout = 5000); // NOLINT(readability-magic-numbers)

		void setCoverUrl(const QString& coverUrl);

		void stop();
		[[nodiscard]] bool isStopped() const;

		[[nodiscard]] MetaDataList tracks() const;

	private slots:
		void webClientFinished();

	private: // NOLINT(readability-redundant-access-specifiers)
		bool parseNextUrl();
		void icyFinished(const QString& url, IcyWebAccess* icyWebAccess);

};

#endif
