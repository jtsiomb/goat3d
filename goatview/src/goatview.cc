#include <stdio.h>
#include <QtOpenGL/QtOpenGL>
#include <GL/glu.h>
#include <vmath/vmath.h>
#include "goatview.h"
#include "goat3d.h"

static void draw_node(goat3d_node *node);

goat3d *scene;
QSettings *settings;

static long anim_time;
static float cam_theta, cam_phi, cam_dist = 8;
static float fov = 60.0;

bool load_scene(const char *fname)
{
	if(scene) {
		goat3d_free(scene);
	}
	if(!(scene = goat3d_create()) || goat3d_load(scene, fname) == -1) {
		return false;
	}

	float bmin[3], bmax[3];
	goat3d_get_bounds(scene, bmin, bmax);
	float bsize = (Vector3(bmax[0], bmax[1], bmax[2]) - Vector3(bmin[0], bmin[1], bmin[2])).length();
	cam_dist = bsize / tan(DEG_TO_RAD(fov) / 2.0) + bsize;

	printf("bounds size: %f, cam_dist: %f\n", bsize, cam_dist);
	return true;
}

GoatView::GoatView()
{
	make_menu();
	make_dock();
	make_center();

	statusBar();

	setWindowTitle("GoatView");
	resize(settings->value("main/size", QSize(1024, 768)).toSize());
	move(settings->value("main/pos", QPoint(100, 100)).toPoint());
}

GoatView::~GoatView()
{
}

void GoatView::closeEvent(QCloseEvent *ev)
{
	settings->setValue("main/size", size());
	settings->setValue("main/pos", pos());
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
	std::string fname = QFileDialog::getOpenFileName(this, "Open scene file", "",
		"Goat3D Scene (*.goatsce);;All Files (*)").toStdString();
	if(fname.empty()) {
		statusBar()->showMessage("Abort: No file selected!");
		return;
	}

	statusBar()->showMessage("opening scene file");
	if(!load_scene(fname.c_str())) {
		statusBar()->showMessage("failed to load scene file");
	}
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
	glClearColor(0.1, 0.1, 0.1, 1);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void GoatViewport::resizeGL(int xsz, int ysz)
{
	glViewport(0, 0, xsz, ysz);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (float)xsz / (float)ysz, 0.5, 5000.0);
}

void GoatViewport::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -cam_dist);
	glRotatef(cam_phi, 1, 0, 0);
	glRotatef(cam_theta, 0, 1, 0);

	if(scene) {
		int node_count = goat3d_get_node_count(scene);
		for(int i=0; i<node_count; i++) {
			goat3d_node *node = goat3d_get_node(scene, i);
			if(!goat3d_get_node_parent(node)) {
				draw_node(node);	// only draw root nodes, the rest will be drawn recursively
			}
		}
	}
}

static void draw_node(goat3d_node *node)
{
	float xform[16];
	goat3d_get_node_matrix(node, xform, anim_time);
	for(int i=0; i<4; i++) {
		float *row = xform + i * 4;
		printf("[%3.3f %3.3f %3.3f %3.3f]\n", row[0], row[1], row[2], row[3]);
	}
	putchar('\n');

	glPushMatrix();
	glMultTransposeMatrixf(xform);

	if(goat3d_get_node_type(node) == GOAT3D_NODE_MESH) {
		goat3d_mesh *mesh = (goat3d_mesh*)goat3d_get_node_object(node);

		int num_faces = goat3d_get_mesh_face_count(mesh);
		int num_verts = goat3d_get_mesh_attrib_count(mesh, GOAT3D_MESH_ATTR_VERTEX);

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, goat3d_get_mesh_attribs(mesh, GOAT3D_MESH_ATTR_VERTEX));

		float *data;
		if((data = (float*)goat3d_get_mesh_attribs(mesh, GOAT3D_MESH_ATTR_NORMAL))) {
			glEnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(GL_FLOAT, 0, data);
		}
		if((data = (float*)goat3d_get_mesh_attribs(mesh, GOAT3D_MESH_ATTR_TEXCOORD))) {
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, 0, data);
		}

		int *indices;
		if((indices = goat3d_get_mesh_faces(mesh))) {
			glDrawElements(GL_TRIANGLES, num_faces * 3, GL_UNSIGNED_INT, indices);
		} else {
			glDrawArrays(GL_TRIANGLES, 0, num_verts * 3);
		}

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	int num_child = goat3d_get_node_child_count(node);
	for(int i=0; i<num_child; i++) {
		draw_node(goat3d_get_node_child(node, i));
	}

	glPopMatrix();
}


static float prev_x, prev_y;
void GoatViewport::mousePressEvent(QMouseEvent *ev)
{
	prev_x = ev->x();
	prev_y = ev->y();
}

void GoatViewport::mouseMoveEvent(QMouseEvent *ev)
{
	int dx = ev->x() - prev_x;
	int dy = ev->y() - prev_y;

	if(!dx && !dy) return;

	if(ev->buttons() & Qt::LeftButton) {
		cam_theta += dx  * 0.5;
		cam_phi += dy * 0.5;

		if(cam_phi < -90) cam_phi = -90;
		if(cam_phi > 90) cam_phi = 90;
	}
	if(ev->buttons() & Qt::RightButton) {
		cam_dist += dy * 0.1;

		if(cam_dist < 0.0) cam_dist = 0.0;
	}
	updateGL();
}
