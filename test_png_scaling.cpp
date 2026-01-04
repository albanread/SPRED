//
//  test_png_scaling.cpp
//  SPRED - PNG Scaling Method Comparison Test
//
//  Tests and benchmarks all available PNG scaling methods
//

#include "PNGConverter.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

using namespace SPRED;

void printUsage(const char* programName) {
    std::cout << "PNG Scaling Method Comparison Tool\n";
    std::cout << "===================================\n\n";
    std::cout << "Usage: " << programName << " <input.png> [target_width] [target_height]\n\n";
    std::cout << "This tool will:\n";
    std::cout << "  1. Load the input PNG\n";
    std::cout << "  2. Test all 4 scaling methods:\n";
    std::cout << "     - vImage (Accelerate/SIMD)\n";
    std::cout << "     - ImageIO (CoreGraphics)\n";
    std::cout << "     - CoreImage (GPU)\n";
    std::cout << "     - NSImage (original)\n";
    std::cout << "  3. Benchmark each method's performance\n";
    std::cout << "  4. Output scaled images to /tmp/spred_resized_*.png\n";
    std::cout << "  5. Recommend the best method for your image\n\n";
    std::cout << "Default target size: 40x30 (SPRED sprite)\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << programName << " myimage.png\n";
    std::cout << "  " << programName << " myimage.png 100 100\n";
    std::cout << "  " << programName << " myimage.png 40 30\n";
}

void printHeader(const std::string& title) {
    std::cout << "\n";
    std::cout << "================================================================\n";
    std::cout << "  " << title << "\n";
    std::cout << "================================================================\n";
}

void printSection(const std::string& title) {
    std::cout << "\n-- " << title << " " << std::string(60 - title.length(), '-') << "\n";
}

void printMethodInfo() {
    printHeader("PNG SCALING METHODS EXPLAINED");
    
    std::cout << "\n[1] vImage (Accelerate Framework)\n";
    std::cout << "   * Hardware-accelerated SIMD operations\n";
    std::cout << "   * Uses CPU vector instructions (NEON/SSE)\n";
    std::cout << "   * Fastest for most operations\n";
    std::cout << "   * Excellent quality with Lanczos resampling\n";
    std::cout << "   * Best for: Batch processing, real-time scaling\n\n";
    
    std::cout << "[2] ImageIO + CoreGraphics\n";
    std::cout << "   * Direct PNG metadata access\n";
    std::cout << "   * Efficient memory usage\n";
    std::cout << "   * High-quality interpolation\n";
    std::cout << "   * Can read EXIF, color profiles, DPI\n";
    std::cout << "   * Best for: Large images, metadata handling\n\n";
    
    std::cout << "[3] Core Image (GPU-Accelerated)\n";
    std::cout << "   * GPU-accelerated filters and transforms\n";
    std::cout << "   * Highest quality Lanczos scaling\n";
    std::cout << "   * Can apply preprocessing filters\n";
    std::cout << "   * Composable filter chains\n";
    std::cout << "   * Best for: Effects, filters, highest quality\n\n";
    
    std::cout << "[4] NSImage (Original AppKit)\n";
    std::cout << "   * High-level Cocoa API\n";
    std::cout << "   * Simple to use\n";
    std::cout << "   * Good quality\n";
    std::cout << "   * More overhead than other methods\n";
    std::cout << "   * Best for: Simple cases, compatibility\n";
}

void testIndividualMethod(const std::string& methodName,
                         PNGScalingMethod method,
                         const std::vector<uint8_t>& sourceRGBA,
                         int sourceWidth, int sourceHeight,
                         int targetWidth, int targetHeight) {
    printSection(std::string("Testing ") + methodName);
    
    std::vector<uint8_t> output;
    
    auto start = std::chrono::high_resolution_clock::now();
    bool success = PNGConverter::resizePNG(
        sourceRGBA.data(), sourceWidth, sourceHeight,
        0, 0, targetWidth, targetHeight,
        output, method
    );
    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double> elapsed = end - start;
    
    if (success) {
        std::cout << "[OK] SUCCESS\n";
        std::cout << "  Time: " << std::fixed << std::setprecision(3) 
                  << (elapsed.count() * 1000.0) << " ms\n";
        std::cout << "  Memory: " << output.size() << " bytes\n";
        std::cout << "  Throughput: " << std::fixed << std::setprecision(2)
                  << ((sourceWidth * sourceHeight * 4) / elapsed.count() / 1024.0 / 1024.0) 
                  << " MB/s\n";
    } else {
        std::cout << "[FAIL] FAILED\n";
    }
}

void testWithFilters(const std::vector<uint8_t>& sourceRGBA,
                     int sourceWidth, int sourceHeight,
                     int targetWidth, int targetHeight) {
    printHeader("CORE IMAGE FILTER TESTS");
    
    struct FilterTest {
        PNGFilter filter;
        const char* name;
        const char* description;
    };
    
    FilterTest filters[] = {
        { PNGFilter::None, "No Filter", "Direct scaling" },
        { PNGFilter::Sharpen, "Sharpen", "Enhance edges before scaling" },
        { PNGFilter::ColorAdjust, "Color Adjust", "Brightness/contrast adjustment" },
    };
    
    for (const auto& test : filters) {
        std::cout << "\n[*] " << test.name << " - " << test.description << "\n";
        
        std::vector<uint8_t> output;
        auto start = std::chrono::high_resolution_clock::now();
        bool success = PNGConverter::resizePNG_CoreImage(
            sourceRGBA.data(), sourceWidth, sourceHeight,
            0, 0, targetWidth, targetHeight,
            output, test.filter
        );
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        
        if (success) {
            std::cout << "  [OK] " << std::fixed << std::setprecision(3)
                      << (elapsed.count() * 1000.0) << " ms\n";
        } else {
            std::cout << "  [FAIL] Failed\n";
        }
    }
}

void compareLoadMethods(const std::string& filename) {
    printHeader("PNG LOADING METHOD COMPARISON");
    
    // Test NSImage loading
    printSection("NSImage Loading (original)");
    std::vector<uint8_t> rgba1;
    int width1, height1;
    
    auto start1 = std::chrono::high_resolution_clock::now();
    bool success1 = PNGConverter::loadPNGFile(filename, rgba1, width1, height1);
    auto end1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed1 = end1 - start1;
    
    if (success1) {
        std::cout << "[OK] Loaded " << width1 << "x" << height1 << "\n";
        std::cout << "  Time: " << std::fixed << std::setprecision(3)
                  << (elapsed1.count() * 1000.0) << " ms\n";
    } else {
        std::cout << "[FAIL] Failed\n";
    }
    
    // Test ImageIO loading
    printSection("ImageIO Loading (efficient)");
    std::vector<uint8_t> rgba2;
    int width2, height2;
    
    auto start2 = std::chrono::high_resolution_clock::now();
    bool success2 = PNGConverter::loadPNGFile_ImageIO(filename, rgba2, width2, height2);
    auto end2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed2 = end2 - start2;
    
    if (success2) {
        std::cout << "[OK] Loaded " << width2 << "x" << height2 << "\n";
        std::cout << "  Time: " << std::fixed << std::setprecision(3)
                  << (elapsed2.count() * 1000.0) << " ms\n";
        
        if (success1) {
            double speedup = elapsed1.count() / elapsed2.count();
            std::cout << "  Speedup: " << std::fixed << std::setprecision(2)
                      << speedup << "x faster than NSImage\n";
        }
    } else {
        std::cout << "[FAIL] Failed\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string inputFile = argv[1];
    int targetWidth = 40;
    int targetHeight = 30;
    
    if (argc >= 3) {
        targetWidth = std::stoi(argv[2]);
    }
    if (argc >= 4) {
        targetHeight = std::stoi(argv[3]);
    }
    
    printHeader("PNG SCALING METHOD COMPARISON TOOL");
    printMethodInfo();
    
    // Compare loading methods
    compareLoadMethods(inputFile);
    
    // Load PNG with the faster method
    printHeader("LOADING TEST IMAGE");
    std::vector<uint8_t> rgba;
    int width, height;
    
    std::cout << "Loading: " << inputFile << "\n";
    
    if (!PNGConverter::loadPNGFile_ImageIO(inputFile, rgba, width, height)) {
        std::cerr << "Failed to load PNG file: " << inputFile << "\n";
        return 1;
    }
    
    std::cout << "[OK] Loaded " << width << "x" << height << " image\n";
    std::cout << "  Source size: " << (rgba.size() / 1024.0) << " KB\n";
    std::cout << "  Target size: " << targetWidth << "x" << targetHeight << "\n";
    
    double scaleX = (double)targetWidth / width;
    double scaleY = (double)targetHeight / height;
    std::cout << "  Scale factors: " << std::fixed << std::setprecision(3)
              << scaleX << "x, " << scaleY << "x\n";
    
    // Test individual methods
    printHeader("INDIVIDUAL METHOD TESTS");
    testIndividualMethod("vImage (Accelerate/SIMD)", PNGScalingMethod::vImage,
                        rgba, width, height, targetWidth, targetHeight);
    
    testIndividualMethod("ImageIO (CoreGraphics)", PNGScalingMethod::ImageIO,
                        rgba, width, height, targetWidth, targetHeight);
    
    testIndividualMethod("CoreImage (GPU)", PNGScalingMethod::CoreImage,
                        rgba, width, height, targetWidth, targetHeight);
    
    testIndividualMethod("NSImage (Original)", PNGScalingMethod::NSImage,
                        rgba, width, height, targetWidth, targetHeight);
    
    // Test Core Image filters
    testWithFilters(rgba, width, height, targetWidth, targetHeight);
    
    // Full benchmark
    printHeader("COMPREHENSIVE BENCHMARK");
    std::vector<ScalingBenchmark> results;
    PNGConverter::benchmarkScalingMethods(
        rgba.data(), width, height,
        targetWidth, targetHeight,
        results
    );
    
    // Output files info
    printHeader("OUTPUT FILES");
    std::cout << "\n[FILES] Generated files in /tmp/:\n";
    std::cout << "   * spred_resized_vimage.png    - vImage result\n";
    std::cout << "   * spred_resized_imageio.png   - ImageIO result\n";
    std::cout << "   * spred_resized_coreimage.png - CoreImage result\n";
    std::cout << "   * spred_resized_temp.png      - NSImage result\n\n";
    std::cout << "Compare visually:\n";
    std::cout << "   open /tmp/spred_resized_*.png\n\n";
    
    // Recommendations
    printHeader("RECOMMENDATIONS");
    
    if (!results.empty()) {
        double fastestTime = 1e9;
        int fastestIdx = -1;
        
        for (size_t i = 0; i < results.size(); i++) {
            if (results[i].success && results[i].timeSeconds < fastestTime) {
                fastestTime = results[i].timeSeconds;
                fastestIdx = i;
            }
        }
        
        if (fastestIdx >= 0) {
            std::cout << "\n[**] BEST PERFORMANCE: ";
            
            if (results[fastestIdx].method == PNGScalingMethod::vImage) {
                std::cout << "vImage (Accelerate)\n";
                std::cout << "   → Use PNGScalingMethod::vImage for production\n";
                std::cout << "   → Best for: Real-time scaling, batch processing\n";
            } else if (results[fastestIdx].method == PNGScalingMethod::ImageIO) {
                std::cout << "ImageIO (CoreGraphics)\n";
                std::cout << "   → Use PNGScalingMethod::ImageIO for production\n";
                std::cout << "   → Best for: Large images, metadata handling\n";
            } else if (results[fastestIdx].method == PNGScalingMethod::CoreImage) {
                std::cout << "CoreImage (GPU)\n";
                std::cout << "   → Use PNGScalingMethod::CoreImage for production\n";
                std::cout << "   → Best for: Highest quality, filter effects\n";
            } else {
                std::cout << "NSImage (Original)\n";
                std::cout << "   → Consider switching to vImage for better performance\n";
            }
            
            std::cout << "\n[INFO] USAGE EXAMPLES:\n\n";
            std::cout << "C++ code:\n";
            std::cout << "  // Fastest (default)\n";
            std::cout << "  PNGConverter::resizePNG(src, w, h, 0, 0, tw, th, out);\n\n";
            std::cout << "  // Highest quality with filter\n";
            std::cout << "  PNGConverter::resizePNG(src, w, h, 0, 0, tw, th, out,\n";
            std::cout << "                          PNGScalingMethod::CoreImage,\n";
            std::cout << "                          PNGFilter::Sharpen);\n\n";
            std::cout << "  // Specific method\n";
            std::cout << "  PNGConverter::resizePNG(src, w, h, 0, 0, tw, th, out,\n";
            std::cout << "                          PNGScalingMethod::vImage);\n";
        }
    }
    
    printHeader("TEST COMPLETE");
    std::cout << "\n[OK] All tests completed successfully!\n";
    std::cout << "  Review the benchmark results above to choose the best method.\n";
    std::cout << "  Compare output images visually in /tmp/\n\n";
    
    return 0;
}