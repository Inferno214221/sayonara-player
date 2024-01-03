/* Station.h
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#ifndef ABSTRACTUTILSTREAM_H
#define ABSTRACTUTILSTREAM_H

#include "Utils/Pimpl.h"

class QString;

namespace Cover
{
	class Location;
}

class Station
{
	public:
		Station();
		virtual ~Station();
		Station(const Station& other);

		Station& station(const Station& other);

		[[nodiscard]] virtual QString url() const = 0;
		[[nodiscard]] virtual QString name() const = 0;
};

class Stream :
	public Station
{
	PIMPL(Stream)

	public:
		Stream();
		Stream(const QString& name, const QString& url, bool isUpdatable = true);
		Stream(const Stream& other);
		~Stream() override;

		Stream& operator=(const Stream& stream);

		[[nodiscard]] QString name() const override;
		void setName(const QString& name);

		[[nodiscard]] QString url() const override;
		void setUrl(const QString& url);

		[[nodiscard]] bool isUpdatable() const;
};

class Podcast :
	public Station
{
	PIMPL(Podcast)

	public:
		Podcast();
		Podcast(const QString& name, const QString& url, bool reversed = false);
		Podcast(const Podcast& other);

		~Podcast() override;

		[[nodiscard]] QString name() const override;
		void setName(const QString& name);

		[[nodiscard]] QString url() const override;
		void setUrl(const QString& url);

		[[nodiscard]] bool reversed() const;
		void setReversed(bool b);

		Podcast& operator=(const Podcast& podcast);
};

using StationPtr = std::shared_ptr<Station>;

#endif // ABSTRACTUTILSTREAM_H
