#include "SayonaraTest.h"
#include "Database/Connector.h"
#include "Utils/Settings/Settings.h"

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"

using Test::Base;

// needs to be done in global namespace
static void init_resources()
{
	Q_INIT_RESOURCE(Test);
	Q_INIT_RESOURCE(Database);
	Q_INIT_RESOURCE(Resources);
}

Test::Base::Base(const QString& testName) :
	QObject(),
	mTmpPath(Util::tempPath(testName))
{
	Util::File::createDirectories(mTmpPath);

	init_resources();
	DB::Connector::instance_custom("", mTmpPath, "");
	Settings* s = Settings::instance();
	s->checkSettings();
	s->set<Set::Logger_Level>( int(Log::Develop) );

	this->setObjectName(testName);
}

Test::Base::~Base()
{
	Util::File::deleteFiles({mTmpPath});
}

QString Test::Base::tempPath() const
{
	return mTmpPath;
}

QString Test::Base::tempPath(const QString& append) const
{
	QDir d(tempPath());
	return d.absoluteFilePath(append);
}
