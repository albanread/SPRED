//
// PaletteLibrary.cpp
// SuperTerminal Framework - Standard Palette Library Implementation
//
// Copyright © 2024 SuperTerminal. All rights reserved.
//

#include "PaletteLibrary.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <climits>
#include <algorithm>
#include <vector>

using namespace SuperTerminal;

// =============================================================================
// Internal Data Structure
// =============================================================================

struct StandardPaletteLibrary::LibraryData {
    PaletteColor palettes[STANDARD_PALETTE_COUNT][STANDARD_PALETTE_COLORS];
    StandardPaletteInfo info[STANDARD_PALETTE_COUNT];
    
    // Storage for metadata strings (so info pointers remain valid)
    std::string names[STANDARD_PALETTE_COUNT];
    std::string descriptions[STANDARD_PALETTE_COUNT];
    std::string categories[STANDARD_PALETTE_COUNT];
    
    bool initialized = false;
    std::string lastError;
    
    LibraryData() {
        // Initialize all palette info
        for (int i = 0; i < STANDARD_PALETTE_COUNT; i++) {
            info[i].id = i;
            info[i].name = "";
            info[i].description = "";
            info[i].category = "";
        }
    }
};

// Static instance initialization
StandardPaletteLibrary::LibraryData* StandardPaletteLibrary::s_data = nullptr;

// =============================================================================
// Initialization
// =============================================================================

bool StandardPaletteLibrary::initialize(const std::string& path) {
    if (path.size() >= 5 && path.substr(path.size() - 5) == ".json") {
        return initializeFromJSON(path);
    } else if (path.size() >= 4 && path.substr(path.size() - 4) == ".pal") {
        return initializeFromBinary(path);
    } else {
        // Try JSON first, then binary
        if (initializeFromJSON(path + ".json")) return true;
        return initializeFromBinary(path + ".pal");
    }
}

bool StandardPaletteLibrary::initializeFromJSON(const std::string& jsonPath) {
    if (!s_data) {
        s_data = new LibraryData();
    }

    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        setError("Failed to open JSON file: " + jsonPath);
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json = buffer.str();
    file.close();

    if (!parseJSON(json)) {
        return false;
    }

    s_data->initialized = true;
    return true;
}

bool StandardPaletteLibrary::initializeFromBinary(const std::string& palPath) {
    if (!s_data) {
        s_data = new LibraryData();
    }

    std::ifstream file(palPath, std::ios::binary);
    if (!file.is_open()) {
        setError("Failed to open binary file: " + palPath);
        return false;
    }

    if (!parseBinary(file)) {
        file.close();
        return false;
    }

    file.close();
    s_data->initialized = true;
    return true;
}

bool StandardPaletteLibrary::isInitialized() {
    return s_data && s_data->initialized;
}

void StandardPaletteLibrary::shutdown() {
    if (s_data) {
        delete s_data;
        s_data = nullptr;
    }
}

// =============================================================================
// Palette Access
// =============================================================================

const PaletteColor* StandardPaletteLibrary::getPalette(uint8_t paletteID) {
    if (!isInitialized() || paletteID >= STANDARD_PALETTE_COUNT) {
        return nullptr;
    }
    return s_data->palettes[paletteID];
}

const char* StandardPaletteLibrary::getPaletteName(uint8_t paletteID) {
    if (!isInitialized() || paletteID >= STANDARD_PALETTE_COUNT) {
        return nullptr;
    }
    return s_data->names[paletteID].c_str();
}

const char* StandardPaletteLibrary::getPaletteDescription(uint8_t paletteID) {
    if (!isInitialized() || paletteID >= STANDARD_PALETTE_COUNT) {
        return nullptr;
    }
    return s_data->descriptions[paletteID].c_str();
}

const char* StandardPaletteLibrary::getPaletteCategory(uint8_t paletteID) {
    if (!isInitialized() || paletteID >= STANDARD_PALETTE_COUNT) {
        return nullptr;
    }
    return s_data->categories[paletteID].c_str();
}

const StandardPaletteInfo* StandardPaletteLibrary::getPaletteInfo(uint8_t paletteID) {
    if (!isInitialized() || paletteID >= STANDARD_PALETTE_COUNT) {
        return nullptr;
    }
    return &s_data->info[paletteID];
}

// =============================================================================
// Validation
// =============================================================================

bool StandardPaletteLibrary::isValidPaletteID(uint8_t paletteID) {
    return paletteID < STANDARD_PALETTE_COUNT;
}

bool StandardPaletteLibrary::isStandardPaletteMode(uint8_t paletteMode) {
    return paletteMode < STANDARD_PALETTE_COUNT;
}

// =============================================================================
// Palette Operations
// =============================================================================

bool StandardPaletteLibrary::copyPalette(uint8_t paletteID, PaletteColor* outColors) {
    const PaletteColor* palette = getPalette(paletteID);
    if (!palette) return false;

    for (int i = 0; i < STANDARD_PALETTE_COLORS; i++) {
        outColors[i] = palette[i];
    }
    return true;
}

bool StandardPaletteLibrary::copyPaletteRGBA(uint8_t paletteID, uint8_t* outRGBA) {
    const PaletteColor* palette = getPalette(paletteID);
    if (!palette) return false;

    for (int i = 0; i < STANDARD_PALETTE_COLORS; i++) {
        outRGBA[i * 4 + 0] = palette[i].r;
        outRGBA[i * 4 + 1] = palette[i].g;
        outRGBA[i * 4 + 2] = palette[i].b;
        outRGBA[i * 4 + 3] = palette[i].a;
    }

    return true;
}

uint8_t StandardPaletteLibrary::findClosestPalette(const PaletteColor* customPalette,
                                                   int32_t* outDistance) {
    if (!isInitialized()) {
        if (outDistance) *outDistance = -1;
        return PALETTE_MODE_CUSTOM;
    }

    // Extract unique colors actually present in the custom palette
    // (Some colors might be duplicates or unused)
    std::vector<PaletteColor> uniqueColors;
    for (int i = 0; i < STANDARD_PALETTE_COLORS; i++) {
        const PaletteColor& color = customPalette[i];
        // Check if this color is already in our list
        bool isDuplicate = false;
        for (const auto& existing : uniqueColors) {
            if (existing == color) {
                isDuplicate = true;
                break;
            }
        }
        if (!isDuplicate) {
            uniqueColors.push_back(color);
        }
    }

    int32_t bestScore = INT32_MAX;
    uint8_t bestPaletteID = PALETTE_MODE_CUSTOM;
    int32_t bestTotalDistance = INT32_MAX;

    // Compare against all standard palettes
    for (uint8_t pid = 0; pid < STANDARD_PALETTE_COUNT; pid++) {
        const PaletteColor* standardPal = s_data->palettes[pid];

        // For each unique color in the custom palette, find the closest color in this standard palette
        int32_t totalDistance = 0;
        int32_t exactMatches = 0;
        int32_t closeMatches = 0;

        for (const auto& customColor : uniqueColors) {
            int32_t minDist = INT32_MAX;

            // Find closest color in standard palette
            for (int j = 0; j < STANDARD_PALETTE_COLORS; j++) {
                int32_t dist = colorDistance(customColor, standardPal[j]);
                if (dist < minDist) {
                    minDist = dist;
                }
            }

            totalDistance += minDist;

            // Track exact and close matches
            if (minDist == 0) {
                exactMatches++;
            } else if (minDist < 100) {  // Very close color (within ~10 per channel)
                closeMatches++;
            }
        }

        // Calculate a score that prioritizes:
        // 1. Palettes that contain all the colors (low total distance)
        // 2. Palettes with more exact matches
        // 3. Palettes with more close matches
        int32_t score = totalDistance - (exactMatches * 10000) - (closeMatches * 1000);

        if (score < bestScore) {
            bestScore = score;
            bestPaletteID = pid;
            bestTotalDistance = totalDistance;
        }
    }

    if (outDistance) {
        *outDistance = bestTotalDistance;
    }

    // Determine if we found a good match
    int numColors = uniqueColors.size();
    int32_t avgDistancePerColor = numColors > 0 ? bestTotalDistance / numColors : INT32_MAX;

    // Thresholds for matching:
    // - If average distance per color is very low (< 50), it's a great match
    // - If average distance is moderate (< 200), it's still usable
    // - If higher, probably better to use custom palette
    const int32_t GREAT_MATCH_THRESHOLD = 50;
    const int32_t GOOD_MATCH_THRESHOLD = 200;

    if (avgDistancePerColor < GOOD_MATCH_THRESHOLD || bestTotalDistance < GREAT_MATCH_THRESHOLD * numColors) {
        return bestPaletteID;
    }

    // No good match found
    return PALETTE_MODE_CUSTOM;
}

// =============================================================================
// Enumeration
// =============================================================================

void StandardPaletteLibrary::enumeratePalettes(void (*callback)(uint8_t id, const StandardPaletteInfo* info)) {
    if (!isInitialized() || !callback) return;

    for (uint8_t i = 0; i < STANDARD_PALETTE_COUNT; i++) {
        callback(i, &s_data->info[i]);
    }
}

int32_t StandardPaletteLibrary::getPalettesByCategory(const char* category, uint8_t* outIDs, int32_t maxIDs) {
    if (!isInitialized() || !category || !outIDs || maxIDs <= 0) {
        return 0;
    }

    int32_t count = 0;
    for (uint8_t i = 0; i < STANDARD_PALETTE_COUNT && count < maxIDs; i++) {
        if (s_data->categories[i] == category) {
            outIDs[count++] = i;
        }
    }
    return count;
}

// =============================================================================
// Error Handling
// =============================================================================

const std::string& StandardPaletteLibrary::getLastError() {
    static std::string empty;
    if (!s_data) return empty;
    return s_data->lastError;
}

void StandardPaletteLibrary::clearError() {
    if (s_data) {
        s_data->lastError.clear();
    }
}

void StandardPaletteLibrary::setError(const std::string& error) {
    if (!s_data) {
        s_data = new LibraryData();
    }
    s_data->lastError = error;
}

// =============================================================================
// JSON Parsing Helpers
// =============================================================================

std::string extractStringValue(const std::string& json, const std::string& key, size_t start) {
    size_t keyPos = json.find("\"" + key + "\"", start);
    if (keyPos == std::string::npos) return "";

    size_t colonPos = json.find(":", keyPos);
    if (colonPos == std::string::npos) return "";

    size_t quoteStart = json.find("\"", colonPos);
    if (quoteStart == std::string::npos) return "";

    size_t quoteEnd = json.find("\"", quoteStart + 1);
    if (quoteEnd == std::string::npos) return "";

    return json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
}

int extractIntValue(const std::string& json, const std::string& key, size_t start) {
    size_t keyPos = json.find("\"" + key + "\"", start);
    if (keyPos == std::string::npos) return 0;

    size_t colonPos = json.find(":", keyPos);
    if (colonPos == std::string::npos) return 0;

    size_t numStart = colonPos + 1;
    while (numStart < json.size() && (json[numStart] == ' ' || json[numStart] == '\t' || json[numStart] == '\n')) {
        numStart++;
    }

    std::string numStr;
    while (numStart < json.size() && (isdigit(json[numStart]) || json[numStart] == '-')) {
        numStr += json[numStart++];
    }

    return numStr.empty() ? 0 : std::stoi(numStr);
}

size_t findKey(const std::string& json, const std::string& key, size_t start) {
    return json.find("\"" + key + "\"", start);
}

bool StandardPaletteLibrary::parseJSON(const std::string& json) {
    // Find "palettes" array
    size_t palettesPos = json.find("\"palettes\"");
    if (palettesPos == std::string::npos) {
        setError("JSON: 'palettes' key not found");
        return false;
    }

    size_t arrayStart = json.find("[", palettesPos);
    if (arrayStart == std::string::npos) {
        setError("JSON: palettes array not found");
        return false;
    }

    // Parse each palette
    size_t pos = arrayStart + 1;
    int paletteIndex = 0;

    while (paletteIndex < STANDARD_PALETTE_COUNT) {
        size_t objStart = json.find("{", pos);
        if (objStart == std::string::npos) break;

        // Find the matching closing brace for this palette object
        int braceDepth = 1;
        size_t objEnd = objStart + 1;
        while (objEnd < json.size() && braceDepth > 0) {
            if (json[objEnd] == '{') braceDepth++;
            else if (json[objEnd] == '}') braceDepth--;
            objEnd++;
        }

        if (braceDepth != 0) {
            setError("JSON: Unmatched braces in palette object");
            return false;
        }

        // Extract palette fields
        int id = extractIntValue(json, "id", objStart);
        std::string name = extractStringValue(json, "name", objStart);
        std::string description = extractStringValue(json, "description", objStart);
        std::string category = extractStringValue(json, "category", objStart);

        if (id < 0 || id >= STANDARD_PALETTE_COUNT) {
            setError("JSON: Invalid palette ID: " + std::to_string(id));
            return false;
        }

        // Store metadata in persistent storage
        s_data->names[id] = name;
        s_data->descriptions[id] = description;
        s_data->categories[id] = category;
        s_data->info[id].id = id;
        s_data->info[id].name = s_data->names[id].c_str();
        s_data->info[id].description = s_data->descriptions[id].c_str();
        s_data->info[id].category = s_data->categories[id].c_str();

        // Find colors array
        size_t colorsPos = json.find("\"colors\"", objStart);
        if (colorsPos == std::string::npos || colorsPos > objEnd) {
            setError("JSON: 'colors' not found for palette " + std::to_string(id));
            return false;
        }

        size_t colorsArrayStart = json.find("[", colorsPos);
        if (colorsArrayStart == std::string::npos || colorsArrayStart > objEnd) {
            setError("JSON: colors array not found for palette " + std::to_string(id));
            return false;
        }

        // Parse colors
        size_t colorPos = colorsArrayStart + 1;
        int colorIndex = 0;

        while (colorIndex < STANDARD_PALETTE_COLORS) {
            size_t colorObjStart = json.find("{", colorPos);
            if (colorObjStart == std::string::npos || colorObjStart > objEnd) break;

            // Find matching closing brace for color object
            int colorBraceDepth = 1;
            size_t colorObjEnd = colorObjStart + 1;
            while (colorObjEnd < json.size() && colorBraceDepth > 0) {
                if (json[colorObjEnd] == '{') colorBraceDepth++;
                else if (json[colorObjEnd] == '}') colorBraceDepth--;
                colorObjEnd++;
            }

            int r = extractIntValue(json, "r", colorObjStart);
            int g = extractIntValue(json, "g", colorObjStart);
            int b = extractIntValue(json, "b", colorObjStart);
            int a = extractIntValue(json, "a", colorObjStart);

            s_data->palettes[id][colorIndex].r = static_cast<uint8_t>(r);
            s_data->palettes[id][colorIndex].g = static_cast<uint8_t>(g);
            s_data->palettes[id][colorIndex].b = static_cast<uint8_t>(b);
            s_data->palettes[id][colorIndex].a = (a == 0) ? 255 : static_cast<uint8_t>(a);

            colorPos = colorObjEnd;
            colorIndex++;
        }

        if (colorIndex != STANDARD_PALETTE_COLORS) {
            setError("JSON: Expected 16 colors for palette " + std::to_string(id) + ", got " + std::to_string(colorIndex));
            return false;
        }

        pos = objEnd;
        paletteIndex++;
    }

    if (paletteIndex != STANDARD_PALETTE_COUNT) {
        setError("JSON: Expected 32 palettes, parsed " + std::to_string(paletteIndex));
        return false;
    }

    return true;
}

// =============================================================================
// Binary Format Parsing
// =============================================================================

bool StandardPaletteLibrary::parseBinary(std::ifstream& file) {
    // Binary format: 32 palettes × 16 colors × 4 bytes (RGBA)
    // Total: 2048 bytes

    for (int pid = 0; pid < STANDARD_PALETTE_COUNT; pid++) {
        for (int cid = 0; cid < STANDARD_PALETTE_COLORS; cid++) {
            uint8_t rgba[4];
            file.read(reinterpret_cast<char*>(rgba), 4);

            if (!file) {
                setError("Binary: Unexpected end of file");
                return false;
            }

            s_data->palettes[pid][cid].r = rgba[0];
            s_data->palettes[pid][cid].g = rgba[1];
            s_data->palettes[pid][cid].b = rgba[2];
            s_data->palettes[pid][cid].a = rgba[3];
        }
    }

    // Metadata is not stored in binary format - use defaults
    const char* categoryNames[4] = {"retro", "biome", "themed", "utility"};
    for (int i = 0; i < STANDARD_PALETTE_COUNT; i++) {
        s_data->info[i].id = i;
        s_data->info[i].name = "Standard Palette";
        s_data->info[i].description = "Binary loaded palette";
        s_data->info[i].category = categoryNames[i / 8];
    }

    return true;
}

// =============================================================================
// Color Distance Calculation
// =============================================================================

int32_t StandardPaletteLibrary::colorDistance(const PaletteColor& c1, const PaletteColor& c2) {
    // Simple Euclidean distance in RGB space
    int32_t dr = static_cast<int32_t>(c1.r) - static_cast<int32_t>(c2.r);
    int32_t dg = static_cast<int32_t>(c1.g) - static_cast<int32_t>(c2.g);
    int32_t db = static_cast<int32_t>(c1.b) - static_cast<int32_t>(c2.b);

    return dr * dr + dg * dg + db * db;
}
