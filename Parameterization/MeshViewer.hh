//=============================================================================
//                                                
//   Code framework for the lecture
//
//   "Surface Representation and Geometric Modeling"
//
//   Mark Pauly, Mario Botsch, Balint Miklos, and Hao Li
//
//   Copyright (C) 2007 by  Computer Graphics Laboratory, ETH Zurich
//
//                                                                            
//-----------------------------------------------------------------------------
//                                                                            
//                                License                                     
//                                                                            
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License
//   as published by the Free Software Foundation; either version 2
//   of the License, or (at your option) any later version.
//   
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//   
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 51 Franklin Street, Fifth Floor, 
//   Boston, MA  02110-1301, USA.
//                                                                            
//=============================================================================

#ifndef MESH_VIEWER_WIDGET_HH
#define MESH_VIEWER_WIDGET_HH

#include "GlutViewer.hh"
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
typedef OpenMesh::TriMesh_ArrayKernelT<>  Mesh;

class MeshViewer : public GlutViewer
{
public:
	/// default constructor
	MeshViewer(const char* _title, int _width, int _height);

	/// open mesh
	virtual bool open_mesh(const char* _filename);

protected:
	/// draw the scene
	virtual void draw(const std::string& _draw_mode);
	virtual void keyboard(int key, int x, int y);

private:
	/// update buffer with face indices
	void update_face_indices();

protected:
	Mesh  mesh_;
	std::vector<unsigned int>  indices_;
	Mesh::Point bbMin, bbMax;
};

#endif 