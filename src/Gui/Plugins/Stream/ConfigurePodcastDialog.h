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

	StationPtr			configuredStation() override;
	QList<QWidget*>		configurationWidgets() override;
	void				configureWidgets(StationPtr station) override;
	QString				labelText(int i) const override;
};

#endif // GUI_CONFIGURESTREAM_H
