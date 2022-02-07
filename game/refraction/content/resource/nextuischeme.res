Scheme {
    Colors {
        NullColor   "0   0   0   0"
        BgHighlight "0   0   0   196"
        FgHighlight "255 127 0   255"
        FgNormal    "255 255 255 255"
        BgGradient  "0   0   0   255"
    }

    BaseSettings {
        GameMenu.Background.FadeColor               "BgGradient"
        GameMenu.Background.FadeWidthPercent        "25"
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
        GameMenu.Button.BackgroundColor.Normal      "NullColor"
        GameMenu.Button.BackgroundColor.Highlight   "BgHighlight"
        GameMenu.Button.HighlightColor.Normal       "NullColor"
        GameMenu.Button.HighlightColor.Highlight    "FgHighlight"
        GameMenu.Button.TextColor.Normal            "FgNormal"
        GameMenu.Button.TextColor.Highlight         "FgHighlight"
        GameMenu.ButtonStack.PositionX              "64"
        GameMenu.ButtonStack.PositionY              "160"
        GameMenu.Title.Color                        "FgNormal"
        GameMenu.Title.PositionX                    "64"
        GameMenu.Title.PositionY                    "64"
    }

    Fonts {
        Default {
            1 {
                name        "Tahoma"
                tall        "16"
                weight      "500"
            }
        }
    
        GameMenuButton {
            1 {
                name        "Alte DIN 1451 Mittelschrift"
                tall        "24"
                weight      "500"
                antialias   "1"
            }
        }
        
        GameMenuTitle {
            1 {
                name        "K12HL2"
                tall        "64"
                weight      "400"
                antialias   "1"
                custom      "1"
            }
        }
    }
    
    CustomFontFiles {
        1 "resource/HALFLIFE2.ttf"
        2 "resource/fonts/din1451alt.ttf"
        3 "resource/fonts/K12HL2.ttf"
    }
}
