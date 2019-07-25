#include "Golyr.h"

QString Lyrics::Golyr::name() const
{
	return "Golyr";
}

QString Lyrics::Golyr::address() const
{
	return "http://www.golyr.de";
}

QMap<QString, QString> Lyrics::Golyr::replacements() const
{
	return QMap<QString, QString>
	{
		{"'", "-"},
		{" ", "-"},
		{"(", ""},
		{")", ""}
	};
}

QString Lyrics::Golyr::call_policy() const
{
	return "<SERVER>/<ARTIST>/songtext-<TITLE>";
}

QMap<QString, QString> Lyrics::Golyr::start_end_tag() const
{
	return QMap<QString, QString>
	{
		{"<div id=\"lyrics\">", "</div> <div class=\"fads\""}
	};
}

bool Lyrics::Golyr::is_start_tag_included() const
{
	return false;
}

bool Lyrics::Golyr::is_end_tag_included() const
{
	return false;
}

bool Lyrics::Golyr::is_numeric() const
{
	return false;
}

bool Lyrics::Golyr::is_lowercase() const
{
	return false;
}

QString Lyrics::Golyr::error_string() const
{
	return QString("404 Not Found");
}
