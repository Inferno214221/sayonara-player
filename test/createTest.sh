#!/bin/bash

if [ $# -ne 1 ] ; then
	echo "Call $0 <testname>"
	exit 1
fi

PARAM=$1
EXT=${PARAM##*.}

if [ $EXT = "cpp" ] ; then
	TESTNAME=${PARAM%.*}
else
	TESTNAME=${PARAM}
fi

FILENAME=${TESTNAME}.cpp
rm -f ${FILENAME}

cat <<EOT >> ${FILENAME}
#include "SayonaraTest.h"
// access working directory with Test::Base::tempPath("somefile.txt");

class ${TESTNAME} : 
    public Test::Base
{
    Q_OBJECT

    public:
        ${TESTNAME}() :
            Test::Base("${TESTNAME}")
        {}

    private slots:
        void test();
};

void ${TESTNAME}::test()
{}

QTEST_GUILESS_MAIN(${TESTNAME})
#include "${TESTNAME}.moc"
EOT

echo "new_test(${FILENAME})" >> CMakeLists.txt
