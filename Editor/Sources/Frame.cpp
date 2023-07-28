#include "Frame.h"
#include "Panel.h"
#include "Application.h"
#include "MeshCollectionScene.h"
#include "Mesh.h"
#include "MeshSetOperation.h"
#include "ConvexPolygonMesh.h"
#include <wx/aboutdlg.h>
#include <wx/menu.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/toolbar.h>

wxDEFINE_EVENT(EVT_SCENE_CHANGED, wxCommandEvent);

Frame::Frame(wxWindow* parent, const wxPoint& pos, const wxSize& size) : wxFrame(parent, wxID_ANY, "Mesh Editor", pos, size), timer(this)
{
	this->inTimerTick = false;

	this->auiManager.SetManagedWindow(this);
	this->auiManager.SetFlags(wxAUI_MGR_LIVE_RESIZE | wxAUI_MGR_DEFAULT);

	wxMenu* fileMenu = new wxMenu();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_ClearScene, "Clear Scene", "Delete all meshes from the scene."));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_ImportMesh, "Import Mesh", "Load a mesh file and add it to the scene."));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_ExportMesh, "Export Mesh", "Save a mesh from the scene to disk."));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_Exit, "Exit", "Terminate this program."));

	wxMenu* helpMenu = new wxMenu();
	helpMenu->Append(new wxMenuItem(helpMenu, ID_About, "About", "Show the about-box."));

	wxMenuBar* menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "File");
	menuBar->Append(helpMenu, "Help");
	this->SetMenuBar(menuBar);

	this->SetStatusBar(new wxStatusBar(this));

	wxBitmap unionBitmap, intersectionBitmap, subtractionBitmap;
	wxBitmap addMeshBitmap, unlitBitmap, faceLitBitmap, vertexLitBitmap;
	wxBitmap faceNormalsBitmap, vertexNormalsBitmap, edgesBitmap;
	wxBitmap axesBitmap;

	intersectionBitmap.LoadFile(wxGetCwd() + "/Textures/IntersectionIcon.png", wxBITMAP_TYPE_PNG);
	unionBitmap.LoadFile(wxGetCwd() + "/Textures/UnionIcon.png", wxBITMAP_TYPE_PNG);
	subtractionBitmap.LoadFile(wxGetCwd() + "/Textures/SubtractionIcon.png", wxBITMAP_TYPE_PNG);
	addMeshBitmap.LoadFile(wxGetCwd() + "/Textures/AddMeshIcon.png", wxBITMAP_TYPE_PNG);
	unlitBitmap.LoadFile(wxGetCwd() + "/Textures/UnlitIcon.png", wxBITMAP_TYPE_PNG);
	faceLitBitmap.LoadFile(wxGetCwd() + "/Textures/FaceLitIcon.png", wxBITMAP_TYPE_PNG);
	vertexLitBitmap.LoadFile(wxGetCwd() + "/Textures/VertexLitIcon.png", wxBITMAP_TYPE_PNG);
	faceNormalsBitmap.LoadFile(wxGetCwd() + "/Textures/FaceNormalsIcon.png", wxBITMAP_TYPE_PNG);
	vertexNormalsBitmap.LoadFile(wxGetCwd() + "/Textures/VertexNormalsIcon.png", wxBITMAP_TYPE_PNG);
	edgesBitmap.LoadFile(wxGetCwd() + "/Textures/EdgesIcon.png", wxBITMAP_TYPE_PNG);
	axesBitmap.LoadFile(wxGetCwd() + "/Textures/AxesIcon.png", wxBITMAP_TYPE_PNG);

	wxToolBar* toolBar = this->CreateToolBar();
	toolBar->AddTool(ID_IntersectMeshes, "Intersect Meshes", intersectionBitmap, "Take the intersection of two meshes.");
	toolBar->AddTool(ID_UnionMeshes, "Union Meshes", unionBitmap, "Take the union of two meshes.");
	toolBar->AddTool(ID_SubtractMeshes, "Subtract Meshes", subtractionBitmap, "Subtract one mesh from another.");
	toolBar->AddSeparator();
	toolBar->AddTool(ID_RenderUnlit, "Render Unlit", unlitBitmap, "Render unlit.", wxITEM_CHECK);
	toolBar->AddTool(ID_RenderFaceLit, "Render Face Lit", faceLitBitmap, "Render lit with face normals.", wxITEM_CHECK);
	toolBar->AddTool(ID_RenderVertexLit, "Render Vertex Lit", vertexLitBitmap, "Render lit with vertex normals.", wxITEM_CHECK);
	toolBar->AddSeparator();
	toolBar->AddTool(ID_RenderAxes, "Render XYZ Axes", axesBitmap, "Render the X, Y and Z-axis near the origin.", wxITEM_CHECK);
	toolBar->AddSeparator();
	toolBar->AddTool(ID_ToggleEdgeRender, "Toggle Edges", edgesBitmap, "Turn edge rendering on/off", wxITEM_CHECK);
	toolBar->AddTool(ID_ToggleFaceNormalRender, "Toggle Face Normals", faceNormalsBitmap, "Turn face normal rendering on/off", wxITEM_CHECK);
	toolBar->AddTool(ID_ToggleVertexNormalRender, "Toggle Vertex Normals", vertexNormalsBitmap, "Turn vertex normal rendering on/off", wxITEM_CHECK);

	wxArrayString meshComboBoxChoices;

	meshComboBoxChoices.Add("Tetrahedron");
	meshComboBoxChoices.Add("Octahedron");
	meshComboBoxChoices.Add("Hexadron");
	meshComboBoxChoices.Add("Icosahedron");
	meshComboBoxChoices.Add("Dodecahedron");
	meshComboBoxChoices.Add("Icosidodecahedron");
	meshComboBoxChoices.Add("Cuboctahedron");
	meshComboBoxChoices.Add("Rhombicosidodecahedron");
	meshComboBoxChoices.Add("Plane");
	meshComboBoxChoices.Add("Sphere");
	meshComboBoxChoices.Add("Cylinder");
	meshComboBoxChoices.Add("Torus");
	meshComboBoxChoices.Add("Mobius Strip");
	meshComboBoxChoices.Add("Klein Bottle");

	this->meshComboBox = new wxComboBox(toolBar, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, meshComboBoxChoices, wxCB_READONLY | wxCB_SORT | wxCB_DROPDOWN);

	toolBar->AddSeparator();
	toolBar->AddControl(this->meshComboBox, "Mesh:");
	toolBar->AddTool(ID_AddMesh, "Add Mesh", addMeshBitmap, "Add the mesh specified in the drop-down to the scene.");

	toolBar->Realize();

	this->Bind(wxEVT_MENU, &Frame::OnExit, this, ID_Exit);
	this->Bind(wxEVT_MENU, &Frame::OnAbout, this, ID_About);
	this->Bind(wxEVT_MENU, &Frame::OnImportMesh, this, ID_ImportMesh);
	this->Bind(wxEVT_MENU, &Frame::OnExportMesh, this, ID_ExportMesh);
	this->Bind(EVT_SCENE_CHANGED, &Frame::OnSceneChanged, this);
	this->Bind(wxEVT_TIMER, &Frame::OnTimerTick, this);
	this->Bind(wxEVT_MENU, &Frame::OnMeshSetOperation, this, ID_IntersectMeshes);
	this->Bind(wxEVT_MENU, &Frame::OnMeshSetOperation, this, ID_UnionMeshes);
	this->Bind(wxEVT_MENU, &Frame::OnMeshSetOperation, this, ID_SubtractMeshes);
	this->Bind(wxEVT_MENU, &Frame::OnAddMesh, this, ID_AddMesh);
	this->Bind(wxEVT_MENU, &Frame::OnToggle, this, ID_RenderUnlit);
	this->Bind(wxEVT_MENU, &Frame::OnToggle, this, ID_RenderFaceLit);
	this->Bind(wxEVT_MENU, &Frame::OnToggle, this, ID_RenderVertexLit);
	this->Bind(wxEVT_MENU, &Frame::OnToggle, this, ID_RenderAxes);
	this->Bind(wxEVT_MENU, &Frame::OnToggle, this, ID_ToggleEdgeRender);
	this->Bind(wxEVT_MENU, &Frame::OnToggle, this, ID_ToggleFaceNormalRender);
	this->Bind(wxEVT_MENU, &Frame::OnToggle, this, ID_ToggleVertexNormalRender);
	this->Bind(wxEVT_MENU, &Frame::OnClearScene, this, ID_ClearScene);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_RenderUnlit);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_RenderFaceLit);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_RenderVertexLit);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_RenderAxes);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_ToggleEdgeRender);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_ToggleFaceNormalRender);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_ToggleVertexNormalRender);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_ClearScene);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_ExportMesh);

	this->MakePanels();
	this->UpdatePanels();

	this->auiManager.Update();

	this->timer.Start(0);
}

/*virtual*/ Frame::~Frame()
{
	this->auiManager.UnInit();
}

void Frame::OnSceneChanged(wxCommandEvent& event)
{
	this->UpdatePanels();
}

void Frame::OnClearScene(wxCommandEvent& event)
{
	MeshCollectionScene* meshScene = wxGetApp().GetMeshScene();
	meshScene->Clear();
	wxPostEvent(this, wxCommandEvent(EVT_SCENE_CHANGED));
}

void Frame::OnToggle(wxCommandEvent& event)
{
	switch (event.GetId())
	{
		case ID_RenderUnlit:
		{
			wxGetApp().lightingMode = Application::LightingMode::UNLIT;
			break;
		}
		case ID_RenderFaceLit:
		{
			wxGetApp().lightingMode = Application::LightingMode::FACE_LIT;
			break;
		}
		case ID_RenderVertexLit:
		{
			wxGetApp().lightingMode = Application::LightingMode::VERTEX_LIT;
			break;
		}
		case ID_RenderAxes:
		{
			wxGetApp().renderAxes = !wxGetApp().renderAxes;
			break;
		}
		case ID_ToggleEdgeRender:
		{
			wxGetApp().renderEdges = !wxGetApp().renderEdges;
			break;
		}
		case ID_ToggleFaceNormalRender:
		{
			wxGetApp().renderFaceNormals = !wxGetApp().renderFaceNormals;
			break;
		}
		case ID_ToggleVertexNormalRender:
		{
			wxGetApp().renderVertexNormals = !wxGetApp().renderVertexNormals;
			break;
		}
	}

	wxCommandEvent sceneChangedEvent(EVT_SCENE_CHANGED);
	wxPostEvent(this, sceneChangedEvent);
}

void Frame::OnUpdateUI(wxUpdateUIEvent& event)
{
	switch (event.GetId())
	{
		case ID_ClearScene:
		{
			event.Enable(wxGetApp().GetMeshScene()->GetSceneObjectCount() > 0);
			break;
		}
		case ID_RenderUnlit:
		{
			event.Check(wxGetApp().lightingMode == Application::LightingMode::UNLIT);
			break;
		}
		case ID_RenderFaceLit:
		{
			event.Check(wxGetApp().lightingMode == Application::LightingMode::FACE_LIT);
			break;
		}
		case ID_RenderVertexLit:
		{
			event.Check(wxGetApp().lightingMode == Application::LightingMode::VERTEX_LIT);
			break;
		}
		case ID_RenderAxes:
		{
			event.Check(wxGetApp().renderAxes);
			break;
		}
		case ID_ToggleEdgeRender:
		{
			event.Check(wxGetApp().renderEdges);
			break;
		}
		case ID_ToggleFaceNormalRender:
		{
			event.Check(wxGetApp().renderFaceNormals);
			break;
		}
		case ID_ToggleVertexNormalRender:
		{
			event.Check(wxGetApp().renderVertexNormals);
			break;
		}
		case ID_ExportMesh:
		{
			event.Enable(wxGetApp().GetMeshScene()->FindFirstSelectedMesh() != nullptr);
			break;
		}
	}
}

void Frame::OnAddMesh(wxCommandEvent& event)
{
	wxBusyCursor busyCursor;

	Mesh* mesh = new Mesh();

	int i = this->meshComboBox->GetSelection();
	wxString meshName = this->meshComboBox->GetString(i);

	int polyhedron = -1;
	if (meshName == "Tetrahedron")
		polyhedron = int(MeshNinja::ConvexPolygonMesh::Polyhedron::TETRAHEDRON);
	else if (meshName == "Octahedron")
		polyhedron = int(MeshNinja::ConvexPolygonMesh::Polyhedron::OCTAHEDRON);
	else if (meshName == "Hexadron")
		polyhedron = int(MeshNinja::ConvexPolygonMesh::Polyhedron::HEXADRON);
	else if (meshName == "Icosahedron")
		polyhedron = int(MeshNinja::ConvexPolygonMesh::Polyhedron::ICOSAHEDRON);
	else if (meshName == "Dodecahedron")
		polyhedron = int(MeshNinja::ConvexPolygonMesh::Polyhedron::DODECAHEDRON);
	else if (meshName == "Icosidodecahedron")
		polyhedron = int(MeshNinja::ConvexPolygonMesh::Polyhedron::ICOSIDODECAHEDRON);
	else if (meshName == "Cuboctahedron")
		polyhedron = int(MeshNinja::ConvexPolygonMesh::Polyhedron::CUBOCTAHEDRON);
	else if (meshName == "Rhombicosidodecahedron")
		polyhedron = int(MeshNinja::ConvexPolygonMesh::Polyhedron::RHOMBICOSIDODECAHEDRON);

	if (polyhedron >= 0)
	{
		if (!mesh->mesh.GeneratePolyhedron(MeshNinja::ConvexPolygonMesh::Polyhedron(polyhedron)))
		{
			wxMessageBox("Failed to generate polyhedron!", "Error", wxICON_ERROR | wxOK, this);
			delete mesh;
			return;
		}
	}
	else
	{
		if (meshName == "Sphere")
			mesh->mesh.GenerateSphere(5.0, 16, 16);
		else if (meshName == "Cylinder")
			mesh->mesh.GenerateCylinder(10.0, 5.0, 16, 16);
		else if (meshName == "Torus")
			mesh->mesh.GenerateTorus(5.0, 10.0, 16, 16);
		else if (meshName == "Mobius Strip")
			mesh->mesh.GenerateMobiusStrip(1.0, 5.0, 16);
		else if (meshName == "Klein Bottle")
			mesh->mesh.GenerateKleinBottle(16);
		else if (meshName == "Plane")
		{
			MeshNinja::ConvexPolygon polygon;
			polygon.vertexArray->push_back(MeshNinja::Vector(-10.0, -10.0, 0.0));
			polygon.vertexArray->push_back(MeshNinja::Vector(10.0, -10.0, 0.0));
			polygon.vertexArray->push_back(MeshNinja::Vector(10.0, 10.0, 0.0));
			polygon.vertexArray->push_back(MeshNinja::Vector(-10.0, 10.0, 0.0));

			std::vector<MeshNinja::ConvexPolygon> polygonArray;
			polygonArray.push_back(polygon);

			mesh->mesh.FromConvexPolygonArray(polygonArray);
		}
	}

	if (mesh->mesh.vertexArray->size() == 0)
	{
		wxMessageBox("Failed to generate mesh!", "Error", wxICON_ERROR | wxOK, this);
		delete mesh;
		return;
	}

	mesh->mesh.UntessellateFaces();

	MeshCollectionScene* meshScene = wxGetApp().GetMeshScene();
	meshScene->GetMeshList().push_back(mesh);

	wxCommandEvent sceneChangedEvent(EVT_SCENE_CHANGED);
	wxPostEvent(this, sceneChangedEvent);
}

void Frame::OnMeshSetOperation(wxCommandEvent& event)
{
	wxBusyCursor busyCursor;

	MeshCollectionScene* meshScene = wxGetApp().GetMeshScene();
	
	std::list<Mesh*> selectedMeshesList;
	if (!meshScene->GetSelectedMeshes(selectedMeshesList, true) || selectedMeshesList.size() != 2)
	{
		wxMessageBox("You must select exactly two meshes.", "Error", wxICON_ERROR | wxOK, this);
		return;
	}

	MeshNinja::MeshSetOperation* operation = nullptr;

	switch (event.GetId())
	{
	case ID_IntersectMeshes:
		operation = new MeshNinja::MeshIntersection();
		break;
	case ID_UnionMeshes:
		operation = new MeshNinja::MeshUnion();
		break;
	case ID_SubtractMeshes:
		operation = new MeshNinja::MeshSubtraction();
		break;
	}

	Mesh* meshA = *selectedMeshesList.begin();
	Mesh* meshB = selectedMeshesList.back();

	MeshNinja::ConvexPolygonMesh transformedMeshA(meshA->mesh);
	MeshNinja::ConvexPolygonMesh transformedMeshB(meshB->mesh);

	transformedMeshA.ApplyTransform(meshA->transform);
	transformedMeshB.ApplyTransform(meshB->transform);

	Mesh* meshResult = new Mesh();

	if (!operation->Perform(transformedMeshA, transformedMeshB, meshResult->mesh))
	{
		wxMessageBox("Mesh operation failed: " + wxString(operation->error->c_str()), "Error", wxICON_ERROR | wxOK, this);
		delete meshResult;
	}
	else
	{
		meshScene->GetMeshList().push_back(meshResult);

		meshA->SetVisible(false);
		meshB->SetVisible(false);

		wxCommandEvent sceneChangedEvent(EVT_SCENE_CHANGED);
		wxPostEvent(this, sceneChangedEvent);
	}

	delete operation;
}

void Frame::OnTimerTick(wxTimerEvent& event)
{
	if (this->inTimerTick)
		return;

	this->inTimerTick = true;

	this->ForAllPanels([](Panel* panel)
		{
			panel->Tick();
		});

	this->inTimerTick = false;
}

void Frame::MakePanels()
{
	wxClassInfo* basePanelClassInfo = wxClassInfo::FindClass("Panel");
	if (!basePanelClassInfo)
		return;

	const wxClassInfo* classInfo = wxClassInfo::GetFirst();
	while (classInfo)
	{
		if (classInfo != basePanelClassInfo && classInfo->IsKindOf(basePanelClassInfo))
		{
			Panel* panel = (Panel*)classInfo->CreateObject();
			panel->Create(this);
			panel->MakeControls();

			wxAuiPaneInfo paneInfo;
			panel->MakePaneInfo(paneInfo);
			paneInfo.CloseButton(false);

			this->auiManager.AddPane(panel, paneInfo);
		}

		classInfo = classInfo->GetNext();
	}
}

void Frame::ForAllPanels(std::function<void(Panel*)> callback)
{
	wxAuiPaneInfoArray& paneInfoArray = this->auiManager.GetAllPanes();

	for (int i = 0; i < (signed)paneInfoArray.GetCount(); i++)
	{
		wxAuiPaneInfo& paneInfo = paneInfoArray[i];
		Panel* panel = wxDynamicCast(paneInfo.window, Panel);
		if (panel)
			callback(panel);
	}
}

void Frame::UpdatePanels()
{
	this->ForAllPanels([](Panel* panel)
		{
			panel->Update();
		});
}

void Frame::OnImportMesh(wxCommandEvent& event)
{
	wxFileDialog fileDialog(this, "Choose mesh file to open.", wxEmptyString, wxEmptyString, "Mesh file (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (wxID_OK == fileDialog.ShowModal())
	{
		wxBusyCursor busyCursor;

		Mesh* mesh = new Mesh();
		mesh->fileSource = fileDialog.GetPath();
		if (mesh->Load())
			wxGetApp().GetMeshScene()->GetMeshList().push_back(mesh);
		else
		{
			wxMessageBox("Failed to read file: " + mesh->fileSource, "Error", wxICON_ERROR | wxOK, this);
			delete mesh;
		}
	}

	wxCommandEvent sceneChangedEvent(EVT_SCENE_CHANGED);
	wxPostEvent(this, sceneChangedEvent);
}

void Frame::OnExportMesh(wxCommandEvent& event)
{
	Mesh* mesh = wxGetApp().GetMeshScene()->FindFirstSelectedMesh();
	if (!mesh)
	{
		wxMessageBox("You must first select the mesh you wish to export.", "Error", wxICON_ERROR | wxOK, this);
		return;
	}

	if (mesh->fileSource.IsEmpty())
	{
		wxFileDialog fileDialog(this, "Choose mesh file to save.", wxEmptyString, wxEmptyString, "Mesh file (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		if (wxID_OK != fileDialog.ShowModal())
			return;
		else
			mesh->fileSource = fileDialog.GetPath();
	}

	bool saveRenderMesh = false;
	int answer = wxMessageBox("Save render mesh?", "Inquery", wxICON_QUESTION | wxYES_NO | wxCANCEL, this);
	if (wxYES == answer)
		saveRenderMesh = true;
	else if (wxCANCEL == answer)
		return;

	wxBusyCursor busyCursor;

	if (!mesh->Save(saveRenderMesh))
		wxMessageBox("Failed to write file: " + mesh->fileSource, "Error", wxICON_ERROR | wxOK, this);
	else
		wxMessageBox("Mesh file (" + mesh->fileSource + ") saved!", "Success", wxICON_INFORMATION | wxOK, this);
}

void Frame::OnExit(wxCommandEvent& event)
{
	this->Close(true);
}

void Frame::OnAbout(wxCommandEvent& event)
{
	wxAboutDialogInfo aboutDialogInfo;

	aboutDialogInfo.SetName("Mesh Editor");
	aboutDialogInfo.SetVersion("1.0");
	aboutDialogInfo.SetDescription("This is not a serious mesh editor.  It's just a testing-area/sandbox for the MeshNinja C++ library.");
	aboutDialogInfo.SetCopyright("Copyright (C) 2023 -- Spencer T. Parkin <SpencerTParkin@gmail.com>");

	wxAboutBox(aboutDialogInfo);
}