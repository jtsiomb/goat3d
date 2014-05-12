#ifndef GOATVIEW_H_
#define GOATVIEW_H_

#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>
#include <QtOpenGL/QGLWidget>
#include "goat3d.h"

extern goat3d *scene;

class GoatViewport;

class GoatView : public QMainWindow {
private:
	Q_OBJECT

	GoatViewport *glview;
	QTreeWidget *scntree;

	void closeEvent(QCloseEvent *ev);
	bool make_menu();
	bool make_dock();
	bool make_center();

private slots:
	void open_scene();
	void open_anim();

public:
	GoatView();
	~GoatView();

	bool load_scene(const char *fname);

	void show_about();
};

class GoatViewport : public QGLWidget {
private:
	Q_OBJECT

	QWidget *main_win;
	bool initialized;

public:
	GoatViewport(QWidget *main_win);
	~GoatViewport();

	QSize sizeHint() const;

	void initializeGL();
	void resizeGL(int xsz, int ysz);
	void paintGL();

	void toggle_lighting();

	void mousePressEvent(QMouseEvent *ev);
	void mouseMoveEvent(QMouseEvent *ev);
};

#endif	// GOATVIEW_H_
