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

static QString convertTagToRegex(const QString& tag, const QMap<QString, QString>& regexConversions)
{
	QString ret(tag);

	const QList<QString> keys = regexConversions.keys();
	for(const QString& key : keys)
	{
		ret.replace(key, regexConversions.value(key));
	}

	ret.replace(" ", "\\s+");

	return ret;
}


QString WebpageParser::parseWebpage(const QByteArray& raw, const QMap<QString, QString>& regexConversions, Server* server)
{
	QString dst(raw);

	Server::StartEndTags tags = server->startEndTag();
	for(const Server::StartEndTag& tag : tags)
	{
		QString startTag = convertTagToRegex(tag.first, regexConversions);
		if(startTag.startsWith("<") && !startTag.endsWith(">")){
			startTag.append(".*>");
		}

		QString endTag = convertTagToRegex(tag.second, regexConversions);

		QString content;
		QRegExp regex;
		regex.setMinimal(true);
		regex.setPattern(startTag + "(.+)" + endTag);
		if(regex.indexIn(dst) != -1){
			content  = regex.cap(1);
		}

		if(content.isEmpty()){
			continue;
		}

		QRegExp reScript;
		reScript.setPattern("<script.+</script>");
		reScript.setMinimal(true);
		while(reScript.indexIn(content) != -1){
			content.replace(reScript, "");
		}

		QString word;
		if(server->isNumeric())
		{
			QRegExp rx("&#(\\d+);|<br />|</span>|</p>");

			QStringList tmplist;
			int pos = 0;
			while ((pos = rx.indexIn(content, pos)) != -1)
			{
				QString str = rx.cap(1);

				pos += rx.matchedLength();
				if(str.size() == 0)
				{
					tmplist.push_back(word);
					word = "";
					tmplist.push_back("<br>");
				}

				else{
					word.append(QChar(str.toInt()));
				}
			}

			dst = "";

			for(const QString& str : tmplist) {
				dst.append(str);
			}
		}

		else {
			dst = content;
		}

		dst.replace("<br>\n", "<br>");
		dst.replace(QRegExp("<br\\s*/>\n"), "<br>");
		dst.replace("\n", "<br>");
		dst.replace("\\n", "<br>");
		dst.replace(QRegExp("<br\\s*/>"), "<br>");
		dst.replace("\\\"", "\"");

		QRegExp re_ptag("<p\\s.*>");
		re_ptag.setMinimal(true);
		dst.remove(re_ptag);
		dst.remove(QRegExp("</p>"));

		QRegExp re_comment("<!--.*-->");
		re_comment.setMinimal(true);
		dst.remove(re_comment);

		QRegExp re_linefeed("<br>\\s*<br>\\s*<br>");
		while(dst.contains(re_linefeed)) {
			dst.replace(re_linefeed, "<br><br>");
		}

		while(dst.startsWith("<br>")){
			dst = dst.right(dst.count() - 4);
		}

		int idx = dst.indexOf("<a");
		while(idx >= 0)
		{
			int idx2 = dst.indexOf("\">", idx);
			dst.remove(idx, idx2 - idx + 2);
			idx = dst.indexOf("<a");
		}

		if(dst.size() > 100){
			break;
		}
	}

	return dst.trimmed();
}
