//
//  SpriteData.h
//  SPRED - Sprite Editor
//
//  Created by SPRED Generator
//

#ifndef SPRED_SPRITE_DATA_H
#define SPRED_SPRITE_DATA_H

#include <cstdint>
#include <string>
#include <vector>

namespace SPRED {

// Constants
constexpr int MAX_SPRITE_SIZE = 40;
constexpr int MAX_SPRITE_PIXELS = MAX_SPRITE_SIZE * MAX_SPRITE_SIZE;
constexpr int PALETTE_SIZE = 16;
constexpr int PALETTE_BYTES = PALETTE_SIZE * 4; // RGBA

/// SpriteData - Manages variable-sized indexed sprite data (8x8, 16x16, 40x40)
class SpriteData {
public:
    SpriteData();
    SpriteData(int width, int height);
    ~SpriteData() = default;

    // Size management
    void resize(int width, int height);
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

    // Data access
    uint8_t getPixel(int x, int y) const;
    void setPixel(int x, int y, uint8_t colorIndex);
    
    // Palette access
    void getPaletteColor(int index, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) const;
    void setPaletteColor(int index, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    
    // Get raw data
    const uint8_t* getPixelData() const { return m_pixels; }
    const uint8_t* getPaletteData() const { return m_palette; }
    
    // File operations
    bool saveSprite(const std::string& filename) const;
    bool loadSprite(const std::string& filename);
    bool savePalette(const std::string& filename) const;
    bool loadPalette(const std::string& filename);
    
    // SPRTZ compressed format (v1 - custom palette)
    bool saveSPRTZ(const std::string& filename) const;
    bool loadSPRTZ(const std::string& filename);
    
    // SPRTZ v2 format (standard palette support)
    bool saveSPRTZv2Standard(const std::string& filename, uint8_t standardPaletteID) const;
    bool saveSPRTZv2Custom(const std::string& filename) const;
    bool loadSPRTZv2(const std::string& filename, bool& outIsStandard, uint8_t& outPaletteID);
    
    // PNG import/export
    bool importPNG(const std::string& filename, int maxWidth, int maxHeight);
    bool exportPNG(const std::string& filename, int scale = 1) const;
    
    // PNG import mode (interactive positioning)
    bool startPNGImport(const std::string& filename, int targetWidth, int targetHeight);
    void shiftPNGImportOffset(int dx, int dy);
    void trimPNGImport(int left, int right, int top, int bottom);  // Trim pixels from PNG edges
    void commitPNGImport();
    void cancelPNGImport();
    bool hasPendingPNGImport() const { return m_hasPendingImport; }
    void getPNGImportInfo(int& width, int& height, int& offsetX, int& offsetY) const;
    
    // Clear sprite
    void clear();
    
    // Get RGBA representation for display
    void getRGBAPixels(uint8_t* outRGBA) const;
    
    // Palette operations
    uint8_t findClosestStandardPalette(int* outDistance = nullptr) const;

private:
    int m_width;
    int m_height;
    uint8_t m_pixels[MAX_SPRITE_PIXELS];    // Up to 1600 bytes (40x40 indexed pixels)
    uint8_t m_palette[PALETTE_BYTES];       // 64 bytes (16 colors × RGBA)
    
    // PNG import state
    std::vector<uint8_t> m_importedPNGData; // RGBA data of imported PNG
    int m_importedPNGWidth = 0;
    int m_importedPNGHeight = 0;
    int m_pngOffsetX = 0;                   // Current pan offset
    int m_pngOffsetY = 0;
    int m_pngTargetWidth = 0;               // Target sprite size for downsampling
    int m_pngTargetHeight = 0;
    bool m_hasPendingImport = false;
    
    void initializeDefaultPalette();
    bool resamplePNGAtOffset();             // Helper: downsample PNG from current offset
};

} // namespace SPRED

#endif // SPRED_SPRITE_DATA_H