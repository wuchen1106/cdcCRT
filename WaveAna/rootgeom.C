void rootgeom()
{
   // gStyle->SetCanvasPreferGL(true);
   gSystem->Load("libGeom");
   TGeoManager *geom = new TGeoManager("simple1", "Simple geometry");
   //--- define some materials
   TGeoMaterial *matVacuum = new TGeoMaterial("Vacuum", 0,0,0);
   TGeoMaterial *matAl = new TGeoMaterial("Al", 26.98,13,2.7);
//   //--- define some media
   TGeoMedium *Vacuum = new TGeoMedium("Vacuum",1, matVacuum);
   TGeoMedium *Al = new TGeoMedium("Root Material",2, matAl);
   //--- define the transformations
   TGeoTranslation *tr1 = new TGeoTranslation(20., 0, 0.);
   TGeoTranslation *tr2 = new TGeoTranslation(10., 0., 0.);
   //--- make the top container volume
   TGeoVolume *top = geom->MakeBox("TOP", Vacuum, 270., 270., 120.);
   geom->SetTopVolume(top);
   //--- make volumes
   TGeoVolume *tub1 = geom->MakeTubs("tub1", Al, 5., 15., 5., 90., 270.);
   TGeoVolume *tub2 = geom->MakeTubs("tub2", Vacuum, 5., 15., 5., 0., 360.);
//   tub1->Raytrace(kTRUE);
//   tub2->Raytrace(kTRUE);
//   top->Raytrace(kTRUE);
   top->AddNode(tub1, 1, new TGeoTranslation(-150, 150, 0));
   top->AddNode(tub2, 1, new TGeoTranslation(150, 150, 0));
   //--- close the geometry
   geom->CloseGeometry();
   //--- draw the ROOT box.
   // by default the picture will appear in the standard ROOT TPad.
   //if you have activated the following line in system.rootrc,
   //it will appear in the GL viewer
   //#Viewer3D.DefaultDrawOption:   ogl
   geom->SetVisLevel(4);
   top->Draw("ogle");
}
