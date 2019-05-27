#ifndef LANGUAGEUTILS_H
#define LANGUAGEUTILS_H

class QString;
template<typename A, typename B>
class QMap;

namespace Util
{
	namespace Language
	{
		QString get_share_path(const QString& four_letter);
		QString get_ftp_path(const QString& four_letter);
		QString get_http_path(const QString& four_letter);

		QString get_checksum_ftp_path();
		QString get_checksum_http_path();

		QString get_home_target_path(const QString& four_letter);
		QString get_used_language_file(const QString& four_letter);
		QString get_icon_path(const QString& four_letter);
		QString extract_four_letter(const QString& language_file);
		QString get_checksum(const QString& four_letter);

		QString get_language_version(const QString& four_letter);
		void update_language_version(const QString& four_letter);

		bool is_outdated(const QString& four_letter);
		QString get_similar_language_4(const QString& four_letter);

		#ifdef DEBUG
			void set_test_mode();
			void set_language_version(const QString& four_letter, const QString& version);
		#endif
	}
}

#endif // LANGUAGEUTILS_H
