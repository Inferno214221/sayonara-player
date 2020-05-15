#ifndef SPLITTER_H
#define SPLITTER_H

#include "Utils/Pimpl.h"
#include <QSplitter>

namespace Gui
{
	class Splitter :
		public QSplitter
	{
		Q_OBJECT
		PIMPL(Splitter)

		signals:
			void sigResizeFinished();

		public:
			explicit Splitter(QWidget* parent=nullptr);
			~Splitter() override;

			void setHandleEnabled(bool b);
			bool isHandleEnabled() const;

		protected:
			QSplitterHandle* createHandle() override;
	};

	class SplitterHandle :
		public QSplitterHandle
	{
		Q_OBJECT

		signals:
			void sigResizeFinished();

		public:
			using QSplitterHandle::QSplitterHandle;
			void isPressed();

		protected:
			void mouseMoveEvent(QMouseEvent* e) override;
	};
} // namespace Gui

#endif // SPLITTER_H
