/* StreamRecorderUtils.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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



#include "StreamRecorderUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"

#include <QStringList>
#include <QDateTime>
#include <QRegExp>
#include <QDir>

using namespace StreamRecorder;
namespace FileUtils=::Util::File;

QList<QString> Utils::supported_tags()
{
	QList<QString> tags;
	const QList<QPair<QString, QString>> descs = Utils::descriptions();

	for(const auto& p : descs)
	{
		tags << p.first;
	}

	return tags;
}

Utils::ErrorCode Utils::validate_template(const QString &target_path_template, int* invalid_idx)
{
	if(target_path_template.isEmpty())
	{
		*invalid_idx = 0;
		return Utils::ErrorCode::Empty;
	}

	int is_open = 0;
	int is_close = 0;
	int i=0;
	for(QChar c : target_path_template)
	{
		if(c == '<')
		{
			is_open++;
			if(is_open > (is_close + 1)) {
				*invalid_idx = i;
				return Utils::ErrorCode::BracketError;
			}
		}

		if(c == '>')
		{
			is_close++;
			if(is_close != is_open)
			{
				*invalid_idx = i;
				return Utils::ErrorCode::BracketError;
			}
		}

		i++;
	}

	if(is_open != is_close){
		*invalid_idx = target_path_template.size() - 1;
		return Utils::ErrorCode::BracketError;
	}

	QList<QString> tags = supported_tags();

	QRegExp re("<(.*)>");
	re.setMinimal(true);

	bool has_track_number = false;
	bool has_title = false;
	int idx = re.indexIn(target_path_template);
	while(idx >= 0 && idx < target_path_template.size())
	{
		QString tag = re.cap(1);
		if(!tags.contains(tag))
		{
			*invalid_idx = idx;
			return Utils::ErrorCode::UnknownTag;
		}

		int old_idx = idx;
		idx = re.indexIn(target_path_template, old_idx + 1);

		if(tag.compare("t") == 0)
		{
			has_title = true;
		}

		else if(tag.compare("tn") == 0)
		{
			has_track_number = true;
		}
	}

	if( (!has_title) && (!has_track_number) )
	{
		*invalid_idx = target_path_template.size() - 1;
		return Utils::ErrorCode::MissingUniqueTag;
	}

	const QStringList invalid_chars {
		":", "\"", "(", ")", " /", "/ ", " *", "?"
	};

	for(const QString& ic : invalid_chars)
	{
		int idx = target_path_template.indexOf(ic);
		if(idx != -1){
			*invalid_idx = idx;
			return Utils::ErrorCode::InvalidChars;
		}
	}

	*invalid_idx = -1;
	return Utils::ErrorCode::OK;
}

QString Utils::target_path_template_default(bool use_session_path)
{
	if(use_session_path)
	{
		return "<rs>/<y>-<m>-<d>-<h>h<min>/<tn> - <ar> - <t>";
	}

	return "<tn> - <ar> - <t>";
}


QList<QPair<QString, QString> > Utils::descriptions()
{
	QList<QPair<QString, QString>> ret;

	QDate d = QDateTime::currentDateTime().date();

	ret << QPair<QString, QString>("tn", Lang::get(Lang::TrackNo).append("*"));
	ret << QPair<QString, QString>("t", Lang::get(Lang::Title).append("*"));
	ret << QPair<QString, QString>("min", Lang::get(Lang::Minutes));
	ret << QPair<QString, QString>("h", Lang::get(Lang::Hours));
	ret << QPair<QString, QString>("d", Lang::get(Lang::Days) + " (" + QString::number(d.day()) + ")");
	ret << QPair<QString, QString>("ds", Lang::get(Lang::Days) + " (" + QDate::shortDayName(d.dayOfWeek()) + ")");
	ret << QPair<QString, QString>("dl", Lang::get(Lang::Days) + " (" + QDate::longDayName(d.dayOfWeek()) + ")");
	ret << QPair<QString, QString>("m", Lang::get(Lang::Months) + " (" + QString::number(d.month()) + ")");
	ret << QPair<QString, QString>("ms", Lang::get(Lang::Months) + " (" + QDate::shortMonthName(d.month()) + ")");
	ret << QPair<QString, QString>("ml", Lang::get(Lang::Months) + " (" + QDate::longMonthName(d.month()) + ")");
	ret << QPair<QString, QString>("y", Lang::get(Lang::Year));
	ret << QPair<QString, QString>("ar", Lang::get(Lang::Artist));
	ret << QPair<QString, QString>("rs", Lang::get(Lang::RadioStation));

	return ret;
}

static QString replace_placeholder(const QString& str, const MetaData& md, QDate date=QDate(), QTime time=QTime())
{
	if(date.isNull()){
		date = QDate::currentDate();
	}

	if(time.isNull()){
		time = QTime::currentTime();
	}

	QString target_path(str);

	target_path.replace("<h>",		QString("%1").arg(time.hour(), 2, 10, QChar('0')));
	target_path.replace("<min>",	QString("%1").arg(time.minute(), 2, 10, QChar('0')));
	target_path.replace("<d>",		QString("%1").arg(date.day(), 2, 10, QChar('0')));
	target_path.replace("<ds>",		QDate::shortDayName(date.dayOfWeek()));
	target_path.replace("<dl>",		QDate::longDayName(date.dayOfWeek()));
	target_path.replace("<m>",		QString("%1").arg(date.month(), 2, 10, QChar('0')));
	target_path.replace("<ms>",		QDate::shortMonthName(date.month()));
	target_path.replace("<ml>",		QDate::longMonthName(date.month()));
	target_path.replace("<y>",		QString("%1").arg(date.year()));
	target_path.replace("<tn>",		QString("%1").arg(md.track_num, 4, 10, QChar('0')));

	QString title = md.title().trimmed();
	QString artist = md.artist().trimmed();
	QString album = md.album().trimmed();

	QStringList forbidden = {
		"/", "\\", ":", "*", "%", "$", "\n", "\t", "\r"
	};

	for(const QString& forbidden_str : forbidden)
	{
		title.replace(forbidden_str, "");
		artist.replace(forbidden_str, "");
		album.replace(forbidden_str, "");
	}

	target_path.replace("<t>",      title);
	target_path.replace("<ar>",     artist);
	target_path.replace("<rs>",     album);

	return target_path;
}

Utils::TargetPaths Utils::full_target_path(const QString& sr_path, const QString& path_template, const MetaData& md, const QDate& date, const QTime& time)
{
	int invalid_idx;
	Utils::TargetPaths ret;

	if(validate_template(path_template, &invalid_idx) != Utils::ErrorCode::OK){
		return ret;
	}

	QString dir, filename;
	FileUtils::split_filename(path_template, dir, filename);

	dir = replace_placeholder(dir, md, date, time);
	filename = replace_placeholder(filename, md);

	QString target_path = dir + QDir::separator() + filename;

	if(!target_path.endsWith(".mp3")){
		target_path += ".mp3";
	}

	ret.first = Util::File::clean_filename(sr_path + QDir::separator() + target_path);
	ret.second =	Util::File::clean_filename
					(
						Util::File::get_parent_directory(ret.first) +
						QDir::separator() +
						"playlist-" +
						date.toString("yyMMdd") +
						"-" +
						time.toString("hhmm") +
						".m3u"
					);

	return ret;
}

QString Utils::parse_error_code(Utils::ErrorCode err)
{
	QString str = Lang::get(Lang::Error).append(": ");
	switch(err)
	{
		case Utils::ErrorCode::OK:
			return "OK";
		case Utils::ErrorCode::BracketError:
			str += "<>";
			break;
		case Utils::ErrorCode::UnknownTag:
			str += Lang::get(Lang::UnknownPlaceholder);
			break;
		case Utils::ErrorCode::MissingUniqueTag:
			str +=  Lang::get(Lang::Missing).append(": ") +
					Lang::get(Lang::TrackNo).space() +
					Lang::get(Lang::Or).space() +
					Lang::get(Lang::Title);
			break;
		case Utils::ErrorCode::InvalidChars:
			str += Lang::get(Lang::InvalidChars);
			break;
		case Utils::ErrorCode::Empty:
			str += Lang::get(Lang::EmptyInput);
			break;

		default: break;
	}

	return str;
}
