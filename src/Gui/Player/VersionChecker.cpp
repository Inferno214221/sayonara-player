#include "VersionChecker.h"

#include "Utils/Utils.h"
#include "Utils/WebAccess/AsyncWebAccess.h"
#include "Utils/Message/Message.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"

#include "Gui/Utils/Style.h"


VersionChecker::VersionChecker(QObject* parent) :
	QObject(parent)
{
	AsyncWebAccess* awa = new AsyncWebAccess(this);
	awa->run("http://sayonara-player.com/current_version");
	connect(awa, &AsyncWebAccess::sig_finished, this, &VersionChecker::version_check_finished);
}

VersionChecker::~VersionChecker() = default;

void VersionChecker::version_check_finished()
{
	AsyncWebAccess::Status status = AsyncWebAccess::Status::Error;
	QByteArray data;

	{
		AsyncWebAccess* awa = static_cast<AsyncWebAccess*>(sender());
		if(!awa){
			return;
		}

		status = awa->status();
		data = awa->data();

		awa->deleteLater();
	}

	if(status != AsyncWebAccess::Status::GotData || data.isEmpty()) {
		return;
	}

	QString new_version = QString(data).trimmed();
	QString cur_version = GetSetting(Set::Player_Version);

	bool notify_new_version = GetSetting(Set::Player_NotifyNewVersion);
	bool dark = Style::is_dark();

	sp_log(Log::Info, this) << "Newest Version: " << new_version;
	sp_log(Log::Info, this) << "This Version:   " << cur_version;

	QString link = Util::create_link("http://sayonara-player.com", dark);

	if(new_version > cur_version && notify_new_version) {
		Message::info(tr("A new version is available!") + "<br />" +  link);
	}

	emit sig_finished();
}
