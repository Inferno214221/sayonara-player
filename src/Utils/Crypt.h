#ifndef CRYPT_H
#define CRYPT_H

#include <QByteArray>
#include <QString>

namespace Util
{
	namespace Crypt
	{
		QString encrypt(const QString& src, const QByteArray& key=QByteArray());
		QString encrypt(const QByteArray& src, const QByteArray& key=QByteArray());

		QString decrypt(const QString& src, const QByteArray& key=QByteArray());
		QString decrypt(const QByteArray& src, const QByteArray& key=QByteArray());
	}
}

#endif // CRYPT_H
