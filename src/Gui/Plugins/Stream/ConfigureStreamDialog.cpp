#include "ConfigureStreamDialog.h"
#include "Utils/Language/Language.h"
#include "Components/Streaming/Streams/StreamHandler.h"

#include <QLineEdit>
#include <QCheckBox>

struct ConfigureStreamDialog::Private
{
	PlaylistCreator* playlistCreator;
	QLineEdit* name;
	QLineEdit* url;

	Private(PlaylistCreator* playlistCreator) :
		playlistCreator {playlistCreator},
		name {new QLineEdit()},
		url {new QLineEdit()} {}
};

ConfigureStreamDialog::ConfigureStreamDialog(PlaylistCreator* playlistCreator, QWidget* parent) :
	GUI_ConfigureStation(parent)
{
	m = Pimpl::make<Private>(playlistCreator);
}

ConfigureStreamDialog::~ConfigureStreamDialog() = default;

StationPtr ConfigureStreamDialog::configuredStation()
{
	auto handler = StreamHandler(m->playlistCreator);
	return handler.createStreamInstance(m->name->text(), m->url->text());
}

void ConfigureStreamDialog::configureWidgets(StationPtr station)
{
	if(station)
	{
		m->name->setText(station->name());
		m->url->setText(station->url());
	}

	else
	{
		m->name->setText(QString());
		m->url->setText(QString());
	}
}

QList<QWidget*> ConfigureStreamDialog::configurationWidgets()
{
	return {m->name, m->url};
}

QString ConfigureStreamDialog::labelText(int index) const
{
	switch(index)
	{
		case 0:
			return Lang::get(Lang::Name);
		case 1:
			return "Url";
		default:
			return QString {};
	}
}
