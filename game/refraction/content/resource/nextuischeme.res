Scheme {
    Colors {
        NullTransp  "0   0   0   0"
        NullSolid   "0   0   0   255"
        BgDim       "0   0   0   64"
        BgBold      "0   0   0   196"
        BgHighlight "255 255 255 255"
        FgHighlight "255 127 0   255"
    }

    BaseSettings {
        GameMenu.Background.FadeColor               "BgBold"
        GameMenu.Background.GradientWidthPercent    "25"
        GameMenu.Button.RolloverSound               "ui/buttonrollover.wav"
        GameMenu.Button.ClickSound                  "ui/buttonclickrelease.wav"
        GameMenu.Button.Width                       "196"
        GameMenu.Button.Height                      "32"
        GameMenu.Button.HighlightWidth.Normal       "0"
        GameMenu.Button.HighlightWidth.Highlight    "6"
        GameMenu.Button.TextOffsetX.Normal          "4"
        GameMenu.Button.TextOffsetX.Highlight       "16"
        GameMenu.Button.TextOffsetY.Normal          "0"
        GameMenu.Button.TextOffsetY.Highlight       "0"
        GameMenu.Button.BackgroundColor.Normal      "NullTransp"
        GameMenu.Button.BackgroundColor.Highlight   "BgBold"
        GameMenu.Button.HighlightColor.Normal       "NullTransp"
        GameMenu.Button.HighlightColor.Highlight    "FgHighlight"
        GameMenu.Button.TextColor.Normal            "FgNormal"
        GameMenu.Button.TextColor.Highlight         "FgHighlight"
        GameMenu.ButtonStack.PositionX              "64"
        GameMenu.ButtonStack.PositionY              "160"
        GameMenu.Title.Color                        "FgNormal"
        GameMenu.Title.PositionX                    "64"
        GameMenu.Title.PositionY                    "64"
        Popup.Background.FadeColor                  "BgBold"
        Popup.Background.FadeTime                   "1.0"
        Popup.Background.GradientHeightPercent      "25"
        Popup.Button.Width                          "128"
        Popup.Button.Height                         "32"
        Popup.Button.BackgroundColor.Normal         "BgDim"
        Popup.Button.BackgroundColor.Highlight      "BgHighlight"
        Popup.Button.TextColor.Normal               "FgNormal"
        Popup.Button.TextColor.Highlight            "NullSolid"
        Popup.Button.BaseX                          "16"
        Popup.Button.BaseY                          "16"
        Popup.Text.Color                            "FgNormal"
        Popup.Text.OffsetY                          "64"
    }

    Fonts {
        Default {
            1 {
                name            "Noto Sans"
                tall            "16"
                weight          "0"
                custom          "1"
            }
        }
    
        GameMenuButton {
            1 {
                name            "Alte DIN 1451 Mittelschrift"
                tall            "24"
                weight          "250"
                antialias       "1"
                custom          "1"
            }
        }
        
        GameMenuTitle {
            1 {
                name            "K12HL2"
                tall            "64"
                weight          "400"
                antialias       "1"
                custom          "1"
            }
        }
        
        PopupButton {
            1 {
                name            "Noto Sans"
                tall            "32"
                weight          "500"
                antialias       "1"
                custom          "1"
            }
        }
        
        PopupText {
            1 {
                name            "Noto Sans"
                tall            "32"
                weight          "1200"
                antialias       "1"
                custom          "1"
            }
        }
    }
    
    CustomFontFiles {
        1 "resource/fonts/NotoSans-Regular.ttf"
        2 "resource/fonts/din1451alt.ttf"
        3 "resource/fonts/K12HL2.ttf"
    }
}
