
#include "Meshpara.h"


int main(int argc, char **argv)
{
  glutInit(&argc, argv);

  MeshPara meshpara("Mesh Viewer", 512, 512);
  meshpara.setup();

  if (argc>1)
	  meshpara.open_mesh(argv[1]);

  glutMainLoop();

  return 0;
}
