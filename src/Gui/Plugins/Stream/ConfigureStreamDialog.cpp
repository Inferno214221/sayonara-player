#include "ConfigureStreamDialog.h"
#include "Utils/Language/Language.h"
#include "Components/Streaming/Streams/StreamHandler.h"

#include <QLineEdit>
#include <QCheckBox>

struct ConfigureStreamDialog::Private
{
	PlaylistCreator* playlistCreator;
	QLineEdit* name=nullptr;
	QLineEdit* url=nullptr;

	Private(PlaylistCreator* playlistCreator) :
		playlistCreator(playlistCreator)
	{}
};

ConfigureStreamDialog::ConfigureStreamDialog(PlaylistCreator* playlistCreator, QWidget* parent) :
	GUI_ConfigureStation(parent)
{
	m = Pimpl::make<Private>(playlistCreator);

	m->name = new QLineEdit();
	m->url = new QLineEdit();
}

ConfigureStreamDialog::~ConfigureStreamDialog() = default;

StationPtr ConfigureStreamDialog::configuredStation()
{
	StreamHandler handler(m->playlistCreator);
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

QString ConfigureStreamDialog::labelText(int i) const
{
	if(i == 0){
		return Lang::get(Lang::Name);
	}

	else if(i == 1){
		return "Url";
	}

	return QString();
}
