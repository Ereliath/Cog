#include "CogWindowManager.h"

#include "CogDebugDrawImGui.h"
#include "CogImguiHelper.h"
#include "CogImguiInputHelper.h"
#include "CogWindow_Layouts.h"
#include "CogWindow_Settings.h"
#include "CogWindow_Spacing.h"
#include "CogWindowConsoleCommandManager.h"
#include "CogWindowHelper.h"
#include "CogWindowWidgets.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerInput.h"
#include "HAL/IConsoleManager.h"
#include "imgui_internal.h"
#include "Misc/CoreMisc.h"
#include "NetImgui_Api.h"

FString UCogWindowManager::ToggleInputCommand   = TEXT("Cog.ToggleInput");
FString UCogWindowManager::DisableInputCommand  = TEXT("Cog.DisableInput");
FString UCogWindowManager::LoadLayoutCommand    = TEXT("Cog.LoadLayout");
FString UCogWindowManager::SaveLayoutCommand    = TEXT("Cog.SaveLayout");
FString UCogWindowManager::ResetLayoutCommand   = TEXT("Cog.ResetLayout");

//--------------------------------------------------------------------------------------------------------------------------
UCogWindowManager::UCogWindowManager()
{
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::InitializeInternal()
{
    Context.Initialize();

    FCogImGuiContextScope ImGuiContextScope(Context);

    ImGuiSettingsHandler IniHandler;
    IniHandler.TypeName = "Cog";
    IniHandler.TypeHash = ImHashStr("Cog");
    IniHandler.ClearAllFn = SettingsHandler_ClearAll;
    IniHandler.ReadOpenFn = SettingsHandler_ReadOpen;
    IniHandler.ReadLineFn = SettingsHandler_ReadLine;
    IniHandler.ApplyAllFn = SettingsHandler_ApplyAll;
    IniHandler.WriteAllFn = SettingsHandler_WriteAll;
    IniHandler.UserData = this;
    ImGui::AddSettingsHandler(&IniHandler);

    SpaceWindows.Add(AddWindow<FCogWindow_Spacing>("Spacing 1", false));
    SpaceWindows.Add(AddWindow<FCogWindow_Spacing>("Spacing 2", false));
    SpaceWindows.Add(AddWindow<FCogWindow_Spacing>("Spacing 3", false));
    SpaceWindows.Add(AddWindow<FCogWindow_Spacing>("Spacing 4", false));

    Settings = GetConfig<UCogWindowConfig_Settings>();
    OnShortcutsDefined();
    
    LayoutsWindow = AddWindow<FCogWindow_Layouts>("Window.Layouts", false);
    SettingsWindow = AddWindow<FCogWindow_Settings>("Window.Settings", false);

    FCogWindowConsoleCommandManager::RegisterWorldConsoleCommand(
        *ToggleInputCommand, 
        TEXT("Toggle the input focus between the Game and ImGui"),
        GetWorld(),
        FCogWindowConsoleCommandDelegate::CreateLambda([this](const TArray<FString>& InArgs, UWorld* InWorld)
        {
            ToggleInputMode();
        }));

    FCogWindowConsoleCommandManager::RegisterWorldConsoleCommand(
        *DisableInputCommand,
        TEXT("Disable ImGui input"), 
        GetWorld(),
        FCogWindowConsoleCommandDelegate::CreateLambda([this](const TArray<FString>& InArgs, UWorld* InWorld)
        {
            DisableInputMode();
        }));

    FCogWindowConsoleCommandManager::RegisterWorldConsoleCommand(
        *ResetLayoutCommand,
        TEXT("Reset the layout."),
        GetWorld(),
        FCogWindowConsoleCommandDelegate::CreateLambda([this](const TArray<FString>& InArgs, UWorld* InWorld)
        {
            if (InArgs.Num() > 0)
            {
                ResetLayout();
            }
        }));

    FCogWindowConsoleCommandManager::RegisterWorldConsoleCommand(
        *LoadLayoutCommand,
        TEXT("Load the layout. Cog.LoadLayout <Index>"),
        GetWorld(),
        FCogWindowConsoleCommandDelegate::CreateLambda([this](const TArray<FString>& InArgs, UWorld* InWorld)
        {
            if (InArgs.Num() > 0)
            {
                LoadLayout(FCString::Atoi(*InArgs[0]));
            }
        }));

    FCogWindowConsoleCommandManager::RegisterWorldConsoleCommand(
        *SaveLayoutCommand,
        TEXT("Save the layout. Cog.SaveLayout <Index>"),
        GetWorld(),
        FCogWindowConsoleCommandDelegate::CreateLambda([this](const TArray<FString>& InArgs, UWorld* InWorld)
        {
            if (InArgs.Num() > 0)
            {
                SaveLayout(FCString::Atoi(*InArgs[0]));
            }
        }));

    IsInitialized = true;

}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::Shutdown()
{
    FCogImGuiContextScope ImGuiContextScope(Context);

    //------------------------------------------------------------------
    // Call PreSaveConfig before destroying imgui context
    // if PreSaveConfig needs to read ImGui IO for example
    //------------------------------------------------------------------
    for (FCogWindow* Window : Windows)
    {
        Window->PreSaveConfig();
    }

    //------------------------------------------------------------------
    // Destroy ImGui before destroying the windows to make sure 
    // imgui serialize their visibility state in imgui.ini
    //------------------------------------------------------------------
    if (IsInitialized == true)
    {
        Context.Shutdown();
    }

    SaveConfig();

    for (FCogWindow* Window : Windows)
    {
        Window->Shutdown();
        delete Window;
    }
    Windows.Empty();

    for (UCogCommonConfig* Config : Configs)
    {
        Config->SaveConfig();
    }

    FCogWindowConsoleCommandManager::UnregisterAllWorldConsoleCommands(GetWorld());
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::Tick(float DeltaTime)
{
    FCogImGuiContextScope ImGuiContextScope(Context);

    if (GEngine->GameViewport == nullptr && IsRunningDedicatedServer() == false)
    {
        return;
    }
    
    if (IsInitialized == false)
    {
        InitializeInternal();
    }
    
    if (LayoutToLoad != -1)
    {
	    const FString Filename = FCogImguiHelper::GetIniFilePath(FString::Printf(TEXT("ImGui_Layout_%d"), LayoutToLoad));
        ImGui::LoadIniSettingsFromDisk(TCHAR_TO_ANSI(*Filename));
        LayoutToLoad = -1;
    }

    for (FCogWindow* Window : Windows)
    {
        Window->GameTick(DeltaTime);
    }

    const bool shouldSkipRendering = NetImgui::IsConnected() && bIsSelectionModeActive == false;
    Context.SetSkipRendering(shouldSkipRendering);

    if (Context.BeginFrame(DeltaTime))
    {
        HandleInputs();
        Render(DeltaTime);
        Context.EndFrame();
    }
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::Render(float DeltaTime)
{
    FCogImGuiContextScope ImGuiContextScope(Context);
    
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoDockingOverCentralNode | ImGuiDockNodeFlags_AutoHideTabBar);
    ImGui::PopStyleColor(1);

    const bool bCompactSaved = Settings->bCompactMode;
    if (bCompactSaved)
    {
        FCogWindowWidgets::PushStyleCompact();
    }

    //----------------------------------------------------------------------
    // There is no need to have Imgui input enabled if the imgui rendering 
    // is only done on the NetImgui server. So we disable imgui input.
    //----------------------------------------------------------------------
    if (Context.GetEnableInput() && NetImgui::IsConnected() && bIsSelectionModeActive == false)
    {
        Context.SetEnableInput(false);
    }

    if ((Context.GetEnableInput() || NetImgui::IsConnected()) && bIsSelectionModeActive == false)
    {
        RenderMainMenu();
    }

    for (FCogWindow* Window : Windows)
    {
        Window->RenderTick(DeltaTime);

        if (Window->GetIsVisible() && bIsSelectionModeActive == false)
        {
            if (Settings->bTransparentMode)
            {
                ImGui::SetNextWindowBgAlpha(0.35f);
            }

            Window->Render(DeltaTime);
        }
    }
    
    if (bCompactSaved)
    {
        FCogWindowWidgets::PopStyleCompact();
    }

    FCogDebugDrawImGui::Draw();
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::AddWindow(FCogWindow* Window, const FString& Name, const bool AddToMainMenu /*= true*/)
{
    Window->SetFullName(Name);
    Window->SetOwner(this);
    Windows.Add(Window);
    Window->Initialize();

    if (Window->HasWidget())
    {
        Widgets.Add(Window);
        //Widgets.Sort()
    }

    if (AddToMainMenu)
    {
        if (FMenu* Menu = AddMenu(Window->GetFullName()))
        {
            Menu->Window = Window;
        }
    }
}

//--------------------------------------------------------------------------------------------------------------------------
FCogWindow* UCogWindowManager::FindWindowByID(const ImGuiID ID)
{
    for (FCogWindow* Window : Windows)
    {
        if (Window->GetID() == ID)
        {
            return Window;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::ResetLayout()
{
    FCogImGuiContextScope ImGuiContextScope(Context);

    for (const FCogWindow* Window : Windows)
    {
        ImGui::SetWindowPos(TCHAR_TO_ANSI(*Window->GetName()), ImVec2(10, 10), ImGuiCond_Always);
    }

    ImGui::ClearIniSettings();
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::CloseAllWindows()
{
    for (FCogWindow* Window : Windows)
    {
        Window->SetIsVisible(false);
    }
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::LoadLayout(const int32 LayoutIndex)
{
    for (FCogWindow* Window : Windows)
    {
        Window->SetIsVisible(false);
    }

    LayoutToLoad = LayoutIndex;
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::SaveLayout(const int32 LayoutIndex)
{
    FCogImGuiContextScope ImGuiContextScope(Context);

	const FString Filename = *FCogImguiHelper::GetIniFilePath(FString::Printf(TEXT("imgui_layout_%d"), LayoutIndex));
    ImGui::SaveIniSettingsToDisk(TCHAR_TO_ANSI(*Filename));
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::SortMainMenu()
{
    MainMenu.SubMenus.Empty();

    TArray<FCogWindow*> SortedWindows = Windows;
    SortedWindows.Sort([](const FCogWindow& Lhs, const FCogWindow& Rhs) { return Lhs.GetFullName() < Rhs.GetFullName(); });

    for (FCogWindow* Window : SortedWindows)
    {
        if (FMenu* Menu = AddMenu(Window->GetFullName()))
        {
            Menu->Window = Window;
        }
    }
}

//--------------------------------------------------------------------------------------------------------------------------
UCogWindowManager::FMenu* UCogWindowManager::AddMenu(const FString& Name)
{
    TArray<FString> Path;
    Name.ParseIntoArray(Path, TEXT("."));

    FMenu* CurrentMenu = &MainMenu;
    for (int i = 0; i < Path.Num(); ++i)
    {
        FString MenuName = Path[i];

        int SubMenuIndex = CurrentMenu->SubMenus.IndexOfByPredicate([&](const FMenu& Menu) { return Menu.Name == MenuName; });
        if (SubMenuIndex != -1)
        {
            CurrentMenu = &CurrentMenu->SubMenus[SubMenuIndex];
        }
        else
        {
            CurrentMenu = &CurrentMenu->SubMenus.AddDefaulted_GetRef();
            CurrentMenu->Name = MenuName;
        }
    }

    return CurrentMenu;
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::RenderMainMenu()
{
    const UPlayerInput* PlayerInput = FCogImguiInputHelper::GetPlayerInput(*GetWorld());

    IsRenderingInMainMenu = true;

    if (ImGui::BeginMainMenuBar())
    {
        for (FMenu& Menu : MainMenu.SubMenus)
        {
            RenderOptionMenu(Menu);
        }

        if (ImGui::BeginMenu("Window"))
        {
            if (ImGui::MenuItem("Close All Windows"))
            {
                CloseAllWindows();
            }
            
            ImGui::Separator();

            RenderMenuItem(*LayoutsWindow, "Layouts");
            RenderMenuItem(*SettingsWindow, "Settings");

            if (ImGui::BeginMenu("Spacing"))
            {
                for (FCogWindow* SpaceWindow : SpaceWindows)
                {
                    bool bSpaceVisible = SpaceWindow->GetIsVisible();
                    if (ImGui::MenuItem(TCHAR_TO_ANSI(*SpaceWindow->GetName()), nullptr, &bSpaceVisible))
                    {
                        SpaceWindow->SetIsVisible(bSpaceVisible);
                    }
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        RenderWidgets();

        ImGui::EndMainMenuBar();
    }

    IsRenderingInMainMenu = false;
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::RenderWidgets()
{
    int32 numVisibleWidgets = 0;
    for (int i = 0; i < Widgets.Num(); ++i)
    {
        FCogWindow* Window = Widgets[i];
        if (Window->GetIsWidgetVisible())
        {
            numVisibleWidgets++;
        }
    }

    if (numVisibleWidgets == 0)
    { return; }
    
    int32 numColumns = numVisibleWidgets;

    if (Settings->WidgetAlignment == ECogWidgetAlignment::Right)
    {
        numColumns++;
    }
    else if (Settings->WidgetAlignment == ECogWidgetAlignment::Center)
    {
        numColumns += 2;
    }

    
    ImGuiTableFlags Flags = ImGuiTableFlags_None;
    if (Settings->ShowWidgetBorders)
    {
        Flags |= ImGuiTableFlags_BordersInnerV;
        if (Settings->WidgetAlignment == ECogWidgetAlignment::Left)
        {
            Flags |= ImGuiTableFlags_BordersOuterV;
        }
    } 

    if (Settings->WidgetAlignment == ECogWidgetAlignment::Manual)
    {
        Flags |= ImGuiTableFlags_Resizable;
        if (Settings->ShowWidgetBorders == false)
        {
            Flags |= ImGuiTableFlags_NoBordersInBodyUntilResize;
        }
    }
    
    if (ImGui::BeginTable("Widgets", numColumns, Flags))
    {
        if (Settings->WidgetAlignment == ECogWidgetAlignment::Right || Settings->WidgetAlignment == ECogWidgetAlignment::Center)
        {
            ImGui::TableSetupColumn("Stretch", ImGuiTableColumnFlags_WidthStretch);
        }
        
        for (int i = 0; i < numVisibleWidgets; ++i)
        {
            ImGui::TableSetupColumn("Fixed", ImGuiTableColumnFlags_WidthFixed);
        }

        if (Settings->WidgetAlignment == ECogWidgetAlignment::Center)
        {
            ImGui::TableSetupColumn("Stretch", ImGuiTableColumnFlags_WidthStretch);
        }
        
        ImGui::TableNextRow();

        if (Settings->WidgetAlignment == ECogWidgetAlignment::Right || Settings->WidgetAlignment == ECogWidgetAlignment::Center)
        {
            ImGui::TableNextColumn();
        }

        for (int i = 0; i < Widgets.Num(); ++i)
        {
            FCogWindow* Window = Widgets[i];
            if (Window->GetIsWidgetVisible() == false)
            { continue; }
            
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
            Window->RenderMainMenuWidget();
        }

        if (Settings->WidgetAlignment == ECogWidgetAlignment::Center)
        {
            ImGui::TableNextColumn();
        }
        
        ImGui::EndTable();
    }
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::RenderOptionMenu(FMenu& Menu)
{
    if (Menu.Window != nullptr)
    {
        RenderMenuItem(*Menu.Window, TCHAR_TO_ANSI(*Menu.Name));
    }
    else
    {
        if (ImGui::BeginMenu(TCHAR_TO_ANSI(*Menu.Name)))
        {
            for (FMenu& SubMenu : Menu.SubMenus)
            {
                RenderOptionMenu(SubMenu);
            }
            ImGui::EndMenu();
        }
    }
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::RenderMenuItem(FCogWindow& Window, const char* MenuItemName)
{
    if (Settings->bShowWindowsInMainMenu)
    {
        ImGui::SetNextWindowSizeConstraints(
            ImVec2(FCogWindowWidgets::GetFontWidth() * 40, ImGui::GetTextLineHeightWithSpacing() * 5),
            ImVec2(FCogWindowWidgets::GetFontWidth() * 50, ImGui::GetTextLineHeightWithSpacing() * 60));

        if (ImGui::BeginMenu(MenuItemName))
        {
            Window.RenderContent();
            ImGui::EndMenu();
        }

        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            Window.SetIsVisible(!Window.GetIsVisible());
        }
        
        RenderMenuItemHelp(Window);
    }
    else
    {
        bool bIsVisible = Window.GetIsVisible();
        if (ImGui::MenuItem(MenuItemName, nullptr, &bIsVisible))
        {
            Window.SetIsVisible(bIsVisible);
        }
     
        RenderMenuItemHelp(Window);
    }
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::RenderMenuItemHelp(FCogWindow& Window)
{
    if (Settings->bShowHelp)
    {
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() - FCogWindowWidgets::GetFontWidth() * 3.0f);
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered())
        {
            ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(29, 42, 62, 240));
            const float HelpWidth = FCogWindowWidgets::GetFontWidth() * 80;
            ImGui::SetNextWindowSizeConstraints(ImVec2(HelpWidth / 2.0f, 0.0f), ImVec2(HelpWidth, FLT_MAX));
            if (ImGui::BeginTooltip())
            {
                ImGui::PushTextWrapPos(HelpWidth - 1 * FCogWindowWidgets::GetFontWidth());
                Window.RenderHelp();
                ImGui::Separator();
                ImGui::TextDisabled("Help can be hidden in Window/Settings.");
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
            ImGui::PopStyleColor();
        }
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(FCogWindowWidgets::GetFontWidth() * 1, 0));
    }
}



//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::SettingsHandler_ClearAll(ImGuiContext* Context, ImGuiSettingsHandler* Handler)
{
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::SettingsHandler_ApplyAll(ImGuiContext* Context, ImGuiSettingsHandler* Handler)
{
    UCogWindowManager* Manager = (UCogWindowManager*)Handler->UserData;

    Manager->Widgets.Sort([](const FCogWindow& Window1, const FCogWindow& Window2)
    {
        return Window1.GetWidgetOrderIndex() < Window2.GetWidgetOrderIndex();
    });
}

//--------------------------------------------------------------------------------------------------------------------------
void* UCogWindowManager::SettingsHandler_ReadOpen(ImGuiContext* Context, ImGuiSettingsHandler* Handler, const char* Name)
{
    if (strcmp(Name, "Windows") == 0)
    {
        return (void*)1;
    }

    if (strcmp(Name, "Widgets") == 0)
    {
        UCogWindowManager* Manager = (UCogWindowManager*)Handler->UserData;
        Manager->WidgetsOrderIndex = 0;

        return (void*)2;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::SettingsHandler_ReadLine(ImGuiContext* Context, ImGuiSettingsHandler* Handler, void* Entry, const char* Line)
{
    //-----------------------------------------------------------------------------------
	// Load the visibility of windows. 
	//-----------------------------------------------------------------------------------
    if (Entry == (void*)1)
    {
        ImGuiID Id;
        int32 ShowMenu;
#if PLATFORM_WINDOWS || PLATFORM_MICROSOFT
        if (sscanf_s(Line, "0x%08X %d", &Id, &ShowMenu) == 2)
#else
        if (sscanf(Line, "0x%08X", &Id) == 1)
#endif
        {
            UCogWindowManager* Manager = (UCogWindowManager*)Handler->UserData;
            if (FCogWindow* Window = Manager->FindWindowByID(Id))
            {
                Window->SetIsVisible(true);
                Window->bShowMenu = (ShowMenu > 0);
            }
        }
    }
    //-----------------------------------------------------------------------------------
    // Load which widgets are present in the main menu bar and with what order. 
    //-----------------------------------------------------------------------------------
    else if (Entry == (void*)2)
    {
        ImGuiID Id;
        int32 Visible = false;
#if PLATFORM_WINDOWS || PLATFORM_MICROSOFT
        if (sscanf_s(Line, "0x%08X %d", &Id, &Visible) == 2)
#else
        if (sscanf(Line, "0x%08X %d", &Id, &Visible) == 2)
#endif
        {
            UCogWindowManager* Manager = (UCogWindowManager*)Handler->UserData;
            if (FCogWindow* Window = Manager->FindWindowByID(Id))
            {
                Window->SetWidgetOrderIndex(Manager->WidgetsOrderIndex);
                Window->SetIsWidgetVisible(Visible > 0);
            }

            Manager->WidgetsOrderIndex++;
        }
    }
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::SettingsHandler_WriteAll(ImGuiContext* Context, ImGuiSettingsHandler* Handler, ImGuiTextBuffer* Buffer)
{
    const UCogWindowManager* Manager = (UCogWindowManager*)Handler->UserData;

    //-----------------------------------------------------------------------------------
	// Save the visibility of windows. Example:
	//      [Cog][Windows]
	//	    0xB5D96693
    //      0xBF3390B5
	//-----------------------------------------------------------------------------------
    Buffer->appendf("[%s][Windows]\n", Handler->TypeName);
    for (const FCogWindow* Window : Manager->Windows)
    {
        if (Window->GetIsVisible())
        {
            Buffer->appendf("0x%08X %d\n", Window->GetID(), (int32)Window->bShowMenu);
        }
    }
    Buffer->append("\n");

    //-----------------------------------------------------------------------------------
	// Save which widgets are present in the main menu bar and with what order. Example:
    //      [Cog][Widgets]
    //      0x639F1181 1
	//      0x52BDE3E0 1
    //-----------------------------------------------------------------------------------
    Buffer->appendf("[%s][Widgets]\n", Handler->TypeName);
    for (const FCogWindow* Window : Manager->Widgets)
    {
        Buffer->appendf("0x%08X %d\n", Window->GetID(), Window->GetIsWidgetVisible());
    }
    Buffer->append("\n");
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::ResetAllWindowsConfig()
{
    for (FCogWindow* Window : Windows)
    {
        Window->ResetConfig();
    }
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::AddCommand(UPlayerInput* PlayerInput, const FString& Command, const FKey& Key)
{
    if (PlayerInput == nullptr)
    {
        return;
    }

    //---------------------------------------------------
    // Reassign conflicting commands
    //---------------------------------------------------
    for (FKeyBind& KeyBind : PlayerInput->DebugExecBindings)
    {
        if (KeyBind.Key == Key && KeyBind.Command != Command)
        {
            KeyBind.Control = true;
            KeyBind.bIgnoreCtrl = false;
        }
    }

    //---------------------------------------------------
    // Find or add desired command
    //---------------------------------------------------
    FKeyBind* ExistingKeyBind = PlayerInput->DebugExecBindings.FindByPredicate([Command](const FKeyBind& KeyBind) { return KeyBind.Command == Command; });
    if (ExistingKeyBind == nullptr)
    {
        ExistingKeyBind = &PlayerInput->DebugExecBindings.AddDefaulted_GetRef();
    }

    //---------------------------------------------------
    // Assign the key to the command
    //---------------------------------------------------
    FKeyBind CogKeyBind;
    CogKeyBind.Command = Command;
    CogKeyBind.Control = false;
    CogKeyBind.bIgnoreCtrl = true;
    CogKeyBind.Key = Key;

    *ExistingKeyBind = CogKeyBind;
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::SortCommands(UPlayerInput* PlayerInput)
{
    PlayerInput->DebugExecBindings.Sort([](const FKeyBind& Key1, const FKeyBind& Key2)
    {
        return Key1.Command.Compare(Key2.Command) < 0;
    });
}

//--------------------------------------------------------------------------------------------------------------------------
UCogCommonConfig* UCogWindowManager::GetConfig(const TSubclassOf<UCogCommonConfig> ConfigClass)
{
    const UClass* Class = ConfigClass.Get();

    for (UCogCommonConfig* Config : Configs)
    {
        if (Config && Config->IsA(Class))
        {
            return Cast<UCogCommonConfig>(Config);
        }
    }

    UCogCommonConfig* Config = NewObject<UCogCommonConfig>(this, Class);
    Configs.Add(Config);
    return Config;
}

//--------------------------------------------------------------------------------------------------------------------------
const UObject* UCogWindowManager::GetAsset(const TSubclassOf<UObject> AssetClass) const
{
    const UClass* Class = AssetClass.Get();

    for (const UObject* Asset : Assets)
    {
        if (Asset && Asset->IsA(Class))
        {
            return Asset;
        }
    }

    const UObject* Asset = FCogWindowHelper::GetFirstAssetByClass(AssetClass);
    if (Asset == nullptr)
    {
        return nullptr;
    }

    Assets.Add(Asset);

    return Asset;
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::ToggleInputMode()
{
    UE_LOG(LogCogImGui, Verbose, TEXT("UCogWindowManager::ToggleInputMode"));
    Context.SetEnableInput(!Context.GetEnableInput());
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::DisableInputMode()
{
    UE_LOG(LogCogImGui, Verbose, TEXT("UCogWindowManager::DisableInputMode"));
    Context.SetEnableInput(false);
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::HandleInputs()
{
    const UPlayerInput* PlayerInput = FCogImguiInputHelper::GetPlayerInput(*GetWorld());
    if (PlayerInput == nullptr)
    { return; }

    if (Settings->bDisableShortcutsWhenImGuiWantTextInput && ImGui::GetIO().WantTextInput)
    { return; }

    if (FCogImguiInputHelper::IsKeyInfoPressed(PlayerInput, Settings->ToggleImGuiInputShortcut))
    {
        ToggleInputMode();
    }
    else if (FCogImguiInputHelper::IsKeyInfoPressed(PlayerInput, Settings->ToggleSelectionShortcut))
    {
        SetActivateSelectionMode(!GetActivateSelectionMode());
    }
    for (int i = 0; i < Settings->LoadLayoutShortcuts.Num(); ++i)
    {
        if (FCogImguiInputHelper::IsKeyInfoPressed(PlayerInput, Settings->LoadLayoutShortcuts[i]))
        {
            LoadLayout(i);
        }
    }
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::SetActivateSelectionMode(const bool Value)
{
    SelectionModeActiveCounter = FMath::Max(SelectionModeActiveCounter + (Value ? 1 : -1), 0);
    bIsSelectionModeActive = SelectionModeActiveCounter > 0;

    if (bIsSelectionModeActive)
    {
        bIsInputEnabledBeforeEnteringSelectionMode = GetContext().GetEnableInput();

        Context.SetEnableInput(true);
    }
    else
    {
        GetContext().SetEnableInput(bIsInputEnabledBeforeEnteringSelectionMode);
    }
}

//--------------------------------------------------------------------------------------------------------------------------
bool UCogWindowManager::GetActivateSelectionMode() const
{
    return SelectionModeActiveCounter > 0;
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogWindowManager::OnShortcutsDefined() const
{
    if (GetWorld() == nullptr)
    { return; }

    TArray Shortcuts = { Settings->ToggleImGuiInputShortcut, Settings->ToggleSelectionShortcut };
    Shortcuts.Append(Settings->LoadLayoutShortcuts);
    Shortcuts.Append(Settings->SaveLayoutShortcuts);
    
    FCogImguiInputHelper::SetShortcuts(*GetWorld(), Shortcuts, Settings->bDisableConflictingCommands);
}

