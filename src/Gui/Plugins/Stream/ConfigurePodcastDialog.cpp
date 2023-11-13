#include "ConfigurePodcastDialog.h"
#include "Utils/Language/Language.h"
#include "Components/Streaming/Streams/PodcastHandler.h"

#include <QLineEdit>
#include <QCheckBox>

struct ConfigurePodcastDialog::Private
{
	QLineEdit* name {new QLineEdit()};
	QLineEdit* url {new QLineEdit()};
	QCheckBox* reverse {new QCheckBox()};
};

ConfigurePodcastDialog::ConfigurePodcastDialog(QWidget* parent) :
	GUI_ConfigureStation(parent),
	m {Pimpl::make<Private>()} {}

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
	const auto podcast = std::dynamic_pointer_cast<Podcast>(station);
	if(podcast)
	{
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

QString ConfigurePodcastDialog::labelText(const int i) const
{
	switch(i)
	{
		case 0:
			Lang::get(Lang::Name);
		case 1:
			return "Url";
		case 2:
			return Lang::get(Lang::ReverseOrder);
		default:
			return QString {};
	}
}
