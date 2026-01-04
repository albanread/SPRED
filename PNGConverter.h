//
//  PNGConverter.h
//  SPRED - Sprite Editor
//
//  PNG import/export with automatic palette extraction
//

#ifndef SPRED_PNG_CONVERTER_H
#define SPRED_PNG_CONVERTER_H

#include <cstdint>
#include <string>
#include <vector>

namespace SPRED {

/// PNG Scaling Method
enum class PNGScalingMethod {
    NSImage,        // Current: NSImage/NSBitmapImageRep (high-level)
    vImage,         // Accelerate framework (SIMD-optimized, fastest)
    ImageIO,        // ImageIO + CoreGraphics (efficient, metadata-aware)
    CoreImage,      // Core Image filters (GPU-accelerated, highest quality)
    Default = vImage
};

/// PNG Preprocessing Filters (Core Image)
enum class PNGFilter {
    None,
    Sharpen,        // Sharpen edges before scaling
    EdgeDetect,     // Emphasize edges
    ColorAdjust     // Adjust brightness/contrast
};

/// Scaling performance result
struct ScalingBenchmark {
    PNGScalingMethod method;
    double timeSeconds;
    size_t memoryBytes;
    bool success;
};

/// RGB color structure
struct Color {
    uint8_t r, g, b, a;
    
    Color() : r(0), g(0), b(0), a(255) {}
    Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
    
    bool operator==(const Color& other) const {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }
    
    // Distance in RGB space (ignoring alpha)
    int distanceTo(const Color& other) const {
        int dr = r - other.r;
        int dg = g - other.g;
        int db = b - other.b;
        return dr*dr + dg*dg + db*db;
    }
    
    bool isTransparent() const {
        return a < 128; // Threshold for transparency
    }
};

/// Color histogram entry
struct ColorEntry {
    Color color;
    int count;
    
    ColorEntry(const Color& c, int cnt) : color(c), count(cnt) {}
};

/// PNG Converter - Import/export PNG with palette quantization
class PNGConverter {
public:
    /// Import PNG and quantize to 14 colors + 2 blacks
    /// @param filename PNG file path
    /// @param maxWidth Maximum sprite width (8, 16, or 40)
    /// @param maxHeight Maximum sprite height (8, 16, or 40)
    /// @param outWidth Output sprite width
    /// @param outHeight Output sprite height
    /// @param outPixels Output pixel buffer (must be at least maxWidth×maxHeight)
    /// @param outPalette Output palette buffer (64 bytes)
    /// @return true if successful
    static bool importPNG(const std::string& filename,
                          int maxWidth, int maxHeight,
                          int& outWidth, int& outHeight,
                          uint8_t* outPixels,
                          uint8_t* outPalette);
    
    /// Export sprite to PNG
    /// @param filename PNG file path
    /// @param width Sprite width
    /// @param height Sprite height
    /// @param pixels Pixel data (indexed)
    /// @param palette Palette data (64 bytes RGBA)
    /// @param scale Scale factor (1 = actual size, 2 = 2x, etc.)
    /// @return true if successful
    static bool exportPNG(const std::string& filename,
                          int width, int height,
                          const uint8_t* pixels,
                          const uint8_t* palette,
                          int scale = 1);

    /// Load PNG file to RGBA buffer
    static bool loadPNGFile(const std::string& filename,
                           std::vector<uint8_t>& rgba,
                           int& width, int& height);
    
    /// Extract most significant colors using median-cut quantization
    /// @param rgba RGBA pixel data
    /// @param pixelCount Number of pixels
    /// @param numColors Number of colors to extract (14)
    /// @param outColors Output colors
    static void extractPalette(const uint8_t* rgba, int pixelCount,
                              int numColors, std::vector<Color>& outColors);
    
    /// Find closest color in palette
    static int findClosestColor(const Color& pixel,
                               const std::vector<Color>& palette);
    
    /// Resize PNG using specified scaling method
    /// @param sourceRGBA Source RGBA pixel data
    /// @param sourceWidth Source width
    /// @param sourceHeight Source height
    /// @param sourceOffsetX X offset in source to start from
    /// @param sourceOffsetY Y offset in source to start from
    /// @param targetWidth Target width
    /// @param targetHeight Target height
    /// @param outRGBA Output RGBA buffer (must be pre-allocated)
    /// @param method Scaling method to use (default: vImage)
    /// @param filter Optional preprocessing filter (default: None)
    /// @return true if successful
    static bool resizePNG(const uint8_t* sourceRGBA, int sourceWidth, int sourceHeight,
                         int sourceOffsetX, int sourceOffsetY,
                         int targetWidth, int targetHeight,
                         std::vector<uint8_t>& outRGBA,
                         PNGScalingMethod method = PNGScalingMethod::Default,
                         PNGFilter filter = PNGFilter::None);
    
    /// Resize PNG using vImage (Accelerate framework) - SIMD optimized
    static bool resizePNG_vImage(const uint8_t* sourceRGBA, int sourceWidth, int sourceHeight,
                                 int sourceOffsetX, int sourceOffsetY,
                                 int targetWidth, int targetHeight,
                                 std::vector<uint8_t>& outRGBA);
    
    /// Resize PNG using ImageIO + CoreGraphics - efficient loading
    static bool resizePNG_ImageIO(const uint8_t* sourceRGBA, int sourceWidth, int sourceHeight,
                                  int sourceOffsetX, int sourceOffsetY,
                                  int targetWidth, int targetHeight,
                                  std::vector<uint8_t>& outRGBA);
    
    /// Resize PNG using Core Image - GPU accelerated filters
    static bool resizePNG_CoreImage(const uint8_t* sourceRGBA, int sourceWidth, int sourceHeight,
                                    int sourceOffsetX, int sourceOffsetY,
                                    int targetWidth, int targetHeight,
                                    std::vector<uint8_t>& outRGBA,
                                    PNGFilter filter = PNGFilter::None);
    
    /// Resize PNG using NSImage (original method)
    static bool resizePNG_NSImage(const uint8_t* sourceRGBA, int sourceWidth, int sourceHeight,
                                  int sourceOffsetX, int sourceOffsetY,
                                  int targetWidth, int targetHeight,
                                  std::vector<uint8_t>& outRGBA);
    
    /// Compare all scaling methods and output benchmark results
    /// @param sourceRGBA Source RGBA pixel data
    /// @param sourceWidth Source width
    /// @param sourceHeight Source height
    /// @param targetWidth Target width
    /// @param targetHeight Target height
    /// @param results Vector to store benchmark results
    /// @return true if at least one method succeeded
    static bool benchmarkScalingMethods(const uint8_t* sourceRGBA, 
                                       int sourceWidth, int sourceHeight,
                                       int targetWidth, int targetHeight,
                                       std::vector<ScalingBenchmark>& results);
    
    /// Load PNG file using ImageIO (more efficient than NSImage)
    static bool loadPNGFile_ImageIO(const std::string& filename,
                                   std::vector<uint8_t>& rgba,
                                   int& width, int& height);
    
    /// Save RGBA buffer to PNG file
    static bool savePNGFile(const std::string& filename,
                           const uint8_t* rgba,
                           int width, int height);

private:
    
    /// Build color histogram
    static void buildHistogram(const uint8_t* rgba, int pixelCount,
                              std::vector<ColorEntry>& histogram);
    
    /// Median cut algorithm for color quantization
    static void medianCut(std::vector<ColorEntry>& colors,
                         int depth, std::vector<Color>& palette);
    
    /// Find representative color from a group
    static Color getRepresentativeColor(const std::vector<ColorEntry>& colors);
    
    /// Map RGBA pixels to palette indices
    static void mapToPalette(const uint8_t* rgba, int pixelCount,
                            const std::vector<Color>& colors,
                            uint8_t* outIndices);
};

} // namespace SPRED

#endif // SPRED_PNG_CONVERTER_H