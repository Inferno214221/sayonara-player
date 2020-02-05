#ifndef GUI_CONFIGURESTREAM_H
#define GUI_CONFIGURESTREAM_H

#include "GUI_ConfigureStation.h"

class ConfigureStreamDialog :
	public GUI_ConfigureStation
{
	PIMPL(ConfigureStreamDialog)

public:
	ConfigureStreamDialog(QWidget* parent);
	~ConfigureStreamDialog() override;

	StationPtr			configured_station() override;
	QList<QWidget*>		configuration_widgets(StationPtr station) override;
	QString				label_text(int i) const override;
};

#endif // GUI_CONFIGURESTREAM_H
