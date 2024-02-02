/* LyricWebpageParser.cpp */

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

#include "LyricWebpageParser.h"
#include "LyricServer.h"

#include <QByteArray>
#include <QRegExp>
#include <QString>
#include <QTextBlock>
#include <QTextDocument>

using namespace Lyrics;

namespace
{
	QString convertTagToRegex(const QString& tag, const QMap<QString, QString>& regexConversions,
	                          bool closeBeginningAngledBracket)
	{
		auto result = tag;

		const auto keys = regexConversions.keys();
		for(const auto& key: keys)
		{
			result.replace(key, regexConversions.value(key));
		}

		result.replace(" ", R"([\s\n\r]+)");

		if(closeBeginningAngledBracket && result.startsWith("<") && !result.endsWith(">"))
		{
			result.append(".*>");
		}

		return result;
	}

	QString parseNumericContent(const QString& data)
	{
		const auto regExp = QRegExp("&#(\\d+);|<br />|</span>|</p>");

		QString word;
		QStringList words;

		auto pos = 0;
		while((pos = regExp.indexIn(data, pos)) != -1)
		{
			const auto caption = regExp.cap(1);

			pos += regExp.matchedLength();
			if(caption.isEmpty())
			{
				words << word;
				word.clear();
			}

			else
			{
				word.append(QChar(caption.toInt()));
			}
		}

		return words.join("<br>");
	}

	QString formatLineFeeds(QString data)
	{
		const auto newLines = std::array {
			"\\r\\n", "\\n", "\r\n", "\n", "<br/>", "<br />"
		};

		for(const auto& c: newLines)
		{
			while(data.contains(c))
			{
				data.replace(c, "<br>");
			}
		}

		data.replace("\\\"", "\"");

		const auto doubleLineFeed = QRegExp("<br>\\s*<br>\\s*<br>");
		while(data.contains(doubleLineFeed))
		{
			data.replace(doubleLineFeed, "<br><br>");
		}

		return data;
	}

	QString removeScript(QString data)
	{
		QRegExp reScript;
		reScript.setPattern("<script.+</script>");
		reScript.setMinimal(true);
		while(reScript.indexIn(data) != -1)
		{
			data.replace(reScript, "");
		}

		return data;
	}

	QString removeLinks(QString data)
	{
		{
			auto startIndexOpening = data.indexOf("<a");
			while(startIndexOpening >= 0)
			{
				const auto endIndex = data.indexOf(">", startIndexOpening);
				const auto length = endIndex - startIndexOpening + 1;

				data.remove(startIndexOpening, length);
				startIndexOpening = data.indexOf("<a", startIndexOpening);
			}
		}

		{
			auto startIndexClosing = data.indexOf("</a>");
			while(startIndexClosing >= 0)
			{
				data.remove(startIndexClosing, 4);
				startIndexClosing = data.indexOf("</a>", startIndexClosing);
			}
		}

		return data;
	}

	QString preProcessContent(QString data)
	{
		return removeScript(std::move(data));
	}

	QString postProcessResult(QString data)
	{
		auto doc = QTextDocument {};
		doc.setHtml(data);
		data = doc.toPlainText();

		data = formatLineFeeds(std::move(data));
		data = removeLinks(std::move(data));

		return data;
	}

	QString extractContentFromWebpage(const QString& startTagRegex, const QString& endTagRegex, const QString& data)
	{
		QRegExp regex(startTagRegex + "(.+)" + endTagRegex);
		regex.setMinimal(true);

		return (regex.indexIn(data) != -1)
		       ? regex.cap(1).trimmed()
		       : QString();
	}
}

QString
WebpageParser::parseWebpage(const QByteArray& rawData, const QMap<QString, QString>& regexConversions, Server* server)
{
	const auto website = QString::fromUtf8(rawData);

	const auto startEndTags = server->startEndTag();
	for(const auto& startEndTag: startEndTags)
	{
		const auto startTag = convertTagToRegex(startEndTag.first, regexConversions, true);
		const auto endTag = convertTagToRegex(startEndTag.second, regexConversions, false);

		auto content = extractContentFromWebpage(startTag, endTag, website);
		if(content.isEmpty())
		{
			continue;
		}

		content = preProcessContent(std::move(content));

		auto result = (server->isNumeric())
		              ? parseNumericContent(content)
		              : std::move(content);

		result = postProcessResult(std::move(result));

		if(result.size() > 100)
		{
			return result.trimmed();
		}
	}

	return {};
}
