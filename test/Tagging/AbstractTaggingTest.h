#ifndef ABSTRACTTAGGINGTEST_H
#define ABSTRACTTAGGINGTEST_H

#include "SayonaraTest.h"

class AbstractTaggingTest :
	public Test::Base
{
	Q_OBJECT

	public:
		AbstractTaggingTest(const QString& testname);

	private:
		QString mResourceFilename;
		QString mFilename;

	private:
		void init();
		void cleanup();
		void run();

	protected:
		virtual void runTest(const QString& filename) = 0;

	protected slots:
		void id3Test();
		void xiphTest();
};

#endif // ABSTRACTTAGGINGTEST_H
