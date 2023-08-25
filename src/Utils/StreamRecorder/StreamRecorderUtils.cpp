/* StreamRecorderUtils.cpp */

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

#include "StreamRecorderUtils.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/MetaData/MetaData.h"

#include <QDateTime>
#include <QDir>
#include <QLocale>
#include <QRegExp>
#include <QStringList>
#include <QUrl>

namespace
{
	QString removeHttp(QString text)
	{
		text.remove(QRegExp("http[s]?://"));

		if(const auto index = text.indexOf('?'); index > 0)
		{
			return text.left(index);
		}

		return text;
	}

	QString formatPotentialUrl(QString text)
	{
		static const auto forbiddenStrings = QStringList {
			"/", "\\", ":", "*", "%", "$", "\n", "\t", "\r"
		};

		text = removeHttp(text.trimmed());

		for(const auto& forbiddenString: forbiddenStrings)
		{
			text.remove(forbiddenString);
		}

		return text;
	}

	QString replacePlaceholder(const QString& str, const MetaData& track, const QDate& date, const QTime& time)
	{
		const auto systemLocale = QLocale::system();
		constexpr const auto TrackNumDigits = 4;
		constexpr const auto DateTimeDigits = 2;
		constexpr const auto NumericalBase = 10;
		const auto fillChar = QChar('0');

		auto targetPath = str;
		targetPath.replace("<h>", QString("%1").arg(time.hour(), DateTimeDigits, NumericalBase, fillChar));
		targetPath.replace("<min>", QString("%1").arg(time.minute(), DateTimeDigits, NumericalBase, fillChar));
		targetPath.replace("<d>", QString("%1").arg(date.day(), DateTimeDigits, NumericalBase, fillChar));
		targetPath.replace("<ds>", systemLocale.dayName(date.dayOfWeek(), QLocale::ShortFormat));
		targetPath.replace("<dl>", systemLocale.dayName(date.dayOfWeek(), QLocale::LongFormat));
		targetPath.replace("<m>", QString("%1").arg(date.month(), DateTimeDigits, NumericalBase, fillChar));
		targetPath.replace("<ms>", systemLocale.dayName(date.month(), QLocale::ShortFormat));
		targetPath.replace("<ml>", systemLocale.dayName(date.month(), QLocale::LongFormat));
		targetPath.replace("<y>", QString("%1").arg(date.year()));
		targetPath.replace("<tn>", QString("%1").arg(track.trackNumber(), TrackNumDigits, NumericalBase, fillChar));
		targetPath.replace("<t>", formatPotentialUrl(track.title()));
		targetPath.replace("<ar>", formatPotentialUrl(track.artist()));
		targetPath.replace("<rs>", formatPotentialUrl(track.album()));

		return targetPath;
	}

	QString getPlaylistPath(const QString& sessionPath, const QDate& date, const QTime& time)
	{
		return Util::File::cleanFilename(
			QString("%1/playlist-%2-%3.m3u")
				.arg(sessionPath)
				.arg(date.toString("yyMMdd"))
				.arg(time.toString("hhmm")));
	}

	bool validateMatchingBrackets(const QString& targetPathTemplate, int* invalidIndex)
	{
		auto isOpen = 0;
		auto isClose = 0;
		auto i = 0;
		for(const auto c: targetPathTemplate)
		{
			if(c == '<')
			{
				isOpen++;
				if(isOpen > (isClose + 1))
				{
					*invalidIndex = i;
					return false;
				}
			}

			if(c == '>')
			{
				isClose++;
				if(isClose != isOpen)
				{
					*invalidIndex = i;
					return false;
				}
			}

			i++;
		}

		if(isOpen != isClose)
		{
			*invalidIndex = targetPathTemplate.size() - 1;
			return false;
		}

		return true;
	}

	bool validateChars(const QString& targetPathTemplate, int* invalidIndex)
	{
		static const auto invalidChars = QStringList {
			":", "\"", "(", ")", " /", "/ ", " *", "?"
		};

		for(const auto& invalidChar: invalidChars) // NOLINT(readability-use-anyofallof)
		{
			auto invalidCharIndex = targetPathTemplate.indexOf(invalidChar);
			if(invalidCharIndex != -1)
			{
				*invalidIndex = invalidCharIndex;
				return false;
			}
		}

		return true;
	}

	StreamRecorder::Utils::ErrorCode validateTags(const QString& targetPathTemplate, int* invalidIndex)
	{
		const auto supportedTags = StreamRecorder::Utils::supportedTags();

		auto re = QRegExp("<(.*)>");
		re.setMinimal(true);

		auto hasTrackNumber = false;
		auto hasTitle = false;
		auto tagIndex = re.indexIn(targetPathTemplate);
		while((tagIndex >= 0) && (tagIndex < targetPathTemplate.size()))
		{
			const auto tag = re.cap(1);
			if(!supportedTags.contains(tag))
			{
				*invalidIndex = tagIndex;
				return StreamRecorder::Utils::ErrorCode::UnknownTag;
			}

			auto oldIndex = tagIndex;
			tagIndex = re.indexIn(targetPathTemplate, oldIndex + 1);

			if(tag.compare("t") == 0)
			{
				hasTitle = true;
			}

			else if(tag.compare("tn") == 0)
			{
				hasTrackNumber = true;
			}
		}

		if(!hasTitle && !hasTrackNumber)
		{
			*invalidIndex = targetPathTemplate.size() - 1;
			return StreamRecorder::Utils::ErrorCode::MissingUniqueTag;
		}

		return StreamRecorder::Utils::ErrorCode::OK;
	}
}

namespace StreamRecorder
{
	QStringList Utils::supportedTags()
	{
		QStringList tags;

		Util::Algorithm::transform(Utils::descriptions(), tags, [](const auto& description) {
			return description.first;
		});

		return tags;
	}

	Utils::ErrorCode Utils::validateTemplate(const QString& targetPathTemplate, int* invalidIndex)
	{
		if(targetPathTemplate.isEmpty())
		{
			*invalidIndex = 0;
			return Utils::ErrorCode::Empty;
		}

		if(!validateMatchingBrackets(targetPathTemplate, invalidIndex))
		{
			return Utils::ErrorCode::BracketError;
		}

		if(const auto error = validateTags(targetPathTemplate, invalidIndex); error != Utils::ErrorCode::OK)
		{
			return error;
		}

		if(!validateChars(targetPathTemplate, invalidIndex))
		{
			return Utils::ErrorCode::InvalidChars;
		}

		*invalidIndex = -1;
		return Utils::ErrorCode::OK;
	}

	QString Utils::targetPathTemplateDefault(const bool useSessionPath)
	{
		return useSessionPath
		       ? "<rs>/<y>-<m>-<d>-<h>h<min>/<tn> - <ar> - <t>"
		       : "<tn> - <ar> - <t>";
	}

	QList<QPair<QString, QString>> Utils::descriptions()
	{
		const auto currentDate = QDateTime::currentDateTime().date();
		const auto systemLocale = QLocale::system();

		return {
			{"tn",  Lang::get(Lang::TrackNo) + "*"},
			{"t",   Lang::get(Lang::Title) + "*"},
			{"min", Lang::get(Lang::Minutes)},
			{"h",   Lang::get(Lang::Hours)},
			{"d",   QString("%1 (%2)")
				        .arg(Lang::get(Lang::Days))
				        .arg(currentDate.day())
			},
			{"ds",  QString("%1 (%2)")
				        .arg(Lang::get(Lang::Days))
				        .arg(systemLocale.dayName(currentDate.dayOfWeek(), QLocale::ShortFormat))
			},
			{"dl",  QString("%1 (%2)")
				        .arg(Lang::get(Lang::Days))
				        .arg(systemLocale.dayName(currentDate.dayOfWeek(), QLocale::LongFormat))
			},
			{"m",   QString("%1 (%2)")
				        .arg(Lang::get(Lang::Months))
				        .arg(currentDate.month())
			},
			{"ms",  QString("%1 (%2)")
				        .arg(Lang::get(Lang::Months))
				        .arg(systemLocale.monthName(currentDate.month(), QLocale::ShortFormat))
			},
			{"ml",  QString("%1 (%2)")
				        .arg(Lang::get(Lang::Months))
				        .arg(systemLocale.monthName(currentDate.month(), QLocale::LongFormat))
			},
			{"y",   Lang::get(Lang::Year)},
			{"ar",  Lang::get(Lang::Artist)},
			{"rs",  Lang::get(Lang::RadioStation)},
		};
	}

	Utils::TargetPath
	Utils::fullTargetPath(const QString& streamRecorderPath, const QString& pathTemplate, const MetaData& track,
	                      const QDate& date, const QTime& time)
	{
		int invalidIndex; // NOLINT(cppcoreguidelines-init-variables)
		if(validateTemplate(pathTemplate, &invalidIndex) != Utils::ErrorCode::OK)
		{
			return {};
		}

		auto [dir, filename] = Util::File::splitFilename(pathTemplate);
		dir = replacePlaceholder(dir, track, date, time);
		filename = replacePlaceholder(filename, track, QDate::currentDate(), QTime::currentTime());

		auto targetPath = dir + QDir::separator() + filename;
		if(!targetPath.endsWith(".mp3"))
		{
			targetPath += ".mp3";
		}

		const auto audioFilePath = Util::File::cleanFilename(
			QString("%1/%2")
				.arg(streamRecorderPath)
				.arg(targetPath));

		return {
			audioFilePath,
			getPlaylistPath(Util::File::getParentDirectory(audioFilePath), date, time)
		};
	}

	QString Utils::parseErrorCode(const Utils::ErrorCode err)
	{
		auto str = Lang::get(Lang::Error).append(": ");
		switch(err)
		{
			case Utils::ErrorCode::OK:
				return "OK";
			case Utils::ErrorCode::BracketError:
				return str + "<>";
			case Utils::ErrorCode::UnknownTag:
				return str + Lang::get(Lang::UnknownPlaceholder);
			case Utils::ErrorCode::MissingUniqueTag:
				return str + Lang::get(Lang::Missing).append(": ") +
				       Lang::get(Lang::TrackNo).space() +
				       Lang::get(Lang::Or).space() +
				       Lang::get(Lang::Title);
			case Utils::ErrorCode::InvalidChars:
				return str + Lang::get(Lang::InvalidChars);
			case Utils::ErrorCode::Empty:
				return str + Lang::get(Lang::EmptyInput);
			default:
				return str;
		}
	}
}