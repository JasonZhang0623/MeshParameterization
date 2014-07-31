#include "gl.hh"
#include "GlutViewer.hh"

// -----------
// static data component
GlutViewer* GlutViewer::current_viewer_ = NULL;

// -----------
// constructor and destructor
GlutViewer::GlutViewer(const char* _title, int _width, int _height)
: width_(_width), height_(_height), fullscreen_(false), title_(_title)
{
	current_viewer_ = this;

	// init mouse buttons
	for (int i = 0; i < 5; ++i)
		button_down_[i] = false;

	center_ = Vec3f(0.0, 0.0, 0.0);
	radius_ = 1;

	// projection parameter
	near_ = 0.01 * radius_;
	far_  = 10.0 * radius_;
	fovy_ = 45.0; 

	// menu related variables
	n_draw_modes_ = 0;
	draw_mode_ = 0;
}
  
GlutViewer::~GlutViewer()
{
  glutDestroyWindow(windowID_);
  glutDestroyMenu(menuID_);
}

// -----------
// public interface
void GlutViewer::setup(void)
{
	setup_glut();
	setup_view();
	setup_scene(Vec3f(0.0, 0.0, 0.0), 1);
	setup_menu();
}

// -----------
// setup 
void GlutViewer::setup_glut(void)
{
	// create window
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE | GLUT_ALPHA);
	glutInitWindowSize(width_, height_);
	windowID_ = glutCreateWindow(title_.c_str());

	// register callbacks
	glutDisplayFunc(display__);
	glutKeyboardFunc(keyboard__);
	glutSpecialFunc(special__);
	glutMouseFunc(mouse__);
	glutMotionFunc(motion__);
	glutPassiveMotionFunc(passivemotion__);
	glutReshapeFunc(reshape__);
	glutVisibilityFunc(visibility__);
}

void GlutViewer::setup_menu(void)
{
	// create menu
	n_draw_modes_ = 0;
	menuID_ = glutCreateMenu(processmenu__);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	
	// draw mode
	add_draw_mode("Wireframe");
	add_draw_mode("Hidden Line");
	add_draw_mode("Solid Flat");
	add_draw_mode("Solid Smooth");
	set_draw_mode(3);
}

void GlutViewer::setup_view(void)
{
	// OpenGL state
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glDisable(GL_DITHER);
	glEnable(GL_DEPTH_TEST);

	// some performance settings
	//   glEnable( GL_CULL_FACE );
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);


	// material
	GLfloat mat_a[] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat mat_d[] = { 0.4, 0.4, 0.4, 1.0 };
	GLfloat mat_s[] = { 0.8, 0.8, 0.8, 1.0 };
	GLfloat shine[] = { 128.0 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_a);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_d);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_s);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine);

	// lighting
	glLoadIdentity();

	GLfloat pos1[] = { 0.1, 0.1, -0.02, 0.0 };
	GLfloat pos2[] = { -0.1, 0.1, -0.02, 0.0 };
	GLfloat pos3[] = { 0.0, 0.0, 0.1, 0.0 };
	GLfloat col1[] = { .05, .05, .6, 1.0 };
	GLfloat col2[] = { .6, .05, .05, 1.0 };
	GLfloat col3[] = { 1.0, 1.0, 1.0, 1.0 };

	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_POSITION, pos1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, col1);
	glLightfv(GL_LIGHT0, GL_SPECULAR, col1);

	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT1, GL_POSITION, pos2);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, col2);
	glLightfv(GL_LIGHT1, GL_SPECULAR, col2);

	glEnable(GL_LIGHT2);
	glLightfv(GL_LIGHT2, GL_POSITION, pos3);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, col3);
	glLightfv(GL_LIGHT2, GL_SPECULAR, col3);
}

void GlutViewer::setup_scene( const Vec3f& _cog, float _radius )
{
	center_ = _cog;
	radius_ = _radius;

	near_  = 0.01 * radius_;
	far_   = 10.0 * radius_;

	update_projection_matrix();

	// scene pos and size
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// view all
	glTranslated(-center_[0], -center_[1], -center_[2]);
	glTranslated(0, 0, -3*radius_);
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix_);
}

// -----------
// reshape, need to update projection matrix
void GlutViewer::reshape(int _w, int _h)
{
	width_  = _w; 
	height_ = _h;
	glViewport(0, 0, _w, _h);
	update_projection_matrix();
	glutPostRedisplay();
}

void GlutViewer::update_projection_matrix()
{
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective(fovy_, (GLfloat)width_/(GLfloat)height_, near_, far_);
	glGetDoublev( GL_PROJECTION_MATRIX, projection_matrix_);
	glMatrixMode( GL_MODELVIEW );
}

// -----------
// draw the sceen
void GlutViewer::draw(const std::string& _draw_mode)
{
	if (_draw_mode == "Wireframe")
	{
		glDisable(GL_LIGHTING);
		glutWireTeapot(0.5);
	}
	else if (_draw_mode == "Solid Flat")
	{
		glEnable(GL_LIGHTING);
		glShadeModel(GL_FLAT);
		glutSolidTeapot(0.5);
	}
	else if (_draw_mode == "Solid Smooth")
	{
		glEnable(GL_LIGHTING);
		glShadeModel(GL_SMOOTH);
		glutSolidTeapot(0.5);
	}
	else if (_draw_mode == "Hidden Line")
	{
		glDisable(GL_LIGHTING);
		glShadeModel(GL_SMOOTH);
		glColor3f(0.0, 0.0, 0.0);

		glDepthRange(0.01, 1.0);
		glutSolidTeapot(0.5);

		glColor3f(1.0, 1.0, 1.0);
		glDepthRange(0.0, 1.0);
		glutWireTeapot(0.5);
	} 
	else 
	{
		std::cout << "This view mode is not supported for this "
			<< "geometry, you need to load a mesh!" << std::endl;
	}
}

void GlutViewer::display(void) 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (draw_mode_names_.size() <= 0) return;
	else if (draw_mode_ < draw_mode_names_.size())
		draw(draw_mode_names_[draw_mode_]);
	else
		draw("");

	glutSwapBuffers();
}

// -----------
// mouse and motion, to zoom, translate and rotate the scene
void GlutViewer::mouse(int button, int state, int x, int y)
{
	// mouse press
	if (state == GLUT_DOWN)
	{
		last_point_2D_ = Vec2i(x,y);
		last_point_ok_ = map_to_sphere( last_point_2D_, last_point_3D_ );
		button_down_[button] = true;
	}
	// mouse release
	else
	{
		last_point_ok_ = false;
		button_down_[button] = false;
	}

	glutPostRedisplay();
}

void GlutViewer::motion(int x, int y)
{
	if (button_down_[0] && button_down_[1])
	{
		translation(x, y);
	}
	else if (button_down_[0])  // left button
	{
		rotation(x, y);
	}
	else if (button_down_[1])  // middle button
	{
		zoom(x, y);
	}

	// remeber points
	last_point_2D_ = Vec2i(x, y);
	last_point_ok_ = map_to_sphere(last_point_2D_, last_point_3D_);

	glutPostRedisplay();
}

// virtual trackball: map 2D screen point to unit sphere
bool GlutViewer::map_to_sphere( const Vec2i& _v2D, Vec3f& _v3D )
{
	if ( (_v2D[0] >= 0) && (_v2D[0] <= width_) &&
		(_v2D[1] >= 0) && (_v2D[1] <= height_) ) 
	{
		double x  = (double)(_v2D[0] - 0.5*width_)  / (double)width_;
		double y  = (double)(0.5*height_ - _v2D[1]) / (double)height_;
		double sinx = sin(M_PI * x * 0.5);
		double siny = sin(M_PI * y * 0.5);
		double sinx2siny2 = sinx * sinx + siny * siny;

		_v3D[0] = sinx;
		_v3D[1] = siny;
		_v3D[2] = sinx2siny2 < 1.0 ? sqrt(1.0 - sinx2siny2) : 0.0;

		return true;
	}
	else return false;
}

void GlutViewer::rotation(int x, int y)
{
	if (last_point_ok_) 
	{
		Vec2i  new_point_2D;
		Vec3f  new_point_3D;
		bool   new_point_ok;

		new_point_2D = Vec2i(x, y);
		new_point_ok = map_to_sphere(new_point_2D, new_point_3D);

		if (new_point_ok)
		{
			Vec3f axis      = (last_point_3D_ % new_point_3D); // cross product
			float cos_angle = (last_point_3D_ | new_point_3D); // dot product

			if (fabs(cos_angle) < 1.0) 
			{
				float angle = 2.0*acos(cos_angle) * 180.0 / M_PI;
				rotate(axis, angle);
			}
		}
	}
}

void GlutViewer::translation(int x, int y)
{
	float dx = x - last_point_2D_[0];
	float dy = y - last_point_2D_[1];

	float z = - (modelview_matrix_[ 2]*center_[0] + 
		modelview_matrix_[ 6]*center_[1] + 
		modelview_matrix_[10]*center_[2] + 
		modelview_matrix_[14]);

	float aspect = (float)width_ / (float)height_;
	float up     = tan(fovy_/2.0f*M_PI/180.f) * near_;
	float right  = aspect*up;

	translate(Vec3f(2.0*dx/width_*right/near_*z, 
		-2.0*dy/height_*up/near_*z, 
		0.0f));
}

void GlutViewer::zoom(int x, int y)
{
	float dy = y - last_point_2D_[1];
	float h  = height_;
	translate(Vec3f(0.0, 0.0, radius_ * dy * 3.0 / h));
}

// translate the scene and update modelview matrix
void GlutViewer::translate( const Vec3f& _trans )
{
	glLoadIdentity();
	glTranslated( _trans[0], _trans[1], _trans[2] );
	glMultMatrixd( modelview_matrix_ );
	glGetDoublev( GL_MODELVIEW_MATRIX, modelview_matrix_);
}

// rotate the scene (around its center) and update modelview matrix
void GlutViewer::rotate( const Vec3f& _axis, float _angle )
{
	Vec3f t( modelview_matrix_[0]*center_[0] + 
		modelview_matrix_[4]*center_[1] +
		modelview_matrix_[8]*center_[2] + 
		modelview_matrix_[12],
		modelview_matrix_[1]*center_[0] + 
		modelview_matrix_[5]*center_[1] +
		modelview_matrix_[9]*center_[2] + 
		modelview_matrix_[13],
		modelview_matrix_[2]*center_[0] + 
		modelview_matrix_[6]*center_[1] +
		modelview_matrix_[10]*center_[2] + 
		modelview_matrix_[14] );

	glLoadIdentity();
	glTranslatef(t[0], t[1], t[2]);
	glRotated( _angle, _axis[0], _axis[1], _axis[2]);
	glTranslatef(-t[0], -t[1], -t[2]); 
	glMultMatrixd(modelview_matrix_);

	glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix_);
}

// -----------
// for keyboard reaction
void GlutViewer::keyboard(int key, int x, int y) 
{
	switch (key)
	{
	case 27:
		exit(0); 
		break;
	case GLUT_KEY_F12:
		if (!fullscreen_) 
		{
			bak_left_   = glutGet(GLUT_WINDOW_X);
			bak_top_    = glutGet(GLUT_WINDOW_Y);
			bak_width_  = glutGet(GLUT_WINDOW_WIDTH);
			bak_height_ = glutGet(GLUT_WINDOW_HEIGHT);
			glutFullScreen();
			fullscreen_ = true;
		}
		else
		{
			glutReshapeWindow(bak_width_, bak_height_);
			glutPositionWindow(bak_left_, bak_top_);
			fullscreen_ = false;
		}
		break;
	}
} 

// -----------
// right button
void GlutViewer::processmenu(int i) 
{
	set_draw_mode(i); 
}

void GlutViewer::clear_draw_modes()
{
	for (unsigned int i=0; i<n_draw_modes_; ++i)
		glutRemoveMenuItem(1);

	n_draw_modes_ = 0;
	draw_mode_names_.clear();
}

int GlutViewer::add_draw_mode(const std::string& _s)
{
	// insert in popup menu
	glutAddMenuEntry(_s.c_str(), n_draw_modes_);

	++n_draw_modes_;
	draw_mode_names_.push_back(_s);

	return n_draw_modes_-1;
}

void GlutViewer::set_draw_mode(int _id)
{
	draw_mode_ = _id;
	glutPostRedisplay();
}

// -----------
void GlutViewer::passivemotion(int x, int y) {}
void GlutViewer::visibility(int visible) {}
void GlutViewer::idle(void) {} 


// -----------
// static function, just interface
void GlutViewer::display__(void) {
	current_viewer_->display();
}

void GlutViewer::idle__(void) {
	current_viewer_->idle();
}

void GlutViewer::keyboard__(unsigned char key, int x, int y) {
	current_viewer_->keyboard((int)key, x, y);
}

void GlutViewer::motion__(int x, int y) {
	current_viewer_->motion(x, y);
}

void GlutViewer::mouse__(int button, int state, int x, int y) {
	current_viewer_->mouse(button, state, x, y);
}

void GlutViewer::passivemotion__(int x, int y) {
	current_viewer_->passivemotion(x, y);
}

void GlutViewer::reshape__(int w, int h) {
	current_viewer_->reshape(w, h);
}

void GlutViewer::special__(int key, int x, int y) {
	current_viewer_->keyboard(key, x, y);
}

void GlutViewer::visibility__(int visible) {
	current_viewer_->visibility(visible);
}

void GlutViewer::processmenu__(int id) {
	current_viewer_->processmenu(id);
}
