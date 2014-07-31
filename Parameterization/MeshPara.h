#pragma once
#include "MeshViewer.hh"
#define IMAGESIZE 128

class MeshPara : public MeshViewer
{
public:
	MeshPara(const char* _title, int _width, int _height);
	~MeshPara();

	/// setup
	void setup();

	/// draw the scene
	virtual void draw(const std::string& _draw_mode);

	/// LSCM Parameterization
	void LSCM();

private:
	void init_slover();
	void project_triangle(Vec3f& p0, Vec3f& p1, Vec3f& p2,
		Vec2f& z0, Vec2f& z1, Vec2f& z2);
	void setup_conformal_map_relations(Mesh::FHandle fh);
	void setup_LSCM();
	void get_result();

	void setup_texture(void);
	void make_check_image(void);

private:
	bool is_Parameterized;
	GLuint tex_name;
	GLubyte check_image[IMAGESIZE][IMAGESIZE][4];
};

