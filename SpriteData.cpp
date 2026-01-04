//
//  SpriteData.cpp
//  SPRED - Sprite Editor
//
//  Created by SPRED Generator
//

#include "SpriteData.h"
#include "PNGConverter.h"
#include "SpriteCompression.h"
#include "PaletteLibrary.h"
#include <cstring>
#include <fstream>
#include <algorithm>

namespace SPRED {

SpriteData::SpriteData() : m_width(8), m_height(8) {
    clear();
}

SpriteData::SpriteData(int width, int height) : m_width(width), m_height(height) {
    if (m_width < 1) m_width = 1;
    if (m_height < 1) m_height = 1;
    if (m_width > MAX_SPRITE_SIZE) m_width = MAX_SPRITE_SIZE;
    if (m_height > MAX_SPRITE_SIZE) m_height = MAX_SPRITE_SIZE;
    clear();
}

void SpriteData::resize(int width, int height) {
    if (width < 1) width = 1;
    if (height < 1) height = 1;
    if (width > MAX_SPRITE_SIZE) width = MAX_SPRITE_SIZE;
    if (height > MAX_SPRITE_SIZE) height = MAX_SPRITE_SIZE;

    m_width = width;
    m_height = height;
    clear();
}

uint8_t SpriteData::getPixel(int x, int y) const {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return 0;
    }
    return m_pixels[y * m_width + x];
}

void SpriteData::setPixel(int x, int y, uint8_t colorIndex) {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return;
    }
    if (colorIndex >= PALETTE_SIZE) {
        colorIndex = 0;
    }
    m_pixels[y * m_width + x] = colorIndex;
}

void SpriteData::getPaletteColor(int index, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) const {
    if (index < 0 || index >= PALETTE_SIZE) {
        r = g = b = 0;
        a = 0;
        return;
    }
    int offset = index * 4;
    r = m_palette[offset + 0];
    g = m_palette[offset + 1];
    b = m_palette[offset + 2];
    a = m_palette[offset + 3];
}

void SpriteData::setPaletteColor(int index, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (index < 0 || index >= PALETTE_SIZE) {
        return;
    }
    int offset = index * 4;
    m_palette[offset + 0] = r;
    m_palette[offset + 1] = g;
    m_palette[offset + 2] = b;
    m_palette[offset + 3] = a;
}

void SpriteData::clear() {
    // Clear all pixels to transparent (index 0)
    std::memset(m_pixels, 0, MAX_SPRITE_PIXELS);

    // Initialize default palette
    initializeDefaultPalette();
}

void SpriteData::initializeDefaultPalette() {
    // Color 0: Transparent black
    setPaletteColor(0, 0, 0, 0, 0);

    // Color 1: Opaque black
    setPaletteColor(1, 0, 0, 0, 255);

    // Colors 2-15: Default grayscale ramp
    for (int i = 2; i < PALETTE_SIZE; i++) {
        uint8_t gray = static_cast<uint8_t>((i - 2) * 255 / 13);
        setPaletteColor(i, gray, gray, gray, 255);
    }
}

void SpriteData::getRGBAPixels(uint8_t* outRGBA) const {
    int numPixels = m_width * m_height;
    for (int i = 0; i < numPixels; i++) {
        uint8_t index = m_pixels[i];
        if (index >= PALETTE_SIZE) {
            index = 0;
        }
        int paletteOffset = index * 4;
        int rgbaOffset = i * 4;
        outRGBA[rgbaOffset + 0] = m_palette[paletteOffset + 0];
        outRGBA[rgbaOffset + 1] = m_palette[paletteOffset + 1];
        outRGBA[rgbaOffset + 2] = m_palette[paletteOffset + 2];
        outRGBA[rgbaOffset + 3] = m_palette[paletteOffset + 3];
    }
}

bool SpriteData::saveSprite(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        return false;
    }

    // Write header: "SPRED" + version (1 byte) + width + height
    file.write("SPRED", 5);
    uint8_t version = 1;
    file.write(reinterpret_cast<const char*>(&version), 1);
    file.write(reinterpret_cast<const char*>(&m_width), sizeof(int));
    file.write(reinterpret_cast<const char*>(&m_height), sizeof(int));

    // Write pixel data
    int numPixels = m_width * m_height;
    file.write(reinterpret_cast<const char*>(m_pixels), numPixels);

    // Write palette
    file.write(reinterpret_cast<const char*>(m_palette), PALETTE_BYTES);

    return file.good();
}

bool SpriteData::loadSprite(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        return false;
    }

    // Read header
    char magic[6] = {0};
    file.read(magic, 5);
    if (std::string(magic) != "SPRED") {
        return false;
    }

    uint8_t version;
    file.read(reinterpret_cast<char*>(&version), 1);
    if (version != 1) {
        return false;
    }

    int width, height;
    file.read(reinterpret_cast<char*>(&width), sizeof(int));
    file.read(reinterpret_cast<char*>(&height), sizeof(int));

    // Update sprite size
    m_width = width;
    m_height = height;

    // Read pixels
    int numPixels = m_width * m_height;
    file.read(reinterpret_cast<char*>(m_pixels), numPixels);

    // Read palette
    file.read(reinterpret_cast<char*>(m_palette), PALETTE_BYTES);

    return file.good();
}

bool SpriteData::savePalette(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        return false;
    }

    // Write header: "STPAL" + version
    file.write("STPAL", 5);
    uint8_t version = 1;
    file.write(reinterpret_cast<const char*>(&version), 1);

    // Write palette
    file.write(reinterpret_cast<const char*>(m_palette), PALETTE_BYTES);

    return file.good();
}

bool SpriteData::loadPalette(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        return false;
    }

    // Read header
    char magic[6] = {0};
    file.read(magic, 5);
    if (std::string(magic) != "STPAL") {
        return false;
    }

    uint8_t version;
    file.read(reinterpret_cast<char*>(&version), 1);
    if (version != 1) {
        return false;
    }

    // Read palette
    file.read(reinterpret_cast<char*>(m_palette), PALETTE_BYTES);

    return file.good();
}

bool SpriteData::saveSPRTZ(const std::string& filename) const {
    return SpriteCompression::saveSPRTZ(filename, m_width, m_height, m_pixels, m_palette);
}

bool SpriteData::loadSPRTZ(const std::string& filename) {
    uint8_t tempPixels[MAX_SPRITE_PIXELS];
    uint8_t tempPalette[PALETTE_BYTES];
    int width, height;

    if (!SpriteCompression::loadSPRTZ(filename, width, height, tempPixels, tempPalette)) {
        return false;
    }

    // Update sprite data
    m_width = width;
    m_height = height;
    std::memcpy(m_pixels, tempPixels, MAX_SPRITE_PIXELS);
    std::memcpy(m_palette, tempPalette, PALETTE_BYTES);

    return true;
}

bool SpriteData::saveSPRTZv2Standard(const std::string& filename, uint8_t standardPaletteID) const {
    return SpriteCompression::saveSPRTZv2Standard(filename, m_width, m_height, m_pixels, standardPaletteID);
}

bool SpriteData::saveSPRTZv2Custom(const std::string& filename) const {
    return SpriteCompression::saveSPRTZv2Custom(filename, m_width, m_height, m_pixels, m_palette);
}

bool SpriteData::loadSPRTZv2(const std::string& filename, bool& outIsStandard, uint8_t& outPaletteID) {
    uint8_t tempPixels[MAX_SPRITE_PIXELS];
    uint8_t tempPalette[PALETTE_BYTES];
    int width, height;

    if (!SpriteCompression::loadSPRTZv2(filename, width, height, tempPixels, tempPalette, outIsStandard, outPaletteID)) {
        return false;
    }

    // Update sprite data
    m_width = width;
    m_height = height;
    std::memcpy(m_pixels, tempPixels, MAX_SPRITE_PIXELS);
    std::memcpy(m_palette, tempPalette, PALETTE_BYTES);

    return true;
}

bool SpriteData::importPNG(const std::string& filename, int maxWidth, int maxHeight) {
    int width, height;
    uint8_t pixels[MAX_SPRITE_PIXELS];
    uint8_t palette[PALETTE_BYTES];

    if (!PNGConverter::importPNG(filename, maxWidth, maxHeight, width, height, pixels, palette)) {
        return false;
    }

    // Update sprite data
    m_width = width;
    m_height = height;
    std::memcpy(m_pixels, pixels, MAX_SPRITE_PIXELS);
    std::memcpy(m_palette, palette, PALETTE_BYTES);

    return true;
}

bool SpriteData::exportPNG(const std::string& filename, int scale) const {
    return PNGConverter::exportPNG(filename, m_width, m_height, m_pixels, m_palette, scale);
}

// =============================================================================
// PNG Import Pipeline - Discrete Steps
// =============================================================================

bool SpriteData::startPNGImport(const std::string& filename, int targetWidth, int targetHeight) {
    printf("\n========================================\n");
    printf("PNG IMPORT PIPELINE START\n");
    printf("========================================\n");

    // STEP 0: Load PNG at full resolution
    printf("[Step 0] Loading source PNG...\n");
    std::vector<uint8_t> rgba;
    int pngWidth, pngHeight;

    if (!PNGConverter::loadPNGFile(filename, rgba, pngWidth, pngHeight)) {
        printf("[Step 0] ERROR: Failed to load PNG file\n");
        return false;
    }
    printf("[Step 0] ✓ Loaded source PNG: %dx%d (%zu bytes)\n",
           pngWidth, pngHeight, rgba.size());

    // Store the PNG data
    m_importedPNGData = rgba;
    m_importedPNGWidth = pngWidth;
    m_importedPNGHeight = pngHeight;
    m_pngOffsetX = 0;
    m_pngOffsetY = 0;
    m_hasPendingImport = true;

    // Calculate sprite size that maintains PNG aspect ratio
    float pngAspect = static_cast<float>(pngWidth) / pngHeight;

    int actualWidth, actualHeight;

    if (pngWidth > pngHeight) {
        actualWidth = targetWidth;
        actualHeight = static_cast<int>(targetWidth / pngAspect);
        if (actualHeight > targetHeight) {
            actualHeight = targetHeight;
            actualWidth = static_cast<int>(targetHeight * pngAspect);
        }
    } else {
        actualHeight = targetHeight;
        actualWidth = static_cast<int>(targetHeight * pngAspect);
        if (actualWidth > targetWidth) {
            actualWidth = targetWidth;
            actualHeight = static_cast<int>(targetWidth / pngAspect);
        }
    }

    // Ensure dimensions are at least 1
    if (actualWidth < 1) actualWidth = 1;
    if (actualHeight < 1) actualHeight = 1;

    m_pngTargetWidth = actualWidth;
    m_pngTargetHeight = actualHeight;

    printf("[Step 0] Calculated target sprite size: %dx%d (preserves aspect ratio)\n",
           actualWidth, actualHeight);

    // Resize sprite to calculated size
    resize(actualWidth, actualHeight);

    printf("[Step 0] Sprite dimensions set: m_width=%d, m_height=%d\n", m_width, m_height);

    // Do initial resample at offset (0, 0)
    return resamplePNGAtOffset();
}

void SpriteData::shiftPNGImportOffset(int dx, int dy) {
    if (!m_hasPendingImport) {
        return;
    }

    // Calculate scale factor between PNG and target sprite
    float scaleX = static_cast<float>(m_importedPNGWidth) / m_pngTargetWidth;
    float scaleY = static_cast<float>(m_importedPNGHeight) / m_pngTargetHeight;

    // Scale the shift amount
    int pngDx = static_cast<int>(dx * scaleX);
    int pngDy = static_cast<int>(dy * scaleY);

    printf("[PNG Shift] (%d,%d) sprite space = (%d,%d) PNG space (scale %.2fx%.2f)\n",
           dx, dy, pngDx, pngDy, scaleX, scaleY);

    // Update offset
    m_pngOffsetX += pngDx;
    m_pngOffsetY += pngDy;

    // Clamp offset to valid range
    int maxOffsetX = m_importedPNGWidth - m_pngTargetWidth;
    int maxOffsetY = m_importedPNGHeight - m_pngTargetHeight;

    if (maxOffsetX < 0) maxOffsetX = 0;
    if (maxOffsetY < 0) maxOffsetY = 0;

    if (m_pngOffsetX < 0) m_pngOffsetX = 0;
    if (m_pngOffsetY < 0) m_pngOffsetY = 0;
    if (m_pngOffsetX > maxOffsetX) m_pngOffsetX = maxOffsetX;
    if (m_pngOffsetY > maxOffsetY) m_pngOffsetY = maxOffsetY;

    // Re-resample from new offset
    resamplePNGAtOffset();
}

void SpriteData::trimPNGImport(int left, int right, int top, int bottom) {
    if (!m_hasPendingImport) {
        return;
    }

    // Calculate new dimensions after trimming
    int newWidth = m_importedPNGWidth - left - right;
    int newHeight = m_importedPNGHeight - top - bottom;

    if (newWidth < 1 || newHeight < 1) {
        printf("[Trim] ERROR: Trimming would result in invalid dimensions\n");
        return;
    }

    // Create trimmed image
    std::vector<uint8_t> trimmedRGBA(newWidth * newHeight * 4);

    for (int y = 0; y < newHeight; y++) {
        for (int x = 0; x < newWidth; x++) {
            int srcOffset = ((top + y) * m_importedPNGWidth + (left + x)) * 4;
            int dstOffset = (y * newWidth + x) * 4;
            trimmedRGBA[dstOffset + 0] = m_importedPNGData[srcOffset + 0];
            trimmedRGBA[dstOffset + 1] = m_importedPNGData[srcOffset + 1];
            trimmedRGBA[dstOffset + 2] = m_importedPNGData[srcOffset + 2];
            trimmedRGBA[dstOffset + 3] = m_importedPNGData[srcOffset + 3];
        }
    }

    // Update imported PNG data
    m_importedPNGData = trimmedRGBA;
    m_importedPNGWidth = newWidth;
    m_importedPNGHeight = newHeight;

    printf("[Trim] Trimmed PNG to %dx%d (removed L:%d R:%d T:%d B:%d)\n",
           newWidth, newHeight, left, right, top, bottom);

    // Re-resample
    resamplePNGAtOffset();
}

void SpriteData::commitPNGImport() {
    // Clear the PNG data from memory
    m_importedPNGData.clear();
    m_importedPNGWidth = 0;
    m_importedPNGHeight = 0;
    m_pngOffsetX = 0;
    m_pngOffsetY = 0;
    m_pngTargetWidth = 0;
    m_pngTargetHeight = 0;
    m_hasPendingImport = false;

    printf("[Commit] PNG import committed, memory cleared\n");
}

void SpriteData::cancelPNGImport() {
    // Clear the PNG data and revert sprite
    m_importedPNGData.clear();
    m_importedPNGWidth = 0;
    m_importedPNGHeight = 0;
    m_pngOffsetX = 0;
    m_pngOffsetY = 0;
    m_pngTargetWidth = 0;
    m_pngTargetHeight = 0;
    m_hasPendingImport = false;

    // Clear sprite
    clear();

    printf("[Cancel] PNG import cancelled\n");
}

void SpriteData::getPNGImportInfo(int& width, int& height, int& offsetX, int& offsetY) const {
    width = m_importedPNGWidth;
    height = m_importedPNGHeight;
    offsetX = m_pngOffsetX;
    offsetY = m_pngOffsetY;
}

bool SpriteData::resamplePNGAtOffset() {
    if (!m_hasPendingImport || m_importedPNGData.empty()) {
        return false;
    }

    printf("\n========================================\n");
    printf("PNG IMPORT PIPELINE - NEW ORDER\n");
    printf("========================================\n");
    printf("Source PNG: %dx%d\n", m_importedPNGWidth, m_importedPNGHeight);
    printf("Target Sprite: %dx%d\n", m_pngTargetWidth, m_pngTargetHeight);

    // =============================================================================
    // STEP (a): LOAD PNG (already done in startPNGImport)
    // =============================================================================
    printf("\n[Step A] LOAD PNG\n");
    printf("[Step A] ✓ Already loaded: %dx%d\n", m_importedPNGWidth, m_importedPNGHeight);

    // =============================================================================
    // STEP (b): QUANTIZE original PNG to 16 colors AND convert background to transparent
    // =============================================================================
    printf("\n[Step B] QUANTIZE and convert background to transparent\n");

    std::vector<uint8_t> quantizedSource = m_importedPNGData;

    // First quantize
    for (size_t i = 0; i < quantizedSource.size(); i += 4) {
        quantizedSource[i+0] = (quantizedSource[i+0] >> 4) << 4;  // R
        quantizedSource[i+1] = (quantizedSource[i+1] >> 4) << 4;  // G
        quantizedSource[i+2] = (quantizedSource[i+2] >> 4) << 4;  // B
        // Alpha unchanged
    }

    // Use top-left pixel as background color to convert to transparent
    uint8_t bgR = quantizedSource[0];
    uint8_t bgG = quantizedSource[1];
    uint8_t bgB = quantizedSource[2];

    printf("[Step B] Background color to make transparent: RGB=(%d,%d,%d)\n", bgR, bgG, bgB);

    // Convert all background pixels to transparent
    int transparentCount = 0;
    for (size_t i = 0; i < quantizedSource.size(); i += 4) {
        if (quantizedSource[i+0] == bgR &&
            quantizedSource[i+1] == bgG &&
            quantizedSource[i+2] == bgB) {
            quantizedSource[i+0] = 0;
            quantizedSource[i+1] = 0;
            quantizedSource[i+2] = 0;
            quantizedSource[i+3] = 0;  // Transparent
            transparentCount++;
        }
    }

    printf("[Step B] ✓ Quantized %zu pixels, made %d pixels transparent\n",
           quantizedSource.size() / 4, transparentCount);

    // =============================================================================
    // STEP (c): CROP away transparent border pixels
    // =============================================================================
    printf("\n[Step C] CROP transparent borders\n");

    int cropLeft = 0, cropRight = m_importedPNGWidth - 1;
    int cropTop = 0, cropBottom = m_importedPNGHeight - 1;

    // Find left border (look for non-transparent pixels)
    for (int x = 0; x < m_importedPNGWidth; x++) {
        bool hasContent = false;
        for (int y = 0; y < m_importedPNGHeight; y++) {
            int offset = (y * m_importedPNGWidth + x) * 4;
            if (quantizedSource[offset+3] != 0) {  // Not transparent
                hasContent = true;
                break;
            }
        }
        if (hasContent) {
            cropLeft = x;
            break;
        }
    }

    // Find right border
    for (int x = m_importedPNGWidth - 1; x >= cropLeft; x--) {
        bool hasContent = false;
        for (int y = 0; y < m_importedPNGHeight; y++) {
            int offset = (y * m_importedPNGWidth + x) * 4;
            if (quantizedSource[offset+3] != 0) {  // Not transparent
                hasContent = true;
                break;
            }
        }
        if (hasContent) {
            cropRight = x;
            break;
        }
    }

    // Find top border
    for (int y = 0; y < m_importedPNGHeight; y++) {
        bool hasContent = false;
        for (int x = 0; x < m_importedPNGWidth; x++) {
            int offset = (y * m_importedPNGWidth + x) * 4;
            if (quantizedSource[offset+3] != 0) {  // Not transparent
                hasContent = true;
                break;
            }
        }
        if (hasContent) {
            cropTop = y;
            break;
        }
    }

    // Find bottom border
    for (int y = m_importedPNGHeight - 1; y >= cropTop; y--) {
        bool hasContent = false;
        for (int x = 0; x < m_importedPNGWidth; x++) {
            int offset = (y * m_importedPNGWidth + x) * 4;
            if (quantizedSource[offset+3] != 0) {  // Not transparent
                hasContent = true;
                break;
            }
        }
        if (hasContent) {
            cropBottom = y;
            break;
        }
    }

    int croppedWidth = cropRight - cropLeft + 1;
    int croppedHeight = cropBottom - cropTop + 1;

    printf("[Step C] Crop bounds: Left=%d, Right=%d, Top=%d, Bottom=%d\n",
           cropLeft, cropRight, cropTop, cropBottom);
    printf("[Step C] Cropped size: %dx%d (removed %d cols, %d rows)\n",
           croppedWidth, croppedHeight,
           m_importedPNGWidth - croppedWidth, m_importedPNGHeight - croppedHeight);

    // Create cropped image
    std::vector<uint8_t> croppedRGBA(croppedWidth * croppedHeight * 4);
    for (int y = 0; y < croppedHeight; y++) {
        for (int x = 0; x < croppedWidth; x++) {
            int srcOffset = ((cropTop + y) * m_importedPNGWidth + (cropLeft + x)) * 4;
            int dstOffset = (y * croppedWidth + x) * 4;
            croppedRGBA[dstOffset + 0] = quantizedSource[srcOffset + 0];
            croppedRGBA[dstOffset + 1] = quantizedSource[srcOffset + 1];
            croppedRGBA[dstOffset + 2] = quantizedSource[srcOffset + 2];
            croppedRGBA[dstOffset + 3] = quantizedSource[srcOffset + 3];
        }
    }

    // =============================================================================
    // STEP (d): RESIZE cropped image to target dimensions (keeps transparency)
    // =============================================================================
    printf("\n[Step D] RESIZE %dx%d -> %dx%d\n",
           croppedWidth, croppedHeight, m_pngTargetWidth, m_pngTargetHeight);

    std::vector<uint8_t> resizedRGBA;

    if (!PNGConverter::resizePNG(croppedRGBA.data(),
                                  croppedWidth, croppedHeight,
                                  0, 0,
                                  m_pngTargetWidth, m_pngTargetHeight,
                                  resizedRGBA,
                                  PNGScalingMethod::vImage)) {
        printf("[Step D] ✗ ERROR: Resize failed!\n");
        return false;
    }

    printf("[Step D] ✓ Resized to %dx%d\n", m_pngTargetWidth, m_pngTargetHeight);

    // =============================================================================
    // STEP (e): QUANTIZE resized image again
    // =============================================================================
    printf("\n[Step E] QUANTIZE resized image (4 bits/channel)\n");

    std::vector<uint8_t> quantizedRGBA = resizedRGBA;
    for (size_t i = 0; i < quantizedRGBA.size(); i += 4) {
        quantizedRGBA[i+0] = (quantizedRGBA[i+0] >> 4) << 4;  // R
        quantizedRGBA[i+1] = (quantizedRGBA[i+1] >> 4) << 4;  // G
        quantizedRGBA[i+2] = (quantizedRGBA[i+2] >> 4) << 4;  // B
        // Alpha unchanged
    }

    printf("[Step E] ✓ Quantized %zu pixels\n", quantizedRGBA.size() / 4);

    // =============================================================================
    // STEP (f): MATCH PALETTE (extract 14 colors)
    // =============================================================================
    printf("\n[Step F] PALETTE EXTRACTION (14 colors)\n");

    std::vector<Color> extractedColors;
    PNGConverter::extractPalette(quantizedRGBA.data(),
                                  m_pngTargetWidth * m_pngTargetHeight,
                                  14, extractedColors);

    printf("[Step F] ✓ Extracted %zu colors:\n", extractedColors.size());
    for (size_t i = 0; i < std::min(size_t(5), extractedColors.size()); i++) {
        printf("  Color[%zu]: RGB=(%d,%d,%d)\n", i,
               extractedColors[i].r, extractedColors[i].g, extractedColors[i].b);
    }
    if (extractedColors.size() > 5) {
        printf("  ... (%zu more colors)\n", extractedColors.size() - 5);
    }

    // Build final 16-color palette
    setPaletteColor(0, 0, 0, 0, 0);         // Index 0: Transparent
    setPaletteColor(1, 0, 0, 0, 255);       // Index 1: Opaque black

    for (int i = 0; i < 14; i++) {
        if (i < extractedColors.size()) {
            setPaletteColor(i + 2, extractedColors[i].r, extractedColors[i].g,
                          extractedColors[i].b, 255);
        } else {
            setPaletteColor(i + 2, 128, 128, 128, 255); // Grey filler
        }
    }

    printf("[Step F] ✓ Built 16-color palette\n");

    // =============================================================================
    // STEP (g): CONVERT TO SPRITE FORMAT (indexed pixels)
    // =============================================================================
    printf("\n[Step G] CONVERT to sprite format (indexed pixels)\n");

    // CRITICAL: Verify dimensions match before mapping
    printf("[Step G] DIMENSION CHECK:\n");
    printf("  m_width = %d, m_height = %d (sprite dimensions)\n", m_width, m_height);
    printf("  m_pngTargetWidth = %d, m_pngTargetHeight = %d (import target)\n",
           m_pngTargetWidth, m_pngTargetHeight);

    if (m_width != m_pngTargetWidth || m_height != m_pngTargetHeight) {
        printf("[Step G] ✗✗✗ CRITICAL ERROR: DIMENSION MISMATCH!\n");
        printf("[Step G] This will cause clipping! Synchronizing dimensions...\n");
        m_width = m_pngTargetWidth;
        m_height = m_pngTargetHeight;
        printf("[Step G] ✓ Synchronized: m_width=%d, m_height=%d\n", m_width, m_height);
    } else {
        printf("[Step G] ✓ Dimensions match correctly\n");
    }

    // Map pixels to palette using 2D coordinates (prevents stride mismatch)
    int pixelsMapped = 0;
    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            int linearIdx = y * m_width + x;
            int offset = linearIdx * 4;

            Color pixel(quantizedRGBA[offset + 0], quantizedRGBA[offset + 1],
                       quantizedRGBA[offset + 2], quantizedRGBA[offset + 3]);

            int paletteIdx = PNGConverter::findClosestColor(pixel, extractedColors);

            // Use setPixel to ensure proper 2D mapping
            setPixel(x, y, static_cast<uint8_t>(paletteIdx));
            pixelsMapped++;
        }
    }

    printf("[Step G] ✓ Mapped %d pixels to palette indices\n", pixelsMapped);

    // Verify rightmost column was mapped correctly
    printf("[Step G] Verification - rightmost column (x=%d):\n", m_width-1);
    for (int y = 0; y < std::min(3, m_height); y++) {
        uint8_t idx = getPixel(m_width-1, y);
        uint8_t r, g, b, a;
        getPaletteColor(idx, r, g, b, a);
        printf("  [%d,%d]: index=%d, RGB=(%d,%d,%d)\n",
               m_width-1, y, idx, r, g, b);
   }

    printf("\n========================================\n");
    printf("PNG IMPORT PIPELINE COMPLETE\n");
    printf("========================================\n\n");

    return true;
}

uint8_t SpriteData::findClosestStandardPalette(int* outDistance) const {
    printf("\n========================================\n");
    printf("FIND CLOSEST STANDARD PALETTE\n");
    printf("========================================\n");

    // Convert current palette to PaletteColor format
    SuperTerminal::PaletteColor customPalette[16];
    for (int i = 0; i < PALETTE_SIZE; i++) {
        int offset = i * 4;
        customPalette[i] = SuperTerminal::PaletteColor(
            m_palette[offset + 0],
            m_palette[offset + 1],
            m_palette[offset + 2],
            m_palette[offset + 3]
        );
    }

    // Print current palette
    printf("Current custom palette:\n");
    for (int i = 0; i < 16; i++) {
        printf("  [%2d] RGBA=(%3d,%3d,%3d,%3d)\n", i,
               customPalette[i].r, customPalette[i].g,
               customPalette[i].b, customPalette[i].a);
    }

    // Find closest standard palette
    int32_t distance = 0;
    uint8_t bestPaletteID = SuperTerminal::StandardPaletteLibrary::findClosestPalette(
        customPalette, &distance);

    if (bestPaletteID == 0xFF) {
        printf("No good matching standard palette found\n");
        if (outDistance) *outDistance = -1;
        return 0xFF;
    }

    const char* paletteName = SuperTerminal::StandardPaletteLibrary::getPaletteName(bestPaletteID);
    const char* paletteDesc = SuperTerminal::StandardPaletteLibrary::getPaletteDescription(bestPaletteID);
    const char* paletteCategory = SuperTerminal::StandardPaletteLibrary::getPaletteCategory(bestPaletteID);

    printf("\n✓ Found closest match:\n");
    printf("  ID: %d\n", bestPaletteID);
    printf("  Name: %s\n", paletteName ? paletteName : "Unknown");
    printf("  Category: %s\n", paletteCategory ? paletteCategory : "Unknown");
    printf("  Description: %s\n", paletteDesc ? paletteDesc : "");
    printf("  Color distance: %d\n", distance);

    // Show the standard palette for comparison
    const SuperTerminal::PaletteColor* standardPalette =
        SuperTerminal::StandardPaletteLibrary::getPalette(bestPaletteID);

    if (standardPalette) {
        printf("\nStandard palette colors:\n");
        for (int i = 0; i < 16; i++) {
            printf("  [%2d] RGBA=(%3d,%3d,%3d,%3d)\n", i,
                   standardPalette[i].r, standardPalette[i].g,
                   standardPalette[i].b, standardPalette[i].a);
        }
    }
    printf("========================================\n\n");

    if (outDistance) *outDistance = distance;
    return bestPaletteID;
}

} // namespace SPRED
