#ifndef GUI_CONFIGURESTREAM_H
#define GUI_CONFIGURESTREAM_H

#include "GUI_ConfigureStation.h"

class PlaylistCreator;

class ConfigureStreamDialog :
	public GUI_ConfigureStation
{
	PIMPL(ConfigureStreamDialog)

	public:
		ConfigureStreamDialog(PlaylistCreator* playlistCreator, QWidget* parent);
		~ConfigureStreamDialog() override;

		[[nodiscard]] StationPtr configuredStation() override;

		void configureWidgets(StationPtr station) override;
		[[nodiscard]] QList<QWidget*> configurationWidgets() override;
		
		[[nodiscard]] QString labelText(int index) const override;
};

#endif // GUI_CONFIGURESTREAM_H
