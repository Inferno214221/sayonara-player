#ifndef GUI_SPECTROGRAMPAINTER_H
#define GUI_SPECTROGRAMPAINTER_H

#include "Gui/Plugins/PlayerPluginBase.h"

#include <QList>
#include <QWidget>

class PlayManager;

class GUI_SpectrogramPainter :
	public PlayerPlugin::Base
{
	Q_OBJECT
	PIMPL(GUI_SpectrogramPainter)

public:
	explicit GUI_SpectrogramPainter(PlayManager* playManager, QWidget* parent = nullptr);
	~GUI_SpectrogramPainter() override;

	QString name() const override;
	QString displayName() const override;
	bool isUiInitialized() const override;

private slots:
	void reset();
	void spectrumChanged(const QList<float>& spectrum, MilliSeconds ms);
	void finished();

	void playstateChanged(PlayState state);
	void trackChanged(const MetaData& md);

protected:
	void retranslate() override;
	void initUi() override;

	void paintEvent(QPaintEvent* e) override;
	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;

	void showEvent(QShowEvent* e) override;
	void closeEvent(QCloseEvent* e) override;

private:
	void drawBuffer(int percent_step);
	QString calcTooltip(float yPercent);

	void showFullsize();
	void positionClicked(QPoint position);

	void startAudioDataProvider(const MetaData& md);
	void stopAudioDataProvider();

	QSize minimumSizeHint() const override;
};

#endif // GUI_SPECTROGRAMPAINTER_H
