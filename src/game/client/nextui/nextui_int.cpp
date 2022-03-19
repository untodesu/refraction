#include "cbase.h"
#include "filesystem.h"
#include "nextui_int.h"
#include "ienginevgui.h"
#include "savegame_version.h"
#include "vgui/ILocalize.h"
#include "vgui_controls/Panel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static CNextUI s_NextUI;
static IGameUI *s_pGameUI = NULL;
static CDllDemandLoader s_GameUIDll("GameUI");
CBaseGameSystemPerFrame *g_pNextUI = &s_NextUI;

static bool SaveReadNameAndComment(FileHandle_t fp, char *pszName, int iNameSize, char *pszComment, int iCommentSize)
{
    int i, tag, size, tokenSize, tokenCount;
    char *pSaveData, *pFieldName, **pTokenList;

    pszName[0] = '\0';
    pszComment[0] = '\0';

    g_pFullFileSystem->Read(&tag, sizeof(int), fp);
    if(tag != MAKEID('J', 'S', 'A', 'V'))
        return false;

    g_pFullFileSystem->Read(&tag, sizeof(int), fp);
    if(tag != SAVEGAME_VERSION)
        return false;

    g_pFullFileSystem->Read(&size, sizeof(int), fp);
    g_pFullFileSystem->Read(&tokenCount, sizeof(int), fp); // These two ints are the token list
    g_pFullFileSystem->Read(&tokenSize, sizeof(int), fp);
    size += tokenSize;

    // Sanity Check.
    if(tokenCount < 0 || tokenCount > 1024 * 1024 * 32)
        return false;
    if(tokenSize < 0 || tokenSize > 1024 * 1024 * 32)
        return false;

    pSaveData = (char *)new char[size];
    g_pFullFileSystem->Read(pSaveData, size, fp);

    int nNumberOfFields;
    char *pData = pSaveData;
    int nFieldSize;

    // Allocate a table for the strings, and parse the table
    if(tokenSize > 0) {
        pTokenList = new char *[tokenCount];

        // Make sure the token strings pointed to by the pToken hashtable.
        for(i = 0; i < tokenCount; i++) {
            pTokenList[i] = *pData ? pData : NULL;	// Point to each string in the pToken table
            while(*pData++); // Find next token (after next null)
        }
    }
    else {
        pTokenList = NULL;
    }

    // short, short (size, index of field name)
    nFieldSize = *((short *)pData);
    pData += sizeof(short);
    pFieldName = pTokenList[*((short *)pData)];

    if(stricmp(pFieldName, "GameHeader")) {
        delete[] pSaveData;
        return false;
    }

    // int (fieldcount)
    pData += sizeof(short);
    nNumberOfFields = *(int *)pData;
    pData += nFieldSize;

    // Each field is a short (size), short (index of name), binary string of "size" bytes (data)
    for(i = 0; i < nNumberOfFields; i++) {
        // Data order is:
        // Size
        // szName
        // Actual Data

        nFieldSize = *(short *)pData;
        pData += sizeof(short);

        pFieldName = pTokenList[*(short *)pData];
        pData += sizeof(short);

        if(!Q_stricmp(pFieldName, "comment")) {
            int copySize = MAX(iCommentSize, nFieldSize);
            Q_strncpy(pszComment, pData, copySize);
        }
        else if(!Q_stricmp(pFieldName, "mapName")) {
            int copySize = MAX(iNameSize, nFieldSize);
            Q_strncpy(pszName, pData, copySize);
        }

        // Move to Start of next field.
        pData += nFieldSize;
    };

    // Delete the string table we allocated
    delete[] pTokenList;
    delete[] pSaveData;

    if(Q_strlen(pszName) > 0 && Q_strlen(pszComment) > 0)
        return true;

    return false;
}

bool SaveFileInfo::ParseFile(const char *pszFilename, const char *pszShortName, SaveFileInfo &save)
{
    if(!pszFilename || !pszShortName)
        return false;

    Q_strncpy(save.m_szFilename, pszFilename, sizeof(save.m_szFilename));
    Q_strncpy(save.m_szShortName, pszShortName, sizeof(save.m_szShortName));

    FileHandle_t fp = g_pFullFileSystem->Open(pszFilename, "rb", "MOD");
    if(!fp)
        return false;

    char szCommentRaw[128] = { 0 };
    char *pszCommentRaw = szCommentRaw;
    bool bReadOk = SaveReadNameAndComment(fp, save.m_szMapName, sizeof(save.m_szMapName), szCommentRaw, sizeof(szCommentRaw));
    g_pFullFileSystem->Close(fp);

    if(!bReadOk)
        return false;

    // We only need the chapter name - legacy UI and GamepadUI
    // don't seem to use elapsed time values like anywhere.
    if(Q_strlen(pszCommentRaw) >= 6) {
        // go to the end and ignore the timestamp
        // which is the last six bytes of the comment.
        while(*pszCommentRaw)
            pszCommentRaw++;
        pszCommentRaw -= 7;

        // eat all the trailing whitespace
        // pointer math 2: electric boogalo
        while(pszCommentRaw > szCommentRaw && pszCommentRaw[-1] == ' ')
            pszCommentRaw--;
        pszCommentRaw[0] = 0;
    }

    Q_strncpy(save.m_szComment, szCommentRaw, sizeof(save.m_szComment));

    // Now get the file timestamp.
    long lFileTime = g_pFullFileSystem->GetFileTime(pszFilename, "MOD");
    save.m_uTimestamp = (unsigned int)lFileTime;
    g_pFullFileSystem->FileTimeToString(save.m_szFileTime, sizeof(save.m_szFileTime), lFileTime);
    char *pszNewline = strchr(save.m_szFileTime, '\n');
    if(pszNewline) {
        // eat the newline
        *pszNewline = 0;
    }

    return true;
}

void SaveFileInfo::FindSaveSlot(char *pszBuffer, int iBufferSize)
{
    static char szFilename[1024] = { 0 };

    pszBuffer[0] = 0;
    for(int i = 0; i < 1000; i++) {
        Q_snprintf(szFilename, sizeof(szFilename), "save/half-life-%03d.sav", i);

        FileHandle_t fp = g_pFullFileSystem->Open(szFilename, "rb");
        if(!fp) {
            Q_strncpy(pszBuffer, szFilename + 5, iBufferSize);
            char *pszExtension = Q_strstr(pszBuffer, ".sav");
            if(pszExtension)
                pszExtension[0] = 0;
            return;
        }

        g_pFullFileSystem->Close(fp);
    }

    ConColorMsg(LOG_COLOR_B_RED, "NextUI: unable to generate a new save file");
    Assert(0);
}

int SaveFileInfo::SortFunc(const void *p1, const void *p2)
{
    const SaveFileInfo *s1 = reinterpret_cast<const SaveFileInfo *>(p1);
    const SaveFileInfo *s2 = reinterpret_cast<const SaveFileInfo *>(p2);
    if(s1->m_uTimestamp < s2->m_uTimestamp)
        return 1;
    if(s1->m_uTimestamp > s2->m_uTimestamp)
        return -1;
    return Q_strcmp(s1->m_szFilename, s2->m_szFilename);
}

const char *CNextUI::Name()
{
    return "NextUI";
}

bool CNextUI::Init()
{
    CreateInterfaceFn pfnGameUiFactory = s_GameUIDll.GetFactory();
    if(!pfnGameUiFactory) {
        ConColorMsg(LOG_COLOR_B_RED, "NextUI: unable to load GameUI module.\n");
        return false;
    }

    s_pGameUI = (IGameUI *)pfnGameUiFactory(GAMEUI_INTERFACE_VERSION, NULL);
    if(!s_pGameUI) {
        ConColorMsg(LOG_COLOR_B_RED, "NextUI: unable to load IGameUI.\n");
        return false;
    }

    m_hScheme = g_pVGuiSchemeManager->LoadSchemeFromFile("resource/nextuischeme.res", "NextUI");

    m_pRootPanel = new CNextUIRootPanel(enginevgui->GetPanel(PANEL_GAMEUIDLL));
    m_pRootPanel->SetScheme(m_hScheme);

    m_pAnimationController = new vgui::AnimationController(m_pRootPanel);
    m_pAnimationController->SetProportional(false);
    m_pAnimationController->SetScheme(m_hScheme);

    s_pGameUI->SetMainMenuOverride(m_pRootPanel->GetVPanel());

    m_pGameMenuScreen = new CGameMenu(m_pRootPanel);
    m_pCurrentScreen = m_pGameMenuScreen;

    return true;
}

void CNextUI::PostInit()
{
    ConColorMsg(LOG_COLOR_B_GREEN, "NextUI: PostInit\n");
}

void CNextUI::Shutdown()
{
    ConColorMsg(LOG_COLOR_B_GREEN, "NextUI: Shutdown\n");
}

void CNextUI::VidInit()
{
    ConColorMsg(LOG_COLOR_B_GREEN, "NextUI: VidInit\n");
}

void CNextUI::Update(float frametime)
{
    m_pAnimationController->UpdateAnimations(Plat_FloatTime());
}

vgui::HScheme CNextUI::GetScheme() const
{
    return m_hScheme;
}

vgui::AnimationController *CNextUI::GetAnimationController() const
{
    return m_pAnimationController;
}

CNextUIRootPanel *CNextUI::GetRootPanel() const
{
    return m_pRootPanel;
}

void CNextUI::GetLocalizedString(const char *pszSource, wchar_t *pszDest, int iDestChars) const
{
    const wchar_t *pszValue = g_pVGuiLocalize->Find(pszSource);
    if(pszValue) {
        Q_wcsncpy(pszDest, pszValue, sizeof(wchar_t) * iDestChars);
        return;
    }

    g_pVGuiLocalize->ConvertANSIToUnicode(pszSource, pszDest, iDestChars);
}

void CNextUI::SetCurrentScreen(CNextUIScreen *pScreen)
{
    if(m_pCurrentScreen != pScreen) {
        if(m_pCurrentScreen)
            m_pCurrentScreen->SetVisible(false);
        m_pPreviousScreen = m_pCurrentScreen;
        m_pCurrentScreen = pScreen;
        m_pCurrentScreen->SetVisible(true);
        m_pCurrentScreen->Activate();
    }
}

void CNextUI::UnsetCurrentScreen(CNextUIScreen *pScreen)
{
    if(m_pCurrentScreen == pScreen) {
        m_pCurrentScreen->SetVisible(false);
        m_pCurrentScreen = m_pPreviousScreen;
        if(!m_pCurrentScreen)
            m_pCurrentScreen = m_pGameMenuScreen;
        m_pCurrentScreen->SetVisible(true);
        m_pPreviousScreen = NULL;
    }
}

CNextUIScreen *CNextUI::GetCurrentScreen()
{
    return m_pCurrentScreen;
}

IGameUI *CNextUI::GetGameUI()
{
    return s_pGameUI;
}

CNextUI *CNextUI::GetInstance()
{
    return &s_NextUI;
}
