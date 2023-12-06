#ifndef SAYONARA_TEST_H
#define SAYONARA_TEST_H

#include <QTest>
#include <QDebug>
#include <QObject>

namespace Test
{
	class Base :
		public QObject
	{
		Q_OBJECT

		public:
			explicit Base(const QString& testName);
			~Base() override;

			[[nodiscard]] QString tempPath() const;
			[[nodiscard]] QString tempPath(const QString& append) const;

		private:
			QString m_localPath;
	};
}

#endif // SAYONARA_TEST_H
