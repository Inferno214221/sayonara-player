/* StreamParser.h */

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

	signals:
		void sigFinished(bool success);
		void sigStopped();
		void sigUrlCountExceeded(int urlCount, int maxUrlCount);

	public:
		explicit StreamParser(QObject* parent);
		~StreamParser() override;

		void parse(const QString& name, const QStringList& urls);
		void parse(const QString& name, const QStringList& urls, const QString& userAgent);
		virtual void parse(const QString& name, const QStringList& urls, const QString& userAgent, int timeout) = 0;
		virtual void stopParsing() = 0;
		[[nodiscard]] virtual MetaDataList tracks() const = 0;
		[[nodiscard]] virtual bool isStopped() const = 0;
		virtual void setCoverUrl(const QString& coverUrl) = 0;
};

class StationParserFactory
{
	public:
		virtual ~StationParserFactory() = default;
		[[nodiscard]] virtual StreamParser* createParser() const = 0;

		static std::shared_ptr<StationParserFactory>
		createStationParserFactory(const std::shared_ptr<WebClientFactory>& webClientFactory, QObject* parent);
};

using StationParserFactoryPtr = std::shared_ptr<StationParserFactory>;

#endif
