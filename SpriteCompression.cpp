//
//  SpriteCompression.cpp
//  SPRED - Sprite Editor
//
//  SPRTZ compressed sprite format implementation
//

#include "SpriteCompression.h"
#include "PaletteLibrary.h"
#include <fstream>
#include <cstring>
#include <cstdio>
#include <zlib.h>

using namespace SuperTerminal;

namespace SPRED {

bool SpriteCompression::saveSPRTZ(const std::string& filename,
                                   int width, int height,
                                   const uint8_t* pixels,
                                   const uint8_t* palette) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // Compress pixel data
    int pixelCount = width * height;
    std::vector<uint8_t> compressed;
    compressRLE(pixels, pixelCount, compressed);

    // Write header
    const char magic[4] = {'S', 'P', 'T', 'Z'};
    file.write(magic, 4);

    uint16_t version = 1;
    file.write(reinterpret_cast<const char*>(&version), sizeof(version));

    uint8_t w = static_cast<uint8_t>(width);
    uint8_t h = static_cast<uint8_t>(height);
    file.write(reinterpret_cast<const char*>(&w), 1);
    file.write(reinterpret_cast<const char*>(&h), 1);

    uint32_t uncompressedSize = static_cast<uint32_t>(pixelCount);
    uint32_t compressedSize = static_cast<uint32_t>(compressed.size());
    file.write(reinterpret_cast<const char*>(&uncompressedSize), sizeof(uncompressedSize));
    file.write(reinterpret_cast<const char*>(&compressedSize), sizeof(compressedSize));

    // Write palette (indices 2-15 only, RGB only)
    // Skip indices 0 and 1 (transparent and opaque black)
    for (int i = 2; i < 16; i++) {
        int offset = i * 4;
        uint8_t rgb[3] = {palette[offset], palette[offset + 1], palette[offset + 2]};
        file.write(reinterpret_cast<const char*>(rgb), 3);
    }

    // Write compressed pixel data
    file.write(reinterpret_cast<const char*>(compressed.data()), compressed.size());

    return file.good();
}

bool SpriteCompression::loadSPRTZ(const std::string& filename,
                                   int& outWidth, int& outHeight,
                                   uint8_t* outPixels,
                                   uint8_t* outPalette) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // Read and verify header
    char magic[4];
    file.read(magic, 4);
    if (magic[0] != 'S' || magic[1] != 'P' || magic[2] != 'T' || magic[3] != 'Z') {
        return false;
    }

    uint16_t version;
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    if (version != 1) {
        return false;
    }

    uint8_t w, h;
    file.read(reinterpret_cast<char*>(&w), 1);
    file.read(reinterpret_cast<char*>(&h), 1);
    outWidth = w;
    outHeight = h;

    uint32_t uncompressedSize, compressedSize;
    file.read(reinterpret_cast<char*>(&uncompressedSize), sizeof(uncompressedSize));
    file.read(reinterpret_cast<char*>(&compressedSize), sizeof(compressedSize));

    // Verify sizes
    int expectedPixels = outWidth * outHeight;
    if (uncompressedSize != static_cast<uint32_t>(expectedPixels)) {
        return false;
    }

    // Read palette (indices 2-15, RGB only)
    // Set up fixed colors 0 and 1
    outPalette[0] = 0;   // R
    outPalette[1] = 0;   // G
    outPalette[2] = 0;   // B
    outPalette[3] = 0;   // A (transparent)

    outPalette[4] = 0;   // R
    outPalette[5] = 0;   // G
    outPalette[6] = 0;   // B
    outPalette[7] = 255; // A (opaque)

    // Read colors 2-15
    for (int i = 2; i < 16; i++) {
        uint8_t rgb[3];
        file.read(reinterpret_cast<char*>(rgb), 3);
        int offset = i * 4;
        outPalette[offset + 0] = rgb[0];
        outPalette[offset + 1] = rgb[1];
        outPalette[offset + 2] = rgb[2];
        outPalette[offset + 3] = 255; // Always opaque for colors 2-15
    }

    // Read compressed pixel data
    std::vector<uint8_t> compressed(compressedSize);
    file.read(reinterpret_cast<char*>(compressed.data()), compressedSize);

    if (!file.good()) {
        return false;
    }

    // Decompress
    return decompressRLE(compressed.data(), compressedSize, outPixels, expectedPixels);
}

void SpriteCompression::compressRLE(const uint8_t* pixels, int pixelCount,
                                     std::vector<uint8_t>& compressed) {
    // Use zlib compression instead of custom RLE
    compressed.clear();
    
    // Estimate compressed size (zlib's compressBound gives upper limit)
    uLongf compressedSize = compressBound(pixelCount);
    compressed.resize(compressedSize);
    
    // Compress using zlib
    int result = compress2(compressed.data(), &compressedSize,
                          pixels, pixelCount,
                          Z_BEST_COMPRESSION);
    
    if (result != Z_OK) {
        printf("[SpriteCompression::compressRLE] ERROR: zlib compression failed with code %d\n", result);
        compressed.clear();
        return;
    }
    
    // Resize to actual compressed size
    compressed.resize(compressedSize);
    printf("[SpriteCompression::compressRLE] Compressed %d bytes to %zu bytes using zlib\n", 
           pixelCount, compressed.size());
}

bool SpriteCompression::decompressRLE(const uint8_t* compressed, size_t compressedSize,
                                       uint8_t* pixels, int pixelCount) {
    printf("[SpriteCompression::decompressRLE] Starting zlib decompression: compressedSize=%zu, pixelCount=%d\n", 
          compressedSize, pixelCount);
    
    // Use zlib decompression
    uLongf uncompressedSize = pixelCount;
    int result = uncompress(pixels, &uncompressedSize,
                           compressed, compressedSize);
    
    if (result != Z_OK) {
        printf("[SpriteCompression::decompressRLE] ERROR: zlib decompression failed with code %d\n", result);
        return false;
    }
    
    if (uncompressedSize != static_cast<uLongf>(pixelCount)) {
        printf("[SpriteCompression::decompressRLE] ERROR: Size mismatch (got %lu, expected %d)\n", 
              uncompressedSize, pixelCount);
        return false;
    }
    
    printf("[SpriteCompression::decompressRLE] Decompression SUCCESS: %lu bytes\n", uncompressedSize);
    return true;
}

size_t SpriteCompression::estimateCompressedSize(const uint8_t* pixels, int pixelCount) {
    // Use zlib's compressBound for accurate estimation
    return compressBound(pixelCount);
}

// =============================================================================
// SPRTZ v2 Functions
// =============================================================================

bool SpriteCompression::saveSPRTZv2Standard(const std::string& filename,
                                             int width, int height,
                                             const uint8_t* pixels,
                                             uint8_t standardPaletteID) {
    if (standardPaletteID >= 32) {
        return false; // Invalid palette ID
    }

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // Compress pixel data
    int pixelCount = width * height;
    std::vector<uint8_t> compressed;
    compressRLE(pixels, pixelCount, compressed);

    // Write header
    const char magic[4] = {'S', 'P', 'T', 'Z'};
    file.write(magic, 4);

    uint16_t version = 2;
    file.write(reinterpret_cast<const char*>(&version), sizeof(version));

    uint8_t w = static_cast<uint8_t>(width);
    uint8_t h = static_cast<uint8_t>(height);
    file.write(reinterpret_cast<const char*>(&w), 1);
    file.write(reinterpret_cast<const char*>(&h), 1);

    uint32_t uncompressedSize = static_cast<uint32_t>(pixelCount);
    uint32_t compressedSize = static_cast<uint32_t>(compressed.size());
    file.write(reinterpret_cast<const char*>(&uncompressedSize), sizeof(uncompressedSize));
    file.write(reinterpret_cast<const char*>(&compressedSize), sizeof(compressedSize));

    // Write palette mode: standard palette ID (0-31)
    file.write(reinterpret_cast<const char*>(&standardPaletteID), 1);

    // Write compressed pixel data
    file.write(reinterpret_cast<const char*>(compressed.data()), compressed.size());

    return file.good();
}

bool SpriteCompression::saveSPRTZv2Custom(const std::string& filename,
                                          int width, int height,
                                          const uint8_t* pixels,
                                          const uint8_t* palette) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // Compress pixel data
    int pixelCount = width * height;
    std::vector<uint8_t> compressed;
    compressRLE(pixels, pixelCount, compressed);

    // Write header
    const char magic[4] = {'S', 'P', 'T', 'Z'};
    file.write(magic, 4);

    uint16_t version = 2;
    file.write(reinterpret_cast<const char*>(&version), sizeof(version));

    uint8_t w = static_cast<uint8_t>(width);
    uint8_t h = static_cast<uint8_t>(height);
    file.write(reinterpret_cast<const char*>(&w), 1);
    file.write(reinterpret_cast<const char*>(&h), 1);

    uint32_t uncompressedSize = static_cast<uint32_t>(pixelCount);
    uint32_t compressedSize = static_cast<uint32_t>(compressed.size());
    file.write(reinterpret_cast<const char*>(&uncompressedSize), sizeof(uncompressedSize));
    file.write(reinterpret_cast<const char*>(&compressedSize), sizeof(compressedSize));

    // Write palette mode: 0xFF for custom palette
    uint8_t paletteMode = 0xFF;
    file.write(reinterpret_cast<const char*>(&paletteMode), 1);

    // Write palette (indices 2-15 only, RGB only)
    for (int i = 2; i < 16; i++) {
        int offset = i * 4;
        uint8_t rgb[3] = {palette[offset], palette[offset + 1], palette[offset + 2]};
        file.write(reinterpret_cast<const char*>(rgb), 3);
    }

    // Write compressed pixel data
    file.write(reinterpret_cast<const char*>(compressed.data()), compressed.size());

    return file.good();
}

bool SpriteCompression::loadSPRTZv2(const std::string& filename,
                                     int& outWidth, int& outHeight,
                                     uint8_t* outPixels,
                                     uint8_t* outPalette,
                                     bool& outIsStandard,
                                     uint8_t& outPaletteID) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // Read and verify header
    char magic[4];
    file.read(magic, 4);
    if (magic[0] != 'S' || magic[1] != 'P' || magic[2] != 'T' || magic[3] != 'Z') {
        return false;
    }

    uint16_t version;
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    
    // Support both v1 and v2
    if (version == 1) {
        // v1 format - treat as custom palette
        outIsStandard = false;
        outPaletteID = 0xFF;
        
        // Rewind and use v1 loader
        file.seekg(0);
        return loadSPRTZ(filename, outWidth, outHeight, outPixels, outPalette);
    }
    
    if (version != 2) {
        return false;
    }

    uint8_t w, h;
    file.read(reinterpret_cast<char*>(&w), 1);
    file.read(reinterpret_cast<char*>(&h), 1);
    outWidth = w;
    outHeight = h;

    uint32_t uncompressedSize, compressedSize;
    file.read(reinterpret_cast<char*>(&uncompressedSize), sizeof(uncompressedSize));
    file.read(reinterpret_cast<char*>(&compressedSize), sizeof(compressedSize));

    // Verify sizes
    int expectedPixels = outWidth * outHeight;
    if (uncompressedSize != static_cast<uint32_t>(expectedPixels)) {
        return false;
    }

    // Read palette mode
    uint8_t paletteMode;
    file.read(reinterpret_cast<char*>(&paletteMode), 1);

    // Set up fixed colors 0 and 1
    outPalette[0] = 0;   // R
    outPalette[1] = 0;   // G
    outPalette[2] = 0;   // B
    outPalette[3] = 0;   // A (transparent)

    outPalette[4] = 0;   // R
    outPalette[5] = 0;   // G
    outPalette[6] = 0;   // B
    outPalette[7] = 255; // A (opaque)

    if (paletteMode == 0xFF) {
        // Custom palette
        outIsStandard = false;
        outPaletteID = 0xFF;
        
        // Read colors 2-15
        for (int i = 2; i < 16; i++) {
            uint8_t rgb[3];
            file.read(reinterpret_cast<char*>(rgb), 3);
            int offset = i * 4;
            outPalette[offset + 0] = rgb[0];
            outPalette[offset + 1] = rgb[1];
            outPalette[offset + 2] = rgb[2];
            outPalette[offset + 3] = 255;
        }
    } else if (paletteMode < 32) {
        // Standard palette
        outIsStandard = true;
        outPaletteID = paletteMode;
        
        // Load from palette library
        if (StandardPaletteLibrary::isInitialized()) {
            if (!StandardPaletteLibrary::copyPaletteRGBA(paletteMode, outPalette)) {
                return false;
            }
        } else {
            // Library not initialized - can't load standard palette
            return false;
        }
    } else {
        // Invalid palette mode
        return false;
    }

    // Read compressed pixel data
    std::vector<uint8_t> compressed(compressedSize);
    file.read(reinterpret_cast<char*>(compressed.data()), compressedSize);

    if (!file.good()) {
        return false;
    }

    // Decompress
    return decompressRLE(compressed.data(), compressedSize, outPixels, expectedPixels);
}

} // namespace SPRED