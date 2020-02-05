#include "ConfigurePodcastDialog.h"
#include "Utils/Language/Language.h"
#include "Components/Streaming/Streams/PodcastHandler.h"

#include <QLineEdit>
#include <QCheckBox>

struct ConfigurePodcastDialog::Private
{
	QLineEdit* name=nullptr;
	QLineEdit* url=nullptr;
	QCheckBox* reverse=nullptr;
};

ConfigurePodcastDialog::ConfigurePodcastDialog(QWidget* parent) :
	GUI_ConfigureStation(parent)
{
	m = Pimpl::make<Private>();

	m->name = new QLineEdit(this);
	m->url = new QLineEdit(this);
	m->reverse = new QCheckBox(this);
}

ConfigurePodcastDialog::~ConfigurePodcastDialog() = default;

StationPtr ConfigurePodcastDialog::configured_station()
{
	return std::make_shared<Podcast>(m->name->text(), m->url->text(), m->reverse->isChecked());
}

QList<QWidget*> ConfigurePodcastDialog::configuration_widgets(StationPtr station)
{
	if(station)
	{
		Podcast* podcast = dynamic_cast<Podcast*>(station.get());

		m->name->setText(podcast->name());
		m->url->setText(podcast->url());
		m->reverse->setChecked(podcast->reversed());
	}

	return {m->name, m->url, m->reverse};
}

QString ConfigurePodcastDialog::label_text(int i) const
{
	if(i == 0){
		return Lang::get(Lang::Name);
	}

	else if(i == 1){
		return "Url";
	}

	else if(i == 2){
		return tr("Reverse");
	}

	return QString();
}
