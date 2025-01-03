#include <ntqapplication.h>
#include "regexptester.h"

int main(int argc, char **argv)
{
    TQApplication app(argc, argv);
    RegexpTester form;
    form.show();
    app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
    return app.exec();
}
