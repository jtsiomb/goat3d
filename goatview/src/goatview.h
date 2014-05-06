#ifndef GOATVIEW_H_
#define GOATVIEW_H_

#include <QtWidgets/QtWidgets>
#include <QtOpenGL/QGLWidget>
#include "goat3d.h"

extern goat3d *scene;

class GoatView : public QMainWindow {
	Q_OBJECT
private:
	bool make_menu();
	bool make_dock();
	bool make_center();

private slots:
	void open_scene();
	void open_anim();

public:
	GoatView();
	~GoatView();
};

class GoatViewport : public QGLWidget {
	Q_OBJECT
public:
	GoatViewport();
	~GoatViewport();

	QSize sizeHint() const;

	void initializeGL();
	void resizeGL(int xsz, int ysz);
	void paintGL();
};

#endif	// GOATVIEW_H_
