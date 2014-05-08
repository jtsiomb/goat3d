#include <QtWidgets/QtWidgets>
#include <QtCore/QtCore>
#include "goatview.h"

int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	app.setOrganizationName("Mutant Stargoat");
	app.setOrganizationDomain("mutantstargoat.com");
	app.setApplicationName("GoatView");
	settings = new QSettings;

	QCommandLineParser argparse;
	argparse.addHelpOption();

	argparse.addPositionalArgument("scene", "scene file to open");
	argparse.addOption(QCommandLineOption("a", "add animation file"));
	argparse.process(app);

	const QStringList &args = argparse.positionalArguments();
	if(!args.isEmpty()) {
		if(args.count() > 1) {
			fprintf(stderr, "please specify at most one scene file to open\n");
			return 1;
		}
		std::string fname = args.at(0).toStdString();
		printf("loading scene file: %s ...\n", fname.c_str());
		if(!load_scene(fname.c_str())) {
			fprintf(stderr, "failed to load scene: %s\n", fname.c_str());
			return 1;
		}
	}

	const QStringList &anims = argparse.values("a");
	QStringList::const_iterator it = anims.begin();
	while(it != anims.end()) {
		std::string fname = it++->toStdString();
		printf("loading animation file: %s ...\n", fname.c_str());

		if(goat3d_load_anim(scene, fname.c_str()) == -1) {
			fprintf(stderr, "failed to load animation: %s\n", fname.c_str());
			return 1;
		}
	}

	GoatView gview;
	gview.show();

	return app.exec();
}
