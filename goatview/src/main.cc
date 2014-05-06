#include <QtWidgets/QtWidgets>
#include "goatview.h"

int main(int argc, char **argv)
{
	QApplication app(argc, argv);

	GoatView gview;
	gview.show();

	return app.exec();
}