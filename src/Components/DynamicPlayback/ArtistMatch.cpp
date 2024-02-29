/* ArtistMatch.cpp */

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

#include "ArtistMatch.h"
#include "Utils/Algorithm.h"

#include <QMap>
#include <QStringList>
#include <QTextStream>

#include <cmath>
#include <utility>

namespace DynamicPlayback
{
	ArtistMatch::Entry::Entry(QString artist, QString mbid, const double similarity) :
		artist(std::move(artist)),
		mbid(std::move(mbid)),
		similarity(similarity) {}

	bool ArtistMatch::Entry::isValid() const
	{
		return !artist.isEmpty() &&
		       !mbid.isEmpty() &&
		       !(similarity < 0) &&
		       !(similarity > 1.0);
	}

	bool ArtistMatch::Entry::operator==(const ArtistMatch::Entry& other) const
	{
		return (artist == other.artist) &&
		       (mbid == other.mbid) &&
		       (std::abs(similarity - other.similarity) < 0.01); // NOLINT(*-magic-numbers)
	}

	struct ArtistMatch::Private
	{
		QList<ArtistMatch::Entry> entries;
		QString artist;

		Private() = default;

		explicit Private(QString artistName) :
			artist(std::move(artistName)) {}

		Private(const Private& other) = default;
		Private& operator=(const Private& other) = default;

		bool operator==(const Private& other) const
		{
			return (entries == other.entries);
		}
	};

	ArtistMatch::ArtistMatch() :
		m {Pimpl::make<Private>()} {}

	ArtistMatch::ArtistMatch(const QString& artistName) :
		m {Pimpl::make<Private>(artistName)} {}

	ArtistMatch::ArtistMatch(const ArtistMatch& other) :
		m {Pimpl::make<Private>(*(other.m))} {}

	ArtistMatch::~ArtistMatch() = default;

	bool ArtistMatch::isValid() const
	{
		return !m->entries.isEmpty();
	}

	bool ArtistMatch::operator==(const ArtistMatch& artistMatch) const
	{
		return (*m == *(artistMatch.m));
	}

	ArtistMatch& ArtistMatch::operator=(const ArtistMatch& other)
	{
		*m = *(other.m);

		return *this;
	}

	void ArtistMatch::add(const Entry& entry)
	{
		m->entries.push_back(entry);

		Util::Algorithm::sort(m->entries, [](const auto& entry1, const auto& entry2) {
			return (entry1.similarity > entry2.similarity);
		});
	}

// ------------------------------------------------------------------------------------------------------
// EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
//                         VVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
//                                        GGGGGGGGGGGGGGGGGGGGGGGGGGGGG
//                                                       PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP
	QList<ArtistMatch::Entry> ArtistMatch::get(Quality quality) const
	{
		auto result = QList<ArtistMatch::Entry>();

		for(const auto& entry: m->entries)
		{
			switch(quality)
			{
				case Quality::Poor:
					if(entry.similarity < 0.40) // NOLINT(*-magic-numbers)
					{
						result.push_back(entry);
					}
					break;
				case Quality::Good:
					if((entry.similarity > 0.20) && (entry.similarity < 0.60)) // NOLINT(*-magic-numbers)
					{
						result.push_back(entry);
					}
					break;
				case Quality::VeryGood:
					if((entry.similarity > 0.40) && (entry.similarity < 0.75)) // NOLINT(*-magic-numbers)
					{
						result.push_back(entry);
					}
					break;
				case Quality::Excellent:
					if(entry.similarity > 0.60) // NOLINT(*-magic-numbers)
					{
						result.push_back(entry);
					}
					break;
			}
		}

		return result;
	}

	QString ArtistMatch::artistName() const { return m->artist; }

	QString ArtistMatch::toString() const
	{
		auto lines = QStringList {};
		for(const auto& entry: m->entries)
		{
			auto line = QString {};
			auto textStream = QTextStream(&line);
			textStream.setRealNumberNotation(QTextStream::FixedNotation);
			textStream << entry.similarity << '\t' << entry.artist << '\t' << entry.mbid;

			lines << line;
		}

		return lines.join('\n');
	}

	ArtistMatch ArtistMatch::fromString(const QString& data)
	{
		auto artistMatch = ArtistMatch {};

		const auto lines = data.split('\n');
		for(const auto& line: lines)
		{
			auto splitted = line.split('\t');
			if(splitted.size() != 3)
			{
				continue;
			}

			// QTextStream cannot split by tab :(
			const auto similarity = splitted[0].toDouble();
			const auto artist = splitted[1];
			const auto mbid = splitted[2];

			const auto entry = ArtistMatch::Entry(artist, mbid, similarity);
			artistMatch.add(entry);
		}

		return artistMatch;
	}
}