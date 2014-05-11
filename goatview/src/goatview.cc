#include <stdio.h>
#include "opengl.h"
#include <QtOpenGL/QtOpenGL>
#include <vmath/vmath.h>
#include "goatview.h"
#include "goat3d.h"

static void update_tree(QTreeWidget *tree);
static void add_tree(QTreeWidget *tree, goat3d_node *node, QTreeWidgetItem *parent);

static void draw_node(goat3d_node *node);
static void draw_mesh(goat3d_mesh *mesh);

goat3d *scene;

static long anim_time;
static float cam_theta, cam_phi, cam_dist = 8;
static float fov = 60.0;
static bool use_nodes = true;


GoatView::GoatView()
{
	glview = 0;

	make_menu();
	make_dock();
	make_center();

	statusBar();

	setWindowTitle("GoatView");

	QSettings *settings = new QSettings;
	resize(settings->value("main/size", QSize(1024, 768)).toSize());
	move(settings->value("main/pos", QPoint(100, 100)).toPoint());
	delete settings;
}

GoatView::~GoatView()
{
}

bool GoatView::load_scene(const char *fname)
{
	if(scene) {
		goat3d_free(scene);
	}
	if(!(scene = goat3d_create()) || goat3d_load(scene, fname) == -1) {
		return false;
	}

	float bmin[3], bmax[3];
	if(goat3d_get_bounds(scene, bmin, bmax) != -1) {
		float bsize = (Vector3(bmax[0], bmax[1], bmax[2]) - Vector3(bmin[0], bmin[1], bmin[2])).length();
		cam_dist = bsize / tan(DEG_TO_RAD(fov) / 2.0) + bsize;
		printf("bounds size: %f, cam_dist: %f\n", bsize, cam_dist);
	}

	update_tree(scntree);
	return true;
}

void GoatView::closeEvent(QCloseEvent *ev)
{
	QSettings *settings = new QSettings;
	settings->setValue("main/size", size());
	settings->setValue("main/pos", pos());
	delete settings;
}

bool GoatView::make_menu()
{
	// file menu
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

	// view menu
	QMenu *menu_view = menuBar()->addMenu("&View");

	QAction *act_use_nodes = new QAction("use nodes", this);
	act_use_nodes->setCheckable(true);
	act_use_nodes->setChecked(use_nodes);
	connect(act_use_nodes, &QAction::triggered, this, [&](){use_nodes = !use_nodes; glview->updateGL();});
	menu_view->addAction(act_use_nodes);
	return true;
}

bool GoatView::make_dock()
{
	// ---- side-dock ----
	QWidget *dock_cont = new QWidget;
	QVBoxLayout *dock_vbox = new QVBoxLayout;
	dock_cont->setLayout(dock_vbox);

	QDockWidget *dock = new QDockWidget("Scene graph", this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	dock->setWidget(dock_cont);
	addDockWidget(Qt::LeftDockWidgetArea, dock);

	// make the tree view widget
	scntree = new QTreeWidget;
	scntree->setColumnCount(1);
	QStringList hdrstr;
	hdrstr << "Node";// << "Type";
	scntree->setHeaderItem(new QTreeWidgetItem((QTreeWidget*)0, hdrstr));
	scntree->setAlternatingRowColors(true);
	dock_vbox->addWidget(scntree);

	update_tree(scntree);

	// misc
	QPushButton *bn_quit = new QPushButton("quit");
	dock_vbox->addWidget(bn_quit);
	connect(bn_quit, &QPushButton::clicked, [&](){qApp->quit();});

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
	glview = new GoatViewport(this);
	setCentralWidget(glview);
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

static void update_tree(QTreeWidget *tree)
{
	if(!scene) return;

	int num_nodes = goat3d_get_node_count(scene);
	for(int i=0; i<num_nodes; i++) {
		goat3d_node *node = goat3d_get_node(scene, i);
		if(goat3d_get_node_parent(node)) {
			continue;
		}

		// only add the root nodes, the rest will be added recursively by them
		add_tree(tree, node, 0);
	}
	tree->expandAll();
}

static void add_tree(QTreeWidget *tree, goat3d_node *node, QTreeWidgetItem *parent)
{
	//char icon_name[64];
	//sprintf(icon_name, ":/icons/icons/icon_%s.png", node->get_type());

	QStringList row;
	row << goat3d_get_node_name(node) << "M";
	QTreeWidgetItem *item = new QTreeWidgetItem(parent, row);
	//item->setIcon(0, QIcon(icon_name));
	tree->addTopLevelItem(item);

	int num_children = goat3d_get_node_child_count(node);
	for(int i=0; i<num_children; i++) {
		add_tree(tree, goat3d_get_node_child(node, i), item);
	}
}



// ---- OpenGL viewport ----
GoatViewport::GoatViewport(QWidget *main_win)
	: QGLWidget(QGLFormat(QGL::DepthBuffer))
{
	this->main_win = main_win;
	initialized = false;
}

GoatViewport::~GoatViewport()
{
}

QSize GoatViewport::sizeHint() const
{
	return QSize(800, 600);
}

#define CRITICAL(error, detail)	\
	do { \
		fprintf(stderr, "%s: %s\n", error, detail); \
		QMessageBox::critical(main_win, error, detail); \
		abort(); \
	} while(0)

void GoatViewport::initializeGL()
{
	if(initialized) return;
	initialized = true;

	init_opengl();

	if(!GLEW_ARB_transpose_matrix) {
		CRITICAL("OpenGL initialization failed", "ARB_transpose_matrix extension not found!");
	}

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

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

	if(!scene) return;

	if(use_nodes) {
		int node_count = goat3d_get_node_count(scene);
		for(int i=0; i<node_count; i++) {
			goat3d_node *node = goat3d_get_node(scene, i);
			draw_node(node);	// only draw root nodes, the rest will be drawn recursively
		}
	} else {
		int mesh_count = goat3d_get_mesh_count(scene);
		for(int i=0; i<mesh_count; i++) {
			goat3d_mesh *mesh = goat3d_get_mesh(scene, i);
			draw_mesh(mesh);
		}
	}
}

#ifndef GLEW_ARB_transpose_matrix
#error "GLEW_ARB_transpose_matrix undefined?"
#endif

static void draw_node(goat3d_node *node)
{
	float xform[16];
	goat3d_get_node_matrix(node, xform, anim_time);

	glPushMatrix();
	glMultTransposeMatrixf(xform);

	if(goat3d_get_node_type(node) == GOAT3D_NODE_MESH) {
		goat3d_mesh *mesh = (goat3d_mesh*)goat3d_get_node_object(node);

		draw_mesh(mesh);
	}

	/*int num_child = goat3d_get_node_child_count(node);
	for(int i=0; i<num_child; i++) {
		draw_node(goat3d_get_node_child(node, i));
	}*/

	glPopMatrix();
}

static void draw_mesh(goat3d_mesh *mesh)
{
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
	prev_x = ev->x();
	prev_y = ev->y();

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
