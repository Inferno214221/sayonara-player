#include "Crypt.h"
#include "Utils/Settings/Settings.h"


static QByteArray _encrypt(const QByteArray& src, QByteArray key)
{
	if(src.isEmpty()){
		return QByteArray();
	}

	if(key.isEmpty()){
		key = Settings::instance()->get<Set::Player_PrivId>();
	}

	QByteArray result;

	const char* data = src.data();
	int len = src.length();

	for(int i=0; i<len; i++)
	{
		char c = *data;

		int pos = i % key.size();
		char mask_c = *(key.data() + pos);

		char result_c = c ^ mask_c;
		result.push_back(result_c);

		data++;
	}

	return result;
}


QString Util::Crypt::encrypt(const QString& src, const QByteArray& key)
{
	QByteArray enc = _encrypt(src.toUtf8(), key);
	return SettingConverter<QByteArray>::cvt_to_string(enc);
}

QString Util::Crypt::encrypt(const QByteArray& src, const QByteArray& key)
{
	QByteArray enc = _encrypt(src, key);
	return SettingConverter<QByteArray>::cvt_to_string(enc);
}


QString Util::Crypt::decrypt(const QString& src, const QByteArray& key)
{
	QByteArray srcba;
	SettingConverter<QByteArray>::cvt_from_string(src, srcba);

	QByteArray dec = _encrypt(srcba, key);
	return QString::fromUtf8(dec);
}

QString Util::Crypt::decrypt(const QByteArray& src, const QByteArray& key)
{
	return _encrypt(src, key);
}
