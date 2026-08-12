#include "MainWindow.h"
#include "DarkTheme.h"

#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    // ── QApplication setup ──
    QApplication app(argc, argv);

    app.setApplicationName("DayZ Types Editor");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("DayZTools");
    app.setOrganizationDomain("dayztools.local");

    // ── Dark theme ──
    DarkTheme::apply(app);

    // ── Command line: optional file path ──
    QCommandLineParser parser;
    parser.setApplicationDescription("DayZ types.xml editor");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "types.xml file to open (optional)");
    parser.process(app);

    const QStringList posArgs = parser.positionalArguments();
    const QString startupFile = posArgs.isEmpty() ? QString() : posArgs.first();

    // ── Main window ──
    MainWindow window;
    window.show();

    if (!startupFile.isEmpty())
        window.openFileOnStartup(startupFile);

    return app.exec();
}
