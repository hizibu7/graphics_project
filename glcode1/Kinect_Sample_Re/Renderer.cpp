#include "Renderer.h"

void draw_center(void)
{
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f); /* R */
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.2f, 0.0f, 0.0f);
	glEnd();
	glRasterPos3f(0.2f, 0.0f, 0.0f);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'x');

	glBegin(GL_LINES);
	glColor3f(0.0f, 1.0f, 0.0f); /* G */
	glVertex3f(0.0f, 0.2f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glEnd();
	glRasterPos3f(0.0f, 0.2f, 0.0f);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'y');

	glBegin(GL_LINES);
	glColor3f(0.0f, 0.0f, 1.0f); /* B */
	glVertex3f(0.0f, 0.0f, -0.2f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glEnd();
	glRasterPos3f(0.0f, 0.0f, -0.2f);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'z');

	glBegin(GL_LINES);
	glColor3f(1.0f, 1.0f, 0.0f); /* Y */
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.2f);
	glEnd();
	glRasterPos3f(0.0f, 0.0f, 0.2f);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'w');

	glBegin(GL_LINES);
	glColor3f(0.5f, 0.5f, 0.5f); /* Gray */
	for (float i = -1.0f; i <= 1.0f; i += 0.2f)
	{
		glVertex3f(i, 0.0f, 0.0f);
		glVertex3f(0.0f, i, 0.0f);
		glVertex3f(0.0f, 0.0f, i);
	}
	glEnd();
}

void idle() {
	static GLuint previousClock = glutGet(GLUT_ELAPSED_TIME);
	static GLuint currentClock = glutGet(GLUT_ELAPSED_TIME);
	static GLfloat deltaT;

	currentClock = glutGet(GLUT_ELAPSED_TIME);
	deltaT = currentClock - previousClock;
	if (deltaT < 1000.0 / 20.0) { return; }
	else { previousClock = currentClock; }

	glutPostRedisplay();
}

void close()
{
	glDeleteTextures(1, &dispBindIndex);
	glutLeaveMainLoop();
	CloseHandle(hMutex);
}

void add_quats(float q1[4], float q2[4], float dest[4])
{
	static int count = 0;
	float t1[4], t2[4], t3[4];
	float tf[4];

	vcopy(q1, t1);
	vscale(t1, q2[3]);

	vcopy(q2, t2);
	vscale(t2, q1[3]);

	vcross(q2, q1, t3);
	vadd(t1, t2, tf);
	vadd(t3, tf, tf);
	tf[3] = q1[3] * q2[3] - vdot(q1, q2);

	dest[0] = tf[0];
	dest[1] = tf[1];
	dest[2] = tf[2];
	dest[3] = tf[3];

	if (++count > RENORMCOUNT) {
		count = 0;
		normalize_quat(dest);
	}
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(58, (double)width / height, 0.1, 100);
	glMatrixMode(GL_MODELVIEW);
}

void motion(int x, int y)
{
	GLfloat spin_quat[4];
	float gain;
	gain = 2.0;

	if (drag_state == GLUT_DOWN)
	{
		if (button_state == GLUT_LEFT_BUTTON)
		{
			trackball(spin_quat,
				(gain * rot_x - 500) / 500,
				(500 - gain * rot_y) / 500,
				(gain * x - 500) / 500,
				(500 - gain * y) / 500);
			add_quats(spin_quat, quat, quat);
		}
		else if (button_state == GLUT_RIGHT_BUTTON)
		{
			t[0] -= (((float)trans_x - x) / 500);
			t[1] += (((float)trans_y - y) / 500);
		}
		else if (button_state == GLUT_MIDDLE_BUTTON)
			t[2] -= (((float)trans_z - y) / 500 * 4);
		else if (button_state == 3 || button_state == 4)
		{

		}

	}

	rot_x = x;
	rot_y = y;

	trans_x = x;
	trans_y = y;
	trans_z = y;
}

void mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		if (button == GLUT_LEFT_BUTTON)
		{
			rot_x = x;
			rot_y = y;



		}
		else if (button == GLUT_RIGHT_BUTTON)
		{
			trans_x = x;
			trans_y = y;
		}
		else if (button == GLUT_MIDDLE_BUTTON)
		{
			trans_z = y;
		}
		else if (button == 3 || button == 4)
		{
			const float sign = (static_cast<float>(button)-3.5f) * 2.0f;
			t[2] -= sign * 500 * 0.00015f;
		}
	}

	drag_state = state;
	button_state = button;
}

void vzero(float* v)
{
	v[0] = 0.0f;
	v[1] = 0.0f;
	v[2] = 0.0f;
}

void vset(float* v, float x, float y, float z)
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

void vsub(const float *src1, const float *src2, float *dst)
{
	dst[0] = src1[0] - src2[0];
	dst[1] = src1[1] - src2[1];
	dst[2] = src1[2] - src2[2];
}

void vcopy(const float *v1, float *v2)
{
	register int i;
	for (i = 0; i < 3; i++)
		v2[i] = v1[i];
}

void vcross(const float *v1, const float *v2, float *cross)
{
	float temp[3];

	temp[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
	temp[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
	temp[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
	vcopy(temp, cross);
}

float vlength(const float *v)
{
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

void vscale(float *v, float div)
{
	v[0] *= div;
	v[1] *= div;
	v[2] *= div;
}

void vnormal(float *v)
{
	vscale(v, 1.0f / vlength(v));
}

float vdot(const float *v1, const float *v2)
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

void vadd(const float *src1, const float *src2, float *dst)
{
	dst[0] = src1[0] + src2[0];
	dst[1] = src1[1] + src2[1];
	dst[2] = src1[2] + src2[2];
}

void trackball(float q[4], float p1x, float p1y, float p2x, float p2y)
{
	float a[3]; 
	float phi;  
	float p1[3], p2[3], d[3];
	float t;

	if (p1x == p2x && p1y == p2y) {
		vzero(q);
		q[3] = 1.0;
		return;
	}

	vset(p1, p1x, p1y, tb_project_to_sphere(TRACKBALLSIZE, p1x, p1y));
	vset(p2, p2x, p2y, tb_project_to_sphere(TRACKBALLSIZE, p2x, p2y));

	vcross(p2, p1, a);

	vsub(p1, p2, d);
	t = vlength(d) / (2.0f*TRACKBALLSIZE);

	if (t > 1.0) t = 1.0;
	if (t < -1.0) t = -1.0;
	phi = 2.0f * asin(t);

	axis_to_quat(a, phi, q);
}

void axis_to_quat(float a[3], float phi, float q[4])
{
	vnormal(a);
	vcopy(a, q);
	vscale(q, sin(phi / 2.0f));
	q[3] = cos(phi / 2.0f);
}

float tb_project_to_sphere(float r, float x, float y)
{
	float d, t, z;

	d = sqrt(x*x + y*y);
	if (d < r * 0.70710678118654752440f) {   
		z = sqrt(r*r - d*d);
	}
	else {         
		t = r / 1.41421356237309504880f;
		z = t*t / d;
	}
	return z;
}

void normalize_quat(float q[4])
{
	int i;
	float mag;

	mag = (q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
	for (i = 0; i < 4; i++) q[i] /= mag;
}

void build_rotmatrix(float m[4][4], float q[4])
{
	m[0][0] = 1.0f - 2.0f * (q[1] * q[1] + q[2] * q[2]);
	m[0][1] = 2.0f * (q[0] * q[1] - q[2] * q[3]);
	m[0][2] = 2.0f * (q[2] * q[0] + q[1] * q[3]);
	m[0][3] = 0.0f;

	m[1][0] = 2.0f * (q[0] * q[1] + q[2] * q[3]);
	m[1][1] = 1.0f - 2.0f * (q[2] * q[2] + q[0] * q[0]);
	m[1][2] = 2.0f * (q[1] * q[2] - q[0] * q[3]);
	m[1][3] = 0.0f;

	m[2][0] = 2.0f * (q[2] * q[0] - q[1] * q[3]);
	m[2][1] = 2.0f * (q[1] * q[2] + q[0] * q[3]);
	m[2][2] = 1.0f - 2.0f * (q[1] * q[1] + q[0] * q[0]);
	m[2][3] = 0.0f;

	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;
}

void InitializeWindow(int argc, char* argv[])
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowSize(1000 / 2, 1000 / 2);

	glutInitWindowPosition(0, 0);

	dispWindowIndex = glutCreateWindow("3D Model");

	trackball(quat, 90.0, 0.0, 0.0, 0.0);

	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutMotionFunc(motion);
	glutMouseFunc(mouse);
	glutCloseFunc(close);
	

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);

	reshape(1000, 1000);

}


void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 1, 0.1, 200);		
	glTranslatef(t[0], t[1], t[2] - 1.0f);
	glScalef(1, 1, 1);	
	GLfloat m[4][4],m1[4][4];
	build_rotmatrix(m, quat);
	gluLookAt(0, 0.2, 2.0, 0, 0, 0, 0, 1.0, 0);

	GLfloat r, g, b;
	glMultMatrixf(&m[0][0]);


	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	GLfloat diffuse0[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat ambient0[4] = { 0.1, 0.1, 0.1, 1.0 };
	GLfloat specular0[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light0_pos[4] = { 0.3, 0.3, 0.5, 1.0 };
	GLfloat spot_dir[3] = { 1.0f, 0.0f, -1.0f };

	glShadeModel(GL_SMOOTH);

	glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular0);


	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.2);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.1);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.05);

	glTexImage2D(GL_TEXTURE_2D, 0, 3, 276, 276, 0, GL_RGB, GL_UNSIGNED_BYTE, mytexels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	
	glNormal3f(0, 1, 0);
	for (float fl = -1.0; fl < 1.0; fl = fl + 0.2)
	{
		for (float fr = -1.0; fr < 1.0; fr = fr + 0.2)
		{
			glTexCoord2d(0.0, 0.0);
			glVertex3f(-0.2 + fl, 0.0, -0.2 + fr);
			glTexCoord2d(1.0, 0.0);
			glVertex3f(-0.2 + fl, 0.0, 0.2 + fr);
			glTexCoord2d(1.0, 1.0);
			glVertex3f(0.2 + fl, 0.0, 0.2 + fr);
			glTexCoord2d(0.0, 1.0);
			glVertex3f(0.2 + fl, 0.0, -0.2 + fr);
		}

	}
	glEnd();

	stone.display(0.05, 0, 0.5, 0, 180);
	player.display(0.05, 0.015, -0.7, 0, 0);
	cat.display(-0.1, 0, -0.9, -90, 0);
	dog.display(0.2, 0, -1, -90, 0);
	sword.display(0.05, 0.3, -0.4, 0, spinner_angle);
	knight.display(0.05, 0, 1, 0, 180);
	//monster.display(1, 0, 1, 0, 180);


	spinner_angle += 5;
	if (spinner_angle > 360)
		spinner_angle -= 360;
	
	glutSwapBuffers();
}


void Object3D::parse0(const char* objName, float model_scale)
{
	char header[128];

	FILE* file = fopen(objName, "r");
	if (file == nullptr)
		throw exception();

	while (fscanf(file, "%s", header) != EOF)
	{
		if (strcmp(header, "v") == 0)
		{
			vertices.emplace_back();
			fscanf(file, "%f %f %f\n", &vertices.back().X, &vertices.back().Y, &vertices.back().Z);
			vertices.back().X *= model_scale;
			vertices.back().Y *= model_scale;
			vertices.back().Z *= model_scale;
		}
		else if (strcmp(header, "vt") == 0)
		{
			uvs.emplace_back();
			fscanf(file, "%f %f\n", &uvs.back().X, &uvs.back().Y);
		}
		else if (strcmp(header, "vn") == 0)
		{
			normals.emplace_back();
			fscanf(file, "%f %f %f\n", &normals.back().X, &normals.back().Y, &normals.back().Z);
		}
		else if (strcmp(header, "f") == 0)
		{
			mesh.emplace_back();
			if (fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n",
				&mesh.back().V1, &mesh.back().T1, &mesh.back().N1,
				&mesh.back().V2, &mesh.back().T2, &mesh.back().N2,
				&mesh.back().V3, &mesh.back().T3, &mesh.back().N3,
				&mesh.back().V4, &mesh.back().T4, &mesh.back().N4) != 12)

			if (mesh.back().V1 < 0)
			{
				mesh.back().V1 = vertices.size() + mesh.back().V1 + 1;
				mesh.back().V2 = vertices.size() + mesh.back().V2 + 1;
				mesh.back().V3 = vertices.size() + mesh.back().V3 + 1;
				mesh.back().T1 = uvs.size() + mesh.back().T1 + 1;
				mesh.back().T2 = uvs.size() + mesh.back().T2 + 1;
				mesh.back().T3 = uvs.size() + mesh.back().T3 + 1;
				mesh.back().N1 = normals.size() + mesh.back().N1 + 1;
				mesh.back().N2 = normals.size() + mesh.back().N2 + 1;
				mesh.back().N3 = normals.size() + mesh.back().N3 + 1;

				if (mesh.back().V4 != 0)
				{
					mesh.back().V4 = vertices.size() + mesh.back().V4 + 1;
					mesh.back().T4 = uvs.size() + mesh.back().T4 + 1;
					mesh.back().N4 = normals.size() + mesh.back().N4 + 1;
				}
			}
		}
	}

	fclose(file);
}

void Object3D::parse(const char* objName, const char* texName, float model_scale)
{
	char header[128];

	FILE* file = fopen(objName, "r");
	if (file == nullptr)
		throw exception();
	
	while (fscanf(file, "%s", header) != EOF)
	{
		if (strcmp(header, "v") == 0)
		{
			vertices.emplace_back();
			fscanf(file, "%f %f %f\n", &vertices.back().X, &vertices.back().Y, &vertices.back().Z);
			vertices.back().X *= model_scale;
			vertices.back().Y *= model_scale;
			vertices.back().Z *= model_scale;
		}
		else if (strcmp(header, "vt") == 0)
		{
			uvs.emplace_back();
			fscanf(file, "%f %f\n", &uvs.back().X, &uvs.back().Y);
		}
		else if (strcmp(header, "vn") == 0)
		{
			normals.emplace_back();
			fscanf(file, "%f %f %f\n", &normals.back().X, &normals.back().Y, &normals.back().Z);
		}
		else if (strcmp(header, "f") == 0)
		{
			mesh.emplace_back();
			if (fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n",
				&mesh.back().V1, &mesh.back().T1, &mesh.back().N1,
				&mesh.back().V2, &mesh.back().T2, &mesh.back().N2,
				&mesh.back().V3, &mesh.back().T3, &mesh.back().N3,
				&mesh.back().V4, &mesh.back().T4, &mesh.back().N4) != 12)

			if (mesh.back().V1 < 0)
			{
				mesh.back().V1 = vertices.size() + mesh.back().V1 + 1;
				mesh.back().V2 = vertices.size() + mesh.back().V2 + 1;
				mesh.back().V3 = vertices.size() + mesh.back().V3 + 1;
				mesh.back().T1 = uvs.size() + mesh.back().T1 + 1;
				mesh.back().T2 = uvs.size() + mesh.back().T2 + 1;
				mesh.back().T3 = uvs.size() + mesh.back().T3 + 1;
				mesh.back().N1 = normals.size() + mesh.back().N1 + 1;
				mesh.back().N2 = normals.size() + mesh.back().N2 + 1;
				mesh.back().N3 = normals.size() + mesh.back().N3 + 1;
				if (mesh.back().V4 != 0)
				{
					mesh.back().V4 = vertices.size() + mesh.back().V4 + 1;
					mesh.back().T4 = uvs.size() + mesh.back().T4 + 1;
					mesh.back().N4 = normals.size() + mesh.back().N4 + 1;
				}
			}
		}
	}

	fclose(file);

	FILE* file2 = fopen(texName, "r");
	if (file2 == nullptr)
		throw "file not exists";

	unsigned char info[54];
	fread(info, sizeof(unsigned char), 54, file2);
	tex_width = *(int*)&info[18];
	tex_height = *(int*)&info[22];
	tex = new unsigned char[tex_width * tex_height * 3];

	fread(tex, sizeof(unsigned char), tex_width * tex_height * 3, file2);
	fclose(file2);

	for (int i = 0; i < tex_width * tex_height * 3; i += 3)
		swap(tex[i], tex[i + 2]);
}


void Object3D::display(float x, float y, float z, float angle_v, float angle_h)
{
	GLfloat mat_ambient[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat mat_diffuse[4] = { 0.6f, 0.6f, 0.6f, 1.0f };
	GLfloat mat_specular[4] = { 0.6f, 0.6f, 0.6f, 1.0f };
	GLfloat mat_shininess = 32.0;

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);


	glTexImage2D(GL_TEXTURE_2D, 0, 3, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_TEXTURE_2D);

	glPushMatrix();
	glTranslatef(x, y, z);
	glRotatef(angle_v, 1, 0, 0);
	glRotatef(angle_h, 0, 1, 0);

	glBegin(GL_TRIANGLES);
	for (int jj = 0; jj < mesh.size(); jj = jj + 1)
	{
		if (mesh[jj].T4 == 0)
		{
			glNormal3f(normals[mesh[jj].N1 - 1].X, normals[mesh[jj].N1 - 1].Y, normals[mesh[jj].N1 - 1].Z);
			glTexCoord2d(uvs[mesh[jj].T1 - 1].X, uvs[mesh[jj].T1 - 1].Y);
			glVertex3f(vertices[mesh[jj].V1 - 1].X, vertices[mesh[jj].V1 - 1].Y, vertices[mesh[jj].V1 - 1].Z);
			glNormal3f(normals[mesh[jj].N2 - 1].X, normals[mesh[jj].N2 - 1].Y, normals[mesh[jj].N2 - 1].Z);
			glTexCoord2d(uvs[mesh[jj].T2 - 1].X, uvs[mesh[jj].T2 - 1].Y);
			glVertex3f(vertices[mesh[jj].V2 - 1].X, vertices[mesh[jj].V2 - 1].Y, vertices[mesh[jj].V2 - 1].Z);
			glNormal3f(normals[mesh[jj].N3 - 1].X, normals[mesh[jj].N3 - 1].Y, normals[mesh[jj].N3 - 1].Z);
			glTexCoord2d(uvs[mesh[jj].T3 - 1].X, uvs[mesh[jj].T3 - 1].Y);
			glVertex3f(vertices[mesh[jj].V3 - 1].X, vertices[mesh[jj].V3 - 1].Y, vertices[mesh[jj].V3 - 1].Z);
		}
		else
		{
			glNormal3f(normals[mesh[jj].N1 - 1].X, normals[mesh[jj].N1 - 1].Y, normals[mesh[jj].N1 - 1].Z);
			glTexCoord2d(uvs[mesh[jj].T1 - 1].X, uvs[mesh[jj].T1 - 1].Y);
			glVertex3f(vertices[mesh[jj].V1 - 1].X, vertices[mesh[jj].V1 - 1].Y, vertices[mesh[jj].V1 - 1].Z);
			glNormal3f(normals[mesh[jj].N2 - 1].X, normals[mesh[jj].N2 - 1].Y, normals[mesh[jj].N2 - 1].Z);
			glTexCoord2d(uvs[mesh[jj].T2 - 1].X, uvs[mesh[jj].T2 - 1].Y);
			glVertex3f(vertices[mesh[jj].V2 - 1].X, vertices[mesh[jj].V2 - 1].Y, vertices[mesh[jj].V2 - 1].Z);
			glNormal3f(normals[mesh[jj].N3 - 1].X, normals[mesh[jj].N3 - 1].Y, normals[mesh[jj].N3 - 1].Z);
			glTexCoord2d(uvs[mesh[jj].T3 - 1].X, uvs[mesh[jj].T3 - 1].Y);
			glVertex3f(vertices[mesh[jj].V3 - 1].X, vertices[mesh[jj].V3 - 1].Y, vertices[mesh[jj].V3 - 1].Z);

			glNormal3f(normals[mesh[jj].N1 - 1].X, normals[mesh[jj].N1 - 1].Y, normals[mesh[jj].N1 - 1].Z);
			glTexCoord2d(uvs[mesh[jj].T1 - 1].X, uvs[mesh[jj].T1 - 1].Y);
			glVertex3f(vertices[mesh[jj].V1 - 1].X, vertices[mesh[jj].V1 - 1].Y, vertices[mesh[jj].V1 - 1].Z);
			glNormal3f(normals[mesh[jj].N3 - 1].X, normals[mesh[jj].N3 - 1].Y, normals[mesh[jj].N3 - 1].Z);
			glTexCoord2d(uvs[mesh[jj].T3 - 1].X, uvs[mesh[jj].T3 - 1].Y);
			glVertex3f(vertices[mesh[jj].V3 - 1].X, vertices[mesh[jj].V3 - 1].Y, vertices[mesh[jj].V3 - 1].Z);
			glNormal3f(normals[mesh[jj].N4 - 1].X, normals[mesh[jj].N4 - 1].Y, normals[mesh[jj].N4 - 1].Z);
			glTexCoord2d(uvs[mesh[jj].T4 - 1].X, uvs[mesh[jj].T4 - 1].Y);
			glVertex3f(vertices[mesh[jj].V4 - 1].X, vertices[mesh[jj].V4 - 1].Y, vertices[mesh[jj].V4 - 1].Z);
		}
	}
	glEnd();

	glPopMatrix();
}

int main(int argc, char* argv[])
{
	vertex = new Vertex[100000];
	vertex_color = new Vertex[100000];
	//mymesh = new Meshmodel[100000];
	
	int i,j,k=0;
	FILE* f = fopen("diffuso.bmp", "rb");
	unsigned char info[54];
	fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header
	int width = *(int*)&info[18];
	int height = *(int*)&info[22];
	int size = 3 * width * height;
	unsigned char* data = new unsigned char[size]; // allocate 3 bytes per pixel
	fread(data, sizeof(unsigned char), size, f); // read the rest of the data at once
	fclose(f);
	for (i = 0; i < width; i ++)
		for (j = 0; j < height; j ++)
		{
			mytexels[j][i][0] = data[k * 3 + 2];
			mytexels[j][i][1] = data[k * 3 + 1];
			mytexels[j][i][2] = data[k * 3];
			k++;
		}

	sword.parse("Sting-Sword-lowpoly.obj", "Sting_Base_Color.bmp", 0.005);
	stone.parse("Stone.obj", "rough.bmp", 0.1);
	player.parse0("girl OBJ.obj", 0.3);
	cat.parse("12221_Cat_v1_l3.obj", "Cat_diffuse.bmp", 0.005);
	dog.parse("13463_Australian_Cattle_Dog_v3.obj", "Australian_Cattle_Dog_dif.bmp", 0.01);
	knight.parse("knight.obj", "armor.bmp", 0.3);
	//monster.parse0("Alien Animal.obj", 0.1);
	InitializeWindow(argc, argv);

	display();

	glutMainLoop();
	delete[] vertex;
	delete[] vertex_color;
	return 0;
}