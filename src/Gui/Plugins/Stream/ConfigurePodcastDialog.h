#ifndef GUI_CONFIGURESTREAM_H
#define GUI_CONFIGURESTREAM_H

#include "GUI_ConfigureStation.h"

class ConfigurePodcastDialog :
	public GUI_ConfigureStation
{
	PIMPL(ConfigurePodcastDialog)

public:
	ConfigurePodcastDialog(QWidget* parent);
	~ConfigurePodcastDialog() override;

	StationPtr			configured_station() override;
	QList<QWidget*>		configuration_widgets(StationPtr station) override;
	QString				label_text(int i) const override;

};

#endif // GUI_CONFIGURESTREAM_H
