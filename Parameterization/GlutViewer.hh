
#ifndef GLUTVIEWER_HH
#define GLUTVIEWER_HH

#include "gl.hh"
#include <vector>
#include <string>
#include <OpenMesh/Core/Geometry/VectorT.hh>
using namespace OpenMesh;

class GlutViewer
{
public:   
	GlutViewer(const char* _title, int _width, int _height);
	virtual ~GlutViewer();
	void setup(void);

protected:
	void setup_glut(void);
	void setup_view(void);
	void setup_menu(void);
	void setup_scene( const Vec3f& _cog, float _radius );

	virtual void processmenu(int i);
	
	virtual void reshape(int w, int h); 

	virtual void display(void);
	virtual void draw(const std::string& _drawmode);
	
	virtual void mouse(int button, int state, int x, int y);
	virtual void motion(int x, int y);

	virtual void keyboard(int key, int x, int y);

	virtual void passivemotion(int x, int y);
	virtual void visibility(int visible);
	virtual void idle(void); 


	void clear_draw_modes();
	void set_draw_mode(int _id);
	int add_draw_mode(const std::string& _s);
	
	void translate(const Vec3f& _trans);
	void rotate(const Vec3f& _axis, float _angle);
	void rotation(int x, int y);
	void translation(int x, int y);
	void zoom(int x, int y);
	
	void update_projection_matrix();
	bool map_to_sphere(const Vec2i& _point, Vec3f& _result);

private:
	static void display__(void);
	static void idle__(void); 
	static void keyboard__(unsigned char key, int x, int y);
	static void motion__(int x, int y);
	static void mouse__(int button, int state, int x, int y);
	static void passivemotion__(int x, int y);
	static void reshape__(int w, int h); 
	static void special__(int key, int x, int y);   
	static void visibility__(int visible);
	static void processmenu__(int i); 

protected:
	// screen width and height
	int  width_, height_;

	// scene position and dimension
	Vec3f center_;
	float radius_;

	// projection parameters
	float near_, far_, fovy_;

	// OpenGL matrices
	double projection_matrix_[16];
	double modelview_matrix_[16];

	// trackball helpers
	Vec2i last_point_2D_;
	Vec3f last_point_3D_;
	bool last_point_ok_;
	bool button_down_[5];

private:
	static GlutViewer* current_viewer_;  

	int  windowID_, menuID_; 
	std::string title_;

	bool fullscreen_;
	int  bak_left_, bak_top_, bak_width_, bak_height_;

	int  draw_mode_;
	int  n_draw_modes_;
	std::vector<std::string>  draw_mode_names_; 

};

#endif 
