#include "ConfigureStreamDialog.h"
#include "Components/Streaming/Streams/StreamHandler.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Streams/Station.h"

#include <QLineEdit>
#include <QCheckBox>

struct ConfigureStreamDialog::Private
{
	PlaylistCreator* playlistCreator;
	QLineEdit* name {new QLineEdit()};
	QLineEdit* url {new QLineEdit()};
	QCheckBox* updateMetadata {new QCheckBox()};

	explicit Private(PlaylistCreator* playlistCreator) :
		playlistCreator {playlistCreator}
	{
		updateMetadata->setChecked(GetSetting(Set::Stream_UpdateMetadata));
	}
};

ConfigureStreamDialog::ConfigureStreamDialog(PlaylistCreator* playlistCreator, QWidget* parent) :
	GUI_ConfigureStation(parent),
	m {Pimpl::make<Private>(playlistCreator)} {}

ConfigureStreamDialog::~ConfigureStreamDialog() = default;

StationPtr ConfigureStreamDialog::configuredStation()
{
	return std::make_shared<Stream>(m->name->text(), m->url->text(), m->updateMetadata->isChecked());
}

void ConfigureStreamDialog::configureWidgets(StationPtr station)
{
	auto stream = std::dynamic_pointer_cast<Stream>(station);
	if(stream)
	{
		m->name->setText(stream->name());
		m->url->setText(stream->url());
		m->updateMetadata->setChecked(stream->isUpdatable());
	}

	else
	{
		m->name->setText({});
		m->url->setText({});
		m->updateMetadata->setChecked(GetSetting(Set::Stream_UpdateMetadata));
	}
}

QList<QWidget*> ConfigureStreamDialog::configurationWidgets()
{
	return {m->name, m->url, m->updateMetadata};
}

QString ConfigureStreamDialog::labelText(const int index) const
{
	switch(index)
	{
		case 0:
			return Lang::get(Lang::Name);
		case 1:
			return "Url";
		case 2:
			return tr("Update Metadata");
		default:
			return QString {};
	}
}
