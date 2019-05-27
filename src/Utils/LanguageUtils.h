#ifndef LANGUAGEUTILS_H
#define LANGUAGEUTILS_H

class QString;

namespace Util
{
	namespace Language
	{
		QString get_share_path(const QString& four_letter);
		QString get_ftp_path(const QString& four_letter);
		QString get_home_target_path(const QString& four_letter);
		QString get_used_language_file(const QString& four_letter);
		QString get_icon_path(const QString& four_letter);
		QString extract_four_letter(const QString& language_file);
		QString get_checksum(const QString& four_letter);
	}
}

#endif // LANGUAGEUTILS_H
