//
//  PalettePresets.h
//  SPRED - Sprite Editor
//
//  Standard 16-color palette presets
//

#ifndef SPRED_PALETTE_PRESETS_H
#define SPRED_PALETTE_PRESETS_H

#include <cstdint>
#include <string>
#include <vector>

namespace SPRED {

/// Palette preset - 64 bytes (16 colors × RGBA)
struct PalettePreset {
    std::string name;
    uint8_t colors[64];  // 16 colors × 4 bytes RGBA
    
    PalettePreset(const std::string& n, const uint8_t palette[64])
        : name(n)
    {
        for (int i = 0; i < 64; i++) {
            colors[i] = palette[i];
        }
    }
};

/// Get all available palette presets
inline std::vector<PalettePreset> getPalettePresets() {
    std::vector<PalettePreset> presets;
    
    // C64 - Classic Commodore 64 palette
    {
        const uint8_t c64[64] = {
            0, 0, 0, 0,           // 0: Transparent black
            0, 0, 0, 255,         // 1: Opaque black
            255, 255, 255, 255,   // 2: White
            136, 0, 0, 255,       // 3: Red
            170, 255, 238, 255,   // 4: Cyan
            204, 68, 204, 255,    // 5: Purple
            0, 204, 85, 255,      // 6: Green
            0, 0, 170, 255,       // 7: Blue
            238, 238, 119, 255,   // 8: Yellow
            221, 136, 85, 255,    // 9: Orange
            102, 68, 0, 255,      // 10: Brown
            255, 119, 119, 255,   // 11: Light red
            51, 51, 51, 255,      // 12: Dark grey
            119, 119, 119, 255,   // 13: Grey
            170, 255, 102, 255,   // 14: Light green
            0, 136, 255, 255      // 15: Light blue
        };
        presets.emplace_back("C64", c64);
    }
    
    // IBM CGA - IBM PC CGA 16-color palette
    {
        const uint8_t ibm[64] = {
            0, 0, 0, 0,           // 0: Transparent black
            0, 0, 0, 255,         // 1: Opaque black
            255, 255, 255, 255,   // 2: White
            170, 0, 0, 255,       // 3: Red
            0, 170, 170, 255,     // 4: Cyan
            170, 0, 170, 255,     // 5: Magenta
            0, 170, 0, 255,       // 6: Green
            0, 0, 170, 255,       // 7: Blue
            170, 170, 0, 255,     // 8: Yellow/Brown
            255, 85, 85, 255,     // 9: Light red
            85, 255, 255, 255,    // 10: Light cyan
            255, 85, 255, 255,    // 11: Light magenta
            85, 255, 85, 255,     // 12: Light green
            85, 85, 255, 255,     // 13: Light blue
            255, 255, 85, 255,    // 14: Light yellow
            85, 85, 85, 255       // 15: Dark grey
        };
        presets.emplace_back("IBM CGA", ibm);
    }
    
    // Desert - Warm desert/sand tones
    {
        const uint8_t desert[64] = {
            0, 0, 0, 0,           // 0: Transparent black
            0, 0, 0, 255,         // 1: Opaque black
            255, 248, 220, 255,   // 2: Light sand
            210, 180, 140, 255,   // 3: Tan
            194, 178, 128, 255,   // 4: Khaki
            160, 82, 45, 255,     // 5: Sienna
            139, 69, 19, 255,     // 6: Saddle brown
            205, 133, 63, 255,    // 7: Peru
            222, 184, 135, 255,   // 8: Burlywood
            244, 164, 96, 255,    // 9: Sandy brown
            210, 105, 30, 255,    // 10: Chocolate
            255, 140, 0, 255,     // 11: Dark orange
            255, 165, 0, 255,     // 12: Orange
            218, 165, 32, 255,    // 13: Goldenrod
            184, 134, 11, 255,    // 14: Dark goldenrod
            128, 128, 0, 255      // 15: Olive
        };
        presets.emplace_back("Desert", desert);
    }
    
    // Ice - Cool blue/white tones
    {
        const uint8_t ice[64] = {
            0, 0, 0, 0,           // 0: Transparent black
            0, 0, 0, 255,         // 1: Opaque black
            255, 255, 255, 255,   // 2: White
            240, 248, 255, 255,   // 3: Alice blue
            230, 230, 250, 255,   // 4: Lavender
            173, 216, 230, 255,   // 5: Light blue
            135, 206, 250, 255,   // 6: Light sky blue
            176, 224, 230, 255,   // 7: Powder blue
            175, 238, 238, 255,   // 8: Pale turquoise
            127, 255, 212, 255,   // 9: Aquamarine
            64, 224, 208, 255,    // 10: Turquoise
            0, 206, 209, 255,     // 11: Dark turquoise
            72, 209, 204, 255,    // 12: Medium turquoise
            32, 178, 170, 255,    // 13: Light sea green
            95, 158, 160, 255,    // 14: Cadet blue
            70, 130, 180, 255     // 15: Steel blue
        };
        presets.emplace_back("Ice", ice);
    }
    
    // Greys - Monochrome grayscale
    {
        const uint8_t greys[64] = {
            0, 0, 0, 0,           // 0: Transparent black
            0, 0, 0, 255,         // 1: Opaque black
            255, 255, 255, 255,   // 2: White
            238, 238, 238, 255,   // 3: Grey 93%
            221, 221, 221, 255,   // 4: Grey 87%
            204, 204, 204, 255,   // 5: Grey 80%
            187, 187, 187, 255,   // 6: Grey 73%
            170, 170, 170, 255,   // 7: Grey 67%
            153, 153, 153, 255,   // 8: Grey 60%
            136, 136, 136, 255,   // 9: Grey 53%
            119, 119, 119, 255,   // 10: Grey 47%
            102, 102, 102, 255,   // 11: Grey 40%
            85, 85, 85, 255,      // 12: Grey 33%
            68, 68, 68, 255,      // 13: Grey 27%
            51, 51, 51, 255,      // 14: Grey 20%
            34, 34, 34, 255       // 15: Grey 13%
        };
        presets.emplace_back("Greys", greys);
    }
    
    // Greens - Nature/forest palette
    {
        const uint8_t greens[64] = {
            0, 0, 0, 0,           // 0: Transparent black
            0, 0, 0, 255,         // 1: Opaque black
            240, 255, 240, 255,   // 2: Honeydew
            144, 238, 144, 255,   // 3: Light green
            152, 251, 152, 255,   // 4: Pale green
            127, 255, 0, 255,     // 5: Chartreuse
            124, 252, 0, 255,     // 6: Lawn green
            0, 255, 0, 255,       // 7: Lime
            50, 205, 50, 255,     // 8: Lime green
            34, 139, 34, 255,     // 9: Forest green
            0, 128, 0, 255,       // 10: Green
            0, 100, 0, 255,       // 11: Dark green
            154, 205, 50, 255,    // 12: Yellow green
            107, 142, 35, 255,    // 13: Olive drab
            85, 107, 47, 255,     // 14: Dark olive green
            46, 139, 87, 255      // 15: Sea green
        };
        presets.emplace_back("Greens", greens);
    }
    
    // Blues - Ocean/sky palette
    {
        const uint8_t blues[64] = {
            0, 0, 0, 0,           // 0: Transparent black
            0, 0, 0, 255,         // 1: Opaque black
            240, 248, 255, 255,   // 2: Alice blue
            135, 206, 250, 255,   // 3: Light sky blue
            135, 206, 235, 255,   // 4: Sky blue
            100, 149, 237, 255,   // 5: Cornflower blue
            65, 105, 225, 255,    // 6: Royal blue
            0, 0, 255, 255,       // 7: Blue
            0, 0, 205, 255,       // 8: Medium blue
            0, 0, 139, 255,       // 9: Dark blue
            25, 25, 112, 255,     // 10: Midnight blue
            0, 191, 255, 255,     // 11: Deep sky blue
            30, 144, 255, 255,    // 12: Dodger blue
            70, 130, 180, 255,    // 13: Steel blue
            176, 196, 222, 255,   // 14: Light steel blue
            123, 104, 238, 255    // 15: Medium slate blue
        };
        presets.emplace_back("Blues", blues);
    }
    
    // Reds - Fire/blood palette
    {
        const uint8_t reds[64] = {
            0, 0, 0, 0,           // 0: Transparent black
            0, 0, 0, 255,         // 1: Opaque black
            255, 240, 245, 255,   // 2: Lavender blush
            255, 192, 203, 255,   // 3: Pink
            255, 182, 193, 255,   // 4: Light pink
            255, 105, 180, 255,   // 5: Hot pink
            255, 20, 147, 255,    // 6: Deep pink
            255, 0, 0, 255,       // 7: Red
            220, 20, 60, 255,     // 8: Crimson
            178, 34, 34, 255,     // 9: Firebrick
            139, 0, 0, 255,       // 10: Dark red
            255, 69, 0, 255,      // 11: Orange red
            255, 99, 71, 255,     // 12: Tomato
            250, 128, 114, 255,   // 13: Salmon
            233, 150, 122, 255,   // 14: Dark salmon
            205, 92, 92, 255      // 15: Indian red
        };
        presets.emplace_back("Reds", reds);
    }
    
    // Neon - Bright vibrant colors
    {
        const uint8_t neon[64] = {
            0, 0, 0, 0,           // 0: Transparent black
            0, 0, 0, 255,         // 1: Opaque black
            255, 255, 255, 255,   // 2: White
            255, 0, 255, 255,     // 3: Magenta
            0, 255, 255, 255,     // 4: Cyan
            255, 255, 0, 255,     // 5: Yellow
            255, 0, 0, 255,       // 6: Red
            0, 255, 0, 255,       // 7: Lime
            0, 0, 255, 255,       // 8: Blue
            255, 128, 0, 255,     // 9: Orange
            128, 0, 255, 255,     // 10: Purple
            0, 255, 128, 255,     // 11: Spring green
            255, 0, 128, 255,     // 12: Rose
            128, 255, 0, 255,     // 13: Chartreuse
            0, 128, 255, 255,     // 14: Azure
            255, 128, 255, 255    // 15: Pink
        };
        presets.emplace_back("Neon", neon);
    }
    
    // Pastel - Soft muted colors
    {
        const uint8_t pastel[64] = {
            0, 0, 0, 0,           // 0: Transparent black
            0, 0, 0, 255,         // 1: Opaque black
            255, 255, 255, 255,   // 2: White
            255, 218, 185, 255,   // 3: Peach puff
            255, 228, 196, 255,   // 4: Bisque
            255, 239, 213, 255,   // 5: Papaya whip
            221, 160, 221, 255,   // 6: Plum
            216, 191, 216, 255,   // 7: Thistle
            255, 182, 193, 255,   // 8: Light pink
            255, 218, 225, 255,   // 9: Misty rose
            240, 230, 140, 255,   // 10: Khaki
            238, 232, 170, 255,   // 11: Pale goldenrod
            152, 251, 152, 255,   // 12: Pale green
            175, 238, 238, 255,   // 13: Pale turquoise
            176, 224, 230, 255,   // 14: Powder blue
            230, 230, 250, 255    // 15: Lavender
        };
        presets.emplace_back("Pastel", pastel);
    }
    
    // Earth - Natural earthy tones
    {
        const uint8_t earth[64] = {
            0, 0, 0, 0,           // 0: Transparent black
            0, 0, 0, 255,         // 1: Opaque black
            245, 245, 220, 255,   // 2: Beige
            222, 184, 135, 255,   // 3: Burlywood
            188, 143, 143, 255,   // 4: Rosy brown
            139, 69, 19, 255,     // 5: Saddle brown
            160, 82, 45, 255,     // 6: Sienna
            205, 133, 63, 255,    // 7: Peru
            210, 105, 30, 255,    // 8: Chocolate
            128, 128, 0, 255,     // 9: Olive
            85, 107, 47, 255,     // 10: Dark olive green
            107, 142, 35, 255,    // 11: Olive drab
            112, 128, 144, 255,   // 12: Slate grey
            119, 136, 153, 255,   // 13: Light slate grey
            47, 79, 79, 255,      // 14: Dark slate grey
            105, 105, 105, 255    // 15: Dim grey
        };
        presets.emplace_back("Earth", earth);
    }
    
    // Retro - Classic 8-bit game palette
    {
        const uint8_t retro[64] = {
            0, 0, 0, 0,           // 0: Transparent black
            0, 0, 0, 255,         // 1: Opaque black
            255, 255, 255, 255,   // 2: White
            190, 38, 51, 255,     // 3: Dark red
            224, 111, 139, 255,   // 4: Pink
            73, 60, 43, 255,      // 5: Dark brown
            164, 100, 34, 255,    // 6: Brown
            235, 137, 49, 255,    // 7: Orange
            247, 226, 107, 255,   // 8: Yellow
            47, 72, 78, 255,      // 9: Dark teal
            68, 137, 26, 255,     // 10: Green
            163, 206, 39, 255,    // 11: Light green
            27, 38, 50, 255,      // 12: Dark blue
            0, 87, 132, 255,      // 13: Blue
            49, 162, 242, 255,    // 14: Light blue
            178, 220, 239, 255    // 15: Very light blue
        };
        presets.emplace_back("Retro", retro);
    }
    
    return presets;
}

} // namespace SPRED

#endif // SPRED_PALETTE_PRESETS_H