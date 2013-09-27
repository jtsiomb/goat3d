#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#ifndef __APPLE__
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#endif
#include "goat3d.h"

static void cleanup(void);
static void disp(void);
static void draw_scene(struct goat3d *g);
static void draw_mesh(struct goat3d_mesh *mesh);
static void reshape(int x, int y);
static void keyb(unsigned char key, int x, int y);
static void mouse(int bn, int st, int x, int y);
static void motion(int x, int y);

static struct goat3d *goat;
static float cam_theta, cam_phi, cam_dist = 10;

int main(int argc, char **argv)
{
	glutInitWindowSize(800, 600);
	glutInit(&argc, argv);

	if(!argv[1]) {
		fprintf(stderr, "you must specify a goat3d scene file to open\n");
		return 1;
	}

	if(!(goat = goat3d_create())) {
		fprintf(stderr, "failed to create goat3d\n");
		return 1;
	}
	if(goat3d_load(goat, argv[1]) == -1) {
		fprintf(stderr, "failed to load goat3d scene: %s\n", argv[1]);
		goat3d_free(goat);
		return 1;
	}

	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutCreateWindow(argv[1]);

	glutDisplayFunc(disp);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyb);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glClearColor(0.1, 0.1, 0.1, 1.0);

	atexit(cleanup);

	glutMainLoop();
	return 0;
}

static void cleanup(void)
{
	goat3d_free(goat);
}

static void disp(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -cam_dist);
	glRotatef(cam_phi, 1, 0, 0);
	glRotatef(cam_theta, 0, 1, 0);

	draw_scene(goat);

	glutSwapBuffers();
	assert(glGetError() == GL_NO_ERROR);
}

static void draw_scene(struct goat3d *g)
{
	int i, num_meshes;

	num_meshes = goat3d_get_mesh_count(g);
	for(i=0; i<num_meshes; i++) {
		struct goat3d_mesh *mesh = goat3d_get_mesh(g, i);
		draw_mesh(mesh);
	}
}

static void draw_mesh(struct goat3d_mesh *mesh)
{
	int vnum, fnum;
	struct goat3d_material *mtl;
	float *verts, *normals, *texcoords;
	int *vidx;

	if((mtl = goat3d_get_mesh_mtl(mesh))) {
		/* TODO */
	}

	vnum = goat3d_get_mesh_attrib_count(mesh, GOAT3D_MESH_ATTR_VERTEX);
	fnum = goat3d_get_mesh_face_count(mesh);

	if(!vnum || !fnum) {
		return;
	}

	verts = goat3d_get_mesh_attribs(mesh, GOAT3D_MESH_ATTR_VERTEX);
	normals = goat3d_get_mesh_attribs(mesh, GOAT3D_MESH_ATTR_NORMAL);
	texcoords = goat3d_get_mesh_attribs(mesh, GOAT3D_MESH_ATTR_TEXCOORD);
	vidx = goat3d_get_mesh_faces(mesh);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, verts);

	if(normals) {
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, normals);
	}
	if(texcoords) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
	}

	glDrawElements(GL_TRIANGLES, fnum * 3, GL_UNSIGNED_INT, vidx);

	glDisableClientState(GL_VERTEX_ARRAY);
	if(normals) {
		glDisableClientState(GL_NORMAL_ARRAY);
	}
	if(texcoords) {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
}

static void reshape(int x, int y)
{
	glViewport(0, 0, x, y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(50.0, (float)x / (float)y, 0.5, 1000.0);
}

static void keyb(unsigned char key, int x, int y)
{
	switch(key) {
	case 27:
		exit(0);
	}
}

static int bnstate[32];
static int prev_x, prev_y;

static void mouse(int bn, int st, int x, int y)
{
	bnstate[bn - GLUT_LEFT_BUTTON] = (st == GLUT_DOWN);
	prev_x = x;
	prev_y = y;
}

static void motion(int x, int y)
{
	int dx = x - prev_x;
	int dy = y - prev_y;
	prev_x = x;
	prev_y = y;

	if(bnstate[0]) {
		cam_theta += dx * 0.5;
		cam_phi += dy * 0.5;

		if(cam_phi < -90) cam_phi = -90;
		if(cam_phi > 90) cam_phi = 90;
		glutPostRedisplay();
	}
	if(bnstate[2]) {
		cam_dist += dy * 0.1;

		if(cam_dist < 0) cam_dist = 0;
		glutPostRedisplay();
	}
}
