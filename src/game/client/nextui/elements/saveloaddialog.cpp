#include "cbase.h"
#include "bitmap/tgaloader.h"
#include "filesystem.h"
#include "nextui/nextui_button.h"
#include "nextui/nextui_dialog.h"
#include "nextui/nextui_int.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/PanelListPanel.h"
#include "vgui/ISurface.h"
#include "tier1/utlvector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CTGAImagePanel : public vgui::Panel {
    DECLARE_CLASS_SIMPLE(CTGAImagePanel, vgui::Panel);

public:
    CTGAImagePanel(vgui::Panel *pParent)
        : BaseClass(pParent), m_szFilename()
    {
        SetPaintBackgroundEnabled(false);
        SetPaintBorderEnabled(false);
        SetPaintEnabled(true);
    }

    virtual ~CTGAImagePanel()
    {
        if(g_pVGuiSurface && m_iTexture != NULL_TEXTURE_ID) {
            g_pVGuiSurface->DestroyTextureID(m_iTexture);
            m_iTexture = NULL_TEXTURE_ID;
        }
    }

    virtual void Paint() override
    {
        if(!m_bLoadAttempted) {
            m_bLoadAttempted = true;

            if(m_iTexture == NULL_TEXTURE_ID)
                m_iTexture = g_pVGuiSurface->CreateNewTextureID(true);
            m_bValidTexture = false;

            int iWidth, iHeight;
            CUtlMemory<unsigned char> pTGA;
            if(m_szFilename[0] && TGALoader::LoadRGBA8888(m_szFilename, pTGA, iWidth, iHeight)) {
                g_pVGuiSurface->DrawSetTextureRGBA(m_iTexture, pTGA.Base(), iWidth, iHeight, false, true);
                m_bValidTexture = true;
            }
        }

        if(!m_bValidTexture) {
            // Don't even bother to paint
            SetPaintEnabled(false);
            return;
        }

        int iWide, iTall;
        GetSize(iWide, iTall);
        g_pVGuiSurface->DrawSetTexture(m_iTexture);
        g_pVGuiSurface->DrawSetColor(0xFF, 0xFF, 0xFF, 0xFF);
        g_pVGuiSurface->DrawTexturedRect(0, 0, iWide, iTall);
    }

    void SetTGAPath(const char *pszFilename)
    {
        Q_snprintf(m_szFilename, sizeof(m_szFilename), "//MOD/%s", pszFilename);
        m_bLoadAttempted = false;
        m_bValidTexture = false;
    }

private:
    int m_iTexture = NULL_TEXTURE_ID;
    bool m_bValidTexture = false;
    bool m_bLoadAttempted = false;
    char m_szFilename[256];

    static const int NULL_TEXTURE_ID = -1;
};


class CBaseSaveGameDialog;
class CSaveGameButton : public CNextUIButton {
    DECLARE_CLASS_SIMPLE(CSaveGameButton, CNextUIButton);

public:
    CSaveGameButton(CBaseSaveGameDialog *pParent)
        : BaseClass(pParent, "", ""), m_pParent(pParent)
    {
        // HACK: we should set this from the outside
        SetScheme(CNextUI::GetInstance()->GetScheme());

        SetPaintEnabled(true);
        SetPaintBackgroundEnabled(true);
        SetPaintBorderEnabled(false);

        // we want to be selectable
        SetSticky(true);

        m_pAutoSaveImage = new vgui::ImagePanel(this, "");
        m_pUserSaveImage = new CTGAImagePanel(this);
    }

    virtual void ApplySchemeSettings(vgui::IScheme *pScheme)
    {
        BaseClass::ApplySchemeSettings(pScheme);

        m_flHeight = YRES(atof(pScheme->GetResourceString("SaveDialog.Button.Height")));

        m_BackgroundColor_Normal = GetSchemeColor("SaveDialog.Button.BackgroundColor.Normal", pScheme);
        m_BackgroundColor_Highlight = GetSchemeColor("SaveDialog.Button.BackgroundColor.Highlight", pScheme);
        m_TextColor_Normal = GetSchemeColor("SaveDialog.Button.TextColor.Normal", pScheme);
        m_TextColor_Highlight = GetSchemeColor("SaveDialog.Button.TextColor.Highlight", pScheme);

        m_BackgroundColor = m_BackgroundColor_Normal;
        m_TextColor = m_TextColor_Normal;

        SetTall(m_flHeight);

        // After scheme reloading we should re-animate the panel
        // so the animation controller re-calculates the correct
        // values for whatever stuff that has been stuck after
        // video mode change (eg. proportional offsets).
        m_bLastHighlight = !m_bHighlight;
    }

    virtual void SetParent(CBaseSaveGameDialog *pParent)
    {
        BaseClass::SetParent(pParent);
        m_pParent = pParent;
    }

    virtual void OnThink()
    {
        BaseClass::OnThink();

        if(m_bLastHighlight != m_bHighlight) {
            vgui::AnimationController *pAC = CNextUI::GetInstance()->GetAnimationController();
            m_bLastHighlight = m_bHighlight;
            if(m_bLastHighlight) {
                pAC->RunAnimationCommand(this, "BackgroundColor", m_BackgroundColor_Highlight, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
                pAC->RunAnimationCommand(this, "TextColor", m_TextColor_Highlight, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
            }
            else {
                pAC->RunAnimationCommand(this, "BackgroundColor", m_BackgroundColor_Normal, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
                pAC->RunAnimationCommand(this, "TextColor", m_TextColor_Normal, 0.0f, 0.25f, pAC->INTERPOLATOR_LINEAR);
            }
        }
    }

private:
    bool m_bLastHighlight = false;
    CBaseSaveGameDialog *m_pParent;
    vgui::ImagePanel *m_pAutoSaveImage;
    CTGAImagePanel *m_pUserSaveImage;

private:
    CPanelAnimationVar(Color, m_BackgroundColor, "BackgroundColor", "0 0 0 0");
    CPanelAnimationVar(Color, m_TextColor, "TextColor", "0 0 0 0");

private:
    float m_flWidth = 0.0f;
    float m_flHeight = 0.0f;
    Color m_BackgroundColor_Normal = Color();
    Color m_BackgroundColor_Highlight = Color();
    Color m_TextColor_Normal = Color();
    Color m_TextColor_Highlight = Color();
};

class CBaseSaveGameDialog : public CNextUIDialog {
    DECLARE_CLASS_SIMPLE(CBaseSaveGameDialog, CNextUIDialog);

public:
    CBaseSaveGameDialog(vgui::Panel *pParent, const char *pszPanelName)
        : BaseClass(pParent, pszPanelName), m_vecSaveList(), m_pSaveListPanel(NULL)
    {
        m_pSaveListPanel = new vgui::PanelListPanel(this, "");
        m_pSaveListPanel->SetFirstColumnWidth(0);
    }

    virtual void OnActivate() override
    {
        BaseClass::OnActivate();

        const char *pszWildcard = "save/*.sav";
        const char szHLSave[] = "HLSave";

        m_pSaveListPanel->DeleteAllItems();
        m_vecSaveList.RemoveAll();

        FileFindHandle_t ffp;
        const char *pszFilename = g_pFullFileSystem->FindFirstEx(pszWildcard, "MOD", &ffp);
        while(pszFilename) {
            if(!Q_strnicmp(pszFilename, szHLSave, sizeof(szHLSave) - 1)) {
                pszFilename = g_pFullFileSystem->FindNext(ffp);
                continue;
            }

            char szFilename[64] = { 0 };
            Q_snprintf(szFilename, sizeof(szFilename), "save/%s", pszFilename);

            SaveFileInfo save = { };
            if(SaveFileInfo::ParseFile(szFilename, pszFilename, save)) {
                ConColorMsg(LOG_COLOR_B_RED, "NextUI DebugButtPlug: found savegame: %s (%s)\n", save.m_szComment, save.m_szMapName);
                m_vecSaveList.AddToTail(save);
            }

            pszFilename = g_pFullFileSystem->FindNext(ffp);
        }
        g_pFullFileSystem->FindClose(ffp);

        // sort the saves
        qsort(m_vecSaveList.Base(), m_vecSaveList.Count(), sizeof(SaveFileInfo), &SaveFileInfo::SortFunc);

        // TODO: FILL THE PANEL LIST PANEL
    }

protected:
    vgui::PanelListPanel *m_pSaveListPanel;
    CUtlVector<SaveFileInfo> m_vecSaveList;
};

CON_COMMAND(nextui_basesavetest, "SUSSY BAKA")
{
    CNextUI *pUI = CNextUI::GetInstance();

    static CBaseSaveGameDialog *s_pDialog = NULL;
    if(!s_pDialog) {
        s_pDialog = new CBaseSaveGameDialog(pUI->GetRootPanel(), "TestDialog");
        s_pDialog->SetTitle(L"Test");
        s_pDialog->AddButton(new CNextUIDialogButton_CloseParent(s_pDialog, L"CLOSE"));
    }

    pUI->SetCurrentScreen(s_pDialog);
}
