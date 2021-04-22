/* LyricWebpageParser.cpp */

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

#include "LyricWebpageParser.h"
#include "LyricServer.h"

#include <QString>
#include <QRegExp>
#include <QByteArray>

using namespace Lyrics;

namespace
{
	QString convertTagToRegex(const QString& tag, const QMap<QString, QString>& regexConversions,
	                          bool closeBeginningAngledBracket)
	{
		auto result = tag;

		const auto keys = regexConversions.keys();
		for(const auto& key : keys)
		{
			result.replace(key, regexConversions.value(key));
		}

		result.replace(" ", "\\s+");

		if(closeBeginningAngledBracket && result.startsWith("<") && !result.endsWith(">"))
		{
			result.append(".*>");
		}

		return result;
	}

	QString parseNumericContent(const QString& content)
	{
		const auto regExp = QRegExp("&#(\\d+);|<br />|</span>|</p>");

		QString word;
		QStringList words;

		auto pos = 0;
		while((pos = regExp.indexIn(content, pos)) != -1)
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

	void removePreformatTag(QString& data)
	{
		auto regExp = QRegExp("<p\\s.*>");
		regExp.setMinimal(true);
		data.remove(regExp);
		data.remove(QRegExp("</p>"));
	}

	void removeComments(QString& data)
	{
		auto regExp = QRegExp("<!--.*-->");
		regExp.setMinimal(true);
		data.remove(regExp);
	}

	void formatLineFeeds(QString& data)
	{
		data.replace("<br>\n", "<br>");
		data.replace(QRegExp("<br\\s*/>\n"), "<br>");
		data.replace("\r\n", "<br>");
		data.replace("\\r\\n", "<br>");
		data.replace("\n", "<br>");
		data.replace("\\n", "<br>");
		data.replace(QRegExp("<br\\s*/>"), "<br>");
		data.replace("\\\"", "\"");
	}

	void removeLineFeeds(QString& data)
	{
		formatLineFeeds(data);

		const auto lineBreaks = QStringLiteral("<br>\\s*<br>\\s*<br>");

		auto regExp = QRegExp(lineBreaks);
		while(data.contains(regExp))
		{
			data.replace(regExp, "<br><br>");
		}

		while(data.startsWith("<br>"))
		{
			data = data.right(data.count() - 4);
		}
	}

	void removeScript(QString& data)
	{
		QRegExp reScript;
		reScript.setPattern("<script.+</script>");
		reScript.setMinimal(true);
		while(reScript.indexIn(data) != -1)
		{
			data.replace(reScript, "");
		}
	}

	void removeLinks(QString& result)
	{
		{
			auto startIndexOpening = result.indexOf("<a");
			while(startIndexOpening >= 0)
			{
				const auto endIndex = result.indexOf(">", startIndexOpening);
				const auto length = endIndex - startIndexOpening + 1;

				result.remove(startIndexOpening, length);
				startIndexOpening = result.indexOf("<a", startIndexOpening);
			}
		}

		{
			auto startIndexClosing = result.indexOf("</a>");
			while(startIndexClosing >= 0)
			{
				result.remove(startIndexClosing, 4);
				startIndexClosing = result.indexOf("</a>", startIndexClosing);
			}
		}
	}

	void preProcessContent(QString& result)
	{
		removeScript(result);
	}

	void postProcessResult(QString& result)
	{
		removePreformatTag(result);
		removeComments(result);
		removeLineFeeds(result);
		removeLinks(result);
	}

	QString extractContentFromWebpage(const QString& startTagRegex, const QString& endTagRegex, const QString& website)
	{
		QRegExp regex(startTagRegex + "(.+)" + endTagRegex);
		regex.setMinimal(true);

		return (regex.indexIn(website) != -1)
		       ? regex.cap(1)
		       : QString();
	}
}

QString
WebpageParser::parseWebpage(const QByteArray& rawData, const QMap<QString, QString>& regexConversions, Server* server)
{
	const auto website = QString::fromUtf8(rawData);

	const auto startEndTags = server->startEndTag();
	for(const auto& startEndTag : startEndTags)
	{
		const auto startTag = convertTagToRegex(startEndTag.first, regexConversions, true);
		const auto endTag = convertTagToRegex(startEndTag.second, regexConversions, false);

		auto content = extractContentFromWebpage(startTag, endTag, website);
		if(content.isEmpty())
		{
			continue;
		}

		preProcessContent(content);

		auto result = (server->isNumeric())
		              ? parseNumericContent(content)
		              : std::move(content);

		postProcessResult(result);

		if(result.size() > 100)
		{
			return result.trimmed();
		}
	}

	return QString();
}
