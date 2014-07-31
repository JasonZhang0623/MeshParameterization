
#include <OpenMesh/Core/IO/MeshIO.hh>
#include "MeshViewer.hh"
#include "gl.hh"
#include <iostream>
#include <fstream>

// -----------
MeshViewer::MeshViewer(const char* _title, int _width, int _height)
  :GlutViewer(_title, _width, _height)
{
	mesh_.request_face_normals();
	mesh_.request_vertex_normals();
}


// -----------
bool MeshViewer::open_mesh(const char* _filename)
{
  // load mesh
  //   OpenMesh::IO::Options opt = OpenMesh::IO::Options::VertexNormal;
  //    opt += OpenMesh::IO::Options::VertexTexCoord;
  if (OpenMesh::IO::read_mesh(mesh_, _filename))
  {
    // set center and radius
    Mesh::ConstVertexIter  v_it(mesh_.vertices_begin()), 
                           v_end(mesh_.vertices_end());

    bbMin = bbMax = mesh_.point(v_it);
    for (; v_it!=v_end; ++v_it)
    {
      bbMin.minimize(mesh_.point(v_it));
      bbMax.maximize(mesh_.point(v_it));
    }
    setup_scene((Vec3f)(bbMin + bbMax)*0.5, 0.5*(bbMin - bbMax).norm());

    // compute face & vertex normals
    mesh_.update_normals();
	
    // update face indices for faster rendering
    update_face_indices();

    // info
    std::cerr << mesh_.n_vertices() << " vertices, "
	      << mesh_.n_faces()    << " faces\n";

    return true;
  }

  return false;
}

void MeshViewer::update_face_indices()
{
  Mesh::ConstFaceIter        f_it(mesh_.faces_sbegin()), 
                             f_end(mesh_.faces_end());
  Mesh::ConstFaceVertexIter  fv_it;

  indices_.clear();
  indices_.reserve(mesh_.n_faces()*3);

  for (; f_it!=f_end; ++f_it)
  {
    indices_.push_back((fv_it=mesh_.cfv_iter(f_it)).handle().idx());
    indices_.push_back((++fv_it).handle().idx());
    indices_.push_back((++fv_it).handle().idx());
  }
}


void MeshViewer::draw(const std::string& _draw_mode)
{
  if (indices_.empty())
  {
    GlutViewer::draw(_draw_mode);
    return;
  }
  if (_draw_mode == "Wireframe")
  {
    glDisable(GL_LIGHTING);
    glColor3f(1.0, 1.0, 1.0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glEnableClientState(GL_VERTEX_ARRAY);
    GL::glVertexPointer(mesh_.points());

    glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, &indices_[0]);

    glDisableClientState(GL_VERTEX_ARRAY);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
  else if (_draw_mode == "Hidden Line")
  {

	  glDisable(GL_LIGHTING);
	  glShadeModel(GL_SMOOTH);
	  glColor3f(0.0, 0.0, 0.0);

	  glEnableClientState(GL_VERTEX_ARRAY);
	  GL::glVertexPointer(mesh_.points());

	  glDepthRange(0.01, 1.0);
	  glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, &indices_[0]);
	  glDisableClientState(GL_VERTEX_ARRAY);
	  glColor3f(1.0, 1.0, 1.0);

	  glEnableClientState(GL_VERTEX_ARRAY);
	  GL::glVertexPointer(mesh_.points());

	  glDrawBuffer(GL_BACK);
	  glDepthRange(0.0, 1.0);
	  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	  glDepthFunc(GL_LEQUAL);
	  glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, &indices_[0]);

	  glDisableClientState(GL_VERTEX_ARRAY);
	  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	  glDepthFunc(GL_LESS);

  }
  else if (_draw_mode == "Solid Flat")
  {
    Mesh::ConstFaceIter        f_it(mesh_.faces_begin()), 
                               f_end(mesh_.faces_end());
    Mesh::ConstFaceVertexIter  fv_it;

    glEnable(GL_LIGHTING);
    glShadeModel(GL_FLAT);

    glBegin(GL_TRIANGLES);
    for (; f_it!=f_end; ++f_it)
    {
      GL::glNormal(mesh_.normal(f_it));
      fv_it = mesh_.cfv_iter(f_it.handle()); 
      GL::glVertex(mesh_.point(fv_it));
      ++fv_it;
      GL::glVertex(mesh_.point(fv_it));
      ++fv_it;
      GL::glVertex(mesh_.point(fv_it));
    }
    glEnd();
  }


  else if (_draw_mode == "Solid Smooth")
  {
    glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    GL::glVertexPointer(mesh_.points());
    GL::glNormalPointer(mesh_.vertex_normals());

    glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, &indices_[0]);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
  }
}
