#include "SayonaraTest.h"
#include "Database/Connector.h"
#include "Utils/Settings/Settings.h"

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"

using Test::Base;

// needs to be done in global namespace
static void init_resources()
{
	Q_INIT_RESOURCE(Test);
	Q_INIT_RESOURCE(Database);
	Q_INIT_RESOURCE(Resources);
}

Test::Base::Base(const QString& test_name) :
	QObject(),
	mTmpPath(Util::tempPath(test_name))
{
	Util::File::createDirectories(mTmpPath);

	init_resources();

	DB::Connector::instance_custom("", mTmpPath, "");
	Settings* s = Settings::instance();
	s->checkSettings();

	this->setObjectName(test_name);
}

Test::Base::~Base()
{
	Util::File::deleteFiles({mTmpPath});
}

QString Test::Base::temp_path() const
{
	return mTmpPath;
}

QString Test::Base::temp_path(const QString& append) const
{
	QDir d(temp_path());
	return d.absoluteFilePath(append);
}
