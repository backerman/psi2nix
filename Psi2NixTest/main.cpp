#include <QGuiApplication>
#include <QTest>

// Test fixture includes
#include "testlink.hpp"

int main(int argc, char **argv)
{
    // We're just blindly copying QTEST_MAIN here and then adding
    // multi-fixture support.
    QGuiApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);
#ifdef QT_TESTCASE_BUILDDIR
    QTest::setMainSourcePath(__FILE__, QT_TESTCASE_BUILDDIR);
#else
    QTest::setMainSourcePath(__FILE__);
#endif
    return QTest::qExec(new TestLink, argc, argv);
}
