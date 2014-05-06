#include "goatview.h"

GoatView::GoatView()
{
	make_menu();
	make_dock();
	make_center();

	statusBar();

	setWindowTitle("GoatView");
}

GoatView::~GoatView()
{
}

bool GoatView::make_menu()
{
	QMenu *menu_file = menuBar()->addMenu("&File");

	QAction *act_open_sce = new QAction("&Open Scene", this);
	act_open_sce->setShortcuts(QKeySequence::Open);
	connect(act_open_sce, &QAction::triggered, this, &GoatView::open_scene);
	menu_file->addAction(act_open_sce);

	QAction *act_open_anm = new QAction("Open &Animation", this);
	connect(act_open_anm, &QAction::triggered, this, &GoatView::open_anim);
	menu_file->addAction(act_open_anm);

	QAction *act_quit = new QAction("&Quit", this);
	act_quit->setShortcuts(QKeySequence::Quit);
	connect(act_quit, &QAction::triggered, [&](){qApp->quit();});
	menu_file->addAction(act_quit);
	return true;
}

bool GoatView::make_dock()
{
	// ---- side-dock ----
	QWidget *dock_cont = new QWidget;
	QVBoxLayout *dock_vbox = new QVBoxLayout;
	dock_cont->setLayout(dock_vbox);

	QPushButton *bn_quit = new QPushButton("quit");
	dock_vbox->addWidget(bn_quit);
	connect(bn_quit, &QPushButton::clicked, [&](){qApp->quit();});

	QDockWidget *dock = new QDockWidget("Scene graph", this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	dock->setWidget(dock_cont);
	addDockWidget(Qt::LeftDockWidgetArea, dock);

	// ---- bottom dock ----
	dock_cont = new QWidget;
	QHBoxLayout *dock_hbox = new QHBoxLayout;
	dock_cont->setLayout(dock_hbox);

	QSlider *slider_time = new QSlider(Qt::Orientation::Horizontal);
	slider_time->setDisabled(true);
	dock_hbox->addWidget(slider_time);

	dock = new QDockWidget("Animation", this);
	dock->setAllowedAreas(Qt::BottomDockWidgetArea);
	dock->setWidget(dock_cont);
	addDockWidget(Qt::BottomDockWidgetArea, dock);

	return true;
}

bool GoatView::make_center()
{
	GoatViewport *vport = new GoatViewport;
	setCentralWidget(vport);
	return true;
}

void GoatView::open_scene()
{
	statusBar()->showMessage("opening scene...");
}

void GoatView::open_anim()
{
	statusBar()->showMessage("opening animation...");
}


// ---- OpenGL viewport ----
GoatViewport::GoatViewport()
	: QGLWidget(QGLFormat(QGL::DepthBuffer))
{
}

GoatViewport::~GoatViewport()
{
}

QSize GoatViewport::sizeHint() const
{
	return QSize(800, 600);
}

void GoatViewport::initializeGL()
{
}

void GoatViewport::resizeGL(int xsz, int ysz)
{
	glViewport(0, 0, xsz, ysz);
}

void GoatViewport::paintGL()
{
	glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
}