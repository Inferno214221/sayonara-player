/* TagExpression.cpp */

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

#include "Expression.h"

#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaData.h"

#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QMap>

#include <cassert>

namespace Algorithm = Util::Algorithm;
using namespace Tagging;

namespace
{
	constexpr const auto EscapeStrings =
		{
			'\\', '?', '+', '*', '[', ']', '(', ')', '{', '}', '.'
		};

	QMap<TagName, TagString> tagNameMap()
	{
		return QMap<TagName, TagString>
			{{TagNone, QString()},
			 {TagTitle,    QStringLiteral("<title>")},
			 {TagAlbum,    QStringLiteral("<album>")},
			 {TagArtist,   QStringLiteral("<artist>")},
			 {TagTrackNum, QStringLiteral("<tracknum>")},
			 {TagYear,     QStringLiteral("<year>")},
			 {TagDisc,     QStringLiteral("<disc>")},
			 {TagIgnore,   QStringLiteral("<ignore>")}};
	}

	QMap<TagName, QString> getTagRegexMap()
	{
		return QMap<TagName, TagString>
			{{TagTitle,    QStringLiteral(".+")},
			 {TagAlbum,    QStringLiteral(".+")},
			 {TagArtist,   QStringLiteral(".+")},
			 {TagTrackNum, QStringLiteral("\\d+")},
			 {TagYear,     QStringLiteral("\\d{4}")},
			 {TagDisc,     QStringLiteral("\\d{1,3}")},
			 {TagIgnore,   QStringLiteral(".+")}};
	}

	TagName tagStringToName(const TagString& tagString)
	{
		const auto map = tagNameMap();
		return map.key(tagString, TagNone);
	}

	QString escapeSpecialChars(const QString& originalString)
	{
		auto stringCopy = originalString;
		for(const auto& c : EscapeStrings)
		{
			stringCopy.replace(c, QString("\\") + c);
		}

		return stringCopy;
	}

	QString createRegexChunk(const QString& splittedChunk, const QMap<TagName, QString>& tagRegexMap)
	{
		if(splittedChunk.isEmpty())
		{
			return QString();
		}

		const auto tagName = tagStringToName(splittedChunk);
		return (tagRegexMap.contains(tagName))
		       ? QString("(%1)").arg(tagRegexMap[tagName])
		       : QString("(%1)").arg(splittedChunk);
	}

	QString calcRegexString(const QStringList& splittedString, const QMap<TagName, QString>& tagRegexMap)
	{
		QString regex;

		for(const auto& splitted : splittedString)
		{
			regex += createRegexChunk(splitted, tagRegexMap);
		}

		return regex;
	}

	QStringList getTagNames()
	{
		QStringList names;
		const auto map = tagNameMap();
		for(const auto& value : map)
		{
			names << value;
		}

		return names;
	}

	QMap<int, TagString> getTagPositions(const QString& expression)
	{
		auto indexStringMap = QMap<int, TagString>();
		const auto tagNames = getTagNames();

		for(const auto& tagName : tagNames)
		{
			if(!tagName.isEmpty())
			{
				if(const auto index = expression.indexOf(tagName); index >= 0)
				{
					indexStringMap.insert(index, tagName);
				}
			}
		}

		return indexStringMap;
	}

	QStringList splitTagString(const QString& lineEditString)
	{
		const auto lineEditEscaped = escapeSpecialChars(lineEditString);
		const auto tagPositions = getTagPositions(lineEditEscaped);

		auto currentIndex = 0;
		auto splittedTagString = QStringList();
		for(auto it = tagPositions.cbegin(); it != tagPositions.cend(); it++)
		{
			const auto index = it.key();
			const auto tagString = it.value();
			const auto length = index - currentIndex;

			const auto stringUntilTag = lineEditEscaped.mid(currentIndex, length);
			if(!stringUntilTag.isEmpty())
			{
				splittedTagString << stringUntilTag;
				currentIndex += stringUntilTag.size();
			}

			assert(!tagString.isEmpty());

			splittedTagString << tagString;
			currentIndex += tagString.size();
		}

		const auto restOfLine = lineEditEscaped.right(lineEditEscaped.length() - currentIndex);
		if(!restOfLine.isEmpty())
		{
			splittedTagString << restOfLine;
		}

		return splittedTagString;
	}

	void logRegexError(const QString& regexString, int tagCount, const QStringList& capturedTexts,
	                   Expression* expression)
	{
		spLog(Log::Debug, expression) << "Regex: " << regexString << ": " << capturedTexts.count()
		                              << " tags found, but requested "
		                              << tagCount;

		for(const auto& s : capturedTexts)
		{
			spLog(Log::Debug, expression) << "Captured texts:  " << s;
		}

		spLog(Log::Debug, expression) << "";
	}
}

struct Expression::Private
{
	QMap<TagName, ReplacedString> capturedTags;
	QMap<TagName, QString> tagRegexMap {getTagRegexMap()};

	bool valid {false};
};

Expression::Expression(const QString& tagString, const QString& filepath)
{
	m = Pimpl::make<Private>();
	m->valid = updateTag(tagString, filepath);
}

Expression::~Expression() = default;

QMap<TagName, QString> Expression::capturedTags() const
{
	return m->capturedTags;
}

bool Expression::isValid() const
{
	return m->valid;
}

bool Expression::apply(MetaData& track) const
{
	auto success = false;

	const auto capturedTags = this->capturedTags();
	for(auto it = capturedTags.begin(); it != capturedTags.end(); it++)
	{
		const auto tagName = it.key();
		const auto value = it.value();

		if(tagName == Tagging::TagTitle)
		{
			success |= (value != track.title());
			track.setTitle(value);
		}

		else if(tagName == Tagging::TagAlbum)
		{
			success |= (value != track.album());
			track.setAlbum(value);
		}

		else if(tagName == Tagging::TagArtist)
		{
			success |= (value != track.artist());
			track.setArtist(value);
		}

		else if(tagName == Tagging::TagTrackNum)
		{
			const auto trackNumber = static_cast<TrackNum>(value.toInt());
			success |= (trackNumber != track.trackNumber());
			track.setTrackNumber(trackNumber);
		}

		else if(tagName == Tagging::TagYear)
		{
			const auto year = static_cast<Year>(value.toInt());
			success |= (year != track.year());
			track.setYear(year);
		}

		else if(tagName == Tagging::TagDisc)
		{
			const auto disc = static_cast<Disc>(value.toInt());
			success |= (disc != track.discnumber());
			track.setDiscnumber(disc);
		}
	}

	return success;
}

bool Expression::updateTag(const QString& lineEditString, const QString& filepath)
{
	m->capturedTags.clear();

	const auto splittedTagString = splitTagString(lineEditString);
	const auto regexString = calcRegexString(splittedTagString, m->tagRegexMap);

	// save content of all entered tags and the rest into captured texts
	auto re = QRegExp(regexString);
	re.indexIn(filepath);

	auto capturedTexts = re.capturedTexts();
	capturedTexts.removeAt(0);
	capturedTexts.removeAll("");

	const auto captureCount = capturedTexts.size();
	const auto regexSplitCount = splittedTagString.size();

	if((captureCount != regexSplitCount) || (captureCount == 0) || (regexSplitCount == 0))
	{
		logRegexError(regexString, regexSplitCount, capturedTexts, this);
		return false;
	}

	for(auto i = 0; i < captureCount; i++)
	{
		const auto splitted = splittedTagString[i];

		const auto tagName = tagStringToName(splitted);
		if(tagName != TagName::TagNone)
		{
			m->capturedTags[tagName] = capturedTexts[i];
		}
	}

	return true;
}

QString Tagging::tagNameToString(TagName tagName)
{
	return tagNameMap()[tagName];
}
