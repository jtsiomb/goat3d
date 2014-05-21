#include <stdio.h>
#include <limits.h>
#include <map>
#include "opengl.h"
#include <QtOpenGL/QtOpenGL>
#include <vmath/vmath.h>
#include "goatview.h"
#include "goat3d.h"

static void draw_grid();
static void draw_grid(float sz, int nlines, float alpha = 1.0f);
static void draw_node(goat3d_node *node);
static void draw_mesh(goat3d_mesh *mesh);
static int next_pow2(int x);

goat3d *scene;
static SceneModel *sdata;
static GoatViewport *glview;

static long anim_time;
static float cam_theta, cam_phi = 25, cam_dist = 8;
static float fov = 60.0;
static bool use_nodes = true;
static bool use_lighting = true;
static bool use_textures = true;

void post_redisplay()
{
	if(glview) {
		glview->updateGL();
	}
}


GoatView::GoatView()
{
	glview = 0;
	scene_model = 0;

	QGLFormat glfmt = QGLFormat::defaultFormat();
	glfmt.setSampleBuffers(true);
	QGLFormat::setDefaultFormat(glfmt);

	QSettings settings;
	resize(settings.value("main/size", QSize(1024, 768)).toSize());
	move(settings.value("main/pos", QPoint(100, 100)).toPoint());
	use_nodes = settings.value("use_nodes", true).toBool();
	use_lighting = settings.value("use_lighting", true).toBool();
	use_textures = settings.value("use_textures", true).toBool();

	make_center();	// must be first
	make_menu();
	make_dock();

	statusBar();

	setWindowTitle("GoatView");
}

GoatView::~GoatView()
{
	delete scene_model;
	sdata = 0;
}

void GoatView::closeEvent(QCloseEvent *ev)
{
	QSettings settings;
	settings.setValue("main/size", size());
	settings.setValue("main/pos", pos());
	settings.setValue("use_nodes", use_nodes);
	settings.setValue("use_lighting", use_lighting);
}


bool GoatView::load_scene(const char *fname)
{
	if(scene) {
		goat3d_free(scene);
	}
	if(!(scene = goat3d_create()) || goat3d_load(scene, fname) == -1) {
		QMessageBox::critical(this, "Error", "Failed to load scene file: " + QString(fname));
		return false;
	}

	float bmin[3], bmax[3];
	if(goat3d_get_bounds(scene, bmin, bmax) != -1) {
		float bsize = (Vector3(bmax[0], bmax[1], bmax[2]) - Vector3(bmin[0], bmin[1], bmin[2])).length();
		cam_dist = bsize / tan(DEG_TO_RAD(fov) / 2.0);
		printf("bounds size: %f, cam_dist: %f\n", bsize, cam_dist);
	}

	scene_model->set_scene(scene);
	treeview->expandAll();
	treeview->resizeColumnToContents(0);

	sdata = scene_model;	// set the global sdata ptr
	post_redisplay();
	return true;
}

bool GoatView::load_anim(const char *fname)
{
	if(!scene) {
		QMessageBox::critical(this, "Error", "You must load a scene before loading any animations!");
		return false;
	}

	if(goat3d_load_anim(scene, fname) == -1) {
		QMessageBox::critical(this, "Error", QString("Failed to load animation: ") + QString(fname));
		return false;
	}


	long tstart = LONG_MAX, tend = LONG_MIN;
	int num_nodes = goat3d_get_node_count(scene);
	for(int i=0; i<num_nodes; i++) {
		goat3d_node *node = goat3d_get_node(scene, i);
		if(goat3d_get_node_parent(node)) {
			continue;
		}

		long t0, t1;
		if(goat3d_get_anim_timeline(node, &t0, &t1) != -1) {
			if(t0 < tstart) tstart = t0;
			if(t1 > tend) tend = t1;
		}
	}

	if(tstart != LONG_MAX) {
		grp_anim_ctl->setDisabled(false);
		grp_anim_time->setDisabled(false);

		slider_time->setMinimum(tstart);
		slider_time->setMaximum(tend);

		spin_time->setMinimum(tstart);
		spin_time->setMaximum(tend);

		label_time_start->setText(QString::number(tstart));
		label_time_end->setText(QString::number(tend));
	}

	post_redisplay();
	return true;
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
	connect(act_quit, &QAction::triggered, [&](){ qApp->quit(); });
	menu_file->addAction(act_quit);

	// view menu
	QMenu *menu_view = menuBar()->addMenu("&View");

	QAction *act_use_nodes = new QAction("use nodes", this);
	act_use_nodes->setCheckable(true);
	act_use_nodes->setChecked(use_nodes);
	connect(act_use_nodes, &QAction::triggered, this,
			[&](){ use_nodes = !use_nodes; post_redisplay(); });
	menu_view->addAction(act_use_nodes);

	QAction *act_use_lighting = new QAction("lighting", this);
	act_use_lighting->setCheckable(true);
	act_use_lighting->setChecked(use_lighting);
	connect(act_use_lighting, &QAction::triggered, glview, &GoatViewport::toggle_lighting);
	menu_view->addAction(act_use_lighting);

	// help menu
	QMenu *menu_help = menuBar()->addMenu("&Help");

	QAction *act_about = new QAction("&About", this);
	connect(act_about, &QAction::triggered, this, &GoatView::show_about);
	menu_help->addAction(act_about);
	return true;
}

bool GoatView::make_dock()
{
	// ---- side-dock ----
	QWidget *dock_cont = new QWidget;
	QVBoxLayout *dock_vbox = new QVBoxLayout;
	dock_cont->setLayout(dock_vbox);

	QDockWidget *dock = new QDockWidget(this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	dock->setWidget(dock_cont);
	addDockWidget(Qt::LeftDockWidgetArea, dock);

	// make the tree view widget
	treeview = new QTreeView;
	treeview->setAlternatingRowColors(true);
	treeview->setSelectionMode(QAbstractItemView::SingleSelection);
	dock_vbox->addWidget(treeview);

	scene_model = new SceneModel;
	connect(scene_model, &SceneModel::dataChanged, [&](){ post_redisplay(); });
	treeview->setModel(scene_model);

	connect(treeview->selectionModel(), &QItemSelectionModel::selectionChanged,
			[&](){ scene_model->selchange(treeview->selectionModel()->selectedIndexes()); });

	// misc
	QPushButton *bn_quit = new QPushButton("quit");
	dock_vbox->addWidget(bn_quit);
	connect(bn_quit, &QPushButton::clicked, [&](){ qApp->quit(); });

	// ---- bottom dock ----
	dock_cont = new QWidget;
	QHBoxLayout *dock_hbox = new QHBoxLayout;
	dock_cont->setLayout(dock_hbox);

	// animation control box
	grp_anim_ctl = new QGroupBox("Animation controls");
	grp_anim_ctl->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
	grp_anim_ctl->setDisabled(true);
	dock_hbox->addWidget(grp_anim_ctl);

	QVBoxLayout *anim_ctl_box = new QVBoxLayout;
	grp_anim_ctl->setLayout(anim_ctl_box);

	chk_loop = new QCheckBox("loop");
	chk_loop->setChecked(false);
	anim_ctl_box->addWidget(chk_loop);

	QToolBar *toolbar_ctl = new QToolBar;
	anim_ctl_box->addWidget(toolbar_ctl);

	act_rewind = new QAction(style()->standardIcon(QStyle::SP_MediaSkipBackward), "Rewind", this);
	connect(act_rewind, &QAction::triggered, [this](){ slider_time->setValue(slider_time->minimum()); });
	toolbar_ctl->addAction(act_rewind);

	act_play = new QAction(style()->standardIcon(QStyle::SP_MediaPlay), "Play", this);
	toolbar_ctl->addAction(act_play);

	/* timeline controls
	 * /------------------------------\ grp_anim_time with vbox_timeline layout
	 * | /-------+---------+--------\ |
	 * | | label | spinbox | spacer | <-- hbox_timespin
	 * | \-------+---------+--------/ |
	 * +------------------------------+
	 * | /-------+---------+--------\ |
	 * | | label | slider  | label  | <-- hbox_timeslider
	 * | \-------+---------+--------/ |
	 * \------------------------------/
	 */
	grp_anim_time = new QGroupBox("Timeline");
	grp_anim_time->setDisabled(true);
	dock_hbox->addWidget(grp_anim_time);

	QVBoxLayout *vbox_timeline = new QVBoxLayout;
	grp_anim_time->setLayout(vbox_timeline);
	QHBoxLayout *hbox_timespin = new QHBoxLayout;
	vbox_timeline->addLayout(hbox_timespin);
	QHBoxLayout *hbox_timeslider = new QHBoxLayout;
	vbox_timeline->addLayout(hbox_timeslider);

	hbox_timespin->addWidget(new QLabel("msec"));
	spin_time = new QSpinBox;
	hbox_timespin->addWidget(spin_time);
	hbox_timespin->addStretch();

	label_time_start = new QLabel;
	hbox_timeslider->addWidget(label_time_start);

	slider_time = new QSlider(Qt::Orientation::Horizontal);
	hbox_timeslider->addWidget(slider_time);

	label_time_end = new QLabel;
	hbox_timeslider->addWidget(label_time_end);

	connect(slider_time, &QSlider::valueChanged,
		[&](){ anim_time = slider_time->value(); spin_time->setValue(anim_time); post_redisplay(); });

	typedef void (QSpinBox::*ValueChangedIntFunc)(int);
	connect(spin_time, (ValueChangedIntFunc)&QSpinBox::valueChanged,
		[&](){ anim_time = spin_time->value(); slider_time->setValue(anim_time); post_redisplay(); });

	dock = new QDockWidget(this);
	dock->setAllowedAreas(Qt::BottomDockWidgetArea);
	dock->setWidget(dock_cont);
	addDockWidget(Qt::BottomDockWidgetArea, dock);

	return true;
}

bool GoatView::make_center()
{
	glview = ::glview = new GoatViewport(this);
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
		return;
	}
	statusBar()->showMessage("Successfully loaded scene: " + QString(fname.c_str()));
}

void GoatView::open_anim()
{
	std::string fname = QFileDialog::getOpenFileName(this, "Open animation file", "",
		"Goat3D Animation (*.goatanm);;All Files (*)").toStdString();
	if(fname.empty()) {
		statusBar()->showMessage("Abot: No file selected!");
		return;
	}

	statusBar()->showMessage("opening animation file");
	if(!load_anim(fname.c_str())) {
		statusBar()->showMessage("failed to load animation file");
		return;
	}
	statusBar()->showMessage("Successfully loaded animation: " + QString(fname.c_str()));
}


// ---- OpenGL viewport ----
GoatViewport::GoatViewport(QWidget *main_win)
	: QGLWidget(QGLFormat(QGL::DepthBuffer | QGL::SampleBuffers))
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
	if(use_lighting) {
		glEnable(GL_LIGHTING);
	}
	glEnable(GL_LIGHT0);

	float ldir[] = {-1, 1, 2, 0};
	glLightfv(GL_LIGHT0, GL_POSITION, ldir);

	glEnable(GL_MULTISAMPLE);
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

	draw_grid();

	if(!scene) return;

	if(use_nodes) {
		int node_count = goat3d_get_node_count(scene);
		for(int i=0; i<node_count; i++) {
			goat3d_node *node = goat3d_get_node(scene, i);
			if(!goat3d_get_node_parent(node)) {
				draw_node(node);	// only draw root nodes, the rest will be drawn recursively
			}
		}
	} else {
		int mesh_count = goat3d_get_mesh_count(scene);
		for(int i=0; i<mesh_count; i++) {
			goat3d_mesh *mesh = goat3d_get_mesh(scene, i);
			draw_mesh(mesh);
		}
	}
}

void GoatViewport::toggle_lighting()
{
	use_lighting = !use_lighting;
	if(use_lighting) {
		glEnable(GL_LIGHTING);
	} else {
		glDisable(GL_LIGHTING);
	}
	updateGL();
}

#ifndef GLEW_ARB_transpose_matrix
#error "GLEW_ARB_transpose_matrix undefined?"
#endif

static void draw_grid()
{
	float viewsz_orig = cam_dist * tan(DEG_TO_RAD(fov) / 2.0);
	float sz1 = next_pow2((int)viewsz_orig);
	float sz0 = sz1 / 2.0;
	float t = (viewsz_orig - sz0) / (sz1 - sz0);
	float alpha = t < 0.333333 ? 0.0 : (t > 0.666666 ? 1.0 : (t - 0.333333) / 0.333333);

	glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BITS);

	glEnable(GL_BLEND);
	glDepthFunc(GL_ALWAYS);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	draw_grid(sz0 * 2.0, 10, 1.0 - alpha);
	draw_grid(sz1 * 2.0, 10, alpha);

	glPopAttrib();
}

static void draw_grid(float sz, int nlines, float alpha)
{
	float hsz = sz / 2.0;
	float offs = sz / (float)nlines;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	glLineWidth(2.0);
	glBegin(GL_LINES);
	glColor4f(1, 0, 0, alpha);
	glVertex3f(-hsz, 0, 0);
	glVertex3f(hsz, 0, 0);
	glColor4f(0, 0, 1, alpha);
	glVertex3f(0, 0, -hsz);
	glVertex3f(0, 0, hsz);
	glEnd();

	glLineWidth(1.0);
	glBegin(GL_LINES);
	glColor4f(0.5, 0.5, 0.5, alpha);
	for(int i=0; i<nlines / 2; i++) {
		float dist = (float)(i + 1) * offs;
		for(int j=0; j<2; j++) {
			float sign = j > 0 ? -1.0 : 1.0;
			glVertex3f(-hsz, 0, dist * sign);
			glVertex3f(hsz, 0, dist * sign);
			glVertex3f(dist * sign, 0, -hsz);
			glVertex3f(dist * sign, 0, hsz);
		}
	}
	glEnd();

	glPopAttrib();
}

static void draw_node(goat3d_node *node)
{
	SceneNodeData *data = sdata ? sdata->get_node_data(node) : 0;
	if(!data) return;

	float xform[16];
	goat3d_get_node_matrix(node, xform, anim_time);

	glPushMatrix();
	glMultTransposeMatrixf(xform);

	if(data->visible) {
		if(goat3d_get_node_type(node) == GOAT3D_NODE_MESH) {
			goat3d_mesh *mesh = (goat3d_mesh*)goat3d_get_node_object(node);

			draw_mesh(mesh);

			if(data->selected) {
				float bmin[3], bmax[3];
				goat3d_get_mesh_bounds(mesh, bmin, bmax);

				glPushAttrib(GL_ENABLE_BIT);
				glDisable(GL_LIGHTING);

				glColor3f(0.3, 1, 0.2);

				glBegin(GL_LINE_LOOP);
				glVertex3f(bmin[0], bmin[1], bmin[2]);
				glVertex3f(bmax[0], bmin[1], bmin[2]);
				glVertex3f(bmax[0], bmin[1], bmax[2]);
				glVertex3f(bmin[0], bmin[1], bmax[2]);
				glEnd();
				glBegin(GL_LINE_LOOP);
				glVertex3f(bmin[0], bmax[1], bmin[2]);
				glVertex3f(bmax[0], bmax[1], bmin[2]);
				glVertex3f(bmax[0], bmax[1], bmax[2]);
				glVertex3f(bmin[0], bmax[1], bmax[2]);
				glEnd();
				glBegin(GL_LINES);
				glVertex3f(bmin[0], bmin[1], bmin[2]);
				glVertex3f(bmin[0], bmax[1], bmin[2]);
				glVertex3f(bmin[0], bmin[1], bmax[2]);
				glVertex3f(bmin[0], bmax[1], bmax[2]);
				glVertex3f(bmax[0], bmin[1], bmin[2]);
				glVertex3f(bmax[0], bmax[1], bmin[2]);
				glVertex3f(bmax[0], bmin[1], bmax[2]);
				glVertex3f(bmax[0], bmax[1], bmax[2]);
				glEnd();

				glPopAttrib();
			}
		}
	}

	int num_child = goat3d_get_node_child_count(node);
	for(int i=0; i<num_child; i++) {
		draw_node(goat3d_get_node_child(node, i));
	}

	glPopMatrix();
}

static void draw_mesh(goat3d_mesh *mesh)
{
	static const float white[] = {1, 1, 1, 1};
	static const float black[] = {0, 0, 0, 1};

	const float *color;
	goat3d_material *mtl = goat3d_get_mesh_mtl(mesh);

	if(mtl && (color = goat3d_get_mtl_attrib(mtl, GOAT3D_MAT_ATTR_DIFFUSE))) {
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
		glColor3fv(color);
	} else {
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, white);
		glColor3fv(white);
	}
	if(mtl && (color = goat3d_get_mtl_attrib(mtl, GOAT3D_MAT_ATTR_SPECULAR))) {
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);
	} else {
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
	}
	if(mtl && (color = goat3d_get_mtl_attrib(mtl, GOAT3D_MAT_ATTR_SHININESS))) {
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, color[0]);
	} else {
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 60.0);
	}
	// TODO texture


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

static const char *about_str =
	"GoatView - Goat3D scene file viewer<br>"
	"Copyright (C) 2014 John Tsiombikas &lt;<a href=\"mailto:nuclear@mutantstargoat.com\">nuclear@mutantstargoat.com</a>&gt;<br>"
	"<br>"
	"This program is free software: you can redistribute it and/or modify<br>"
	"it under the terms of the GNU General Public License as published by<br>"
	"the Free Software Foundation, either version 3 of the License, or<br>"
	"(at your option) any later version.<br>"
	"<br>"
	"This program is distributed in the hope that it will be useful,<br>"
	"but WITHOUT ANY WARRANTY; without even the implied warranty of<br>"
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the<br>"
	"GNU General Public License for more details.<br>"
	"<br>"
	"You should have received a copy of the GNU General Public License<br>"
	"along with this program.  If not, see <a href=\"http://www.gnu.org/licenses/gpl\">http://www.gnu.org/licenses/gpl</a>.";

void GoatView::show_about()
{
	QMessageBox::information(this, "About GoatView", about_str);
}


static int next_pow2(int x)
{
	x--;
	x = (x >> 1) | x;
	x = (x >> 2) | x;
	x = (x >> 4) | x;
	x = (x >> 8) | x;
	x = (x >> 16) | x;
	return x + 1;
}
