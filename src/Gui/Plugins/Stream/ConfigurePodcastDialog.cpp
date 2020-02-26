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

StationPtr ConfigurePodcastDialog::configuredStation()
{
	return std::make_shared<Podcast>(m->name->text(), m->url->text(), m->reverse->isChecked());
}

QList<QWidget*> ConfigurePodcastDialog::configurationWidgets()
{
	return {m->name, m->url, m->reverse};
}

void ConfigurePodcastDialog::configureWidgets(StationPtr station)
{
	if(station)
	{
		Podcast* podcast = dynamic_cast<Podcast*>(station.get());

		m->name->setText(podcast->name());
		m->url->setText(podcast->url());
		m->reverse->setChecked(podcast->reversed());
	}

	else
	{
		m->name->setText(QString());
		m->url->setText(QString());
		m->reverse->setChecked(false);
	}
}

QString ConfigurePodcastDialog::labelText(int i) const
{
	if(i == 0){
		return Lang::get(Lang::Name);
	}

	else if(i == 1){
		return "Url";
	}

	else if(i == 2){
		return Lang::get(Lang::ReverseOrder);
	}

	return QString();
}
