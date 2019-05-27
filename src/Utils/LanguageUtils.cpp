#include "LanguageUtils.h"

#include "FileUtils.h"
#include "Utils.h"

#include <QFile>
#include <QRegExp>

namespace Language=Util::Language;

static bool check_four_letter(const QString& four_letter)
{
	QRegExp re("[a-z][a-z]_[A-Z][A-Z]");
	int idx = re.indexIn(four_letter);

	return (idx == 0);
}

QString Language::get_share_path(const QString& four_letter)
{
	if(!check_four_letter(four_letter)){
		return QString();
	}
	return Util::share_path("translations") + "/" + QString("sayonara_lang_%1.qm").arg(four_letter);
}

QString Language::get_ftp_path(const QString& four_letter)
{
	if(!check_four_letter(four_letter)){
		return QString();
	}
	return QString("ftp://sayonara-player.com/translation/sayonara_lang_%1.qm").arg(four_letter);
}

QString Language::get_home_target_path(const QString& four_letter)
{
	if(!check_four_letter(four_letter)){
		return QString();
	}

	QString translation_dir = Util::sayonara_path("translations");
	if(!Util::File::exists(translation_dir)){
		Util::File::create_dir(Util::sayonara_path("translations"));
	}

	return translation_dir + "/" + QString("sayonara_lang_%1.qm").arg(four_letter);
}

QString Language::get_used_language_file(const QString& four_letter)
{
	if(!check_four_letter(four_letter)){
		return QString();
	}

	if(Util::File::exists(get_home_target_path(four_letter))){
		return get_home_target_path(four_letter);
	}

	return get_share_path(four_letter);
}

QString Language::extract_four_letter(const QString& language_file)
{
	QRegExp re(".*sayonara_lang_(.+)\\..+");
	int idx = re.indexIn(language_file);
	if(idx < 0){
		return QString();
	}

	QString four_letter = re.cap(1);
	if(!check_four_letter(four_letter)){
		return QString();
	}

	return four_letter;
}

QString Language::get_icon_path(const QString& four_letter)
{
	return Util::share_path(QString("translations/icons/%1.png").arg(four_letter));
}

QString Language::get_checksum(const QString& four_letter)
{
	if(!check_four_letter(four_letter)){
		return QString();
	}

	QString path = get_used_language_file(four_letter);
	return QString::fromUtf8(Util::File::calc_md5_sum(path));
}
