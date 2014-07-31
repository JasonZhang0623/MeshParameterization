#include "MeshPara.h"
#include <NL/nl.h>


MeshPara::MeshPara(const char* _title, int _width, int _height) :
MeshViewer(_title, _width, _height), is_Parameterized(false)
{
	mesh_.request_vertex_texcoords2D();
}

MeshPara::~MeshPara()
{
}

void MeshPara::setup()
{
	MeshViewer::setup();

	// add menu item
	add_draw_mode("Texture");

	// setup texture
	setup_texture();
}

void MeshPara::setup_texture()
{
	// set texture
	make_check_image();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &tex_name);
	glBindTexture(GL_TEXTURE_2D, tex_name);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64,
		0, GL_RGBA, GL_UNSIGNED_BYTE, check_image);
}

void MeshPara::make_check_image()
{
	int i, j, c;
	for (i = 0; i < IMAGESIZE; i++)
	{
		for (j = 0; j < IMAGESIZE; j++)
		{
			c = ((((i & 0x2) == 0) ^ ((j & 0x2)) == 0)) * 255;
			check_image[i][j][0] = (GLubyte)c;
			check_image[i][j][1] = (GLubyte)c;
			check_image[i][j][2] = (GLubyte)c;
			check_image[i][j][3] = (GLubyte)255;
		}
	}
}

void MeshPara::draw(const std::string& _draw_mode)
{
	MeshViewer::draw(_draw_mode);
	if (_draw_mode == "Texture")
	{
		if (indices_.empty())
		{
			//std::cout << "Need to load a TRIANGLE MESH!" << std::endl;
			return ;
		}

		if (!is_Parameterized)
		{
			std::cout << "Have No Texture Coordinates!" << std::endl;
			std::cout << "Need Parameterization." << std::endl;
			LSCM();
			std::cout << "Parameterization End." << std::endl;
		}	

		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		glBindTexture(GL_TEXTURE_2D, tex_name);
		glEnable(GL_LIGHTING);
		//glShadeModel(GL_FLAT);

		Mesh::FaceIter f_it, f_end(mesh_.faces_end());
		glBegin(GL_TRIANGLES);
		for (f_it = mesh_.faces_begin(); f_it != f_end; ++f_it)
		{
			//GL::glNormal(mesh_.normal(f_it));
			auto fv_it = mesh_.fv_iter(*f_it);
			while (fv_it.is_valid())
			{
				Mesh::Point pt = mesh_.point(*fv_it);
				Mesh::TexCoord2D tx = mesh_.texcoord2D(*fv_it);
				GL::glTexCoord(tx);
				GL::glVertex(pt);
				++fv_it;
			}
		}
		glEnd();

		glDisable(GL_TEXTURE_2D);
	}
}

void MeshPara::LSCM()
{
	is_Parameterized = true;
	int nb_vertices = mesh_.n_vertices();

	nlNewContext();
	nlSolverParameteri(NL_SOLVER, NL_CG);
	nlSolverParameteri(NL_PRECONDITIONER, NL_PRECOND_JACOBI);
	nlSolverParameteri(NL_NB_VARIABLES, 2 * nb_vertices);
	nlSolverParameteri(NL_LEAST_SQUARES, NL_TRUE);
	nlSolverParameteri(NL_MAX_ITERATIONS, 5 * nb_vertices);
	nlSolverParameterd(NL_THRESHOLD, 1e-10);
	
	nlBegin(NL_SYSTEM);
	init_slover();
	nlBegin(NL_MATRIX);
	setup_LSCM();
	nlEnd(NL_MATRIX);
	nlEnd(NL_SYSTEM);
	std::cout << "Solving ..." << std::endl;
	nlSolve();

	// Get results
	get_result();

	// Display time and iter_num
	double time;
	NLint iterations;
	nlGetDoublev(NL_ELAPSED_TIME, &time);
	nlGetIntergerv(NL_USED_ITERATIONS, &iterations);
	std::cout << "Solver time: " << time << std::endl;
	std::cout << "Used iterations: " << iterations << std::endl;

	nlDeleteContext(nlGetCurrent());	
}

// Choose an initial solution, and lock two vertices
void MeshPara::init_slover() 
{
	// Get bbox
	Vec3f bAxis = bbMax - bbMin;

	// Get the Projection dirction
	int d1 ,d2, d3, i;
	d1 = d2 = d3 = 0;
	for (i = 1; i < 3; i++)
	{
		if (bAxis[i] > bAxis[d1])
			d1 = i;
		if (bAxis[i] < bAxis[d3])
			d3 = i;
	}
	for (d2 = 0; d2 < 3; d2++)
	{
		if (d2 != d1 && d2 != d3)
			break;
	}

	// Project vertices
	auto v_end(mesh_.vertices_end());
	float u1 = -1.0e30, u2 = 1.0e30;
	int lock1 = 0, lock2 = 0;
	for (auto v_it = mesh_.vertices_begin(); v_it != v_end; ++v_it)
	{
		float u = mesh_.point(*v_it)[d1];
		float v = mesh_.point(*v_it)[d2];
		int idx =  (*v_it).idx();
		mesh_.set_texcoord2D(*v_it, Vec2f(u, v));

		// set initial solution
		nlSetVariable(2 * idx, u);
		nlSetVariable(2 * idx + 1, v);

		if (u > u1) 
		{
			lock1 = idx;
			u1 = u;
		}
		if (u < u2)
		{
			lock2 = idx;
			u2 = u;
		}
	}

	// set locked variables
	nlLockVariable(2 * lock1);
	nlLockVariable(2 * lock1 + 1);
	nlLockVariable(2 * lock2);
	nlLockVariable(2 * lock2 + 1);
}

void MeshPara::setup_LSCM()
{
	auto f_it(mesh_.faces_begin());
	auto f_end(mesh_.faces_end());
	for (; f_it != f_end; f_it++)
	{
		setup_conformal_map_relations(*f_it);
	}
}

// LSCM equation, geometric form :
// (Z1 - Z0)(U2 - U0) = (Z2 - Z0)(U1 - U0)
// Where Uk = uk + i.vk is the complex number 
//                       corresponding to (u,v) coords
//       Zk = xk + i.yk is the complex number 
//                       corresponding to local (x,y) coords
void MeshPara::setup_conformal_map_relations(Mesh::FHandle fh)
{
	int id[3];
	Vec3f p[3];
	auto fv = mesh_.fv_begin(fh);
	for (int i = 0; i < 3; i++, fv++)
	{
		p[i] = mesh_.point(*fv);
		id[i] = (*fv).idx();
	}

	Vec2f z[3];
	project_triangle(p[0], p[1], p[2], z[0], z[1], z[2]);
	Vec2f z01 = z[1] - z[0];
	Vec2f z02 = z[2] - z[0];
	double a = z01[0];
	double b = z01[1];
	double c = z02[0];
	double d = z02[1];
	assert(b == 0.0);

	// Note  : 2*id + 0 --> u
	//         2*id + 1 --> v
	int u0_id = 2 * id[0];
	int v0_id = 2 * id[0] + 1;
	int u1_id = 2 * id[1];
	int v1_id = 2 * id[1] + 1;
	int u2_id = 2 * id[2];
	int v2_id = 2 * id[2] + 1;

	// Note : b = 0

	// Real part
	nlBegin(NL_ROW);
	nlCoefficient(u0_id, -a + c);
	nlCoefficient(v0_id, b - d);
	nlCoefficient(u1_id, -c);
	nlCoefficient(v1_id, d);
	nlCoefficient(u2_id, a);
	nlEnd(NL_ROW);

	// Imaginary part
	nlBegin(NL_ROW);
	nlCoefficient(u0_id, -b + d);
	nlCoefficient(v0_id, -a + c);
	nlCoefficient(u1_id, -d);
	nlCoefficient(v1_id, -c);
	nlCoefficient(v2_id, a);
	nlEnd(NL_ROW);
}

// Computes the coordinates of the vertices of a triangle
// in a local 2D orthonormal basis of the triangle's plane.
void MeshPara::project_triangle(Vec3f& p0, Vec3f& p1, Vec3f& p2,
	Vec2f& z0, Vec2f& z1, Vec2f& z2)
{
	Vec3f X = p1 - p0;
	float x1 = X.norm();
	X.normalize();
	Vec3f p02 = p2 - p0;
	Vec3f Z = cross(X, p02);
	Z.normalize();
	Vec3f Y = cross(Z, X);

	float x2 = dot(X, p02);
	float y2 = dot(Y, p02);

	z0 = Vec2f(0, 0);
	z1 = Vec2f(x1, 0);
	z2 = Vec2f(x2, y2);
}

void MeshPara::get_result()
{
	Vec2f tc1, tc2;
	tc1[0] = tc2[0] = nlGetVariable(0);
	tc1[1] = tc2[1] = nlGetVariable(1);
	auto v_it(mesh_.vertices_begin());
	auto v_end(mesh_.vertices_end()); 
	for (; v_it != v_end; v_it++)
	{
		int idx = (*v_it).idx();
		float u = nlGetVariable(2 * idx);
		float v = nlGetVariable(2 * idx + 1);
		mesh_.set_texcoord2D(*v_it, Vec2f(u, v));
		if (u < tc1[0])tc1[0] = u;
		if (u > tc2[0])tc2[0] = u;
		if (v < tc1[1])tc1[1] = v;
		if (v > tc2[1])tc2[1] = v;
	}

	// Normalize
	double dx = tc2[0] - tc1[0];
	double dy = tc2[1] - tc1[1];
	if (dy > dx) dx = dy;

	for (v_it = mesh_.vertices_begin(); v_it != v_end; v_it++)
	{
		Vec2f tc = mesh_.texcoord2D(*v_it);
		tc[0] = (tc[0] - tc1[0]) / dx;
		tc[1] = (tc[1] - tc1[1]) / dx;
		mesh_.set_texcoord2D(*v_it, tc);
	}
}