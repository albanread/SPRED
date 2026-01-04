//
//  SPREDWindow.h
//  SPRED - Sprite Editor
//
//  Main window class for the sprite editor
//

#ifndef SPRED_WINDOW_H
#define SPRED_WINDOW_H

#import <Cocoa/Cocoa.h>
#include "SpriteData.h"

namespace SPRED {
    class SpriteData;
}

// Custom view for sprite editing grid
@interface SpriteGridView : NSView
@property (nonatomic, assign) SPRED::SpriteData* spriteData;
@property (nonatomic, assign) int selectedColorIndex;
@property (nonatomic, assign) int gridSize;  // Pixel size multiplier (4x)

- (instancetype)initWithFrame:(NSRect)frame spriteData:(SPRED::SpriteData*)data;
@end

// Custom view for sprite preview (1x size)
@interface SpritePreviewView : NSView
@property (nonatomic, assign) SPRED::SpriteData* spriteData;

- (instancetype)initWithFrame:(NSRect)frame spriteData:(SPRED::SpriteData*)data;
@end

// Custom view for palette selection
@interface PaletteView : NSView
@property (nonatomic, assign) SPRED::SpriteData* spriteData;
@property (nonatomic, assign) int selectedColorIndex;
@property (nonatomic, assign) BOOL isStandardPalette;
@property (nonatomic, copy) void (^onColorSelected)(int index);
@property (nonatomic, copy) void (^onColorEdit)(int index);

- (instancetype)initWithFrame:(NSRect)frame spriteData:(SPRED::SpriteData*)data;
- (void)updateForStandardPalette:(BOOL)isStandard;
@end

// Main window controller
@interface SPREDWindowController : NSWindowController <NSWindowDelegate>
{
    SPRED::SpriteData* _spriteData;
    SpriteGridView* _gridView;
    SpritePreviewView* _previewView;
    PaletteView* _paletteView;
    NSTextField* _statusLabel;
    NSTextField* _gridLabel;
    NSTextField* _previewLabel;
    NSTextField* _paletteInfoLabel;
    NSTextField* _instructionsLabel;
    NSTextField* _paletteInstructionsLabel;
    NSString* _currentSpriteFile;
    NSString* _currentPaletteFile;
    
    // Standard palette support
    BOOL _isStandardPalette;
    uint8_t _standardPaletteID;
}

- (instancetype)init;
- (void)setupUI;
- (void)setupMenu;

// Menu actions
- (void)newSprite:(id)sender;
- (void)newSprite8x8:(id)sender;
- (void)newSprite16x16:(id)sender;
- (void)newSprite40x40:(id)sender;
- (void)openSprite:(id)sender;
- (void)saveSprite:(id)sender;
- (void)saveSpriteAs:(id)sender;
- (void)openSPRTZ:(id)sender;
- (void)saveSPRTZ:(id)sender;
- (void)saveSPRTZAs:(id)sender;
- (void)importPNG:(id)sender;
- (void)exportPNG:(id)sender;
- (void)exportPNG2x:(id)sender;
- (void)exportPNG4x:(id)sender;
- (void)exportPNGWithScale:(int)scale;
- (void)openPalette:(id)sender;
- (void)savePalette:(id)sender;
- (void)savePaletteAs:(id)sender;

// Edit actions
- (void)clearSprite:(id)sender;
- (void)shiftLeft:(id)sender;
- (void)shiftRight:(id)sender;
- (void)shiftUp:(id)sender;
- (void)shiftDown:(id)sender;
- (void)flipHorizontal:(id)sender;
- (void)flipVertical:(id)sender;
- (void)rotate90CW:(id)sender;
- (void)rotate90CCW:(id)sender;

// Standard palette actions
- (void)loadStandardPaletteFromMenu:(id)sender;
- (void)switchToCustomPalette:(id)sender;
- (void)loadStandardPalette:(uint8_t)paletteID;

// UI updates
- (void)updateViews;
- (void)updateStatus:(NSString*)message;
- (void)resizeUIForSprite;
- (void)updatePaletteModeUI;

@end

#endif // SPRED_WINDOW_H