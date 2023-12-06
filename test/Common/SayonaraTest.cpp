#include "test/Common/SayonaraTest.h"
#include "test/Common/PlayManagerMock.h"

#include "Database/Connector.h"

#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"
#include "Utils/StandardPaths.h"
#include "Utils/Utils.h"

#include <QStandardPaths>
#include <QApplication>
#include <QDir>

// needs to be done in global namespace
static void initResources()
{
	Q_INIT_RESOURCE(Test);
	Q_INIT_RESOURCE(Database);
	Q_INIT_RESOURCE(Resources);
}

namespace
{
	void initFileSystem(const QString& localPath)
	{
		Util::File::removeFilesInDirectory(QDir::home().absoluteFilePath(".qttest"));
		Util::File::createDirectories(localPath);
		QStandardPaths::setTestModeEnabled(true);
	}

	void initDatabase(const QString& targetDirectory)
	{
		DB::Connector::customInstance("", targetDirectory, "");
	}

	void initSettings()
	{
		auto* settings = Settings::instance();
		settings->checkSettings();
		settings->set<Set::Logger_Level>(static_cast<int>(Log::Develop));
	}
}

namespace Test
{
	Base::Base(const QString& testName) :
		m_localPath {Util::tempPath(testName)}
	{
		QApplication::setApplicationName("sayonara");

		initResources();
		initFileSystem(m_localPath);
		initDatabase(m_localPath);
		initSettings();

		setObjectName(testName);
	}

	Base::~Base()
	{
		Util::File::deleteFiles({m_localPath});
		Util::File::removeFilesInDirectory(QDir::home().absoluteFilePath(".qttest"));
	}

	QString Base::tempPath() const { return m_localPath; }

	QString Base::tempPath(const QString& append) const
	{
		return QDir(m_localPath).absoluteFilePath(append);
	}
}