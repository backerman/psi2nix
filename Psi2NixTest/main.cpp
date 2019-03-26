#include <QGuiApplication>
#include <QTest>

// Test fixture includes
#include "testlink.hpp"
#include "testprotocol.hpp"

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
    auto result = QTest::qExec(new TestLink, argc, argv);
    result |= QTest::qExec(new TestProtocol, argc, argv);
    return result;
}
