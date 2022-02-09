Scheme {
    Colors {
        NullTransp  "0   0   0   0"
        NullSolid   "0   0   0   255"
        BgDim       "0   0   0   64"
        BgBold      "0   0   0   196"
        BgHighlight "255 255 255 255"
        FgHighlight "255 127 0   255"
        FgTitle     "255 255 255 196"
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
        GameMenu.Button.TextColor.Normal            "FgTitle"
        GameMenu.Button.TextColor.Highlight         "FgHighlight"
        GameMenu.ButtonStack.PositionX              "64"
        GameMenu.ButtonStack.PositionY              "192"
        GameMenu.Title.Color                        "FgTitle"
        GameMenu.Title.PositionX                    "64"
        GameMenu.Title.PositionY                    "128"
        Screen.Title.Color                          "FgTitle"
        Screen.Title.PositionX                      "32"
        Screen.Title.PositionY                      "32"
        Dialog.Background.FadeColor                 "BgBold"
        Dialog.Background.FadeTime                  "2.0"
        Dialog.Background.GradientHeightPercent     "25"
        Dialog.Button.Width                         "128"
        Dialog.Button.Height                        "32"
        Dialog.Button.BackgroundColor.Normal        "BgDim"
        Dialog.Button.BackgroundColor.Highlight     "BgHighlight"
        Dialog.Button.TextColor.Normal              "FgTitle"
        Dialog.Button.TextColor.Highlight           "NullSolid"
        Dialog.Button.BaseX                         "16"
        Dialog.Button.BaseY                         "16"
        Dialog.Text.Color                           "FgNormal"
        Dialog.Text.PositionX                       "32"
        Dialog.Text.PositionY                       "80"
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
        
        ScreenTitle {
            1 {
                name            "Alte DIN 1451 Mittelschrift"
                tall            "48"
                weight          "200"
                antialias       "1"
                custom          "1"
            }
        }
        
        DialogButton {
            1 {
                name            "Noto Sans"
                tall            "32"
                weight          "500"
                antialias       "1"
                custom          "1"
            }
        }
        
        DialogText {
            1 {
                name            "Noto Sans"
                tall            "32"
                weight          "100"
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
