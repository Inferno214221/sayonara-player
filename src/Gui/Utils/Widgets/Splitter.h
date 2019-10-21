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

		public:
			explicit Splitter(QWidget* parent=nullptr);
			~Splitter() override;

			void set_handle_enabled(bool b);
			bool is_handle_enabled() const;

		protected:
			QSplitterHandle* createHandle() override;
	};

	class SplitterHandle :
		public QSplitterHandle
	{
		Q_OBJECT

		public:
			using QSplitterHandle::QSplitterHandle;

		protected:
			void mouseMoveEvent(QMouseEvent* e) override;
	};
} // namespace Gui

#endif // SPLITTER_H
