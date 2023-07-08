#include "Frame.h"
#include "Panel.h"
#include <wx/aboutdlg.h>
#include <wx/menu.h>

Frame::Frame(wxWindow* parent, const wxPoint& pos, const wxSize& size) : wxFrame(parent, wxID_ANY, "Mesh Editor", pos, size)
{
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

	this->MakePanels();

	this->auiManager.Update();
}

/*virtual*/ Frame::~Frame()
{
	this->auiManager.UnInit();
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

void Frame::UpdatePanels()
{
	wxAuiPaneInfoArray& paneInfoArray = this->auiManager.GetAllPanes();

	for (int i = 0; i < (signed)paneInfoArray.GetCount(); i++)
	{
		wxAuiPaneInfo& paneInfo = paneInfoArray[i];
		Panel* panel = wxDynamicCast(paneInfo.window, Panel);
		if (panel)
			panel->Update();
	}
}

void Frame::OnImportMesh(wxCommandEvent& event)
{
}

void Frame::OnExportMesh(wxCommandEvent& event)
{
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