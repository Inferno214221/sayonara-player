#include "ConfigureStreamDialog.h"
#include "Utils/Language/Language.h"
#include "Components/Streaming/Streams/StreamHandler.h"

#include <QLineEdit>
#include <QCheckBox>

struct ConfigureStreamDialog::Private
{
	QLineEdit* name=nullptr;
	QLineEdit* url=nullptr;
};

ConfigureStreamDialog::ConfigureStreamDialog(QWidget* parent) :
	GUI_ConfigureStation(parent)
{
	m = Pimpl::make<Private>();

	m->name = new QLineEdit();
	m->url = new QLineEdit();
}

ConfigureStreamDialog::~ConfigureStreamDialog() = default;

StationPtr ConfigureStreamDialog::configured_station()
{
	StreamHandler handler;
	return handler.create_stream(m->name->text(), m->url->text());
}

QList<QWidget*> ConfigureStreamDialog::configuration_widgets(StationPtr station)
{
	if(station)
	{
		m->name->setText(station->name());
		m->url->setText(station->url());
	}

	return {m->name, m->url};
}

QString ConfigureStreamDialog::label_text(int i) const
{
	if(i == 0){
		return Lang::get(Lang::Name);
	}

	else if(i == 1){
		return "Url";
	}

	return QString();
}
