#include "Frame.h"
#include "Panel.h"
#include "Application.h"
#include "MeshCollectionScene.h"
#include "Mesh.h"
#include <wx/aboutdlg.h>
#include <wx/menu.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>

wxDEFINE_EVENT(EVT_SCENE_CHANGED, wxCommandEvent);

Frame::Frame(wxWindow* parent, const wxPoint& pos, const wxSize& size) : wxFrame(parent, wxID_ANY, "Mesh Editor", pos, size), timer(this)
{
	this->inTimerTick = false;

	this->auiManager.SetManagedWindow(this);
	this->auiManager.SetFlags(wxAUI_MGR_LIVE_RESIZE | wxAUI_MGR_DEFAULT);

	wxMenu* fileMenu = new wxMenu();
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

	this->Bind(wxEVT_MENU, &Frame::OnExit, this, ID_Exit);
	this->Bind(wxEVT_MENU, &Frame::OnAbout, this, ID_About);
	this->Bind(wxEVT_MENU, &Frame::OnImportMesh, this, ID_ImportMesh);
	this->Bind(wxEVT_MENU, &Frame::OnExportMesh, this, ID_ExportMesh);
	this->Bind(EVT_SCENE_CHANGED, &Frame::OnSceneChanged, this);
	this->Bind(wxEVT_TIMER, &Frame::OnTimerTick, this);

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

	wxBusyCursor busyCursor;

	if (!mesh->Save())
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
	aboutDialogInfo.SetDescription("This is just a basic mesh editing program.  Whoopy-freakin'-do.");
	aboutDialogInfo.SetCopyright("Copyright (C) 2023 -- Spencer T. Parkin <SpencerTParkin@gmail.com>");

	wxAboutBox(aboutDialogInfo);
}